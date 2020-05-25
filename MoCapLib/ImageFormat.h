#pragma once

#include <opencv2/opencv.hpp>

namespace TD_MoCap {
	enum class TDMOCAP_API ImageFormat {
		R_8U,

		Unsupported
	};

	ImageFormat TDMOCAP_API imageFormatFromCvMat(const cv::Mat&);

	void TDMOCAP_API setTOP_OutputFormat(TOP_OutputFormat*, const cv::Mat &);
	void TDMOCAP_API setTOP_OutputFormat(TOP_OutputFormat*, ImageFormat);
}