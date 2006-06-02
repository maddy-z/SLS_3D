/*
*
*	CalcShape.h
*
*	EXPLANATION:
*		calc shape
*
*	EXTERNAL LIBRARIES:
*		cv.lib
*		cxcore.lib
*		cvaux.lib
*		highgui.lib
*
*	IMPLEMENTATION:
*		CalcShape.cpp
*
*	AUTHOR:
*		Daisuke Iwai
*
*/

#pragma once

#include <iostream>
#include <windows.h>

#include <OpenCV_2.3.1\opencv2\opencv.hpp>

class CCalcShape
{
private:

	// 2d position data in camera image
	int *_pos2d;

	// calculated 3d shape data
	float *_pos3d;

	// number of points whose shape data have been measured
	int _validPixNum;

	// camera/projector size
	int _cameraW;
	int _cameraH;
	int _projectorW;
	int _projectorH;

public:

	/*
	*	c'tor / d'tor
	*/
	CCalcShape( int cameraW, int cameraH, int projectorW, int projectorH );
	~CCalcShape();

	/*
	*	calc shape (based on "??")
	*		- C2P: captured graycode
	*		- mask: mask image (only "true" region will be calculated)
	*		- cam_intr/cam_extr: camera intrinsic/extrinsic parameter
	*		- pro_intr/pro_extr: camera intrinsic/extrinsic parameter
	*/
	void CalcShape( double *C2P, bool *mask, CvMat *cam_intr, CvMat* cam_extr, CvMat* pro_intr, CvMat *pro_extr );

	/*
	*	write out shape data in OBJ format
	*		- fname: file name
	*/
	void writeOBJ( char *fname );
};
