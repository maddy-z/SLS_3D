#include <cstdio>
#include <cstdlib>
#include <iostream>

#include <OpenCV_2.3.1\opencv2\opencv.hpp>

#include "ExtrCalibrator.h"

ExtrCalibrator::
ExtrCalibrator ( int	MarkerNum )
{
	m_MarkerNum = MarkerNum;

	m_CamIntr.create (3, 3, CV_64F);
	m_CamDist.create (4, 1, CV_64F);
	m_CamRot.create (3, 1, CV_64F);
	m_CamTrans.create (3, 1, CV_64F);
	m_CamExtr.create (3, 4, CV_64F);

	m_ProIntr.create (3, 3, CV_64F);
	m_ProDist.create (4, 1, CV_64F);
	m_ProRot.create (3, 1, CV_64F);
	m_ProTrans.create (3, 1, CV_64F);
	m_ProExtr.create (3, 4, CV_64F);
}

ExtrCalibrator::
ExtrCalibrator ( int	MarkerNum, 
						const char * fnCamIntr, 
						const char * fnCamDist, 
						const char * fnProIntr, 
						const char * fnProDist )
{
	
	m_MarkerNum = MarkerNum;

	m_CamIntr.create (3, 3, CV_64F);
	m_CamDist.create (4, 1, CV_64F);
	m_CamRot.create (3, 1, CV_64F);
	m_CamTrans.create (3, 1, CV_64F);
	m_CamExtr.create (3, 4, CV_64F);
	m_ProIntr.create (3, 3, CV_64F);
	m_ProDist.create (4, 1, CV_64F);
	m_ProRot.create (3, 1, CV_64F);
	m_ProTrans.create (3, 1, CV_64F);
	m_ProExtr.create (3, 4, CV_64F);

	ReadIntrParaFile ( fnCamIntr, fnCamDist, fnProIntr, fnProDist );

}

ExtrCalibrator::
~ExtrCalibrator() 
{}

/*
bool 
ExtrCalibrator::ReadIntrParaFile ( void ) 
{
	return ( ReadIntrParaFileSub ( CAMERA ) ) && ( ReadIntrParaFileSub ( PROJECTOR ) );
}
*/

bool 
ExtrCalibrator::ReadIntrParaFile ( const char * camIntrFile, const char * camDistFile, const char * proIntrFile, const char * proDistFile )
{
	return ( ReadIntrParaFileSub ( CAMERA, camIntrFile, camDistFile ) && ReadIntrParaFileSub ( PROJECTOR, proIntrFile, proDistFile ) );
}

/*
bool 
ExtrCalibrator::ReadIntrParaFileSub ( int type )
{
	// Set Pointers of Camera / Projector Parameter
	// Copy file names

	cv::Mat intrinsic;
	cv::Mat distortion;

	char intrFile[128];
	char distFile[128];

	if ( type == CAMERA ) {
		intrinsic = m_CamIntr;
		distortion = m_CamDist;
		
		sprintf( intrFile, "%s", m_FnCamIntr );
		sprintf( distFile, "%s", m_FnCamDist );
	}
	else {
		intrinsic = m_ProIntr;
		distortion = m_ProDist;
		
		sprintf( intrFile, "%s", m_FnProIntr );
		sprintf( distFile, "%s", m_FnProDist );
	}

	FILE * fp;																// Read parameter
	double para;

	// Intrinsic parameter 
	if ( ( fp = fopen ( intrFile, "r" ) ) == NULL ) { 
		return false; 
	}
	for ( int y = 0; y < 3; ++y )
	for ( int x = 0; x < 3; ++x )
	{
		fscanf ( fp, "%lf", &para );
		intrinsic.at<double>(y, x) = para;
	}
	fclose( fp );

	// Distortion parameter
	if ( ( fp = fopen ( distFile, "r" ) ) == NULL )	{
		return false;
	}
	for ( int x = 0; x < 4; ++x )
	{
		fscanf ( fp, "%lf", &para ); 
		distortion.at<double>(x, 0) = para;
	}
	fclose( fp );

	return true;
}
*/

bool 
ExtrCalibrator::ReadIntrParaFileSub ( int type, const char * intrFile, const char * distFile )
{
	// Set Pointers of Camera / Projector Parameter
	// Copy file names

	cv::Mat intrinsic;
	cv::Mat distortion;

	if ( type == CAMERA ) {
		intrinsic = m_CamIntr;
		distortion = m_CamDist;
	}
	else {
		intrinsic = m_ProIntr;
		distortion = m_ProDist;
	}

	FILE * fp;																// Read parameter
	double para;

	// 
	// Intrinsic parameter 
	// 

	if ( ( fp = fopen ( intrFile, "r" ) ) == NULL ) { 
		return false; 
	}
	for ( int y = 0; y < 3; ++y )
	for ( int x = 0; x < 3; ++x )
	{
		fscanf ( fp, "%lf", &para );
		intrinsic.at<double>(y, x) = para;
	}
	fclose( fp );

	// 
	// Distortion parameter
	// 

	if ( ( fp = fopen ( distFile, "r" ) ) == NULL )	{
		return false;
	}
	for ( int x = 0; x < 4; ++x )
	{
		fscanf ( fp, "%lf", &para ); 
		distortion.at<double>(x, 0) = para;
	}
	fclose( fp );

	return true;
}

