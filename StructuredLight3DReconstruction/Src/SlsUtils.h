#ifndef _SLS_UTILS_H_
#define _SLS_UTILS_H_

#include	<OpenCV_2.3.1\opencv2\opencv.hpp>

//
// Raw Image Buffer <-> cv::Mat
// 

bool CopyRawImageBuf2CvMat(const unsigned char * srcImg, unsigned int srcChannel, cv::Mat & destImg);
bool CopyCvMat2RawImageBuf(const cv::Mat & srcImg, unsigned char * destImg, unsigned int destChannel);

void ShowImageInOpenCvWindow(const char * windowName, unsigned char * img, int channel, int camH, int camW);

#endif