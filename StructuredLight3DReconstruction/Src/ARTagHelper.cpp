#include	<cstdio>
#include	<cstdlib>
#include	<iostream>
#include	<windows.h>

#include	<ARTag\artag_rev2.h>
#include	<OpenCV_2.3.1\opencv2\opencv.hpp>
#include	<OpenGL\Glut\glut.h>

#include	"ARTagHelper.h"
#include	"GrayCode.h"

// ================
// ARTagHelper
// ================

ARTagHelper::ARTagHelper ( int cameraW, int cameraH, const char * fnConfig, const char * fnCornerPos ) :
	m_MarkerNum(0)
{
	assert ( fnConfig != NULL );
	assert ( fnCornerPos != NULL );

	m_CameraWidth = cameraW;
	m_CameraHeight = cameraH;

	// sprintf ( m_ConfigFile, "%s", fnConfig );
	// sprintf ( m_CornerPos, "%s", fnCornerPos );

	// Read Marker Information
	// Number of Markers / Positions of Marker Corners

	FILE * fp;
	char str[256];

	fp = fopen ( fnCornerPos, "r" );
	// fp = fopen ( m_CornerPos, "r" );

	do { fgets ( str, 256, fp ); } while ( str[0] == '#' );
	sscanf ( str, "%d", &m_MarkerNum );

	assert (m_MarkerNum > 0);
	m_MarkerID_LUT = new int[m_MarkerNum];

	m_MarkerCornerPos3d = ( double(*)[3] )( malloc ( m_MarkerNum * 4 * 3 * sizeof ( double ) ) );
	m_MarkerCornerPosCam2d = ( double(*)[2] )( malloc ( m_MarkerNum * 4 * 2 * sizeof ( double ) ) );
	m_MarkerCornerPosPro2d = ( double(*)[2] )( malloc ( m_MarkerNum * 4 * 2 * sizeof ( double ) ) );
	
	m_ValidFlagCam = new bool[m_MarkerNum];
	m_ValidFlagPro = new bool[m_MarkerNum];

	for ( int i = 0; i < m_MarkerNum; ++i )
	{
		m_ValidFlagCam[i] = false;
		m_ValidFlagPro[i] = false;
	}

	for ( int id = 0; id < m_MarkerNum; ++id )
	for ( int i = 0; i < 4; ++i )
	{
		int h = id * 4 + i;
		do { fgets(str, 256, fp); } while ( str[0] == '#' );
		
		sscanf ( str, "%lf %lf %lf", &m_MarkerCornerPos3d[h][0], &m_MarkerCornerPos3d[h][1], &m_MarkerCornerPos3d[h][2] );
	}
	fclose ( fp );

	// 
	// Init ARTag
	// 

	init_artag ( m_CameraWidth, m_CameraHeight, 3 );
	
	int res = load_array_file ( (char *)(fnConfig) );
	// int res = load_array_file ( m_ConfigFile );
	if ( res == -1 )
	{
		printf( "%c is not found\n", fnConfig );
		// printf( "%c is not found\n", m_ConfigFile );
		
		system("pause");
		exit(0);
	}

	char id[8];
	for ( int i = 0; i < m_MarkerNum; ++i )
	{
		sprintf( id, "%d", i );
		m_MarkerID_LUT[i] = artag_associate_array( id );
	}

	return;
}
ARTagHelper::~ARTagHelper()
{
	if (m_MarkerID_LUT) { delete [] m_MarkerID_LUT; }

	if (m_MarkerCornerPos3d) { free(m_MarkerCornerPos3d); }
	if (m_MarkerCornerPosCam2d) { free(m_MarkerCornerPosCam2d); }
	if (m_MarkerCornerPosPro2d) { free(m_MarkerCornerPosPro2d); }

	/*	
	if (m_MarkerCornerPos3d) { delete [] m_MarkerCornerPos3d; }
	if (m_MarkerCornerPosCam2d) { delete [] m_MarkerCornerPosCam2d; }
	if (m_MarkerCornerPosPro2d) { delete [] m_MarkerCornerPosPro2d; }
	*/

	if (m_ValidFlagCam) { delete [] m_ValidFlagCam; }
	if (m_ValidFlagPro) { delete [] m_ValidFlagPro; }
}

