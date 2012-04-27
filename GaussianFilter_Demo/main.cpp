#include <OpenCV_2.3.1\opencv2\opencv.hpp>

// ==============
//   MACRO DEFINITIONS
// ==============

#define			KERNEL_SIZE				15
#define			MAX_KERNEL_SIZE			127

// ====================
//   Global Variables
// ====================

cv::Mat		src;
cv::Mat		dest;

char	SRC_WIN_NAME[]					= "Src Image";
char	DEST_WIN_NAME[]					= "Dest Image";

char TRACKBAR_KERNELSIZE_NAME[]			= "Kernel Size";
char TRACKBAR_SIGMA1_NAME[]				= "Sigma1 (Sigma 2)";

int		kernelSize						= 2 * KERNEL_SIZE + 1;
int		sigma1							= KERNEL_SIZE;

// 
// Callback Functions
// 

void onTrackBar ( int, void * )
{
	cv::GaussianBlur (src, dest, cv::Size(2 * kernelSize + 1, 2 * kernelSize + 1), sigma1);
	cv::imshow ( DEST_WIN_NAME, dest);
}

// =================
//   Main Function
// =================

int main(int argc, char ** argv)
{
	cv::namedWindow ( SRC_WIN_NAME, CV_WINDOW_AUTOSIZE );
	cv::namedWindow ( DEST_WIN_NAME, CV_WINDOW_AUTOSIZE );
	
	src = cv::imread("lena.jpg", 1);
	cv::imshow( SRC_WIN_NAME, src);
	
	dest = src.clone();

	cv::createTrackbar(TRACKBAR_KERNELSIZE_NAME, DEST_WIN_NAME, &kernelSize, MAX_KERNEL_SIZE, onTrackBar);
	cv::createTrackbar(TRACKBAR_SIGMA1_NAME, DEST_WIN_NAME, &sigma1, 2 * MAX_KERNEL_SIZE, onTrackBar);
	
	onTrackBar(0, 0);

	cv::waitKey(0);
	return EXIT_SUCCESS;
}
