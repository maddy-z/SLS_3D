#ifndef _EXTR_CALIBRATOR_H_
#define _EXTR_CALIBRATOR_H_

#include <OpenCV_2.3.1\opencv2\opencv.hpp>

// 
// Class used to Calibrate Extrinsic Parameters
// 

class ExtrCalibrator
{

public:
	
	enum 
	{
		CAMERA, 
		PROJECTOR
	};

	enum
	{
		INTR,  
		DIST,
		ROT,
		TRANS,
		EXTR,
	};

	/*
	*	Constructor & Destructor
	*
	*	- MarkerNum:	Number of printed ARTag markers
	*	- (Others):			File name of Calibration parameters
	*/

	ExtrCalibrator (	int MarkerNum );
	ExtrCalibrator (	int MarkerNum, 
							const char * fnCamIntr, 
							const char * fnCamDist, 
							const char * fnProIntr, 
							const char * fnProDist );
	
	virtual ~ExtrCalibrator();

	/*
	*	Read parameters from external files
	*
	*	- Intrinsic File & Distortion File:		File Name
	*	- Type:											Identifier ( Projector or Camera )
	*/

	// bool ReadIntrParaFile( void );							
	bool ReadIntrParaFile ( const char * camIntrFile, const char * camDistFile, const char * proIntrFile, const char * proDistFile );
	// bool ReadIntrParaFileSub( int type );
	bool ReadIntrParaFileSub ( int type, const char * intrFile, const char * distFile );

	// bool ReadExtrParaFile( void );							
	bool ReadExtrParaFile ( const char * camRotFile, const char * camTransFile, const char * proRotFile, const char * proTransFile );
	// bool ReadExtrParaFileSub( int type );
	bool ReadExtrParaFileSub ( int type, const char * rotFile, const char * transFile );

	/*
	*	Extrinsic Calibration
	*	
	*	 - Type: identifier ( Projector or Camera )
	*	 - markerPos2d: marker position in camera/projector image
	*	 - markerPos3d: 3d position of markers in world-coordinate
	*	 - valid_flag: 
	*		True: each marker is visible from camera/projector
	*		false: is not visible
	*
	*/

	void ExtrCalib ( int type, double (* markerPos2d)[2], double (* markerPos3d)[3], bool * valid_flag );

	/*
	*	Save / Print cv::Mat Matrix
	*/ 

	const cv::Mat & GetMatrix ( int deviceType, int matType );
	static void PrintMatrix ( const cv::Mat & matrix );
	void SaveMatrix ( const cv::Mat & mat, const char * fname);
	void SaveMatrix ( int deviceType, int matType, const char * fname );

private:

	int m_CameraWidth;
	int m_CameraHeight;

	double * m_MarkerPos2d;									// AR Marker Positions in Camera Image (2D)
	int m_MarkerNum;												// Number of AR Markers

	// Camera & Projector Parameter Matries
	cv::Mat m_CamIntr, m_CamDist, m_CamRot, m_CamTrans, m_CamExtr;
	cv::Mat m_ProIntr, m_ProDist, m_ProRot, m_ProTrans, m_ProExtr;

// private:

	// File Name of Parameters
	// char m_FnCamIntr[128];
	// char m_FnCamDist[128];
	// char m_FnCamRot[128];
	// char m_FnCamTrans[128];
	// char m_FnProIntr[128];
	// char m_FnProDist[128];
	// char m_FnProRot[128];
	// char m_FnProTrans[128];

	char m_FnProExtrPos[128];
	char m_FnCamExtrPos[128];

};

#endif
