#include	<cstdio>
#include	<cstdlib>
#include	<cassert>

#include	<OpenCV_2.3.1\opencv2\opencv.hpp>

#include	<OpenGL\Glui\glui.h>
#include	<OpenGL\Glut\glut.h>

#include	"SlsMainWnd.h"
#include	"SlsParam.h"
#include	"PointGreyCamera.h"
#include	"ARTagHelper.h"

// =======================
//	 MACROS DEFINITIONS
// =======================

#define						SLS_CONFIG_XMLFILEPATH						"SlsConfig.xml"
#define						SLS_CAPTUREDIMAGE_WINNAME				"Captured Image"

// ===================
//  Global Variables
// ===================

int								glutMainWndHandler		= -1;

SlsMainWnd				* gluiMainWndHandler	= NULL;
SlsParam					* slsParameters				= NULL;
PointGreyCamera		* pgCamera					= NULL;
ARTagHelper				* artagHelper					= NULL;

unsigned char			* capturedRawImage		= NULL;
unsigned char			* processedImage			= NULL;

// ======================
//  Global Utility Functions
// ======================

void SphereDisplay(void)
{
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glutSwapBuffers();
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
	assert ( gluiMainWndHandler != NULL );

	int monitorW = slsParameters->GetMonitorWidth();
	int monitorH = slsParameters->GetMonitorHeight();

	int cameraW = slsParameters->GetCameraWidth();
	int cameraH = slsParameters->GetCameraHeight();

	int projectorW = slsParameters->GetProjectorWidth();
	int projectorH = slsParameters->GetProjectorHeight();

	int mainWndW = slsParameters->GetMainWindowWidth();
	int mainWndH = slsParameters->GetMainWindowHeight();
	
	int rasterX = slsParameters->GetRasterPosX();
	int rasterY = slsParameters->GetRasterPoxY();

	float pixelZoomX = (float)(monitorW) / (float)(cameraW);
	float pixelZoomY = (float)(monitorH) / (float)(cameraH);

	bool flag = false;

	SlsMainWnd::SLS_MODE mode = gluiMainWndHandler->getSlsMode();
	
	switch ( mode )
	{
	case SlsMainWnd::SLS_IDLE:
		break;

	case SlsMainWnd::SLS_CAPTURE_IMAGE:
		flag = CaptureImage(processedImage);
		gluiMainWndHandler->setSlsMode(SlsMainWnd::SLS_IDLE);
		
		break;
	
	case SlsMainWnd::SLS_DETECT_ARTAG:
		artagHelper->FindMarkerCorners(processedImage);
		gluiMainWndHandler->setSlsMode(SlsMainWnd::SLS_IDLE);

		break;

	case SlsMainWnd::SLS_PROJECTION:

		break;

	case SlsMainWnd::SLS_CALC_SHAPE:
		
		break;
	}

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glViewport(0, 0, mainWndW, mainWndH);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, mainWndW, 0, mainWndH, 0.0f, 1.0f);

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

	glLoadIdentity();
	glTranslatef(rasterX, mainWndH - monitorH - 10, 0.0f);
	artagHelper->DrawMarkersInCameraImage(pixelZoomX, pixelZoomY);

	// 
	// Projecting White Light
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

	// if (mode == SlsMainWnd::SLS_DETECT_ARTAG) 
	// {
	//		glLoadIdentity();
	//		glTranslatef(rasterX, rasterY, 0.0f);
	//		artagHelper->DrawMarkersInCameraImage(pixelZoomX, pixelZoomY);

	//		gluiMainWndHandler->setSlsMode(SlsMainWnd::SLS_IDLE);
	// }

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
		slsParameters->GetARTagConfigFilePath().c_str(),
		slsParameters->GetARTagPosFilePath().c_str()
		);

	capturedRawImage = new unsigned char[slsParameters->GetCameraHeight() * slsParameters->GetCameraWidth() * 4];
	processedImage	 = new unsigned char[slsParameters->GetCameraHeight() * slsParameters->GetCameraWidth() * 3];

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

	// glutDisplayFunc(SphereDisplay);
	glutPositionWindow(slsParameters->GetWindowPosX(), 0);
	glutDisplayFunc(Display);
	
	// 
	// Start to Glut Loop
	// 

	glutMainLoop();

	delete [] capturedRawImage;
	delete [] processedImage;

	exit(EXIT_SUCCESS);
}

// ==================
// Experiment Functions
// ==================

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