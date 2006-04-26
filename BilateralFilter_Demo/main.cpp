#include <opencv2\opencv.hpp>

// 
// Global Variables
// 

cv::Mat src;
cv::Mat dest;

char SRC_WIN_NAME[] = "Src Image";
char DEST_WIN_NAME[] = "Dest Image";

#define		KERNEL_SIZE		31

//
// Main Function
// 

int main(int argc, char ** argv)
{
	
	cv::namedWindow ( SRC_WIN_NAME, CV_WINDOW_AUTOSIZE );
	cv::namedWindow ( DEST_WIN_NAME, CV_WINDOW_AUTOSIZE );
	
	src = cv::imread("lena.jpg", 1);
	dest = cv::Mat::zeros(src.size(), src.type());

	cv::bilateralFilter(src, dest, KERNEL_SIZE, KERNEL_SIZE * 2, KERNEL_SIZE / 2);
	
	cv::imshow( SRC_WIN_NAME, src);
	cv::imshow( DEST_WIN_NAME, dest);

	cv::waitKey(0);

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