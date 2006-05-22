#include <cstdio>
#include <cstdlib>
#include <iostream>

#include	<OpenCV_2.3.1\opencv2\opencv.hpp>

#include "SlsUtils.h"

// ===========================
// Raw Image Buffer <-> cv::Mat
// ===========================

bool CopyRawImageBuf2CvMat(const unsigned char * srcImg, unsigned int srcChannel, cv::Mat & destImg)
{
	if ( destImg.empty() ) { return false; }
	// if ( destImg.channels() <= srcChannel ) { return false; }

	int destChannel = destImg.channels(); 

	unsigned char * destRowStart = destImg.data;
	unsigned char * destDataPtr = NULL;
	unsigned int minC = (srcChannel < destChannel) ? (srcChannel) : (destChannel);

	for (int y = 0; y < destImg.rows; ++y, destRowStart += destImg.step)
	{
		destDataPtr = destRowStart;
	
		for (int x = 0; x < destImg.cols; ++x, destDataPtr += destChannel, srcImg += srcChannel) 
		for (int k = 0; k < minC; ++k) {
				destDataPtr[k] = srcImg[k];
		}
	}

	return true;
}

bool CopyCvMat2RawImageBuf(const cv::Mat & srcImg, unsigned char * destImg, unsigned int destChannel)
{
	if ( srcImg.empty() ) { return false; }
	// if ( srcImg.channels() <= destChannel ) { return false; }

	int srcChannel = srcImg.channels(); 

	unsigned char * srcRowStart = srcImg.data;
	unsigned char * srcDataPtr = NULL;
	unsigned int minC = (srcChannel < destChannel) ? (srcChannel) : (destChannel);

	for (int y = 0; y < srcImg.rows; ++y, srcRowStart += srcImg.step)
	{
		srcDataPtr = srcRowStart;
	
		for (int x = 0; x < srcImg.cols; ++x, srcDataPtr += srcChannel, destImg += destChannel) 
		for (int k = 0; k < minC; ++k) {
				destImg[k] = srcDataPtr[k];
		}
	}

	return true;
}

// ==================
// Experiment Functions
// ==================

void ShowImageInOpenCvWindow(const char * windowName, unsigned char * srcImg, int srcChannel, int camH, int camW)
{
	cv::Mat destImg(cv::Size(camW, camH), CV_8UC3);
	CopyRawImageBuf2CvMat(srcImg, srcChannel, destImg);

	/*
	unsigned char * srcData = img;
	unsigned char * destImgRow = destImg.data;
	
	for (int i = 0; i < destImg.rows; ++i, destImgRow += destImg.step) 
	{
		unsigned char * destImgData = destImgRow;
		
		for (int j = 0; j < destImg.cols; ++j, destImgData += destImg.channels(), srcData += 4) 
		{
			destImgData[0] = srcData[2];
			destImgData[1] = srcData[1];
			destImgData[2] = srcData[0];
		}
	}
	*/

	cv::namedWindow(windowName, CV_WINDOW_AUTOSIZE);
	cv::imshow(windowName, destImg);

	return;
}