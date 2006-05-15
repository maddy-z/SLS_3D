#ifndef _ARTAG_HELPER_H_
#define _ARTAG_HELPER_H_

//#define		MARKER_CONFIG_FILE			"../../data/marker_cfg_RealPlay.txt"
//#define		MARKER_NUM_MAX				6

// 
// Class -- ARTagHelper
// 

class ARTagHelper

{

private:
	
	// Camera Size
	int m_CameraWidth;
	int m_CameraHeight;

	// LUT for marker ID
	int * m_MarkerID_LUT;

	char m_ConfigFile[128];												// ARTag config file name
	char m_CornerPos[128];												// name of a file which contains 3d position of corners of ARTag markers in world-coordinate

public:

	// number of markers
	int m_MarkerNum;

	double ( * m_MarkerCornerPos3d )[3];						// 3d marker corner position in world-coordinate
	double ( * m_MarkerCornerPosCam2d )[2];					// 2d marker corner position in camera-coordinate
	double ( * m_MarkerCornerPosPro2d)[2];					// 2d marker corner position in projector-coordinate

	// Flag for each marker
	//			- true: marker corner is visible from camera/projector
	//			- false: marker corner is not visible
	//			( for projector, visible means gray code can be obtained at the marker corner )
	
	bool	* m_ValidFlagCam;
	bool	* m_ValidFlagPro;

public:

	// =========================
	// Constructor & Destructor
	// =========================
	
	ARTagHelper( int cameraW, int cameraH, const char * fnConfig, const char * fnCornerPos );
	virtual ~ARTagHelper();

	void FindMarkerCorners ( unsigned char * image );
	// void GetMarkerCornerPos2dInProjector ( CGrayCode * gc );
	void DrawMarkersInCameraImage ( float pixZoomX, float pixZoomY );

};

#endif