void ARTagHelper::FindMarkerCorners( unsigned char * image )
{
	std::cout << "Start:\tvoid ARTagHelper::FindMarkerCorners ( unsigned char * )" << std::endl;
	std::cout << "Marker Number = " << m_MarkerNum << std::endl;

	artag_find_objects ( image, 1 );
	int nFound = 0;
	
	for ( int i = 0; i < m_MarkerNum; ++i )
	{
		if ( artag_is_object_found( m_MarkerID_LUT[i] ) )
		{
			m_ValidFlagCam[i] = true;
			nFound++;
			
			// 
			// All the corners of the marker should be visible from the camera.
			// 

			for ( int j = 0; j < 4; j++ )
			{
				float camX, camY;
				
				artag_project_point( m_MarkerID_LUT[i], 
									m_MarkerCornerPos3d[i*4+j][0], m_MarkerCornerPos3d[i*4+j][1], m_MarkerCornerPos3d[i*4+j][2], 
									&camX, &camY );
				
				m_MarkerCornerPosCam2d[i*4+j][0] = camX;
				m_MarkerCornerPosCam2d[i*4+j][1] = camY;
				
				if ( camX < 0.0 || camX >= m_CameraWidth || camY < 0.0 || camY >= m_CameraHeight ) {
					m_ValidFlagCam[i] = false;
				}
			}
		}
	}

	std::cout << "Mark Found " << nFound << std::endl;
	std::cout << "End:\tvoid ARTagHelper::FindMarkerCorners ( unsigned char * )" << std::endl;

	return;
}

void ARTagHelper::PrintMarkerCornersPos2dInCam () const
{
	for ( int i = 0; i < m_MarkerNum; ++i ) 
	{
		if ( !m_ValidFlagCam[i] ) { continue; }
		
		for ( int j = 0; j < 4; ++j ) {
			printf("<%.3f, %.3f>\n", m_MarkerCornerPosCam2d[i*4+j][0], m_MarkerCornerPosCam2d[i*4+j][1]);
		}
	}
}
void ARTagHelper::PrintMarkerCornersPos2dInProjector() const
{
	for ( int i = 0; i < m_MarkerNum; ++i ) 
	{
		if ( !m_ValidFlagPro[i] ) { continue; }
		
		for ( int j = 0; j < 4; ++j ) {
			printf("<%.3f, %.3f>\n", m_MarkerCornerPosPro2d[i*4+j][0], m_MarkerCornerPosPro2d[i*4+j][1]);
		}
	}
}
void ARTagHelper::PrintMarkerCornersPos3d() const
{
	for ( int i = 0; i < m_MarkerNum; ++i ) 
	for ( int j = 0; j < 4; ++j )
	{
		double x = m_MarkerCornerPos3d[i*4+j][0];
		double y = m_MarkerCornerPos3d[i*4+j][1];
		double z = m_MarkerCornerPos3d[i*4+j][2];
		
		printf("<%.3f, %.3f, %.3f>\n", x, y, z);
	}
}
void ARTagHelper::PrintRecalcMarkerCornersPos3d() const
{
	int count = 0;

	for ( int i = 0; i < m_MarkerNum; ++i ) 
	{
		if ( !m_ValidFlagCam[i] || !m_ValidFlagPro[i] ) { continue; }

		for ( int j = 0; j < 4; ++j ) 
		{
			printf ( "<" );
			for ( int k = 0; k < m_Reconstructed3dPts.cols; ++k ) {
				printf ( "%.3f ", m_Reconstructed3dPts.at<double>(count, k) );
			}	
			printf ( "> " );

			++count;

			double x = m_MarkerCornerPos3d[i*4+j][0];
			double y = m_MarkerCornerPos3d[i*4+j][1];
			double z = m_MarkerCornerPos3d[i*4+j][2];
		
			printf("<%.3f, %.3f, %.3f>\n", x, y, z);
		}
	}

	return;
}

void ARTagHelper::SaveMarkerCornersPos ( int type, const char * fileName ) const
{
	if ( fileName == NULL ) { return; }

	FILE * fp = fopen ( fileName, "w" );
	if ( fp == NULL ) {
		return;
	}

	if ( type == MARKER_IN_CAMERA ) 
	{
		for ( int i = 0; i < m_MarkerNum; ++i ) {
			if ( !m_ValidFlagCam[i] ) { continue; }
		
			for ( int j = 0; j < 4; ++j ) {
				fprintf ( fp, "<%.3f, %.3f>\n", m_MarkerCornerPosCam2d[i*4+j][0], m_MarkerCornerPosCam2d[i*4+j][1] );
			}
		}
	}
	else if ( type == MARKER_IN_PROJECTOR ) 
	{
		for ( int i = 0; i < m_MarkerNum; ++i ) 
		{
			if ( !m_ValidFlagPro[i] ) { continue; }
		
			for ( int j = 0; j < 4; ++j ) {
				fprintf ( fp, "<%.3f, %.3f>", m_MarkerCornerPosPro2d[i*4+j][0], m_MarkerCornerPosPro2d[i*4+j][1] );
				fprintf ( fp, "\t" );
				fprintf ( fp, "<%.3f, %.3f, %.3f>", m_MarkerCornerPos3d[i*4+j][0], m_MarkerCornerPos3d[i*4+j][1], m_MarkerCornerPos3d[i*4+j][2] );
				fprintf ( fp, "\n" );
			}
		}
	}
	else if ( type == MARKER_IN_REALWORLD )
	{
		for ( int i = 0; i < m_MarkerNum; ++i ) 
		for ( int j = 0; j < 4; ++j )
		{
			double x = m_MarkerCornerPos3d[i*4+j][0];
			double y = m_MarkerCornerPos3d[i*4+j][1];
			double z = m_MarkerCornerPos3d[i*4+j][2];
		
			fprintf ( fp, "<%.3f, %.3f, %.3f>\n", x, y, z );
		}
	}
	
	fclose ( fp );
}

