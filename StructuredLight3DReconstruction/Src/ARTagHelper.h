#ifndef _ARTAG_HELPER_H_
#define _ARTAG_HELPER_H_

//#define		MARKER_CONFIG_FILE			"../../data/marker_cfg_RealPlay.txt"
//#define		MARKER_NUM_MAX				6

#include	<OpenCV_2.3.1\opencv2\opencv.hpp>

//
// Forward Declaration
// 

class GrayCode;
class ExtrCalibrator;

// 
// Class -- ARTagHelper
// 

class ARTagHelper
{
	friend void Display(void);

private:
	
	int m_CameraWidth;														// Camera Size
	int m_CameraHeight;

	int * m_MarkerID_LUT;													// LUT for marker ID
	int m_MarkerNum;															// Number of markers

	// char m_ConfigFile[128];												// ARTag config file name
	// char m_CornerPos[128];												// Name of a file which contains 3d position of corners of ARTag markers in world-coordinate

	double ( * m_MarkerCornerPos3d )[3];							// 3d marker corner position in world-coordinate
	double ( * m_MarkerCornerPosCam2d )[2];						// 2d marker corner position in camera-coordinate
	double ( * m_MarkerCornerPosPro2d )[2];						// 2d marker corner position in projector-coordinate

	// Flag for each marker
	// 
	//	- True: Marker corner is visible from Camera / Projector
	//	- False: Marker corner is not visible
	//	( For projector, visible means gray code can be obtained at the marker corner )
	//

	bool	* m_ValidFlagCam;
	bool	* m_ValidFlagPro;

	cv::Mat m_Reconstructed3dPts;

public:

	enum 
	{
		CAMERA,
		PROJECTOR
	};

	enum
	{
		MARKER_IN_CAMERA,
		MARKER_IN_PROJECTOR,
		MARKER_IN_REALWORLD,
	};

	// =========================
	// Constructor & Destructor
	// =========================
	
	ARTagHelper ( int cameraW, int cameraH, const char * fnConfig, const char * fnCornerPos );
	virtual ~ARTagHelper();

	void FindMarkerCorners ( unsigned char * image );
	void GetMarkerCornerPos2dInProjector ( GrayCode * gc );
	void DrawMarkersInCameraImage ( float pixZoomX, float pixZoomY );

	void PrintMarkerCornersPos2dInCam () const;
	void PrintMarkerCornersPos2dInProjector () const;
	void PrintMarkerCornersPos3d () const;
	void PrintRecalcMarkerCornersPos3d() const;

	void SaveMarkerCornersPos ( int type, const char * fileName ) const; 

	void RecalcMarkerCornersPos3d (	const cv::Mat & camIntr, 
														const cv::Mat & camExtr, 
														const cv::Mat & proIntr, 
														const cv::Mat & proExtr );

	// 
	// Getters
	// 

	int GetMarkerNumber() const { return m_MarkerNum; }
	void GetValidFlagArray(int deviceType, const bool *& validArray, int & markerNum) const;
};

#endif