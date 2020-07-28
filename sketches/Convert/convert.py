#%% Start
import cv2
import numpy as np
import IPython
import os
import yaml
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D

# os.chdir('sketches/Triangulation/Calibration_data_200713/')
os.chdir(os.path.dirname(os.path.realpath(__file__)))
os.chdir('../Calibration_data_200713/')

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
points_left = load_yaml('Points/left_points_1.yaml')
points_right = load_yaml('Points/right_points1.yaml')

fs_write = cv2.FileStorage('camera_left.yaml', cv2.FILE_STORAGE_WRITE)
for key in camera_left:
	fs_write.write(key, camera_left[key])
fs_write.write('image_width', size[0])
fs_write.write('image_height', size[1])
fs_write.release()

fs_write = cv2.FileStorage('camera_right.yaml', cv2.FILE_STORAGE_WRITE)
for key in camera_right:
	fs_write.write(key, camera_right[key])
fs_write.write('image_width', size[0])
fs_write.write('image_height', size[1])
fs_write.release()

fs_write = cv2.FileStorage('calibration_stereo.yaml', cv2.FILE_STORAGE_WRITE)
for key in calibration_stereo:
	fs_write.write(key, calibration_stereo[key])
fs_write.release()

fs_write = cv2.FileStorage('points_left.yaml', cv2.FILE_STORAGE_WRITE)
for key in points_left:
	shape = points_left[key].shape
	if len(shape) == 3:
		points_left[key] = np.reshape(points_left[key], (shape[0], shape[2]))
	fs_write.write(key, points_left[key])
fs_write.release()

fs_write = cv2.FileStorage('points_right.yaml', cv2.FILE_STORAGE_WRITE)
for key in points_right:
	shape = points_left[key].shape
	if len(shape) == 3:
		points_left[key] = np.reshape(points_left[key], (shape[0], shape[2]))
	fs_write.write(key, points_right[key])
fs_write.release()