void ARTagHelper::RecalcMarkerCornersPos3d ( const cv::Mat & camIntr, 
																		const cv::Mat & camExtr, 
																		const cv::Mat & proIntr, 
																		const cv::Mat & proExtr )
{
	int nValidCam = 0;
	for ( int i = 0; i < m_MarkerNum; ++i ) { 
		if ( m_ValidFlagCam[i] ) { ++nValidCam; }
	}

	int nValidPro = 0;
	for ( int i = 0; i < m_MarkerNum; ++i ) {
		if ( m_ValidFlagPro[i] ) { ++nValidPro; }
	}

	int nValid = ( nValidCam < nValidPro ) ? ( nValidCam ) : ( nValidPro );

	CvMat * camPts = cvCreateMat ( 2, nValid * 4, CV_64FC1 );
	CvMat * proPts = cvCreateMat ( 2, nValid * 4, CV_64FC1 );
	CvMat * posPts = cvCreateMat ( 4, nValid * 4, CV_64FC1 );

	int count = 0;
	
	for ( int i = 0; i < m_MarkerNum; ++i ) 
	{
		if ( !m_ValidFlagPro[i] || !m_ValidFlagCam[i] ) { continue; }

		for ( int j = 0; j < 4; ++j ) 
		{
			cvmSet ( camPts, 0, count, m_MarkerCornerPosCam2d[i*4+j][0] );
			cvmSet ( camPts, 1, count, m_MarkerCornerPosCam2d[i*4+j][1] );

			cvmSet ( proPts, 0, count, m_MarkerCornerPosPro2d[i*4+j][0] );
			cvmSet ( proPts, 1, count, m_MarkerCornerPosPro2d[i*4+j][1] );

			++count;
		}
	}

	assert ( count == nValid * 4);

	cv::Mat camProj = camIntr * camExtr;
	cv::Mat proProj = proIntr * proExtr;

	CvMat * cProj = cvCreateMat ( 3, 4, CV_64FC1 );
	CvMat * pProj = cvCreateMat ( 3, 4, CV_64FC1 );
	
	for ( int i = 0; i < 3; ++i )
	for ( int j = 0; j < 4; ++j ) {
		cvmSet ( cProj, i, j, camProj.at<double>(i, j) );
		cvmSet ( pProj, i, j, proProj.at<double>(i, j) );
	}

	m_Reconstructed3dPts.create ( nValid * 4, 3, CV_64FC1 );

	//cv::Mat F ( 3, 1, CV_64FC1 );
	//cv::Mat Q ( 3, 3, CV_64FC1 );
	//cv::Mat invQ ( 3, 3, CV_64FC1 );
	//cv::Mat matPos3d ( 3, 1, CV_64FC1 );

	//for ( int i = 0; i < nValid * 4; ++i )
	//{
	//	double x = cvmGet ( camPts, 0, i );
	//	double y = cvmGet ( camPts, 1, i );
	//	double xa = cvmGet ( proPts, 0, i );
	//	double ya = cvmGet ( proPts, 1, i );

	//	F.at<double>(0, 0) = x * camProj.at<double>(2, 3) - camProj.at<double>(0, 3);
	//	F.at<double>(1, 0) = y * camProj.at<double>(2, 3) - camProj.at<double>(1, 3);
	//	F.at<double>(2, 0) = xa * proProj.at<double>(2, 3) - proProj.at<double>(0, 3);
	//	// F.at<double>(2, 0) = ya * proProj.at<double>(2, 3) - proProj.at<double>(1, 3);

	//	Q.at<double>(0, 0) = camProj.at<double>(0, 0) - x * camProj.at<double>(2, 0);
	//	Q.at<double>(0, 1) = camProj.at<double>(0, 1) - x * camProj.at<double>(2, 1);
	//	Q.at<double>(0, 2) = camProj.at<double>(0, 2) - x * camProj.at<double>(2, 2);
	//	Q.at<double>(1, 0) = camProj.at<double>(1, 0) - y * camProj.at<double>(2, 0);
	//	Q.at<double>(1, 1) = camProj.at<double>(1, 1) - y * camProj.at<double>(2, 1);
	//	Q.at<double>(1, 2) = camProj.at<double>(1, 2) - y * camProj.at<double>(2, 2);
	//	/*
	//	Q.at<double>(2, 0) = proProj.at<double>(1, 0) - ya * camProj.at<double>(2, 0);
	//	Q.at<double>(2, 1) = proProj.at<double>(1, 1) - ya * camProj.at<double>(2, 1);
	//	Q.at<double>(2, 2) = proProj.at<double>(1, 2) - ya * camProj.at<double>(2, 2);
	//	*/
	//	Q.at<double>(2, 0) = proProj.at<double>(0, 0) - xa * camProj.at<double>(2, 0);
	//	Q.at<double>(2, 1) = proProj.at<double>(0, 1) - xa * camProj.at<double>(2, 1);
	//	Q.at<double>(2, 2) = proProj.at<double>(0, 2) - xa * camProj.at<double>(2, 2);
	//
	//	invQ = Q.inv(cv::DECOMP_LU);
	//	matPos3d = invQ * F;
	//		
	//	m_Reconstructed3dPts.at<double>(i, 0) = matPos3d.at<double>(0, 0);
	//	m_Reconstructed3dPts.at<double>(i, 1) = matPos3d.at<double>(1, 0);
	//	m_Reconstructed3dPts.at<double>(i, 2) = matPos3d.at<double>(2, 0);

	//}

	cvTriangulatePoints ( cProj, pProj, camPts, proPts, posPts ); 

	for ( int i = 0; i < nValid * 4; ++i ) 
	{
		double z = cvmGet(posPts, 3, i);

		m_Reconstructed3dPts.at<double>(i, 0) = cvmGet(posPts, 0, i) / z;
		m_Reconstructed3dPts.at<double>(i, 1) = cvmGet(posPts, 1, i) / z;
		m_Reconstructed3dPts.at<double>(i, 2) = cvmGet(posPts, 2, i) / z;
	}

	return;
}

