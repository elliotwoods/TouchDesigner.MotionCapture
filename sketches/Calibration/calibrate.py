#%% Start
import cv2
import numpy as np
import IPython
import os
import yaml
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
from pathlib import Path

os.chdir(os.path.dirname(os.path.realpath(__file__)))
os.chdir('../Calibration_data_200713/')

cache_path = "Cache"
Path(cache_path).mkdir(parents=True, exist_ok=True) #make the cache folder if it doesn't exist

image_size = (1936,1216)
camera_matrix_default = np.array([
	[ 1.3983204002534794e+03, 0., 9.5017606615261934e+02],
	[0,  1.3976046682303358e+03, 6.1823689807995254e+02],
	[0., 0., 1. ]
])
distortion_coefficients_default = np.array([ -1.3364299338587463e-01, 1.0515853005960132e-01,
       4.9421246633190316e-04, -2.0023566259940433e-03,
       -8.3766571834111307e-04 ])

#%% Board definition
board_size = (9, 6)
board_spacing = 0.025
board_points = []
for row in range(board_size[1]):
	for col in range(board_size[0]):
		board_points.append([col * board_spacing, row * board_spacing, 0])
board_points = np.array(board_points)

#%% Find boards
def find_board(file_path):
	#first check if it exists in the cache
	cache_file_path = "{}/{}.yml".format(cache_path, file_path)
	if os.path.exists(cache_file_path):
		try:
			fs_read = cv2.FileStorage(cache_file_path, cv2.FILE_STORAGE_READ)
			image_points = fs_read.getNode("image_points").mat()
			fs_read.release()
			print("Used cache for {}".format(file_path))
			return image_points
		except:
			pass

	print("Finding board corners in {}".format(file_path))
	image = cv2.imread(file_path, cv2.IMREAD_GRAYSCALE)
	success, image_points = cv2.findChessboardCorners(image, board_size)
	image_points = image_points.reshape(image_points.shape[0], 2)

	term_criteria = (cv2.TERM_CRITERIA_EPS + cv2.TERM_CRITERIA_MAX_ITER, 30, 0.001)
	image_points_refined = cv2.cornerSubPix(image, image_points, (5,5), (-1, -1), term_criteria)

	print("Writing {}".format(cache_file_path))
	Path(os.path.dirname(cache_file_path)).mkdir(parents=True, exist_ok=True)
	fs_write = cv2.FileStorage(cache_file_path, cv2.FILE_STORAGE_WRITE)
	fs_write.write('image_points', image_points_refined)
	fs_write.release()

	return image_points


def find_boards_in_folder(folder_name):
	result = []
	filenames = os.listdir(folder_name)
	for filename in filenames:
		if filename.lower().endswith(".png"):
			single_result = find_board("{}/{}".format(folder_name, filename))
			result.append(single_result)
	return np.array(result)

image_points_left = find_boards_in_folder("LEFT")
image_points_right = find_boards_in_folder("RIGHT")

#%% Calibrate intrinsics
def calibrate_camera(image_points, camera_name):
	cache_file_path = "{}/{}.yml".format(cache_path, camera_name)
	if os.path.exists(cache_file_path):
		try:
			fs_read = cv2.FileStorage(cache_file_path, cv2.FILE_STORAGE_READ)
			camera_matrix = fs_read.getNode("camera_matrix").mat()
			distortion_coefficients = fs_read.getNode("camera_matrix").mat()
			reprojection_error = fs_read.getNode("reprojection_error").real()
			fs_read.release()
			print("Used cache for {}".format(camera_name))
			return camera_matrix, distortion_coefficients, reprojection_error
		except:
			pass
	
	camera_matrix = camera_matrix_default
	distortion_coefficients = distortion_coefficients_default

	object_points_many = np.array([board_points for i in range(image_points.shape[0])], dtype=np.float32)

	print("Calibrating {} intrinsics...".format(camera_name))
	reprojection_error, camera_matrix, distortion_coefficients, rvecs, tvecs = cv2.calibrateCamera(object_points_many
		, image_points
		, image_size
		, camera_matrix
		, distortion_coefficients
		, flags=cv2.CALIB_USE_INTRINSIC_GUESS)
	print("reprojection error = {}".format(reprojection_error))

	
	print("Writing {}".format(cache_file_path))
	Path(os.path.dirname(cache_file_path)).mkdir(parents=True, exist_ok=True)
	fs_write = cv2.FileStorage(cache_file_path, cv2.FILE_STORAGE_WRITE)
	fs_write.write("camera_matrix", camera_matrix)
	fs_write.write("distortion_coefficients", distortion_coefficients)
	fs_write.write("reprojection_error", reprojection_error)
	fs_write.write("rvecs", np.array(rvecs))
	fs_write.write("tvecs", np.array(tvecs))
	fs_write.write("image_width", image_size[0])
	fs_write.write("image_height", image_size[1])
	fs_write.release()

	return camera_matrix, distortion_coefficients, reprojection_error

camera_matrix_left, distortion_coefficients_left, reprojection_error_left = calibrate_camera(image_points_left, "camera_left")
camera_matrix_right, distortion_coefficients_right, reprojection_error_right = calibrate_camera(image_points_right, "camera_right")

#%% Stereo calibration
def calibrate_stereo():
	cache_file_path = "{}/{}.yml".format(cache_path, 'stereo_calibration')
	if os.path.exists(cache_file_path):
		try:
			fs_read = cv2.FileStorage(cache_file_path, cv2.FILE_STORAGE_READ)
			E = fs_read.getNode("essential_matrix").mat()
			F = fs_read.getNode("fundamental_matrix").mat()
			rotation = fs_read.getNode("rotation").mat()
			translation = fs_read.getNode("translation").mat()
			fs_read.release()
			print("Used cache for stereo calibration")
			return rotation, translation, E, F
		except:
			pass

	print("Performing stereo calibrate")
	object_points_many = np.array([board_points for i in range(image_points_left.shape[0])], dtype=np.float32)
	reprojection_error, _, _, _, _, rotation, translation, essential_matrix, fundamental_matrix = cv2.stereoCalibrate(object_points_many
		, image_points_left
		, image_points_right
		, camera_matrix_left
		, distortion_coefficients_left
		, camera_matrix_right
		, distortion_coefficients_right
		, image_size
		, flags=cv2.CALIB_FIX_INTRINSIC)
	print("reprojection error = {}".format(reprojection_error))

	print("Writing {}".format(cache_file_path))
	Path(os.path.dirname(cache_file_path)).mkdir(parents=True, exist_ok=True)
	fs_write = cv2.FileStorage(cache_file_path, cv2.FILE_STORAGE_WRITE)
	fs_write.write("rotation", rotation)
	fs_write.write("translation", translation)
	fs_write.write("essential_matrix", essential_matrix)
	fs_write.write("fundamental_matrix", fundamental_matrix)
	fs_write.write("reprojection_error", reprojection_error)

	fs_write.release()

	return rotation, translation, essential_matrix, fundamental_matrix

rotation, translation, E, F = calibrate_stereo()