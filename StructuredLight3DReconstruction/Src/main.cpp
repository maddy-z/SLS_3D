#include	<cstdio>
#include	<cstdlib>
#include	<cassert>

#include	<windows.h>

#include	<OpenCV_2.3.1\opencv2\opencv.hpp>
#include	<OpenGL\Glui\glui.h>
#include	<OpenGL\Glut\glut.h>

#include	"SlsMainWnd.h"
#include	"SlsParam.h"
#include	"SlsUtils.h"
#include	"PointGreyCamera.h"
#include	"ARTagHelper.h"
#include	"ExtrCalibrator.h"
#include	"GrayCode.h"
#include	"BilateralFilter.h"
#include	"ShapeCalculator.h"

#include	"CalcShape.h"

// =======================
//	 MACROS DEFINITIONS
// =======================

#define						SLS_CONFIG_XMLFILEPATH						"SlsConfig.xml"
#define						SLS_CAPTUREDIMAGE_WINNAME				"Captured Raw Image"
#define						SLS_PROCIMAGE_WINNAME						"Processed Image"

// #define						SHOW_CAPIMAGE_IN_OPENCVWIN

// ======================
//  Global Variables
// ======================

int								glutMainWndHandler			= -1;

SlsMainWnd				* gluiMainWndHandler		= NULL;
SlsParam					* slsParameters					= NULL;
PointGreyCamera		* pgCamera						= NULL;
ARTagHelper				* artagHelper						= NULL;
ExtrCalibrator			* extrCalib						= NULL;
GrayCode					* grayCode						= NULL;
BilateralFilter				* bFilter								= NULL;
ShapeCalculator			* shapeCalc						= NULL;
CCalcShape				*cCalcShape						= NULL;

double						* filteredC2P_H					= NULL;
double						* filteredC2P_V					= NULL;

unsigned char			* capturedRawImage			= NULL;
unsigned char			* capturedRawImage3Ch	= NULL;
unsigned char			* processedImage				= NULL;

int								lastGrayCodeDispMode		= GrayCode::DISP_GRAYCODE;
int								lastGrayCodeHVMode		= GrayCode::VERT;

// =======================
//  Global Utility Functions
// =======================

void SphereDisplay ( void )
{
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glutSwapBuffers();
}

