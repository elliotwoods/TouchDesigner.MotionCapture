#%% imports
import cv2
import numpy as np

#%%
image_prior = cv2.imread('D:/2020-09-03 19.20.57/0/1000.bmp', cv2.IMREAD_GRAYSCALE)
image_current = cv2.imread('D:/2020-09-03 19.20.57/0/1001.bmp', cv2.IMREAD_GRAYSCALE)

# %% contours
blur_size = 16
difference_amplify = 4
threshold = 200
dilation_size = 1

class CentroidsResult:
    def __init__(self):
        self.blurred = None
        self.difference = None
        self.binary = None
        self.contours = None
        self.bounds = []
        self.moments = []
        self.centroids = []


def centroids(image):
    result = CentroidsResult()
    result.blurred = cv2.blur(image, (blur_size, blur_size))
    result.difference_signed = np.array(image, dtype=np.int32) - np.array(result.blurred, dtype=np.int32)
    result.difference = np.array(np.clip(result.difference_signed, 0, 255), dtype=np.uint8)
    result.difference *= difference_amplify
    _, result.binary = cv2.threshold(result.difference, threshold, 255, cv2.THRESH_BINARY)
    result.contours, _ = cv2.findContours(result.binary, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_NONE)

    for contour in result.contours:
        rect = cv2.boundingRect(contour)
        
        # we don't check if touches edge of frame since we're not tracking here

        dilated_rect = (rect[0] - dilation_size
            , rect[1] - dilation_size
            , rect[2] + dilation_size * 2
            , rect[3] + dilation_size * 2)

        if dilated_rect[0] < 0 or dilated_rect[1] < 0 or dilated_rect[0] + dilated_rect[2] >= image.shape[1] or dilated_rect[1] + dilated_rect[3] >= image.shape[0]:
            continue
        
        roi = image[dilated_rect[1] : dilated_rect[1] + dilated_rect[3], 
            dilated_rect[0] : dilated_rect[0] + dilated_rect[2]]
        
        moment = cv2.moments(roi)
        centroid = [moment['m10'] / moment['m00'] + dilated_rect[0], 
            moment['m01'] / moment['m00'] + dilated_rect[1]]

        result.bounds.append(rect)
        result.moments.append(moment)
        result.centroids.append(centroid)


    return result

result_prior = centroids(image_prior)
result_current = centroids(image_current)

    

# %% look ahead one
test_index = 0
test_centroid = result_prior.centroids[test_index]

amp = 1
bright_prior = result_prior.difference
bright_current = result_current.difference

centroid_prior = np.array([test_centroid], dtype=np.float32)
centroid_look_ahead = np.array([test_centroid], dtype=np.float32)
centroid_current = np.array([result_current.centroids[test_index]], dtype=np.float32)

cv2.calcOpticalFlowPyrLK(bright_prior, bright_current, centroid_prior, centroid_look_ahead)

print("prior to look ahead = {}".format(centroid_look_ahead - centroid_prior))
print("prior to current = {}".format(centroid_current - centroid_prior))

preview_prior = cv2.cvtColor(bright_prior, cv2.COLOR_GRAY2RGB)
cv2.circle(preview_prior, (centroid_prior[0, 0], centroid_prior[0,1]), 5, (255, 0, 0), )
cv2.drawContours(preview_prior, result_prior.contours, -1, (0, 255, 0), 1)
cv2.imshow('prior', preview_prior)

preview_current = cv2.cvtColor(bright_current, cv2.COLOR_GRAY2RGB)
cv2.circle(preview_current, (centroid_look_ahead[0, 0], centroid_look_ahead[0,1]), 5, (255, 0, 0), )
cv2.imshow('current', preview_current)
cv2.waitKey(0)
cv2.destroyAllWindows()

# %% look ahead all
prior_all = np.array(result_prior.centroids, dtype=np.float32)
look_ahead_all = prior_all.copy()
cv2.calcOpticalFlowPyrLK(bright_prior, bright_current, prior_all, look_ahead_all)
print(look_ahead_all - prior_all)

# %% CONCLUSION
'''
* We do get results (not all zeros)
* We get better results when dealing with local difference rather than grayscale
* Results don't improve if we amplify the value
'''