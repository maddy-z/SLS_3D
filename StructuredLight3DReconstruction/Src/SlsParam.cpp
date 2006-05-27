#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <TinyXML\tinystr.h>
#include <TinyXML\tinyxml.h>

#include "SlsParam.h"

SlsParam::SlsParam()
{
	m_CameraWidth = 0;
	m_CameraHeight = 0;
	m_CameraDevice = 0;
	m_CameraType = SlsParam::DRAGONFLY_EXPRESS;

	m_ProjectorWidth = 0;
	m_ProjectorHeight = 0;

	m_MainWindowWidth = 0;
	m_MainWindowHeight = 0;

	m_MainWindowInMainDisplayWidth = 0;

	m_MonitorWidth = 0;
	m_MonitorHeight = 0;

	/*
	m_SubMonitorWidth = 0;
	m_SubMonitorHeight = 0;
	*/

	m_InitWindowPosX = 0;
	m_InitWindowPosY = 0;

	m_RasterX = 0;
	m_RasterY = 0;

	m_SubRasterX = 0;
	m_SubRasterY = 0;

	m_MarkerSize = 0;

	m_WaitTime = 0;

	m_CheckerMetricSize = 0;
	m_CheckerNumX = 0;
	m_CheckerNumY = 0;
}
SlsParam::~SlsParam()
{
}

