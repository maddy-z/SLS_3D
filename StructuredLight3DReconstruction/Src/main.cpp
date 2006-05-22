#include	<cstdio>
#include	<cstdlib>
#include	<cassert>

#include	<OpenCV_2.3.1\opencv2\opencv.hpp>

#include	<OpenGL\Glui\glui.h>
#include	<OpenGL\Glut\glut.h>

#include	"SlsMainWnd.h"
#include	"SlsParam.h"
#include	"SlsUtils.h"
#include	"PointGreyCamera.h"
#include	"ARTagHelper.h"
#include	"ExtrCalibrator.h"

// =======================
//	 MACROS DEFINITIONS
// =======================

#define						SLS_CONFIG_XMLFILEPATH						"SlsConfig.xml"
#define						SLS_CAPTUREDIMAGE_WINNAME				"Captured Raw Image"
#define						SLS_PROCIMAGE_WINNAME						"Processed Image"

// ======================
//  Global Variables
// ======================

int								glutMainWndHandler		= -1;

SlsMainWnd				* gluiMainWndHandler	= NULL;
SlsParam					* slsParameters				= NULL;
PointGreyCamera		* pgCamera					= NULL;
ARTagHelper				* artagHelper					= NULL;
ExtrCalibrator			* extrCalib					= NULL;

unsigned char			* capturedRawImage		= NULL;
unsigned char			* processedImage			= NULL;

// =======================
//  Global Utility Functions
// =======================

void SphereDisplay(void)
{
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glutSwapBuffers();
}

bool CaptureImage ( unsigned char * rawImg, unsigned char * procImg, const cv::Mat & camIntr, const cv::Mat & camDist )
{
	int camH = slsParameters->GetCameraHeight();
	int camW = slsParameters->GetCameraWidth();

	if (slsParameters->GetCameraType() == SlsParam::DRAGONFLY_EXPRESS) 
	{
		pgCamera->Capture(rawImg);
		
		cv::Mat distortedImg(camH, camW, CV_8UC4);
		cv::Mat unDistortedImg = distortedImg.clone();
		
		CopyRawImageBuf2CvMat(rawImg, 4, distortedImg);
		cv::undistort(distortedImg, unDistortedImg, camIntr, camDist);
		CopyCvMat2RawImageBuf(unDistortedImg, procImg, 3);

		/*
		const unsigned char * srcData = rawImg;
		unsigned char * destData = procImg;
		
		for (int i = 0; i < camH * camW; ++i, srcData += 4, destData += 3)
		{
			destData[0] = srcData[2];
			destData[1] = srcData[1];
			destData[2] = srcData[0];
		}
		*/

		return true;
	}

	return false;
}

// ==============
// Main Loop
// ==============

