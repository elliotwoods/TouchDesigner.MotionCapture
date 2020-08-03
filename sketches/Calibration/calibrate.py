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
	
	camera_matrix = None
	distortion_coefficients = None

	object_points_many = np.array([board_points for i in range(image_points.shape[0])], dtype=np.float32)

	print("Calibrating {} intrinsics...".format(camera_name))
	reprojection_error, camera_matrix, distortion_coefficients, rvecs, tvecs = cv2.calibrateCamera(object_points_many
		, image_points
		, image_size
		, camera_matrix
		, distortion_coefficients
		, flags=None)
	print("reprojection error = {}".format(reprojection_error))

	
	print("Writing {}".format(cache_file_path))
	Path(os.path.dirname(cache_file_path)).mkdir(parents=True, exist_ok=True)
	fs_write = cv2.FileStorage(cache_file_path, cv2.FILE_STORAGE_WRITE)
	fs_write.write("camera_matrix", camera_matrix)
	fs_write.write("distortion_coefficients", distortion_coefficients)
	fs_write.write("reprojection_error", reprojection_error)
	fs_write.write("rvecs", np.array(rvecs))
	fs_write.write("tvecs", np.array(tvecs))
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

#%% ...


size = (1936,1216)

def convert_arrays(dictionary):
	new_object = {}
	for key in dictionary:
		value = dictionary[key]

		if isinstance(value, list):
			new_object[key] = np.array(value)
		elif isinstance(value, dict):
			new_object[key] = convert_arrays(value)
		else:
			new_object[key] = value
	return new_object

def load_yaml(file_name):
	with open(file_name) as file:
		data = yaml.load(file, Loader=yaml.FullLoader)
	if isinstance(data, dict):
		data = convert_arrays(data)
	return data

#%% Loading Calibrations

camera_left = load_yaml('Calibration_Left.yaml')
camera_right = load_yaml('Calibration_Right.yaml')
calibration_stereo = load_yaml('Stereo.yaml')

#%% Process calibration
R1, R2, P1, P2, Q, _, _ = cv2.stereoRectify(camera_left['camera_matrix']
	, camera_left['dist_coeff']
	, camera_right['camera_matrix']
	, camera_right['dist_coeff']
	, size
	, calibration_stereo['r']
	, calibration_stereo['t']
)

camera_left['P'] = P1
camera_right['P'] = P2

#%% Loading Points

points_left = []
points_right = []

file_load_count = 1 # 109
def extract_points(content):
	content = content['Points']
	content = np.array(list(map(lambda x: x[0], content)))
	return content

for i in range(1, file_load_count + 1):
	content_left = load_yaml('Points/left_points_{}.yaml'.format(i))
	points_left.append(extract_points(content_left))

	content_right = load_yaml('Points/right_points{}.yaml'.format(i))
	points_right.append(extract_points(content_right))

#%% Undistort Points
def undistort_points(points, camera_matrix, distortion_coefficients):
	def remap(point):
		return point * np.array([camera_matrix[0, 0], camera_matrix[1, 1]])	+ np.array([camera_matrix[0, 2], camera_matrix[1, 2]])

	points = cv2.undistortPoints(points, camera_matrix, distortion_coefficients)
	points = np.array([remap(point) for point in points])
	return points.reshape(points.shape[0], 2)

undistorted_points_left = undistort_points(points_left[0], camera_left['camera_matrix'], camera_left['dist_coeff'])
undistorted_points_right = undistort_points(points_right[0], camera_right['camera_matrix'], camera_right['dist_coeff'])

print(undistorted_points_left)

#%% Triangulating Points
homogenous_points_from_raw_points = cv2.triangulatePoints(P1, P2, points_left[0].transpose(), points_right[0].transpose()).transpose()
homogenous_points_from_undistorted_points = cv2.triangulatePoints(P1, P2, undistorted_points_left.transpose(), undistorted_points_right.transpose()).transpose()
xyz = cv2.convertPointsFromHomogeneous(homogenous_points_from_raw_points)
xyz = xyz.reshape(xyz.shape[0],3)

'''WIERD!
the undistorted points give wrong xyz's
'''
#%% Plotting 3D points

fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')
ax.scatter(xyz[:,0], xyz[:,1], xyz[:,2])
plt.show()

#%% Custom triangulation
xyw = np.matmul(camera_left['P'], np.append(xyz[0], 1))
xy = xyw[0:2] / xyw[2]
print(xy - points_left[0][0])

# This result is essentially zero - this is so weird. Where is distortion?

#%% Right camera transform
# reference : https://github.com/elliotwoods/ofxCvMin/blob/master/src/ofxCvMin/Helpers.cpp

