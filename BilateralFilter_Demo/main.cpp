#include <OpenCV_2.3.1\opencv2\opencv.hpp>
#include <OpenCV_2.3.1\opencv2\imgproc\imgproc.hpp>

#include "BilateralFilter.h"

// ===================
//  MACRO DEFINITIONS
// ===================

#define	KERNEL_SIZE					15
#define	MAX_KERNEL_SIZE				64

// ====================
//   Global Variables
// ====================

cv::Mat	src;

cv::Mat	mylib_bf_dest;
cv::Mat opencv_bf_dest;
cv::Mat opencv_gf_dest;

double	* srcArray					= NULL;
double	* destArray					= NULL;

char	SRC_WIN_NAME[]				= "Src Image";
char	MYLIB_BF_WIN_NAME[]		= "My BF Image";
char	OPENCV_BF_WIN_NAME[]	= "OpenCV BF Image";
char OPENCV_GF_WIN_NAME[]	= "OpenCV GF Image";

char	TRACKBAR_KERNELSIZE_NAME[]	= "Kernel Size";
char	TRACKBAR_SIGMACOLOR_NAME[]	= "Sigma Color";
char	TRACKBAR_SIGMASPACE_NAME[]	= "Sigma Space";

int		kernelSize					= KERNEL_SIZE;

int		sigmaColor					= 60;
int		sigmaSpace					= 60;

// 
// Utility Function
// 

void DoubleArrayCopyToCvMat(const double * src, cv::Mat & dest)
{
	int destH = dest.size().height, destW = dest.size().width;
	int destRowStep = dest.step, destChannel = dest.channels();
	
	assert (dest.type() == CV_32FC1);

	uchar * destRowStart = dest.data;
	float * destDataPtr = NULL;

	for (int y = 0; y < destH; ++y, destRowStart += destRowStep) {
		destDataPtr = (float *)(destRowStart);

		for (int x = 0; x < destW; ++x, destDataPtr += destChannel) {
			*destDataPtr = (*src++) / 255.f; 
		}
	}
}

void CvMatCopyToDoubleArray(const cv::Mat & src, double * dest)
{
	int srcH = src.size().height, srcW = src.size().width;
	int srcRowStep = src.step, srcChannel = src.channels();
	
	assert (src.type() == CV_8UC1);
	assert (dest != NULL);

	const uchar * srcRowStart = src.data;
	const uchar * srcDataPtr = NULL;

	for (int y = 0; y < srcH; ++y, srcRowStart += srcRowStep) {
		srcDataPtr = srcRowStart;	

		for (int x = 0; x < srcW; ++x, srcDataPtr += srcChannel) {
			*dest++ = (double)(*srcDataPtr); 
		}
	}
}

// =====================
//   Callback Function
// =====================

void onTrackBar(int, void *)
{
	int tmpKSize = 2 * kernelSize + 1;

	BilateralFilter bFilter(sigmaSpace, sigmaColor, tmpKSize, 256);
	bFilter.Filter(srcArray, destArray, src.rows, src.cols, 1);
	if (destArray == NULL) {
		printf ("Not successful Bilateral Filtering\n");
		return;
	}
	DoubleArrayCopyToCvMat(destArray, mylib_bf_dest);

	cv::bilateralFilter(src, opencv_bf_dest, tmpKSize, sigmaColor, sigmaSpace, cv::BORDER_CONSTANT);
	cv::GaussianBlur(src, opencv_gf_dest, cv::Size(tmpKSize, tmpKSize), sigmaSpace);

	cv::imshow( MYLIB_BF_WIN_NAME, mylib_bf_dest);
	cv::imshow( OPENCV_BF_WIN_NAME, opencv_bf_dest);
	cv::imshow( OPENCV_GF_WIN_NAME, opencv_gf_dest);

}

// =================
//   Main Function
// =================

