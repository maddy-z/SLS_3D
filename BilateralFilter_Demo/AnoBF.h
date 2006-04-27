/*
*
*	BilateralFilter.h
*
*	EXPLANATION:
*		bilateral filtering
*
*	EXTERNAL LIBRARIES:
*		n/a
*
*	IMPLEMENTATION:
*		BilateralFilter.cpp
*
*	AUTHOR:
*		Daisuke Iwai
*
*/

#pragma once

#include <iostream>
#include <cmath>

using namespace std;

class CBilateralFilter

{

private:

	// kernel size
	int kernelSize;

	// approximated resolution of the data range (for pre-computing)
	int approxResolution;

	// spatial / range filter
	float *filter1;
	float *filter2;

	// normalized data based on approxResolution
	int *normalize;

public:

	/*
	*	c'tor / d'tor
	*		- sigma1: for spatial filter
	*		- sigma2: for range filter
	*		- kernelSize: kernel size
	*		- approxResolution: approximated resoloution for data range
	*/
	CBilateralFilter( float sigma1, float sigma2, int kernelSize, int approxResolution );
	~CBilateralFilter();

	/*
	*	bilateral filter
	*		- original: data which will be filtered
	*		- width/height/channel: data size
	*		- return: filtered data
	*/
	double * filter ( double * original, int width, int height, int channels );
};
