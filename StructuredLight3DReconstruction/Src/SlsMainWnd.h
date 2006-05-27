#ifndef _SLS_MAINWND_H_
#define _SLS_MAINWND_H_

#include <memory>

#include <OpenGL\Glui\glui.h>
#include <OpenGL\Glut\glut.h>

// #include <gl\glui.h>
// #include <gl\glut.h>

// ============
//   Singleton Mode
// ============

class SlsMainWnd
{

	friend class std::auto_ptr<SlsMainWnd>;
	friend void Display(void);

public:

	enum SLS_MODE 
	{
		SLS_IDLE,
		SLS_CAPTURE_IMAGE,
		SLS_DETECT_ARTAG,
		SLS_INIT_PROJECTING,
		SLS_PROJECTING,
		SLS_CALC_SHAPE,
		SLS_EXIT
	};

	enum SLS_CP_MODE
	{
		SLS_CAMERA_INTR,
		SLS_CAMERA_EXTR,
		SLS_PROJECTOR_INTR,
		SLS_PROJECTOR_EXTR
	};

	static SlsMainWnd * Instance(int glutWndHandler = -1);

	static void SLSStatusCB(int value);
	static void SLSIdleFunc(void);

	virtual ~SlsMainWnd();

	int registBindingGlutSubWnd(int hGlutSubWnd) 
	{
		m_GlutSubWndHandler = hGlutSubWnd;
		m_GluiMainWnd->set_main_gfx_window(m_GlutSubWndHandler);
	}

	inline int getBindingGlutSubWnd() const { return m_GlutSubWndHandler; }
	inline SlsMainWnd::SLS_MODE getSlsMode() const { return m_SlsStatusMode; }
	
	inline float getGluiVersion() const { if (m_GluiMainWnd) return GLUI_Master.get_version(); }

private:

	// 
	// Disabled Default Constructor
	// 
	
	SlsMainWnd(int glutWndHandler = -1);										

	inline void setSlsMode(SlsMainWnd::SLS_MODE m) { m_SlsStatusMode = m; }
	inline void setSlsModeIdle() { m_SlsStatusMode = SlsMainWnd::SLS_IDLE; }
	
	static std::auto_ptr<SlsMainWnd> s_Instance;

	int						m_GlutSubWndHandler;
	SLS_MODE			m_SlsStatusMode;

	GLUI					* m_GluiMainWnd;

	GLUI_Button		* m_GluiButtonIdle;
	GLUI_Button		* m_GluiButtonCapture;
	GLUI_Button		* m_GluiButtonDetectArtag;

	GLUI_Button		* m_GluiButtonProject;
	GLUI_Button		* m_GluiButtonCalcShape;

	GLUI_Button		* m_GluiButtonExit;

};

#endif