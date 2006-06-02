#ifndef _SHAPE_CALCULATOR_H_
#define _SHAPE_CALCULATOR_H_

#include <OpenCV_2.3.1\opencv2\opencv.hpp>

class ShapeCalculator
{

public:

	// 
	// Constructor & Destructor
	// 
	
	ShapeCalculator ( int cameraW, int cameraH, int projectorW, int projectorH );
	virtual ~ShapeCalculator();

	/*
	*	Calc Shape
	*	- C2P:	Captured Graycode
	*	- mask: Mask Image ( only "true" region will be calculated )
	*	- camIntr / camExtr: camera intrinsic / extrinsic parameter
	*	- proIntr/ proExtr: camera intrinsic / extrinsic parameter
	*/

	void CalcShape (	double *C2P, 
								bool * mask, 
								const cv::Mat & camIntr, 
								const cv::Mat & camExtr, 
								const cv::Mat & proIntr, 
								const cv::Mat & proExtr );
	
	void CalcShape (	double *C2P_H,
								double *C2P_V,
								bool * mask, 
								const cv::Mat & camIntr, 
								const cv::Mat & camExtr, 
								const cv::Mat & proIntr, 
								const cv::Mat & proExtr );

	/*
	*	Write out Shape Data in OBJ Format
	*	- fileName: file name
	*/

	void WriteOBJ ( const char * fileName );

private:
	
	int m_CameraWidth, m_CameraHeight;					// Camera Size
	int m_ProjectorWidth, m_ProjectorHeight;				// Projector Size
	
	int * m_Pos2d;														// 2D Position Data in Camera Image
	float * m_Pos3d;													// Calculated 3d Shape Data
	int m_ValidPixNum;												// Number of Points whose Shape Data have been measured

};

#endif