bool CaptureImage ( unsigned char * rawImg, unsigned char * procImg, const cv::Mat & camIntr, const cv::Mat & camDist )
{
	int camH = slsParameters->GetCameraHeight();
	int camW = slsParameters->GetCameraWidth();

	if ( slsParameters->GetCameraType() == SlsParam::DRAGONFLY_EXPRESS ) 
	{
		pgCamera->Capture(rawImg);
		
		cv::Mat distortedImg(camH, camW, CV_8UC4);
		cv::Mat unDistortedImg = distortedImg.clone();
		
		CopyRawImageBuf2CvMat ( rawImg, 4, distortedImg );
		cv::undistort(distortedImg, unDistortedImg, camIntr, camDist );
		CopyCvMat2RawImageBuf ( unDistortedImg, procImg, 3 );

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

// ===============
// Main Loop
// ===============

static void Display ( void )
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
	
	int rasterX = slsParameters->GetRasterPosX ();
	int rasterY = slsParameters->GetRasterPosY ();

	float pixelZoomX = ( float )( monitorW ) / ( float )( cameraW );
	float pixelZoomY = ( float )( monitorH ) / ( float )( cameraH );
	
	bool isCaptured = false;

	//
	// Process SLS Status
	// 
	
	if ( gluiMainWndHandler->getSlsMode() == SlsMainWnd::SLS_IDLE ) 
	{ 
		; 
	}
	else if ( gluiMainWndHandler->getSlsMode() == SlsMainWnd::SLS_CAPTURE_IMAGE ) 
	{
		std::cout << "Camera Intrinsic and Distortion Params:" << std::endl;
		extrCalib->PrintMatrix ( extrCalib->GetMatrix ( ExtrCalibrator::CAMERA, ExtrCalibrator::INTR ) );
		extrCalib->PrintMatrix ( extrCalib->GetMatrix ( ExtrCalibrator::CAMERA, ExtrCalibrator::DIST ) );

		// std::cout << "Projector Intrinsic and Distortion Params:" << std::endl;
		// extrCalib->PrintMatrix(extrCalib->GetMatrix(ExtrCalibrator::PROJECTOR, ExtrCalibrator::INTR));
		// extrCalib->PrintMatrix(extrCalib->GetMatrix(ExtrCalibrator::PROJECTOR, ExtrCalibrator::DIST));

		isCaptured = CaptureImage (	capturedRawImage, 
													processedImage, 
													extrCalib->GetMatrix(ExtrCalibrator::CAMERA, ExtrCalibrator::INTR),
													extrCalib->GetMatrix(ExtrCalibrator::CAMERA, ExtrCalibrator::DIST)
													);

		if ( isCaptured ) 
		{
			CopyRawImageBufByDiffChannel (	capturedRawImage, 4, 
																capturedRawImage3Ch, 3, 
																cameraH, cameraW
																);
		}
		else { std::cout << "Invalid Captured Image" << std::endl; }

#ifdef SHOW_CAPIMAGE_IN_OPENCVWIN
		ShowImageInOpenCvWindow(SLS_CAPTUREDIMAGE_WINNAME, capturedRawImage, 4, cameraH, cameraW);
		ShowImageInOpenCvWindow(SLS_PROCIMAGE_WINNAME, processedImage, 3, cameraH, cameraW);
		cv::setMouseCallback(SLS_PROCIMAGE_WINNAME, ClickOnMouse, 0);
#endif

		/*
		extrCalib->ReadExtrParaFile(	slsParameters->GetRotFilePath(SlsParam::CAMERA),
													slsParameters->GetTransFilePath(SlsParam::CAMERA),
													slsParameters->GetRotFilePath(SlsParam::PROJECTOR),
													slsParameters->GetTransFilePath(SlsParam::PROJECTOR)
													);

		extrCalib->PrintMatrix(extrCalib->GetMatrix(ExtrCalibrator::CAMERA, ExtrCalibrator::EXTR));
		extrCalib->PrintMatrix(extrCalib->GetMatrix(ExtrCalibrator::PROJECTOR, ExtrCalibrator::EXTR));

		artagHelper->RecalcMarkerCornersPos3d (	extrCalib->GetMatrix(ExtrCalibrator::CAMERA, ExtrCalibrator::INTR),
																		extrCalib->GetMatrix(ExtrCalibrator::CAMERA, ExtrCalibrator::EXTR),
																		extrCalib->GetMatrix(ExtrCalibrator::PROJECTOR, ExtrCalibrator::INTR),
																		extrCalib->GetMatrix(ExtrCalibrator::PROJECTOR, ExtrCalibrator::EXTR)
																		);

		artagHelper->PrintRecalcMarkerCornersPos3d();
		*/

		gluiMainWndHandler->setSlsModeIdle();
	}
	else if ( gluiMainWndHandler->getSlsMode() == SlsMainWnd::SLS_DETECT_ARTAG )
	{
		/*
		artagHelper->FindMarkerCorners ( capturedRawImage3Ch );
		extrCalib->ExtrCalib (	ExtrCalibrator::CAMERA, 
											artagHelper->m_MarkerCornerPosCam2d, 
											artagHelper->m_MarkerCornerPos3d, 
											artagHelper->m_ValidFlagCam );
		
		std::cout << "Camera Extr Params From Raw Captured Image:" << std::endl;
		extrCalib->PrintMatrix ( extrCalib->GetMatrix(ExtrCalibrator::CAMERA, ExtrCalibrator::EXTR));
		extrCalib->SaveMatrix ( ExtrCalibrator::CAMERA, ExtrCalibrator::EXTR, slsParameters->GetExtrFilePath(SlsParam::CAMERA) );
		*/

		// --------------------------------------------------
		// --------------------------------------------------

		artagHelper->FindMarkerCorners ( processedImage );
		extrCalib->ExtrCalib ( ExtrCalibrator::CAMERA, artagHelper->m_MarkerCornerPosCam2d, artagHelper->m_MarkerCornerPos3d, artagHelper->m_ValidFlagCam );
		
		std::cout << "Camera Extr Params From Processed Image:" << std::endl;
		extrCalib->PrintMatrix ( extrCalib->GetMatrix ( ExtrCalibrator::CAMERA, ExtrCalibrator::EXTR ) );
		extrCalib->SaveMatrix ( ExtrCalibrator::CAMERA, ExtrCalibrator::EXTR, slsParameters->GetExtrFilePath(SlsParam::CAMERA) );

		gluiMainWndHandler->setSlsModeIdle();
	}
	else if ( gluiMainWndHandler->getSlsMode() == SlsMainWnd::SLS_INIT_PROJECTING )
	{
		grayCode->InitDispCode ( 1, GrayCode::DISP_GRAYCODE, GrayCode::VERT );
		lastGrayCodeDispMode = GrayCode::DISP_GRAYCODE;
		lastGrayCodeHVMode = GrayCode::VERT;
		
		gluiMainWndHandler->setSlsMode ( SlsMainWnd::SLS_PROJECTING );
	}
	else if ( gluiMainWndHandler->getSlsMode() == SlsMainWnd::SLS_PROJECTING )
	{
		Sleep(250);
		
		isCaptured = CaptureImage (	capturedRawImage, 
													processedImage, 
													extrCalib->GetMatrix(ExtrCalibrator::CAMERA, ExtrCalibrator::INTR),
													extrCalib->GetMatrix(ExtrCalibrator::CAMERA, ExtrCalibrator::DIST)
													);

		if ( isCaptured ) 
		{
			CopyRawImageBufByDiffChannel (	capturedRawImage, 4, 
																capturedRawImage3Ch, 3, 
																cameraH, cameraW
																);

			// grayCode->CaptureCode ( capturedRawImage3Ch );
			grayCode->CaptureCode ( processedImage );												// Decode Gray Code In Camera from Projector
			grayCode->SaveCurrFrame ();

			if ( grayCode->GetNextFrame () == false ) 
			{
				if ( lastGrayCodeDispMode == GrayCode::DISP_GRAYCODE ) 
				{
					grayCode->EncodeGray2Binary();

					if ( lastGrayCodeHVMode == GrayCode::VERT ) 
					{
						grayCode->InitDispCode(1, GrayCode::DISP_ILLUMI);
						
						lastGrayCodeDispMode = GrayCode::DISP_ILLUMI;
						lastGrayCodeHVMode = GrayCode::VERT;
					}
					else if ( lastGrayCodeHVMode == GrayCode::HORI ) 
					{
						// grayCode->EncodeGray2Binary();
						grayCode->WriteC2P();
						grayCode->SetDispModeIdle();

						gluiMainWndHandler->setSlsModeIdle();
					
						// 
						// Calculate Projector Extr Parameters
						// 
						
						artagHelper->GetMarkerCornerPos2dInProjector ( grayCode );
						artagHelper->PrintMarkerCornersPos2dInProjector ();
						artagHelper->SaveMarkerCornersPos ( ARTagHelper::MARKER_IN_PROJECTOR, "Data/Projector/MarkerPosInPro.txt" );

						extrCalib->ExtrCalib ( ExtrCalibrator::PROJECTOR, artagHelper->m_MarkerCornerPosPro2d, artagHelper->m_MarkerCornerPos3d, artagHelper->m_ValidFlagPro );
						extrCalib->SaveMatrix ( ExtrCalibrator::PROJECTOR, ExtrCalibrator::EXTR, slsParameters->GetExtrFilePath(SlsParam::PROJECTOR) );

						std::cout << "Camera Extr Params:" << std::endl;		extrCalib->PrintMatrix ( extrCalib->GetMatrix(ExtrCalibrator::CAMERA, ExtrCalibrator::EXTR) );
						std::cout << "Projector Extr Params:" << std::endl;	extrCalib->PrintMatrix ( extrCalib->GetMatrix(ExtrCalibrator::PROJECTOR, ExtrCalibrator::EXTR) );

						artagHelper->RecalcMarkerCornersPos3d (		extrCalib->GetMatrix(ExtrCalibrator::CAMERA, ExtrCalibrator::INTR),
																							extrCalib->GetMatrix(ExtrCalibrator::CAMERA, ExtrCalibrator::EXTR),
																							extrCalib->GetMatrix(ExtrCalibrator::PROJECTOR, ExtrCalibrator::INTR),
																							extrCalib->GetMatrix(ExtrCalibrator::PROJECTOR, ExtrCalibrator::EXTR) 
																							);

						artagHelper->PrintRecalcMarkerCornersPos3d();
					}
				}
				else if ( lastGrayCodeDispMode == GrayCode::DISP_ILLUMI ) 
				{
					grayCode->InitDispCode(1, GrayCode::DISP_GRAYCODE, GrayCode::HORI);
					
					lastGrayCodeDispMode = GrayCode::DISP_GRAYCODE;
					lastGrayCodeHVMode = GrayCode::HORI;
				}
			}		
		}
	}
	else if ( gluiMainWndHandler->getSlsMode() == SlsMainWnd::SLS_CALC_SHAPE )
	{
		// bFilter->Filter ( grayCode->m_C2P_DB[GrayCode::VERT], filteredC2P_V, cameraH, cameraW, 1 );
		// bFilter->Filter ( grayCode->m_C2P_DB[GrayCode::HORI], filteredC2P_H, cameraH, cameraW, 1 );

		shapeCalc->CalcShape (	grayCode->m_C2P_DB[GrayCode::VERT], 
												grayCode->m_Mask, 
												extrCalib->GetMatrix ( ExtrCalibrator::CAMERA, ExtrCalibrator::INTR ), 
												extrCalib->GetMatrix ( ExtrCalibrator::CAMERA, ExtrCalibrator::EXTR ),
												extrCalib->GetMatrix ( ExtrCalibrator::PROJECTOR, ExtrCalibrator::INTR ),
												extrCalib->GetMatrix ( ExtrCalibrator::PROJECTOR, ExtrCalibrator::EXTR ) 
												);

		/*shapeCalc->CalcShape (	grayCode->m_C2P_DB[GrayCode::HORI],
												grayCode->m_C2P_DB[GrayCode::VERT],
												grayCode->m_Mask, 
												extrCalib->GetMatrix ( ExtrCalibrator::CAMERA, ExtrCalibrator::INTR ), 
												extrCalib->GetMatrix ( ExtrCalibrator::CAMERA, ExtrCalibrator::EXTR ),
												extrCalib->GetMatrix ( ExtrCalibrator::PROJECTOR, ExtrCalibrator::INTR ),
												extrCalib->GetMatrix ( ExtrCalibrator::PROJECTOR, ExtrCalibrator::EXTR ) 
												);*/
		
		shapeCalc->WriteOBJ ( slsParameters->Get3dObjFilePath() );

		/*CvMat * _cam_intr = cvCreateMat( 3,3,CV_64F );
		CvMat * _cam_extr = cvCreateMat( 3,4,CV_64F );
		CvMat * _pro_intr = cvCreateMat( 3,3,CV_64F );
		CvMat * _pro_extr = cvCreateMat( 3,4,CV_64F );

		ConvertMatrix(extrCalib->GetMatrix ( ExtrCalibrator::CAMERA, ExtrCalibrator::INTR ), _cam_intr);
		ConvertMatrix(extrCalib->GetMatrix ( ExtrCalibrator::CAMERA, ExtrCalibrator::EXTR ), _cam_extr);
		ConvertMatrix(extrCalib->GetMatrix ( ExtrCalibrator::PROJECTOR, ExtrCalibrator::INTR ), _pro_intr);
		ConvertMatrix(extrCalib->GetMatrix ( ExtrCalibrator::PROJECTOR, ExtrCalibrator::EXTR ), _pro_extr);

		cCalcShape->CalcShape (	grayCode->m_C2P_DB[GrayCode::HORI], 
												grayCode->m_Mask, 
												_cam_intr, 
												_cam_extr, 
												_pro_intr,
												_pro_extr
												); 
		cCalcShape->writeOBJ ( (char *)(slsParameters->Get3dObjFilePath()) );*/

		gluiMainWndHandler->setSlsModeIdle();
	}
	else if ( gluiMainWndHandler->getSlsMode() == SlsMainWnd::SLS_EXIT )
	{
		pgCamera->Stop();

		gluiMainWndHandler->setSlsModeIdle();
	}

	// 
	// Initialize OpenGL Environment
	// 

	glClearColor ( 1.0f, 1.0f, 1.0f, 1.0f );
	glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glViewport ( 0, 0, mainWndW, mainWndH );
	
	glMatrixMode ( GL_PROJECTION );
	glLoadIdentity ();
	glOrtho ( 0, mainWndW, 0, mainWndH, -1.0f, 1.0f );

	glMatrixMode(GL_MODELVIEW);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// 
	// Draw Captured Image
	// 
	
	glLoadIdentity ();
	glRasterPos2d ( rasterX, rasterY );
	glPixelZoom ( pixelZoomX, -pixelZoomY );
	// glDrawPixels(cameraW, cameraH, GL_RGB, GL_UNSIGNED_BYTE, capturedRawImage3Ch);
	glDrawPixels ( cameraW, cameraH, GL_RGB, GL_UNSIGNED_BYTE, processedImage );

	// 
	// Draw Identitied Marker
	// 

	glLoadIdentity ();
	glTranslatef ( rasterX, mainWndH - monitorH - 10, 0.0f );
	artagHelper->DrawMarkersInCameraImage ( pixelZoomX, pixelZoomY );

	// 
	// Project White Light
	// 

	glLoadIdentity();
	glTranslatef( slsParameters->GetMainWindowInMainDisplayWidth(), 0.0f, 0.0f );
	glColor3f( 1.0f, 1.0f, 1.0f );
	glBegin(GL_QUADS);
		glVertex2i(0, 0);
		glVertex2i(projectorW, 0);
		glVertex2i(projectorW, projectorH);
		glVertex2i(0, projectorH);
	glEnd();

	if ( gluiMainWndHandler->getSlsMode() == SlsMainWnd::SLS_PROJECTING ) 
	{
		glLoadIdentity();
		glTranslatef(slsParameters->GetMainWindowInMainDisplayWidth(), 0.0f, 0.0f);
		grayCode->DispCode();
	}

	glutSwapBuffers();
}

// 
// Main Function
// 

int main ( int argc, char ** argv )
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

	std::cout << std::endl << "Marker Corners Position 3D:" << std::endl;
	artagHelper->PrintMarkerCornersPos3d();
	std::cout << std::endl;

	extrCalib = new ExtrCalibrator ( artagHelper->GetMarkerNumber(),
													slsParameters->GetIntrFilePath(SlsParam::CAMERA),
													slsParameters->GetDistortionFilePath(SlsParam::CAMERA),
													slsParameters->GetIntrFilePath(SlsParam::PROJECTOR),
													slsParameters->GetDistortionFilePath(SlsParam::PROJECTOR)
													);
	
	std::cout << "Camera Intrinsic and Distortion Parameters:" << std::endl;
	extrCalib->PrintMatrix(extrCalib->GetMatrix(ExtrCalibrator::CAMERA, ExtrCalibrator::INTR));
	extrCalib->PrintMatrix(extrCalib->GetMatrix(ExtrCalibrator::CAMERA, ExtrCalibrator::DIST));

	std::cout << "Projector Intrinsic and Distortion Parameters:" << std::endl;
	extrCalib->PrintMatrix(extrCalib->GetMatrix(ExtrCalibrator::PROJECTOR, ExtrCalibrator::INTR));
	extrCalib->PrintMatrix(extrCalib->GetMatrix(ExtrCalibrator::PROJECTOR, ExtrCalibrator::DIST));

	bFilter = new BilateralFilter ( 10.0f, 1.0f, 5, 1024 );

	grayCode = new GrayCode (	slsParameters->GetCameraWidth(), 
												slsParameters->GetCameraHeight(), 
												slsParameters->GetProjectorWidth(), 
												slsParameters->GetProjectorHeight(), 
												false, 
												slsParameters->GetGrayCodeDir() 
												);
	
	shapeCalc	= new ShapeCalculator (	slsParameters->GetCameraWidth(), 
															slsParameters->GetCameraHeight(), 
															slsParameters->GetProjectorWidth(),
															slsParameters->GetProjectorHeight() 
															);

	cCalcShape = new CCalcShape ( slsParameters->GetCameraWidth(), 
													slsParameters->GetCameraHeight(), 
													slsParameters->GetProjectorWidth(),
													slsParameters->GetProjectorHeight() );

	capturedRawImage		= new unsigned char[slsParameters->GetCameraHeight() * slsParameters->GetCameraWidth() * 4];
	capturedRawImage3Ch	= new unsigned char[slsParameters->GetCameraHeight() * slsParameters->GetCameraWidth() * 3];
	processedImage			= new unsigned char[slsParameters->GetCameraHeight() * slsParameters->GetCameraWidth() * 3];
	
	filteredC2P_H				= new double[slsParameters->GetCameraHeight() * slsParameters->GetCameraWidth()];
	filteredC2P_V				= new double[slsParameters->GetCameraHeight() * slsParameters->GetCameraWidth()];

	memset(capturedRawImage, 0, slsParameters->GetCameraHeight() * slsParameters->GetCameraWidth() * 4);
	memset(capturedRawImage3Ch, 0, slsParameters->GetCameraHeight() * slsParameters->GetCameraWidth() * 3);
	memset(processedImage, 0, slsParameters->GetCameraHeight() * slsParameters->GetCameraWidth() * 3);

	memset ( filteredC2P_H, 0, slsParameters->GetCameraHeight() * slsParameters->GetCameraWidth() );
	memset ( filteredC2P_V, 0, slsParameters->GetCameraHeight() * slsParameters->GetCameraWidth() );

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
	
	// 
	// Start to Glut Loop
	// 

	glutMainLoop();

	// 
	// Clean SLS Components
	// 

	delete [] capturedRawImage;
	delete [] capturedRawImage3Ch;
	delete [] processedImage;

	delete [] filteredC2P_H;
	delete [] filteredC2P_V;

	delete gluiMainWndHandler;
	delete slsParameters;
	delete pgCamera;
	delete artagHelper	;
	delete extrCalib;
	delete grayCode;
	delete bFilter	;
	delete shapeCalc;

	exit ( EXIT_SUCCESS );
}
