#include "pch_MoCapLib.h"
#include "ImageFormat.h"
#include "Exception.h"

namespace TD_MoCap {
	//----------
	ImageFormat
		imageFormatFromCvMat(const cv::Mat& mat)
	{
		switch (mat.type()) {
		case CV_8U:
			return ImageFormat::R_8U;
		default:
			return ImageFormat::Unsupported;
		}
	}

	//----------
	void
		setTOP_OutputFormat(TOP_OutputFormat* outputFormat, const cv::Mat & image)
	{
		outputFormat->width = image.cols;
		outputFormat->height = image.rows;
		
		outputFormat->aspectX = 1.0f;
		outputFormat->aspectY = (float) image.rows / (float) image.cols;

		setTOP_OutputFormat(outputFormat, imageFormatFromCvMat(image));
	}

	//----------
	void
		setTOP_OutputFormat(TOP_OutputFormat* outputFormat, ImageFormat imageFormat)
	{
		switch (imageFormat) {

		case ImageFormat::R_8U:
			outputFormat->redChannel = true;
			outputFormat->greenChannel = false;
			outputFormat->blueChannel = false;
			outputFormat->alphaChannel = false;
			outputFormat->bitsPerChannel = 8;
			
			outputFormat->floatPrecision = false;
			break;

		case ImageFormat::Unsupported:
			throw(Exception("Texture cannot be allocated for unsupported image format"));

		}
	}
}