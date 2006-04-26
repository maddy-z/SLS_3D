#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <TinyXML\tinyxml.h>
#include <TinyXML\tinystr.h>

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

	m_SubMonitorWidth = 0;
	m_SubMonitorHeight = 0;

	m_InitWindowPosX = 0;
	m_InitWindowPosY = 0;

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
					subChildNode->ToElement()->QueryIntAttribute("width", &m_SubMonitorWidth);
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
					sprintf(m_GrayCodeDir, "%s", strBuf);
				}
				if (strcmp(subChildNode->Value(), "Camera") == 0) {
					strBuf = subChildNode->ToElement()->Attribute("name");
					sprintf(m_CameraDir, "%s", strBuf);
				}
				if (strcmp(subChildNode->Value(), "Projector") == 0) {
					strBuf = subChildNode->ToElement()->Attribute("name");
					sprintf(m_ProjectorDir, "%s", strBuf);
				}
				if (strcmp(subChildNode->Value(), "ARTag") == 0) {
					strBuf = subChildNode->ToElement()->Attribute("name");
					sprintf(m_ARTagDir, "%s", strBuf);
				}
				if (strcmp(subChildNode->Value(), "Shape") == 0) {
					strBuf = subChildNode->ToElement()->Attribute("name");
					sprintf(m_ShapeDir, "%s", strBuf);
				}
				if (strcmp(subChildNode->Value(), "Intermediate") == 0) {
					strBuf = subChildNode->ToElement()->Attribute("name");
					sprintf(m_IntermediateDir, "%s", strBuf);
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
					sprintf(m_IntrFile, "%s", strBuf);
				}
				if (strcmp(subChildNode->Value(), "Dist") == 0) { 
					strBuf = subChildNode->ToElement()->Attribute("name");
					sprintf(m_DistFile, "%s", strBuf);
				}
				if (strcmp(subChildNode->Value(), "Rot") == 0) { 
					strBuf = subChildNode->ToElement()->Attribute("name");
					sprintf(m_RotFile, "%s", strBuf);
				}
				if (strcmp(subChildNode->Value(), "Trans") == 0) { 
					strBuf = subChildNode->ToElement()->Attribute("name");
					sprintf(m_TransFile, "%s", strBuf);
				}
				if (strcmp(subChildNode->Value(), "ARTag") == 0) { 
					strBuf = subChildNode->ToElement()->Attribute("pos");
					sprintf(m_ARTagPosFile, "%s", strBuf);
					strBuf = subChildNode->ToElement()->Attribute("config");
					sprintf(m_ARTagConfigFile, "%s", strBuf);
				}
				if (strcmp(subChildNode->Value(), "Obj") == 0) { 
					strBuf = subChildNode->ToElement()->Attribute("name");
					sprintf(m_ObjFile, "%s", strBuf);
				}
				if (strcmp(subChildNode->Value(), "Extrpos") == 0) { 
					strBuf = subChildNode->ToElement()->Attribute("name");
					sprintf(m_ExtrposFile, "%s", strBuf);
				}
				if (strcmp(subChildNode->Value(), "Intrpos") == 0) { 
					strBuf = subChildNode->ToElement()->Attribute("name");
					sprintf(m_IntrposFile, "%s", strBuf);
				}
			}
		}
	}

	m_MonitorHeight = (float)(m_MonitorWidth) / (float)(m_CameraWidth) * (float)(m_CameraHeight);
	m_MainWindowWidth = m_MainWindowInMainDisplayWidth + m_ProjectorWidth;
	m_MainWindowHeight = m_ProjectorHeight;
	m_InitWindowPosX = m_MainDisplayWidth - m_MainWindowInMainDisplayWidth;
	
	return true;
}

bool SlsParam::LoadFromXmlFile(const std::string & filename)
{
	return LoadFromXmlFile(filename.c_str());
}

void SlsParam::printSlsParams() const 
{
	printf("------------------------------------------\n");
	printf("Sls Config Parameters:\n\n");

	printf("***** Params *****\n");

	printf("Camera <w, h> = <%d, %d>\n", m_CameraWidth, m_CameraHeight);
	printf("Camera Device Num = %d\n", m_CameraDevice);
	// CameraType m_CameraType;
	printf("Projector <w, h> = <%d, %d>\n", m_ProjectorWidth, m_ProjectorHeight);
	printf("MainWindow <w, h> = <%d, %d>\n", m_MainWindowWidth, m_MainWindowHeight);
	printf("MainWindowInMainDisplayWidth = %d\n", m_MainWindowInMainDisplayWidth);
	// printf("\n");
	printf("Monitor <w, h> = <%d, %d>\n", m_MonitorWidth, m_MonitorHeight);
	printf("SubMonitor <w, h> = <%d, %d>\n", m_SubMonitorWidth, m_SubMonitorHeight);
	printf("InitWindowPos <x, y> = <%d, %d>\n", m_InitWindowPosX, m_InitWindowPosY);
	printf("WaitTime = %d\n", m_WaitTime);
	printf("Checker Info <Size, NumX, NumY> = <%d, %d, %d>\n", m_CheckerMetricSize, m_CheckerNumX, m_CheckerNumY);
	
	printf("\n");

	printf("***** Dirs *****\n");

	printf("Data Dir = %s\n", m_DataDir);
	printf("GrayCode Dir = %s\n", m_GrayCodeDir);
	printf("Camera Dir = %s\n", m_CameraDir);
	printf("Projector Dir = %s\n", m_ProjectorDir);
	printf("ARTag Dir = %s\n", m_ARTagDir);
	printf("Shape Dir = %s\n", m_ShapeDir);
	printf("Intermediate Dir = %s\n", m_IntermediateDir);
	
	printf("\n");

	printf("***** Files *****\n");

	printf("Intr File = %s\n", m_IntrFile);
	printf("Dist File = %s\n", m_DistFile);
	printf("Rot File = %s\n", m_RotFile);
	printf("Trans File = %s\n", m_TransFile);
	printf("ARTag Config File = %s\n", m_ARTagConfigFile);
	printf("ARTag Position File = %s\n", m_ARTagPosFile);
	printf("Obj File = %s\n", m_ObjFile);
	printf("Extrpos File = %s\n", m_ExtrposFile);
	printf("Intrpos File = %s\n", m_IntrposFile);
	
	printf("\n");

	printf("------------------------------------------\n");
}