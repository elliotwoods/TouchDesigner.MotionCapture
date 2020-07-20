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
		case CV_8UC2:
			return ImageFormat::RGB_8U;
		case CV_8UC3: // note this is unsupported
			return ImageFormat::RGB_8U;
		case CV_8UC4:
			return ImageFormat::RGBA_8U;
		default:
			return ImageFormat::Unsupported;
		}
	}

	//----------
	void
		setTOP_OutputFormat(TOP_OutputFormat* outputFormat, const cv::Mat & image, OP_CPUMemPixelType & memType)
	{
		outputFormat->width = image.cols;
		outputFormat->height = image.rows;
		
		outputFormat->aspectX = 1.0f;
		outputFormat->aspectY = (float) image.rows / (float) image.cols;

		setTOP_OutputFormat(outputFormat, imageFormatFromCvMat(image), memType);
	}

	//----------
	void
		setTOP_OutputFormat(TOP_OutputFormat* outputFormat, ImageFormat imageFormat, OP_CPUMemPixelType& memType)
	{
		switch (imageFormat) {

		case ImageFormat::R_8U:
			outputFormat->redChannel = true;
			outputFormat->greenChannel = false;
			outputFormat->blueChannel = false;
			outputFormat->alphaChannel = false;
			outputFormat->bitsPerChannel = 8;
			
			
			outputFormat->floatPrecision = false;

			memType = OP_CPUMemPixelType::R8Fixed;
			break;

		case ImageFormat::RG_8U:
			outputFormat->redChannel = true;
			outputFormat->greenChannel = true;
			outputFormat->blueChannel = false;
			outputFormat->alphaChannel = false;
			outputFormat->bitsPerChannel = 8;

			outputFormat->floatPrecision = false;

			memType = OP_CPUMemPixelType::RG8Fixed;
			break;

		case ImageFormat::RGBA_8U:
			outputFormat->redChannel = true;
			outputFormat->greenChannel = true;
			outputFormat->blueChannel = true;
			outputFormat->alphaChannel = true;
			outputFormat->bitsPerChannel = 8;

			outputFormat->floatPrecision = false;

			memType = OP_CPUMemPixelType::RGBA8Fixed;
			break;

		case ImageFormat::Unsupported:
		default:
			throw(Exception("Texture cannot be allocated for unsupported image format"));

		}
	}
}