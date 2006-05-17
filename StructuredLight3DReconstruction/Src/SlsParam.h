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
	
	std::string GetARTagConfigFilePath() const { return std::string(m_ARTagConfigFile); }
	std::string GetARTagPosFilePath() const { return std::string(m_ARTagPosFile); }

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

	int m_SubMonitorWidth;
	int m_SubMonitorHeight;

	// Init Window Position
	int m_InitWindowPosX;
	int m_InitWindowPosY;

	// Raster Size
	int m_RasterX;
	int m_RasterY;

	// Wait Time for Projector
	int m_WaitTime;

	// Checker Board
	int m_CheckerMetricSize;
	int m_CheckerNumX;
	int m_CheckerNumY;

	char m_DataDir[128];											// Data Folder
	char m_GrayCodeDir[128];									// GrayCode Folder
	char m_CameraDir[128];
	char m_ProjectorDir[128];
	char m_ARTagDir[128];
	char m_ShapeDir[128];
	char m_IntermediateDir[128];
	
	char m_IntrFile[128];
	char m_DistFile[128];
	char m_RotFile[128];
	char m_TransFile[128];
	char m_ARTagConfigFile[128];
	char m_ARTagPosFile[128];
	char m_ObjFile[128];
	char m_ExtrposFile[128];
	char m_IntrposFile[128];

};

#endif