bool SlsParam::LoadFromXmlFile(const char * filename)
{
	if (filename == NULL) {
		return false;
	}

	TiXmlDocument doc(filename);

	bool IsLoadOK = doc.LoadFile();
	if (!IsLoadOK) 
	{
		printf("Invalid SLS Config XML File: %s\n", filename);
		return false;
	}

	TiXmlElement * rootElement = doc.RootElement();
	if (strcmp(rootElement->Value(), "SLSConfiguration") != 0) 
	{
		printf("Invalid Root Element: %s\n", rootElement->Value());
		return false;
	}

	TiXmlNode * childNode = NULL;
	while (childNode = rootElement->IterateChildren(childNode)) 
	{
		if (strcmp(childNode->Value(), "Param") == 0) 
		{
			// printf ("Child Node: Param\n");

			TiXmlNode * subChildNode = NULL;
			while (subChildNode = childNode->IterateChildren(subChildNode)) 
			{
				const char * strBuf = NULL;

				if (strcmp(subChildNode->Value(), "Camera") == 0) 
				{
					subChildNode->ToElement()->QueryIntAttribute("width", &m_CameraWidth);
					subChildNode->ToElement()->QueryIntAttribute("height", &m_CameraHeight);
					subChildNode->ToElement()->QueryIntAttribute("device", &m_CameraDevice);

					strBuf = subChildNode->ToElement()->Attribute("type");
					if (strcmp(strBuf, "DragonFly Express") == 0) 
					{
						m_CameraType = SlsParam::DRAGONFLY_EXPRESS;
					}
				}
				if (strcmp(subChildNode->Value(), "Projector") == 0) {
					subChildNode->ToElement()->QueryIntAttribute("width", &m_ProjectorWidth);
					subChildNode->ToElement()->QueryIntAttribute("height", &m_ProjectorHeight);
				}
				if (strcmp(subChildNode->Value(), "MainDisplay") == 0) {
					subChildNode->ToElement()->QueryIntAttribute("width", &m_MainDisplayWidth);
				}
				if (strcmp(subChildNode->Value(), "MainWindowInMainDisplayWidth") == 0) {
					subChildNode->ToElement()->QueryIntAttribute("width", &m_MainWindowInMainDisplayWidth);
				}
				if (strcmp(subChildNode->Value(), "Monitor") == 0) {
					subChildNode->ToElement()->QueryIntAttribute("width", &m_MonitorWidth);
				}
				if (strcmp(subChildNode->Value(), "SubMonitor") == 0) {
					subChildNode->ToElement()->QueryIntAttribute("width", &m_SubMonitorSize);
				}
				if (strcmp(subChildNode->Value(), "InitWindowPos") == 0) {
					subChildNode->ToElement()->QueryIntAttribute("y", &m_InitWindowPosY);
				}
				if (strcmp(subChildNode->Value(), "WaitTime") == 0) {
					subChildNode->ToElement()->QueryIntAttribute("time", &m_WaitTime);
				}
				if (strcmp(subChildNode->Value(), "Checker") == 0) {
					subChildNode->ToElement()->QueryIntAttribute("size", &m_CheckerMetricSize);
					subChildNode->ToElement()->QueryIntAttribute("x", &m_CheckerNumX);
					subChildNode->ToElement()->QueryIntAttribute("y", &m_CheckerNumY);
				}
			}
		}

		if (strcmp(childNode->Value(), "DirName") == 0) 
		{
			// printf ("Child Node: DirName\n");

			TiXmlNode * subChildNode = NULL;
			while (subChildNode = childNode->IterateChildren(subChildNode))
			{
				const char * strBuf = NULL;

				if (strcmp(subChildNode->Value(), "Data") == 0) { 
					strBuf = subChildNode->ToElement()->Attribute("name");
					sprintf(m_DataDir, "%s", strBuf);
				}
				if (strcmp(subChildNode->Value(), "GrayCode") == 0) {
					strBuf = subChildNode->ToElement()->Attribute("name");
					sprintf(m_GrayCodeDir, "%s/%s", m_DataDir, strBuf);
				}
				if (strcmp(subChildNode->Value(), "Camera") == 0) {
					strBuf = subChildNode->ToElement()->Attribute("name");
					sprintf(m_CameraDir, "%s/%s", m_DataDir, strBuf);
				}
				if (strcmp(subChildNode->Value(), "Projector") == 0) {
					strBuf = subChildNode->ToElement()->Attribute("name");
					sprintf(m_ProjectorDir, "%s/%s", m_DataDir, strBuf);
				}
				if (strcmp(subChildNode->Value(), "ARTag") == 0) {
					strBuf = subChildNode->ToElement()->Attribute("name");
					sprintf(m_ARTagDir, "%s/%s", m_DataDir, strBuf);
				}
				if (strcmp(subChildNode->Value(), "Shape") == 0) {
					strBuf = subChildNode->ToElement()->Attribute("name");
					sprintf(m_ShapeDir, "%s/%s", m_DataDir, strBuf);
				}
				if (strcmp(subChildNode->Value(), "Intermediate") == 0) {
					strBuf = subChildNode->ToElement()->Attribute("name");
					sprintf(m_IntermediateDir, "%s/%s", m_DataDir, strBuf);
				}
			}
		}

		if (strcmp(childNode->Value(), "FileName") == 0) 
		{
			// printf ("Child Node: FileName\n");
			
			TiXmlNode * subChildNode = NULL;
			while (subChildNode = childNode->IterateChildren(subChildNode))
			{
				const char * strBuf = NULL;

				if (strcmp(subChildNode->Value(), "Intr") == 0) { 
					strBuf = subChildNode->ToElement()->Attribute("name");
					sprintf(m_CameraIntrFile, "%s/%s", m_CameraDir, strBuf);
					sprintf(m_ProjectorIntrFile, "%s/%s", m_ProjectorDir, strBuf);
				}
				if (strcmp(subChildNode->Value(), "Dist") == 0) { 
					strBuf = subChildNode->ToElement()->Attribute("name");
					sprintf(m_CameraDistFile, "%s/%s", m_CameraDir, strBuf);
					sprintf(m_ProjectorDistFile, "%s/%s", m_ProjectorDir, strBuf);
				}
				if (strcmp(subChildNode->Value(), "Rot") == 0) { 
					strBuf = subChildNode->ToElement()->Attribute("name");
					sprintf(m_CameraRotFile, "%s/%s", m_CameraDir, strBuf);
					sprintf(m_ProjectorRotFile, "%s/%s", m_ProjectorDir, strBuf);
				}
				if (strcmp(subChildNode->Value(), "Trans") == 0) { 
					strBuf = subChildNode->ToElement()->Attribute("name");
					sprintf(m_CameraTransFile, "%s/%s", m_CameraDir, strBuf);
					sprintf(m_ProjectorTransFile, "%s/%s", m_ProjectorDir, strBuf);
				}
				if (strcmp(subChildNode->Value(), "Extr") == 0) { 
					strBuf = subChildNode->ToElement()->Attribute("name");
					sprintf(m_CameraExtrFile, "%s/%s", m_CameraDir, strBuf);
					sprintf(m_ProjectorExtrFile, "%s/%s", m_ProjectorDir, strBuf);
				}
				if (strcmp(subChildNode->Value(), "ARTag") == 0) { 
					strBuf = subChildNode->ToElement()->Attribute("pos");
					sprintf(m_ARTagPosFile, "%s/%s", m_ARTagDir, strBuf);
					strBuf = subChildNode->ToElement()->Attribute("config");
					sprintf(m_ARTagConfigFile, "%s/%s", m_ARTagDir, strBuf);
				}
				if (strcmp(subChildNode->Value(), "Obj") == 0) { 
					strBuf = subChildNode->ToElement()->Attribute("name");
					sprintf(m_ObjFile, "%s/%s", m_ShapeDir, strBuf);
				}
				if (strcmp(subChildNode->Value(), "Extrpos") == 0) { 
					strBuf = subChildNode->ToElement()->Attribute("name");
					sprintf(m_CameraExtrPosFile, "%s/%s", m_CameraDir, strBuf);
					sprintf(m_ProjectorExtrPosFile, "%s/%s", m_ProjectorDir, strBuf);
				}
				if (strcmp(subChildNode->Value(), "Intrpos") == 0) { 
					strBuf = subChildNode->ToElement()->Attribute("name");
					sprintf(m_CameraIntrPosFile, "%s/%s", m_CameraDir, strBuf);
					sprintf(m_ProjectorIntrPosFile, "%s/%s", m_ProjectorDir, strBuf);
				}
			}
		}
	}

	m_MonitorHeight = (float)(m_MonitorWidth) / (float)(m_CameraWidth) * (float)(m_CameraHeight);
	m_MainWindowWidth = m_MainWindowInMainDisplayWidth + m_ProjectorWidth;
	m_MainWindowHeight = m_ProjectorHeight;
	m_InitWindowPosX = m_MainDisplayWidth - m_MainWindowInMainDisplayWidth;
	
	m_RasterX	= m_MainWindowInMainDisplayWidth - m_MonitorWidth - 10;
	m_RasterY	= m_MainWindowHeight - 10;

	return true;
}
bool SlsParam::LoadFromXmlFile(const std::string & filename)
{
	return LoadFromXmlFile(filename.c_str());
}

