#include <cstdio>
#include <cstdlib>
#include <cassert>

#include <OpenCV_2.3.1\opencv2\opencv.hpp>

#include <OpenGL\Glui\glui.h>
#include <OpenGL\Glut\glut.h>

#include "SlsMainWnd.h"
#include "SlsParam.h"
#include "PointGreyCamera.h"

// 
// MACROS DEFINITIONS
// 

#define						SLS_CONFIG_XMLFILEPATH						"SlsConfig.xml"
#define						SLS_CAPTUREDIMAGE_WINNAME				"Captured Image"

// 
// Global Variables
// 

int								glutMainWndHandler		= -1;

SlsMainWnd				* gluiMainWndHandler	= NULL;
SlsParam					* slsParameters				= NULL;
PointGreyCamera		* pgCamera					= NULL;

unsigned char			* capturedRawImage		= NULL;
unsigned char			* processedImage			= NULL;

// ================
//  Global Utility Functions
// ================

void SphereDisplay(void)
{
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glutSwapBuffers();
}

// 
// Experiment Functions
// 

void ShowImageInOpenCvWindow(unsigned char * img, int camH, int camW)
{
	cv::Mat destImg(cv::Size(camW, camH), CV_8UC3);

	unsigned char * srcData = img;
	unsigned char * destImgRow = destImg.data;
	
	for (int i = 0; i < destImg.rows; ++i, destImgRow += destImg.step) 
	{
		unsigned char * destImgData = destImgRow;
		
		for (int j = 0; j < destImg.cols; ++j, destImgData += destImg.channels(), srcData += 4) 
		{
			destImgData[0] = srcData[2];
			destImgData[1] = srcData[1];
			destImgData[2] = srcData[0];
		}
	}
		
	cv::namedWindow(SLS_CAPTUREDIMAGE_WINNAME, CV_WINDOW_AUTOSIZE);
	cv::imshow(SLS_CAPTUREDIMAGE_WINNAME, destImg);

	return;
}

bool CaptureImage(unsigned char * img)
{
	int camH = slsParameters->GetCameraHeight();
	int camW = slsParameters->GetCameraWidth();

	if (slsParameters->GetCameraType() == SlsParam::DRAGONFLY_EXPRESS) 
	{
		pgCamera->Capture(capturedRawImage);
		// ShowImageInOpenCvWindow(capturedRawImage, camH, camW);
		
		const unsigned char * srcData = capturedRawImage;
		unsigned char * destData = processedImage;
		for (int i = 0; i < camH * camW; ++i, srcData += 4, destData += 3)
		{
			destData[0] = srcData[2];
			destData[1] = srcData[1];
			destData[2] = srcData[0];
		}

		return true;
	}

	return false;
}

static void Display(void)
{
	assert (gluiMainWndHandler != NULL);

	int monitorW = slsParameters->GetMonitorWidth();
	int monitorH = slsParameters->GetMonitorHeight();

	int cameraW = slsParameters->GetCameraWidth();
	int cameraH = slsParameters->GetCameraHeight();

	int mainWndH = slsParameters->GetMainWindowHeight();
	int mainWndW = slsParameters->GetMainWindowWidth();

	int rasterX = slsParameters->GetRasterPosX();
	int rasterY = slsParameters->GetRasterPoxY();

	float pixelZoomX = (float)(monitorW)/(float)(cameraW);
	float pixelZoomY = (float)(monitorH)/(float)(cameraH);

	SlsMainWnd::SLS_MODE mode = gluiMainWndHandler->getSlsMode();
	bool flag = false;

	switch ( mode )
	{
	case SlsMainWnd::SLS_IDLE:
		break;

	case SlsMainWnd::SLS_CAPTURE_IMAGE:
		flag = CaptureImage(processedImage);
		gluiMainWndHandler->setSlsMode(SlsMainWnd::SLS_IDLE);
		
		break;
	
	case SlsMainWnd::SLS_DETECT_ARTAG:

		break;
	}

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	 glViewport(0, 0, mainWndW, mainWndH);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, mainWndW, 0, mainWndH, -1.0, 1.0);

	glMatrixMode(GL_MODELVIEW);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// 
	// Draw Captured Image
	// 

	glLoadIdentity();
	glRasterPos2d(rasterX, rasterY);
	glPixelZoom(pixelZoomX, -pixelZoomY);
	glDrawPixels(cameraW, cameraH, GL_RGB, GL_UNSIGNED_BYTE, processedImage);

	// 
	// Projecting White Light
	// 

	// glLoadIdentity();
	// glRasterPos2d();

	glutSwapBuffers();

	return;
}

// 
// Main Function
// 

int main(int argc, char ** argv)
{
	// ===============
	//	  Load SLS Params
	// ===============

	slsParameters = new SlsParam();
	slsParameters -> LoadFromXmlFile(SLS_CONFIG_XMLFILEPATH);
	slsParameters -> printSlsParams();

	pgCamera = new PointGreyCamera(0, slsParameters->GetCameraWidth(), slsParameters->GetCameraHeight(), 60.0f);
	pgCamera->Init();
	pgCamera->Start();

	capturedRawImage = new unsigned char[slsParameters->GetCameraHeight() * slsParameters->GetCameraWidth() * 4];
	processedImage	 = new unsigned char[slsParameters->GetCameraHeight() * slsParameters->GetCameraWidth() * 3];

	// =============
	//   Glut Code
	// =============

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowSize(slsParameters->GetMainWindowWidth(), 
								 slsParameters->GetMainWindowHeight());
	glutInitWindowPosition(slsParameters->GetWindowPosX(), 0);

	glutMainWndHandler = glutCreateWindow("Structured Light 3D Reconstruction System");
	gluiMainWndHandler = SlsMainWnd::Instance(glutMainWndHandler);

	glutDisplayFunc(Display);
	// glutPositionWindow(slsParameters->GetWindowPosX(), 0);

	// 
	// Start to Glut Loop
	// 

	glutMainLoop();

	delete [] capturedRawImage;
	delete [] processedImage;

	exit(EXIT_SUCCESS);

}