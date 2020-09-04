#%% Start
import cv2
import numpy as np
import IPython
import os
import yaml
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
from pathlib import Path
from multiprocessing import Pool
import json
from matplotlib import pyplot as plt

os.chdir(os.path.dirname(os.path.realpath(__file__)))
os.chdir('../Calibrationdata200904/')

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
board_size = (11, 7)
board_spacing = 0.020
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

	if not success:
		print(" - EMPTY No board found for {}".format(file_path))
		return None

	image_points = image_points.reshape(image_points.shape[0], 2)

	term_criteria = (cv2.TERM_CRITERIA_EPS + cv2.TERM_CRITERIA_MAX_ITER, 30, 0.001)
	image_points_refined = cv2.cornerSubPix(image, image_points, (5,5), (-1, -1), term_criteria)

	print(" - Writing {}".format(cache_file_path))
	Path(os.path.dirname(cache_file_path)).mkdir(parents=True, exist_ok=True)
	fs_write = cv2.FileStorage(cache_file_path, cv2.FILE_STORAGE_WRITE)
	fs_write.write('image_points', image_points_refined)
	fs_write.release()

	return image_points


def find_boards_in_folder(folder_name):
	image_points = []
	filenames = os.listdir(folder_name)

	filenames_out = []
	for filename in filenames:
		if filename.lower().endswith(".png"):
			file_path = "{}/{}".format(folder_name, filename)
			single_result = find_board(file_path)
			image_points.append(single_result)
			filenames_out.append(file_path)
			
	return image_points, filenames_out

image_points_left_single_raw, filenames_left_single_raw = find_boards_in_folder("SingleLEFT")
image_points_left_stereo_raw, filenames_left_stereo_raw = find_boards_in_folder("StereoLEFT")

image_points_right_single_raw, filenames_right_single_raw = find_boards_in_folder("SingleRIGHT")
image_points_right_stereo_raw, filenames_right_stereo_raw = find_boards_in_folder("StereoRIGHT")

#%% Filter the results
def filter_empty_image_points(image_points, filenames):
	image_points_filtered = []
	filenames_filtered = []
	for i in range(len(image_points)):
		if image_points[i] is not None:
			image_points_filtered.append(image_points[i])
			filenames_filtered.append(filenames[i])
	return image_points_filtered, filenames_filtered

image_points_left_single, filenames_left_single = filter_empty_image_points(image_points_left_single_raw, filenames_left_single_raw)
image_points_right_single, filenames_right_single = filter_empty_image_points(image_points_right_single_raw, filenames_right_single_raw)

image_points_left_from_stereo, filenames_left_from_stereo = filter_empty_image_points(image_points_left_stereo_raw, filenames_left_stereo_raw)
image_points_right_from_stereo, filenames_right_from_stereo = filter_empty_image_points(image_points_right_stereo_raw, filenames_right_stereo_raw)

image_points_left = np.array(image_points_left_single + image_points_left_from_stereo)
image_points_right = np.array(image_points_right_single + image_points_right_from_stereo)

filenames_left = filenames_left_single + filenames_left_from_stereo
filenames_right = filenames_right_single + filenames_right_from_stereo

image_points_left_stereo = []
image_points_right_stereo = []
filenames_stereo_left = []
filenames_stereo_right = []

for i in range(len(image_points_left_stereo_raw)):
	absent = image_points_left_stereo_raw[i] is None or image_points_right_stereo_raw[i] is None
	if not absent:
		image_points_left_stereo.append(image_points_left_stereo_raw[i])
		image_points_right_stereo.append(image_points_right_stereo_raw[i])
		filenames_stereo_left.append(filenames_left_stereo_raw[i])
		filenames_stereo_right.append(filenames_right_stereo_raw[i])

image_points_left_stereo = np.array(image_points_left_stereo)
image_points_right_stereo = np.array(image_points_right_stereo)

print("Left boards : {}".format(len(image_points_left)))
print("Right boards : {}".format(len(image_points_right)))
print("Stereo boards : {}".format(len(image_points_left_stereo)))