int main(int argc, char ** argv)
{
	cv::namedWindow ( SRC_WIN_NAME, CV_WINDOW_AUTOSIZE );
	cv::namedWindow ( MYLIB_BF_WIN_NAME, CV_WINDOW_AUTOSIZE );
	cv::namedWindow ( OPENCV_BF_WIN_NAME, CV_WINDOW_AUTOSIZE );	
	cv::namedWindow ( OPENCV_GF_WIN_NAME, CV_WINDOW_AUTOSIZE );

	src = cv::imread("lena.jpg", 0);
	mylib_bf_dest = cv::Mat::zeros(src.size(), CV_32FC1);
	opencv_bf_dest = cv::Mat::zeros(src.size(), src.type());
	opencv_gf_dest = cv::Mat::zeros(src.size(), src.type());

	srcArray = new double[src.cols * src.rows];
	destArray = new double[src.cols * src.rows];
	CvMatCopyToDoubleArray(src, srcArray);
	CvMatCopyToDoubleArray(src, destArray);

	cv::imshow( SRC_WIN_NAME, src);

	cv::createTrackbar(TRACKBAR_KERNELSIZE_NAME, MYLIB_BF_WIN_NAME, &kernelSize, MAX_KERNEL_SIZE, onTrackBar);
	cv::createTrackbar(TRACKBAR_SIGMACOLOR_NAME, MYLIB_BF_WIN_NAME, &sigmaColor, 2 * MAX_KERNEL_SIZE, onTrackBar);
	cv::createTrackbar(TRACKBAR_SIGMASPACE_NAME, MYLIB_BF_WIN_NAME, &sigmaSpace, 2 * MAX_KERNEL_SIZE, onTrackBar);

	onTrackBar(0, 0);

	cv::waitKey(0);

	delete [] srcArray;
	delete [] destArray;
	
	return EXIT_SUCCESS;
}

/*

// 
// Global Variables
// 

int	 DELAY_CAPTION = 1500;
int	 DELAY_BLUR = 100;
int	 MAX_KERNEL_LENGTH = 31;

cv::Mat src; 
cv::Mat dst;

char window_name[] = "Filter Demo 1";

// 
// Function headers
// 

int display_caption ( char * caption );
int display_dst ( int delay );

// 
// Main Function
// 

int main ( int argc, char ** argv )
{
	cv::namedWindow ( window_name, CV_WINDOW_AUTOSIZE );

	// Load the source image
	src = cv::imread( "lena.jpg", 1 );

	if ( display_caption( "Original Image" ) != 0 ) { return 0; }

	dst = src.clone();
	if ( display_dst( DELAY_CAPTION ) != 0 ) { return 0; }

	// Applying Homogeneous blur
	if ( display_caption( "Homogeneous Blur" ) != 0 ) { return 0; }

	for ( int i = 1; i < MAX_KERNEL_LENGTH; i = i + 2 )
	{ 
		cv::blur( src, dst, cv::Size( i, i ), cv::Point(-1,-1) );

		if ( display_dst( DELAY_BLUR ) != 0 ) { 
			return 0; 
		} 
	}

	// Applying Gaussian blur
    if ( display_caption( "Gaussian Blur" ) != 0 ) 
	{ 
		return 0; 
	}

    for ( int i = 1; i < MAX_KERNEL_LENGTH; i = i + 2 )
	{ 
		cv::GaussianBlur( src, dst, cv::Size( i, i ), 0, 0 );
		if ( display_dst( DELAY_BLUR ) != 0 ) 
		{ 
			return 0; 
		} 
	}

	// Applying Median blur
	if ( display_caption( "Median Blur" ) != 0 ) 
	{ 
		return 0; 
	}

    for ( int i = 1; i < MAX_KERNEL_LENGTH; i = i + 2 )
	{ 
		cv::medianBlur ( src, dst, i );
		
		if( display_dst( DELAY_BLUR ) != 0 ) 
		{ 
			return 0; 
		} 
	}

	// Applying Bilateral Filter
	if( display_caption( "Bilateral Blur" ) != 0 ) 
	{ 
		return 0;
	}

	for ( int i = 1; i < MAX_KERNEL_LENGTH; i = i + 2 )
	{ 
		cv::bilateralFilter ( src, dst, i, i * 2, i/2 );
		
		if( display_dst( DELAY_BLUR ) != 0 ) 
		{ 
			return 0; 
		} 
	}

	// Wait until user press a key
    display_caption( "End: Press a key!" );

    cv::waitKey(0);
     
	return 0;
}

int display_caption( char * caption )
{
	dst = cv::Mat::zeros( src.size(), src.type() );
	cv::putText( dst, caption, cv::Point( src.cols/4, src.rows/2), CV_FONT_HERSHEY_COMPLEX, 1, cv::Scalar(255, 255, 255) );

	cv::imshow( window_name, dst );
		
	int c = cv::waitKey( DELAY_CAPTION );
	if ( c >= 0 ) { 
		return -1; 
	}
	
	return 0;
}

int display_dst( int delay )
{
	cv::imshow( window_name, dst );
    
	int c = cv::waitKey ( delay );
    if ( c >= 0 ) 
	{ 
		return -1; 
	}

	return 0;
}

*/