/*
bool 
ExtrCalibrator::ReadExtrParaFile ( void ) 
{ 
	return ( ReadExtrParaFileSub ( CAMERA ) ) && ( ReadExtrParaFileSub ( PROJECTOR ) ); 
}
*/

bool 
ExtrCalibrator::ReadExtrParaFile ( const char * camRotFile, const char * camTransFile, const char * proRotFile, const char * proTransFile ) 
{ 
	return ( ReadExtrParaFileSub ( CAMERA, camRotFile, camTransFile ) ) && ( ReadExtrParaFileSub ( PROJECTOR, proRotFile, proTransFile ) ); 
}

/*
bool 
ExtrCalibrator::ReadExtrParaFileSub ( int type )
{
	// Set pointers of Camera / Projector parameters
	// Copy file names
	
	cv::Mat extrinsic;
	cv::Mat translation31;
	cv::Mat rotation31;

	char rot_file[128];
	char trans_file[128];

	if ( type == CAMERA )
	{
		extrinsic = m_CamExtr;
		rotation31 = m_CamRot;
		translation31 = m_CamTrans;
		
		sprintf( rot_file, "%s", m_FnCamRot );
		sprintf( trans_file, "%s", m_FnCamTrans );
	}
	else
	{
		extrinsic = m_ProExtr;
		rotation31 = m_ProRot;
		translation31 = m_ProTrans;
		
		sprintf( rot_file, "%s", m_FnProRot );
		sprintf( trans_file, "%s", m_FnProTrans );
	}

	FILE * fp;															// Read parameters
	double para;
	
	// Rotation
	if ( (fp = fopen( rot_file, "r" )) == NULL )	 { 
		return false;
	}
	for ( int x = 0; x < 3; ++x )
	{		
		fscanf( fp, "%lf", &para ); 
		rotation31.at<double>(x, 0) = para;
	}
	fclose( fp );

	// Rotation Vector to matrix
	cv::Mat rotation33(3, 3, CV_64F);
	cv::Rodrigues( rotation31, rotation33 );

	// Translation
	if ( (fp = fopen( trans_file, "r" )) == NULL ) {
		return false;
	}
	for ( int x = 0; x < 3; ++x )
	{
		fscanf( fp, "%lf", &para ); 	
		translation31.at<double>(x, 0) = para;
	}
	fclose( fp );

	// Extrinsic = Trans | Rot
	for ( int x = 0; x < 3; ++x )
	{
		extrinsic.at<double>(x, 0) = rotation33.at<double>(x, 0);
		extrinsic.at<double>(x, 1) = rotation33.at<double>(x, 1);
		extrinsic.at<double>(x, 2) = rotation33.at<double>(x, 2);

		extrinsic.at<double>(x, 3) = translation31.at<double>(x, 0);
	}

	return true;
}
*/

bool 
ExtrCalibrator::ReadExtrParaFileSub ( int type, const char * rotFile, const char * transFile )
{
	// Set pointers of Camera / Projector parameters
	// Copy file names
	
	cv::Mat extrinsic;
	cv::Mat translation31;
	cv::Mat rotation31;

	if ( type == CAMERA )
	{
		extrinsic = m_CamExtr;
		rotation31 = m_CamRot;
		translation31 = m_CamTrans;
	}
	else
	{
		extrinsic = m_ProExtr;
		rotation31 = m_ProRot;
		translation31 = m_ProTrans;
	}

	FILE * fp;															// Read parameters
	double para;
	
	// Rotation
	if ( (fp = fopen( rotFile, "r" )) == NULL )	 { 
		return false;
	}
	for ( int x = 0; x < 3; ++x )
	{		
		fscanf( fp, "%lf", &para ); 
		rotation31.at<double>(x, 0) = para;
	}
	fclose( fp );

	// Rotation Vector to matrix
	cv::Mat rotation33(3, 3, CV_64F);
	cv::Rodrigues( rotation31, rotation33 );

	// Translation
	if ( (fp = fopen( transFile, "r" )) == NULL ) {
		return false;
	}
	for ( int x = 0; x < 3; ++x )
	{
		fscanf( fp, "%lf", &para ); 	
		translation31.at<double>(x, 0) = para;
	}
	fclose( fp );

	// Extrinsic = Translation | Rotation
	for ( int x = 0; x < 3; ++x )
	{
		extrinsic.at<double>(x, 0) = rotation33.at<double>(x, 0);
		extrinsic.at<double>(x, 1) = rotation33.at<double>(x, 1);
		extrinsic.at<double>(x, 2) = rotation33.at<double>(x, 2);

		extrinsic.at<double>(x, 3) = translation31.at<double>(x, 0);
	}

	return true;
}