#%% Calibrate intrinsics
def calibrate_camera(image_points, camera_name):
	cache_file_path = "{}/{}.yml".format(cache_path, camera_name)
	if os.path.exists(cache_file_path):
		try:
			fs_read = cv2.FileStorage(cache_file_path, cv2.FILE_STORAGE_READ)
			camera_matrix = fs_read.getNode("camera_matrix").mat()
			distortion_coefficients = fs_read.getNode("distortion_coefficients").mat()
			reprojection_error = fs_read.getNode("reprojection_error").real()
			rvecs = fs_read.getNode("rvecs").mat()
			tvecs = fs_read.getNode("tvecs").mat()
			fs_read.release()
			print("Used cache for {}".format(camera_name))
			return camera_matrix, distortion_coefficients, reprojection_error, rvecs, tvecs
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
		, distortion_coefficients)
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

	return camera_matrix, distortion_coefficients, reprojection_error, rvecs, tvecs

camera_matrix_left, distortion_coefficients_left, reprojection_error_left, rvecs_left, tvecs_left = calibrate_camera(image_points_left, "camera_left")
camera_matrix_right, distortion_coefficients_right, reprojection_error_right, rvecs_right, tvecs_right = calibrate_camera(image_points_right, "camera_right")

#%% Calculate reprojection error for intrinsics
def write_reprojection_error(image_points, filenames, camera_matrix, distortion_coefficients, rvecs, tvecs, output_filename):
	rms_per_image = {}

	plt.figure()

	for i in range(len(image_points)):
		projected_points, _ = cv2.projectPoints(board_points, rvecs[i], tvecs[i], camera_matrix, distortion_coefficients)
		rms_error = np.sqrt(np.mean(np.square(image_points[i] - projected_points.reshape(len(projected_points), 2))))
		rms_per_image[filenames[i]] = rms_error
		y = i % 10
		plt.scatter(rms_error, y)
		plt.annotate(filenames[i], xy=(rms_error, y))
	
	with open(output_filename, "w") as file:
		file.write(json.dumps(rms_per_image, indent=4))
	plt.title("Reprojection error for {}".format(output_filename))
	plt.xlabel("px")
	plt.show()

write_reprojection_error(image_points_left, filenames_left, camera_matrix_left, distortion_coefficients_left, rvecs_left, tvecs_left, "reprojection_error_per_image_left.json")
write_reprojection_error(image_points_right, filenames_right, camera_matrix_right, distortion_coefficients_right, rvecs_right, tvecs_right, "reprojection_error_per_image_right.json")

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
	object_points_many = np.array([board_points for i in range(image_points_left_stereo.shape[0])], dtype=np.float32)
	reprojection_error, _, _, _, _, rotation, translation, essential_matrix, fundamental_matrix = cv2.stereoCalibrate(object_points_many
		, image_points_left_stereo
		, image_points_right_stereo
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

#%% Calculate reprojection errors
stereo_epipolar_rms_per_image_pair = []

plt.figure()

for i in range(len(image_points_left_stereo)):
	lines = cv2.computeCorrespondEpilines(image_points_left_stereo[i], 1, F)
	
	distances = []
	for j in range(len(lines)):
		line = lines[j][0]
		right_centroid = image_points_right_stereo[i][j]
		distance = abs(line[0] * right_centroid[0] + line[1] * right_centroid[1] + line[2]) / np.sqrt(line[0] * line[0] + line[1] * line[1])
		distances.append(float(distance))

	rms = np.sqrt(np.mean(np.square(distances)))

	stereo_epipolar_rms_per_image_pair.append({
		"left_image" : filenames_stereo_left[i],
		"right_image" : filenames_stereo_right[i],
		"epipolar_distance_rms" : rms
	})

	plt.scatter(float(rms), i % 5)
	plt.annotate(filenames_stereo_left[i], xy=(rms, i % 5))

plt.title("Stereo epipolar distance RMS")
plt.xlabel("px")
plt.show()

with open("stereo_epipolar_rms_per_image_pair.json", "w") as file:
	file.write(json.dumps(stereo_epipolar_rms_per_image_pair, indent=4))

# %%
