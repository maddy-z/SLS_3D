#ifndef _SLS_PARAM_H_
#define _SLS_PARAM_H_

#include <string>

class SlsParam
{

public:

	enum CameraType 
	{
		DRAGONFLY_EXPRESS,
		CANON_EOS
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

	/*
	int GetCameraWidth() const { return m_CameraWidth; }
	int GetCameraHeight() const { return m_CameraHeight; }

	int GetProjectorWidth() const { return m_ProjectorWidth; }
	int GetProjectorHeight() const { return m_ProjectorHeight; }

	int GetMainWindowWidth() const { return m_MainWindowWidth; }
	int GetMainWindowHeight() const { return m_MainWindowHeight; }
	*/

	void printSlsParams() const;

// private:
public:

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