void ARTagHelper::GetMarkerCornerPos2dInProjector ( GrayCode * gc )
{
	for ( int i = 0; i < m_MarkerNum; ++i )
	{
		m_ValidFlagPro[i] = true;

		for ( int j = 0; j < 4; ++j )
		{
			double x = m_MarkerCornerPosCam2d[i*4+j][0];
			double y = m_MarkerCornerPosCam2d[i*4+j][1];

			if ( gc->dblCode(gc->VERT, x, y) != 0.0 && 
				 gc->dblCode(gc->HORI, x, y) != 0.0 &&
				 gc->m_Mask[(int)(x) + (int)(y) * m_CameraWidth] )
			{
				// Gray Code can be obtained at the markers.
				m_MarkerCornerPosPro2d[i*4+j][0] = gc->dblCode(gc->VERT, x, y);
				m_MarkerCornerPosPro2d[i*4+j][1] = gc->dblCode(gc->HORI, x, y);
			}
			else { m_ValidFlagPro[i] = false; }
		}
	}
}

void ARTagHelper::DrawMarkersInCameraImage ( float pixZoomX, float pixZoomY )
{
	for ( int i = 0; i < m_MarkerNum; ++i )
	{
		if ( m_ValidFlagCam[i] )
		{
			glColor3f(1.0f, 0.0f, 0.0f);

			glBegin(GL_LINE_LOOP);
				glVertex2f ( pixZoomX * m_MarkerCornerPosCam2d[i*4+0][0], pixZoomY * (m_CameraHeight - m_MarkerCornerPosCam2d[i*4+0][1]) );
				glVertex2f ( pixZoomX * m_MarkerCornerPosCam2d[i*4+1][0], pixZoomY * (m_CameraHeight - m_MarkerCornerPosCam2d[i*4+1][1]) );
				glVertex2f ( pixZoomX * m_MarkerCornerPosCam2d[i*4+2][0], pixZoomY * (m_CameraHeight - m_MarkerCornerPosCam2d[i*4+2][1]) );
				glVertex2f ( pixZoomX * m_MarkerCornerPosCam2d[i*4+3][0], pixZoomY * (m_CameraHeight - m_MarkerCornerPosCam2d[i*4+3][1]) );
			glEnd();
		}
	}

	return;
}

void 
ARTagHelper::GetValidFlagArray(int deviceType, const bool *& validArray, int & markerNum) const 
{
	markerNum = m_MarkerNum;
		
	if ( deviceType == CAMERA ) { 
		validArray = m_ValidFlagCam; 
	}
	else if ( deviceType == PROJECTOR ) { 
		validArray = m_ValidFlagPro; 
	}
}