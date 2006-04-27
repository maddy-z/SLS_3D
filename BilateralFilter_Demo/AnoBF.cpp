#include "AnoBF.h"

CBilateralFilter::CBilateralFilter( float sigma1, float sigma2, int kernelSize, int approxResolution )
{
	this->kernelSize = kernelSize;
	this->approxResolution = approxResolution;

	filter1 = new float[(kernelSize*2+1)*(kernelSize*2+1)];
	filter2 = new float[approxResolution];

	// init spatial filter
	for( int dy = -kernelSize; dy <= kernelSize; dy++ )
	for( int dx = -kernelSize; dx <= kernelSize; dx++ )
	{
		int pos = (dx+kernelSize)+(dy+kernelSize)*(kernelSize*2+1);
		filter1[pos] = exp(-(dx*dx+dy*dy)/(2*sigma1*sigma1));
	}

	// init range filter
	for( int i = 0; i < approxResolution; i++ )
	{
		filter2[i] = exp(-((float)i/(float)approxResolution)*((float)i/(float)approxResolution)/(2*sigma2*sigma2));
	}
}

CBilateralFilter::~CBilateralFilter() 
{
	delete[] filter1;
	delete[] filter2;
}

double 
*CBilateralFilter::filter( double *original, int width, int height, int channels )
{
	double *bilateral = new double[width*height*channels];

	// init bilateral
	for( int i = 0; i < width*height*channels; i++ )
	{
		bilateral[i] = 0.0;
	}

	// normalize data value

	normalize = new int[width*height*channels];
	double max, min;
	max = min = (double)original[0];
	for( int i = 0; i < width*height*channels; i++ )
	{
		if( max < original[i] )	max = original[i];
		if( min > original[i] ) min = original[i];
	}
	for( int i = 0; i < width*height*channels; i++ )
	{
		normalize[i] = (int)( ( (double)original[i] - min )/( max - min ) * (double)(approxResolution-1) );
	}

	for( int y = kernelSize; y < height - kernelSize; y++ )
	for( int x = kernelSize; x < width - kernelSize; x++ )
	for( int c = 0; c < channels; c++ )
	{
		float child, mother;
		child = mother = 0.0;
		for( int dy = -kernelSize; dy <= kernelSize; dy++ )
		for( int dx = -kernelSize; dx <= kernelSize; dx++ )
		{
			int pos[2];
			int value[2];

			pos[0] = x+y*width;
			pos[1] = (x+dx)+(y+dy)*width;
			value[0] = normalize[pos[0]*channels+c];
			value[1] = normalize[pos[1]*channels+c];

			if( value[0] && value[1] && abs(value[1]-value[0])<approxResolution )
			{
				float f1 = filter1[(dx+kernelSize)+(dy+kernelSize)*(kernelSize*2+1)];
				float f2 = filter2[abs(value[1]-value[0])];
				child += f1*f2*original[pos[1]*channels+c];
				mother += f1*f2;
			}
		}
		int pos = (x+y*width)*channels+c;
		if( mother )	bilateral[pos] = child/mother;
	}

	delete[] normalize;

	return bilateral;
}
