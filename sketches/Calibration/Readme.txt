Procedure:

1. Set up your folders, e.g.:

	../Calibration2020XXXX/SingleLEFT
	../Calibration2020XXXX/SingleRIGHT
	../Calibration2020XXXX/StereoLEFT
	../Calibration2020XXXX/StereoRIGHT

	Each folder should contain .png files (filenames are not important)

	The StereoLEFT and StereoRIGHT should contain matching images only.
	The filenames do not need to match, but they should appear in a matching sequence (e.g. 'left000.png' x 'right000.png' is OK)

2. Run this Python script

	It can take a really long time if there are many images (e.g. > 100).

3. Check the plots which come out and selectively prune
	
	Are there any outliers in the reprojected results for intrinsics or Stereo?
	(Check the intrinsics first and work on Stereo after that's fixed).

	e.g. if one image has more than 2px reprojection error, then it might be a bad image (e.g. blurred / tracking failed)
	Note : don't just remove all the captures which are close to the camera (as these will naturally have large  reprojection because the board is large in th image, but also may be blurred)
		In this case, remove the worst ones from the close-up images.

	Try selectively removing a couple at a time until you get a good result for calibration

	Repeat the process for Streo pairs (remember to remove the image from both folders, otherwise the calibration will be really confused)

	Each time you remove images, you will need to remove the associated calibration from the ../Calibration2020XXXX/Cache folder, to force a recalibration
