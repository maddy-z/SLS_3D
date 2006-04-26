#include "SlsMainWnd.h"

#include <gl\glui.h>
#include <gl\glut.h>

// =================
//   Public Member Functions
// =================

std::auto_ptr<SlsMainWnd> SlsMainWnd::s_Instance;

SlsMainWnd * SlsMainWnd::Instance(int glutWndHandler)
{
	if ( SlsMainWnd::s_Instance.get() == 0 ) 
	{
		// SLSMainWnd * tmp = new SLSMainWnd();
		// tmp -> init(hGlutMainWnd);

		SlsMainWnd::s_Instance.reset(new SlsMainWnd(glutWndHandler));
	}

	return s_Instance.get();
}

void SlsMainWnd::SLSStatusCB(int value)
{
	SlsMainWnd::Instance() -> m_SlsStatusMode = ( SlsMainWnd::SLS_MODE )( value );

	char str[50];

	switch (value) 
	
	{

	case SLS_IDLE:							sprintf(str, "SLS_IDLE\n");						break;
	case SLS_CAPTURE_IMAGE:			sprintf(str, "SLS_CAPTURE_IMAGE\n");		break;
	case SLS_DETECT_ARTAG:			sprintf(str, "SLS_DETECT_ARTAG\n");		break;
	case SLS_PROJECTION:				sprintf(str, "SLS_PROJECTION\n");			break;
	case SLS_CALC_SHAPE:				sprintf(str, "SLS_CALC_SHAPE\n");			break;
	
	case SLS_EXIT:							printf("SLS_EXIT\n");						
													system("pause");
													exit(0);

	default :									sprintf(str, "INVALID SLS MODE\n");			break;
	
	}

	printf ("%s", str);
	return;

}

void SlsMainWnd::SLSIdleFunc(void)
{
	if ( glutGetWindow() != SlsMainWnd::Instance()->m_GlutSubWndHandler ) 
	{ 
		glutSetWindow(SlsMainWnd::Instance()->m_GlutSubWndHandler);  
	}

	glutPostRedisplay();
}

SlsMainWnd::~SlsMainWnd()
{
}

// =================
//  Private Member Functions
// =================

SlsMainWnd::SlsMainWnd(int glutWndHandler) :	
	m_GlutSubWndHandler(glutWndHandler), 
	m_SlsStatusMode(SlsMainWnd::SLS_IDLE)
{
	GLUI_Master.set_glutIdleFunc(SlsMainWnd::SLSIdleFunc);

	m_GluiMainWnd = GLUI_Master.create_glui_subwindow(m_GlutSubWndHandler, GLUI_SUBWINDOW_RIGHT);
	m_GluiMainWnd->set_main_gfx_window(m_GlutSubWndHandler);

	new GLUI_StaticText(m_GluiMainWnd, "");
	m_GluiButtonIdle = new GLUI_Button (m_GluiMainWnd, "IDLE", SlsMainWnd::SLS_IDLE, SlsMainWnd::SLSStatusCB);
	new GLUI_Separator(m_GluiMainWnd);

	m_GluiButtonCapture = new GLUI_Button (m_GluiMainWnd, "Capture Image", SlsMainWnd::SLS_CAPTURE_IMAGE, SlsMainWnd::SLSStatusCB);
	m_GluiButtonDetectArtag = new GLUI_Button (m_GluiMainWnd, "Detect ARTag", SlsMainWnd::SLS_DETECT_ARTAG, SlsMainWnd::SLSStatusCB);

	m_GluiButtonProject = new GLUI_Button (m_GluiMainWnd, "Project Structured Light", SlsMainWnd::SLS_PROJECTION, SlsMainWnd::SLSStatusCB);
	m_GluiButtonCalcShape = new GLUI_Button (m_GluiMainWnd, "Calc 3D Shape", SlsMainWnd::SLS_CALC_SHAPE, SlsMainWnd::SLSStatusCB);
	new GLUI_StaticText(m_GluiMainWnd, "");
	new GLUI_Separator(m_GluiMainWnd);

	m_GluiButtonExit = new GLUI_Button(m_GluiMainWnd, "Exit", SlsMainWnd::SLS_EXIT, SlsMainWnd::SLSStatusCB);
}


