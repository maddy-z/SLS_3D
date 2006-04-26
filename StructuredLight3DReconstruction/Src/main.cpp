#include <cstdio>
#include <cstdlib>

#include <gl\glut.h>
#include <gl\glui.h>

#include "SlsMainWnd.h"
#include "SlsParam.h"

// 
// MACROS DEFINITIONS
// 

#define		SLS_CONFIG_XMLFILEPATH		"SlsConfig.xml"

// 
// Global Variables
// 

int glutMainWndHandler = -1;

SlsMainWnd * gluiMainWndHandler = NULL;

// ================
//  Main Functions
// ================

void SphereDisplay(void)
{
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glutSwapBuffers();

	// TODO:
}

int main(int argc, char ** argv)
{
	// ============
	//	  Load SLS Params
	// ============

	SlsParam slsParameters;
	slsParameters.LoadFromXmlFile(SLS_CONFIG_XMLFILEPATH);
	slsParameters.printSlsParams();

	// system("pause");
	// exit(0);

	// ========
	//   Glut Code
	// ========

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(800, 600);
	glutInitWindowPosition(100, 100);

	glutMainWndHandler = glutCreateWindow("Structured Light 3D Reconstruction System");

	glutDisplayFunc(SphereDisplay);

	// ========
	//   Glui Code
	// ========

	gluiMainWndHandler = SlsMainWnd::Instance(glutMainWndHandler);

	// 
	// Start to Glut Loop
	// 

	glutMainLoop();

	return EXIT_SUCCESS;

}