static void Display(void)
{
	assert ( gluiMainWndHandler != NULL );

	int cameraW = slsParameters->GetCameraWidth();
	int cameraH = slsParameters->GetCameraHeight();
	int projectorW = slsParameters->GetProjectorWidth();
	int projectorH = slsParameters->GetProjectorHeight();
	int monitorW = slsParameters->GetMonitorWidth();
	int monitorH = slsParameters->GetMonitorHeight();

	int mainWndW = slsParameters->GetMainWindowWidth();
	int mainWndH = slsParameters->GetMainWindowHeight();
	
	int rasterX = slsParameters->GetRasterPosX();
	int rasterY = slsParameters->GetRasterPosY();

	float pixelZoomX = (float)(monitorW) / (float)(cameraW);
	float pixelZoomY = (float)(monitorH) / (float)(cameraH);
	
	bool isCaptured = false;

	//
	// Processing SLS Status
	// 

	SlsMainWnd::SLS_MODE mode = gluiMainWndHandler->getSlsMode();
	switch ( mode )
	{
	
	case SlsMainWnd::SLS_IDLE:
		break;

	case SlsMainWnd::SLS_CAPTURE_IMAGE:

		extrCalib->PrintMatrix(extrCalib->GetMatrix(ExtrCalibrator::CAMERA, ExtrCalibrator::INTR));
		extrCalib->PrintMatrix(extrCalib->GetMatrix(ExtrCalibrator::CAMERA, ExtrCalibrator::DIST));

		isCaptured = CaptureImage (	capturedRawImage, 
													processedImage, 
													extrCalib->GetMatrix(ExtrCalibrator::CAMERA, ExtrCalibrator::INTR),
													extrCalib->GetMatrix(ExtrCalibrator::CAMERA, ExtrCalibrator::DIST)
													);

		// memset(processedImage, 0, cameraW * cameraH * 3);
		
		if ( !(isCaptured) ) { 
			std::cout << "Invalid Captured Image" << std::endl;
		}

		ShowImageInOpenCvWindow(SLS_CAPTUREDIMAGE_WINNAME, capturedRawImage, 4, cameraH, cameraW);
		ShowImageInOpenCvWindow(SLS_PROCIMAGE_WINNAME, processedImage, 3, cameraH, cameraW);
		gluiMainWndHandler->setSlsMode(SlsMainWnd::SLS_IDLE);
		
		break;
	
	case SlsMainWnd::SLS_DETECT_ARTAG:
		artagHelper->FindMarkerCorners(processedImage);
		gluiMainWndHandler->setSlsMode(SlsMainWnd::SLS_IDLE);

		break;

	case SlsMainWnd::SLS_INIT_PROJECTING:
		// TODO:
		gluiMainWndHandler->setSlsMode(SlsMainWnd::SLS_PROJECTING);
		
		break;

	case SlsMainWnd::SLS_PROJECTING:

		break;

	case SlsMainWnd::SLS_CALC_SHAPE:
		
		break;
	
	}

	// 
	// Initialize OpenGL Environment
	// 

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glViewport(0, 0, mainWndW, mainWndH);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, mainWndW, 0, mainWndH, -1.0f, 1.0f);

	glMatrixMode(GL_MODELVIEW);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	// glLoadIdentity();
	// gluLookAt(0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

	// 
	// Draw Captured Image
	// 
	
	glLoadIdentity();
	glRasterPos2d(rasterX, rasterY);
	glPixelZoom(pixelZoomX, -pixelZoomY);
	glDrawPixels(cameraW, cameraH, GL_RGB, GL_UNSIGNED_BYTE, processedImage);

	// 
	// Draw Identitied Marker
	// 

	glLoadIdentity();
	glTranslatef(rasterX, mainWndH - monitorH - 10, 0.0f);
	artagHelper->DrawMarkersInCameraImage(pixelZoomX, pixelZoomY);

	// 
	// Project White Light
	// 

	glLoadIdentity();
	glTranslatef( slsParameters->GetMainWindowInMainDisplayWidth(), 0.0f, 0.0f );
	glColor3d( 1.0f, 1.0f, 1.0f );
	glBegin(GL_QUADS);
		glVertex2i(0, 0);
		glVertex2i(projectorW, 0);
		glVertex2i(projectorW, projectorH);
		glVertex2i(0, projectorH);
	glEnd();

	if (mode == SlsMainWnd::SLS_PROJECTING) 
	{
		glLoadIdentity();
		glTranslatef(slsParameters->GetMainWindowInMainDisplayWidth(), 0.0f, 0.0f);
		// Display Gray Code
		// TODO:
	}

	// 
	// Swap Buffers
	//

	glutSwapBuffers();
}

// 
// Main Function
// 

int main(int argc, char ** argv)
{
	// =============================
	//	  Load SLS Params & Initializations
	// =============================

	slsParameters = new SlsParam();
	slsParameters -> LoadFromXmlFile(SLS_CONFIG_XMLFILEPATH);
	slsParameters -> printSlsParams();

	pgCamera = new PointGreyCamera(0, slsParameters->GetCameraWidth(), slsParameters->GetCameraHeight(), 60.0f);
	pgCamera->Init();
	pgCamera->Start();

	artagHelper = new ARTagHelper(
		slsParameters->GetCameraWidth(), 
		slsParameters->GetCameraHeight(), 
		slsParameters->GetARTagConfigFilePath(),
		slsParameters->GetARTagPosFilePath()
		);

	extrCalib = new ExtrCalibrator(artagHelper->GetMarkerNumber());
	extrCalib->ReadIntrParaFile(	slsParameters->GetIntrFilePath(SlsParam::CAMERA),
												slsParameters->GetDistortionFilePath(SlsParam::CAMERA),
												slsParameters->GetIntrFilePath(SlsParam::PROJECTOR),
												slsParameters->GetDistortionFilePath(SlsParam::PROJECTOR)
												);

	capturedRawImage = new unsigned char[slsParameters->GetCameraHeight() * slsParameters->GetCameraWidth() * 4];
	processedImage	 = new unsigned char[slsParameters->GetCameraHeight() * slsParameters->GetCameraWidth() * 3];

	memset(capturedRawImage, 0, slsParameters->GetCameraHeight() * slsParameters->GetCameraWidth() * 4);
	memset(processedImage, 0, slsParameters->GetCameraHeight() * slsParameters->GetCameraWidth() * 3);

	// ===================
	//   Glut Code & Glui Code
	// ===================

	int mainWndW = slsParameters->GetMainWindowWidth();
	int mainWndH = slsParameters->GetMainWindowHeight();

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(slsParameters->GetWindowPosX(), 50);
	glutInitWindowSize(mainWndW, mainWndH);

	glutMainWndHandler = glutCreateWindow("Structured Light 3D Reconstruction System");
	gluiMainWndHandler = SlsMainWnd::Instance(glutMainWndHandler);

	glutPositionWindow(slsParameters->GetWindowPosX(), 0);
	glutDisplayFunc(Display);
	// glutDisplayFunc(SphereDisplay);

	// 
	// Start to Glut Loop
	// 

	glutMainLoop();

	delete [] capturedRawImage;
	delete [] processedImage;

	exit(EXIT_SUCCESS);
}
