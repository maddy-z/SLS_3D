#ifndef _SLS_PARAM_H_
#define _SLS_PARAM_H_

#include <string>

// 
// SLS Parameters
// 

class SlsParam
{

public:

	enum CameraType 
	{
		DRAGONFLY_EXPRESS,
		CANON_EOS,
		DRAGONFLY2
	};

	enum
	{
		CAMERA = 0,
		PROJECTOR = 1
	};

	// 
	// Constructor & Destructor
	// 
	
	SlsParam();
	virtual ~SlsParam();

	// 
	// Load Parameters From XML File
	// 

	bool LoadFromXmlFile(const char * filename);
	bool LoadFromXmlFile(const std::string & filename);

	// 
	// Getters
	// 

	int GetCameraWidth() const { return m_CameraWidth; }
	int GetCameraHeight() const { return m_CameraHeight; }
	CameraType GetCameraType() const { return m_CameraType; }

	int GetProjectorWidth() const { return m_ProjectorWidth; }
	int GetProjectorHeight() const { return m_ProjectorHeight; }

	int GetMonitorWidth() const { return m_MonitorWidth; }
	int GetMonitorHeight() const { return m_MonitorHeight; }

	int GetMainWindowWidth() const { return m_MainWindowWidth; }
	int GetMainWindowHeight() const { return m_MainWindowHeight; }
	int GetMainWindowInMainDisplayWidth() const { return m_MainWindowInMainDisplayWidth; }

	int GetRasterPosX() const { return m_RasterX; }
	int GetRasterPosY() const { return m_RasterY; }

	int GetWindowPosX() const { return m_InitWindowPosX; }
	
	const char * GetARTagConfigFilePath() const { return m_ARTagConfigFile; }
	const char * GetARTagPosFilePath() const { return m_ARTagPosFile; }

	const char * GetIntrFilePath ( int device ) const;
	const char * GetDistortionFilePath ( int device ) const;
	const char * GetRotFilePath ( int device ) const;
	const char * GetTransFilePath ( int device ) const;
	const char * GetExtrFilePath ( int device ) const;

	const char * GetGrayCodeDir() const { return m_GrayCodeDir; }

	// std::string GetARTagConfigFilePath() const { return std::string(m_ARTagConfigFile); }
	// std::string GetARTagPosFilePath() const { return std::string(m_ARTagPosFile); }

	// 
	// Print Sls Parameters
	// 

	void printSlsParams() const;

private:

	// Camera Size & Type & Device Num
	int m_CameraWidth;
	int m_CameraHeight;
	int m_CameraDevice;
	CameraType m_CameraType;

	// Projector Size 
	int m_ProjectorWidth;
	int m_ProjectorHeight;
	
	// Main Display Size ( LCD or CRT )
	int m_MainDisplayWidth;

	// Main Window Size
	int m_MainWindowWidth;
	int m_MainWindowHeight;
	int m_MainWindowInMainDisplayWidth;

	// Monitor Size for Captured Image
	int m_MonitorWidth;
	int m_MonitorHeight;

	// int m_SubMonitorWidth;
	// int m_SubMonitorHeight;

	// Init Window Position
	int m_InitWindowPosX;
	int m_InitWindowPosY;

	// Raster Size
	int m_RasterX;
	int m_RasterY;

	// High Resolution Monitor / Raster size for Captured Image
	int m_SubMonitorSize;
	int m_SubRasterX;
	int m_SubRasterY;

	// Wait Time for Projector
	int m_WaitTime;
	
	// ARTag Marker Size
	int m_MarkerSize;

	// Checker Board
	int m_CheckerMetricSize;
	int m_CheckerNumX;
	int m_CheckerNumY;

	// Data Folder
	char m_DataDir[128];						

	// GrayCode Folder
	char m_GrayCodeDir[128];

	// ARTag Folder
	char m_ARTagDir[128];
	char m_ARTagConfigFile[128];
	char m_ARTagPosFile[128];

	// Calculated Shape Folder
	char m_ShapeDir[128];
	char m_ObjFile[128];

	/*
	*	File-Name for Camera parameters ( In OpenCV format )
	*	- m_CameraIntrFile: Intrinsic parameter
	*	- m_CameraDistFile: Distortion parameter
	*	- m_CameraRotFile: Rotation parameter
	*	- m_CameraTransFile: Translation parameter
	*/
	
	char m_CameraDir[128];
	char m_CameraIntrFile[128];
	char m_CameraDistFile[128];
	char m_CameraRotFile[128];
	char m_CameraTransFile[128];
	char m_CameraExtrFile[128];

	/*
	*	File-Name for Projector parameters ( In OpenCV format )
	*	- m_ProIntrFile: Intrinsic parameter
	*	- m_ProDistFile: Distortion parameter
	*	- m_ProRotFile: Rotation parameter
	*	- m_ProTransFile: Translation parameter
	*/

	char m_ProjectorDir[128];
	char m_ProjectorIntrFile[128];
	char m_ProjectorDistFile[128];
	char m_ProjectorRotFile[128];
	char m_ProjectorTransFile[128];
	char m_ProjectorExtrFile[128];
	
	/*
	* File-Name for Projector intermediate data ( In list format )
	*/
	
	char m_IntermediateDir[128];

	char m_ProjectorExtrPosFile[128];
	char m_ProjectorIntrPosFile[128];

	char m_CameraExtrPosFile[128];
	char m_CameraIntrPosFile[128];
	
	/*
	char m_IntermediateDir[128];

	char m_ExtrposFile[128];
	char m_IntrposFile[128];
	*/

};

#endif