const char * SlsParam::GetIntrFilePath(int device) const
{
	if ( device == CAMERA ) { return m_CameraIntrFile; }
	else if ( device == PROJECTOR ) { return m_ProjectorIntrFile; }
	else { return NULL; }
}
const char * SlsParam::GetDistortionFilePath(int device) const
{
	if ( device == CAMERA ) { return m_CameraDistFile; }
	else if ( device == PROJECTOR ) { return m_ProjectorDistFile; }
	else { return NULL; }
}
const char * SlsParam::GetRotFilePath(int device) const
{
	if ( device == CAMERA ) { return m_CameraRotFile; }
	else if ( device == PROJECTOR ) { return m_ProjectorRotFile; }
	else { return NULL; }
}
const char * SlsParam::GetTransFilePath(int device) const
{
	if ( device == CAMERA ) { return m_CameraTransFile; }
	else if ( device == PROJECTOR ) { return m_ProjectorTransFile; }
	else { return NULL; }
}
const char * SlsParam::GetExtrFilePath(int device) const
{
	if ( device == CAMERA ) { return m_CameraExtrFile; }
	else if ( device == PROJECTOR ) { return m_ProjectorExtrFile; }
	else { return NULL; }
}

void SlsParam::printSlsParams() const 
{
	printf("------------------------------------------\n");
	printf("Sls Config Parameters:\n\n");

	printf("***** Params *****\n");

	printf("Camera <w, h> = <%d, %d>\n", m_CameraWidth, m_CameraHeight);
	printf("Camera Device Num = %d\n", m_CameraDevice);
	printf("Projector <w, h> = <%d, %d>\n", m_ProjectorWidth, m_ProjectorHeight);
	printf("Main Display Width = %d\n", m_MainDisplayWidth);
	printf("MainWindow <w, h> = <%d, %d>\n", m_MainWindowWidth, m_MainWindowHeight);
	printf("MainWindowInMainDisplayWidth = %d\n", m_MainWindowInMainDisplayWidth);
	printf("Monitor <w, h> = <%d, %d>\n", m_MonitorWidth, m_MonitorHeight);
	printf("InitWindowPos <x, y> = <%d, %d>\n", m_InitWindowPosX, m_InitWindowPosY);
	printf("Raster Pos <x, y> = <%d, %d>\n", m_RasterX, m_RasterY);
	printf("SubMonitor Size = %d\n", m_SubMonitorSize);
	printf("SubRaster Pos <x, y> = <%d, %d>\n", m_SubRasterX, m_SubRasterY);
	printf("WaitTime = %d\n", m_WaitTime);
	printf("Marker Size = %d\n", m_MarkerSize);
	printf("Checker Info <Size, NumX, NumY> = <%d, %d, %d>\n", m_CheckerMetricSize, m_CheckerNumX, m_CheckerNumY);
	
	printf("\n");

	printf("***** Directories && Files *****\n");

	printf("Data Dir = %s\n", m_DataDir);
	printf("GrayCode Dir = %s\n", m_GrayCodeDir);

	printf("Camera Dir = %s\n", m_CameraDir);
	printf("\tCamera Intr File = %s\n", m_CameraIntrFile);
	printf("\tCamera Dist File = %s\n", m_CameraDistFile);
	printf("\tCamera Rot File = %s\n", m_CameraRotFile);
	printf("\tCamera Trans File = %s\n", m_CameraTransFile);

	printf("Projector Dir = %s\n", m_ProjectorDir);
	printf("\tProjector Intr File = %s\n", m_ProjectorIntrFile);
	printf("\tProjector Dist File = %s\n", m_ProjectorDistFile);
	printf("\tProjector Rot File = %s\n", m_ProjectorRotFile);
	printf("\tProjector Trans File = %s\n", m_ProjectorTransFile);

	printf("ARTag Dir = %s\n", m_ARTagDir);
	printf("\tARTag Config File = %s\n", m_ARTagConfigFile);
	printf("\tARTag Position File = %s\n", m_ARTagPosFile);

	printf("Shape Dir = %s\n", m_ShapeDir);
	printf("\tObj File = %s\n", m_ObjFile);

	printf("Intermediate Dir = %s\n", m_IntermediateDir);
	printf("\tCamera Extrpos File = %s\n", m_CameraExtrPosFile);
	printf("\tCamera Intrpos File = %s\n", m_CameraIntrPosFile);
	printf("\tProjector Extrpos File = %s\n", m_ProjectorExtrPosFile);
	printf("\tProjector Intrpos File = %s\n", m_ProjectorIntrPosFile);
	
	printf("\n");

	printf("------------------------------------------\n");
}