V_right = np.vstack([
	np.hstack([calibration_stereo['r'], calibration_stereo['t']])
	, np.array([0, 0, 0, 1])
	])

print('Right camera position')
print(np.matmul(np.linalg.inv(V_right), np.array([0, 0, 0, 1])))

#%% Unproject points
def make_projection_matrix(camera_matrix, size):
	f_x = camera_matrix[0, 0]
	f_y = camera_matrix[1, 1]
	pp_x = camera_matrix[0, 2]
	pp_y = camera_matrix[1, 2]

	P = np.zeros([4,4])
	P[0, 0] = 2.0 * f_x / size[0]
	P[1, 1] = -2.0 * f_y / size[1]
	P[2, 3] = 1.0
	P[2, 0] = 2 * pp_x / size[0] - 1
	P[2, 1] = 1.0 - 2 * pp_y / size[1]

	# near clip = 0.05, far clip = 100
	P[2, 2] = 1.0005003213882450
	P[3, 2] = -0.0500250160694122
	
	return P

def mul(x, T):
	x_dash = np.matmul(np.append(x, 1), T)
	x_dash /= x_dash[3]
	return x_dash[0:3]

P_left = make_projection_matrix(camera_left['camera_matrix'], size)
P_inv_left = np.linalg.inv(P_left)

P_right = make_projection_matrix(camera_left['camera_matrix'], size)
P_inv_right = np.linalg.inv(P_right)

#np.matmul(np.linalg.inv(camera_left['camera_matrix']), np.append(undistorted_points_left[0], 1))
mul(np.array([0, 0, 0]), P_inv_left)
mul(np.array([0, 0, 1]), P_inv_left)

#%% Unproject rays

class Ray:
	def __init__(self, s, t):
		self.s = s
		self.t = t
	
	def intersect(self, other):
		p1 = self.s
		p2 = self.s + self.t
		p3 = other.s
		p4 = other.s + other.t
		EPS = 1E-15

		p13 = p1 - p3
		p43 = p4 - p3
		if np.linalg.norm(p43) < EPS:
			raise Exception("No intersection found")
			
		p21 = p2 - p1
		if np.linalg.norm(p21) < EPS:
			raise Exception("No intersection found")
		
		d1343 = np.dot(p13, p43)
		d4321 = np.dot(p43, p21)
		d1321 = np.dot(p13, p21)
		d4343 = np.dot(p43, p43)
		d2121 = np.dot(p21, p21)

		denom = d2121 * d4343 - d4321 * d4321
		if abs(denom) < EPS:
			raise Exception("No intersection found")
		numer = d1343 * d4321 - d1321 * d4343

		ma = numer / denom
		mb = (d1343 + d4321 * (ma)) / d4343
	
		s = p1 + ma * p21
		t = p3 + mb * p43
		
		return Ray(s, t)

	def plot(self, axis):
		axis.plot([self.s[0], self.t[0]], [self.s[1], self.t[1]], [self.s[2], self.t[2]])

def unproject_ray(P_inv, V_inv, undistorted_point):
	xy = np.array([ 										\
		undistorted_point[0] / size[0] * 2.0 - 1.0 			\
		, 1.0 - undistorted_point[1] / size[1] * 2.0
	])

	s = mul(mul(np.append(xy, 0), P_inv), V_inv.transpose())
	t = mul(mul(np.append(xy, 1), P_inv), V_inv.transpose())
	
	return Ray(s, t)


def unproject_rays(P_inv, V_inv, undistorted_points):
	return [unproject_ray(P_inv, V_inv, undistort_point) for undistort_point in undistorted_points]

#%% Unproject one ray
test_ray = unproject_ray(P_inv_left, np.eye(4), undistorted_points_left[0])

fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')
test_ray.plot(ax)
plt.show()

#%% Unproject many rays
rays_left = unproject_rays(P_inv_left, np.eye(4), undistorted_points_left)
rays_right = unproject_rays(P_inv_right, np.linalg.inv(V_right), undistorted_points_right)

fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')
for ray in rays_left:
	ray.plot(ax)
for ray in rays_right:
	ray.plot(ax)
plt.show()

#%% Intersect rays
intsersections = []
for i in range(len(rays_left)):
	intsersections.append(rays_left[i].intersect(rays_right[i]))

fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')
for intsersection in intsersections:
	intsersection.plot(ax)

for ray in rays_left:
	ray.plot(ax)
for ray in rays_right:
	ray.plot(ax)
plt.show()


#%% Open IPython
#IPython.embed()


# %%