void 
ExtrCalibrator::ExtrCalib ( int type, double (* markerPos2d)[2], double (* markerPos3d)[3], bool * valid_flag )
{
	// 
	// Set Pointers of Camera / Projector Parameters
	// Copy File Name
	// 

	cv::Mat intrinsic;
	cv::Mat distortion;
	cv::Mat rotation;
	cv::Mat translation;
	cv::Mat extrinsic;

	char dst_file_rot[128];
	char dst_file_trans[128];

	if ( type == CAMERA )
	{
		intrinsic = m_CamIntr;
		distortion = m_CamDist;
		rotation = m_CamRot;
		translation = m_CamTrans;
		extrinsic = m_CamExtr;
	}
	else {
		intrinsic = m_ProIntr;
		distortion = m_ProDist;
		rotation = m_ProRot;
		translation = m_ProTrans;
		extrinsic = m_ProExtr;
	}

	// 
	// Copy 2D / 3D position data of markers
	// 

	int ValidMarkerNum = 0;
	for ( int i = 0; i < m_MarkerNum; ++i ) {
		if ( valid_flag[i] ) { ++ValidMarkerNum; }
	}

	if ( ValidMarkerNum == 0 ) {
		std::cout << "Valid Marker Number = " << ValidMarkerNum << std::endl;
		return;
	}

	cv::Mat pos3d ( ValidMarkerNum * 4, 3, CV_64FC1 );
	cv::Mat pos2d ( ValidMarkerNum * 4, 2, CV_64FC1 );

	int j = 0;
	
	for ( int i = 0; i < m_MarkerNum; ++i )
	{
		if ( valid_flag[i] )
		{
			for ( int k = 0; k < 4; ++k, ++j )
			{
				pos3d.at<double>(j, 0) = markerPos3d[i*4+k][0];
				pos3d.at<double>(j, 1) = markerPos3d[i*4+k][1];
				pos3d.at<double>(j, 2) = markerPos3d[i*4+k][2];
				
				pos2d.at<double>(j, 0) = markerPos2d[i*4+k][0];
				pos2d.at<double>(j, 1) = markerPos2d[i*4+k][1];
			}
		}
	}

	// Extrinsic Parameter Calibration using an OpenCV Function
	cv::solvePnP(pos3d, pos2d, intrinsic, distortion, rotation, translation);

	// 
	// Extrinsic = Rotation | Translation
	// 

	cv::Mat rotation33(3, 3, CV_64FC1);
	cv::Rodrigues( rotation, rotation33 );
	
	for ( int x = 0; x < 3; ++x )
	{
		extrinsic.at<double>(x, 0) = rotation33.at<double>(x, 0);
		extrinsic.at<double>(x, 1) = rotation33.at<double>(x, 1);
		extrinsic.at<double>(x, 2) = rotation33.at<double>(x, 2);

		extrinsic.at<double>(x, 3) = translation.at<double>(x, 0);
	}

	// SaveMatrix ( rotation, dst_file_rot );
	// SaveMatrix ( translation, dst_file_trans );
	// ReadExtrParaFileSub ( type );

	return;
}

const cv::Mat & 
ExtrCalibrator::GetMatrix(int deviceType, int matType)
{
	switch (deviceType)
	{
	case CAMERA:

		switch (matType) 
		{
		case INTR:		return m_CamIntr;
		case DIST:		return m_CamDist;
		case EXTR:		return m_CamExtr;
		}
			
		break;

	case PROJECTOR:

		switch (matType) 
		{
		case INTR:		return m_ProIntr;
		case DIST:		return m_ProDist;
		case EXTR:		return m_ProExtr;
		}
			
		break;

	default:
		return m_CamIntr;
	}

	return m_CamIntr;
}

void 
ExtrCalibrator::PrintMatrix ( const cv::Mat & matrix )
{
	for ( int y = 0; y < matrix.size().height; ++y ) 
	{
		for ( int x = 0; x < matrix.size().width; ++x ) {
			printf ( "%.6f\t", matrix.at<double>(y, x) ); 
		}
		printf ( "\n" );
	}

	printf ( "\n" );
	return;
}

void 
ExtrCalibrator::SaveMatrix ( const cv::Mat & mat, const char * fname)
{
	FILE * fp = fopen ( fname, "w" );
	if ( fp == NULL ) { return; }

	for ( int y = 0; y < mat.rows; ++y ) 
	{
		for ( int x = 0; x < mat.cols; ++x ) {
			fprintf ( fp, "%f\t", mat.at<double>(y, x) ); 
		}
		fprintf ( fp, "\n" );
	}

	fclose ( fp );
}

void 
ExtrCalibrator::SaveMatrix ( int deviceType, int matType, const char * fname )
{
	SaveMatrix(GetMatrix(deviceType, matType), fname);
}
