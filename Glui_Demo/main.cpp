#include <cstdio>
#include <cstdlib>

#include <OpenGL\Glui\glui.h>
#include <OpenGL\Glut\glut.h>

// #include <gl\glui.h>
// #include <gl\glut.h>

// 
// MACROS DEFINITIONS
// 

#define			GLUI_CHECKBOX_WIREFRAME_ID			101

#define			ENABLE_ID							300
#define			DISABLE_ID						301

#define			SHOW_ID							302
#define			HIDE_ID								303

// 
// ===========================================
//  GLUI & GLUT Control Callback Declarations
// ===========================================
// 

void gluiControlCB(int control_id);
void gluiMenuCB(int value);

void glutIdleCB(void);
void glutKeyboardCB(unsigned char key, int x, int y);
void glutMouseCB(int button, int button_state, int x, int y);
void glutMotionCB(int x, int y);
void glutPassiveMotionCB(int x, int y);
void glutReshapeCB(int x, int y);

// 
//  GLUT Display
// 

void glutDisplayCB(void);

//
//  Utility Functions
// 

void drawAxes(float scale);

// 
//  GLUI Components
// 

GLUI *			gluiLeftSubWindow		= NULL;
GLUI *			gluiBottomSubWindow		= NULL;

GLUI_Panel *	optionsPanel			= NULL;
GLUI_Panel *	objPropertyPanel		= NULL;


// ==================================================
//  Global Live Variables && GLUT MainWindow Handler
// ==================================================

GLfloat light0_ambient[]  = { 0.1f, 0.1f, 0.3f, 1.0f };
GLfloat light0_diffuse[]    = { 0.6f, 0.6f, 1.0f, 1.0f };
GLfloat light0_position[]  = { 0.5f, 0.5f, 1.0f, 0.0f };

int				hMainWndInstance		= -1;
float			xy_aspect				= 1.0f;

int				wireframe				= 0;
float			scale					= 1.0f;

int				sphereSegments			= 8;

int				showSphere				= 1;
int				showAxes				= 1;
int				showText				= 1;

float sphere_rotate[16] = { 1, 0, 0, 0, 
							0, 1, 0, 0, 
							0, 0, 1, 0, 
							0, 0, 0, 1 };

float obj_pos[] = { 0.0f, 0.0f, 0.0f };

// 
// Main Function
// 

int main(int argc, char ** argv)
{
	// 
	// Initialize GLUT && Create Window
	// 

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(800, 600);

	hMainWndInstance = glutCreateWindow("GLUI Demo");
	
	glutDisplayFunc(glutDisplayCB);
	glutMotionFunc(glutMotionCB);
	glutPassiveMotionFunc(glutPassiveMotionCB);
	GLUI_Master.set_glutReshapeFunc(glutReshapeCB);
	GLUI_Master.set_glutKeyboardFunc(glutKeyboardCB);
	GLUI_Master.set_glutSpecialFunc(NULL);
	GLUI_Master.set_glutMouseFunc(glutMouseCB);
	GLUI_Master.set_glutIdleFunc(glutIdleCB);

	// ======================
	//  Set up OpenGL Params
	// ======================

	glEnable(GL_LIGHTING);
	glEnable(GL_NORMALIZE);

	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0, GL_AMBIENT, light0_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, light0_position);

	glEnable(GL_DEPTH_TEST);

	//
	// Here is the GLUI code
	// 

	printf("GLUI Version = %3.2f\n", GLUI_Master.get_version());

	// 
	// Create Left Subwindow
	// 

	gluiLeftSubWindow = GLUI_Master.create_glui_subwindow(hMainWndInstance, GLUI_SUBWINDOW_RIGHT);
	gluiLeftSubWindow -> set_main_gfx_window(hMainWndInstance);
	
	// 
	// Create UI Components of GLUI
	// 
	
	objPropertyPanel = new GLUI_Rollout(gluiLeftSubWindow, "Properties", false);
	
	new GLUI_Checkbox(objPropertyPanel, "Wireframe", &wireframe, GLUI_CHECKBOX_WIREFRAME_ID, gluiControlCB);
	GLUI_Spinner * spinner = new GLUI_Spinner( objPropertyPanel, "Sphere Segments:", &sphereSegments);
	spinner -> set_int_limits(3, 60); 
	spinner->set_alignment( GLUI_ALIGN_RIGHT );
	
	GLUI_Spinner * scale_spinner = new GLUI_Spinner( objPropertyPanel, "Scale:", &scale);
	scale_spinner -> set_float_limits(0.2f, 4.0f);
	scale_spinner -> set_alignment( GLUI_ALIGN_RIGHT );
	
	optionsPanel = new GLUI_Rollout(gluiLeftSubWindow, "Options", true);
	new GLUI_Checkbox(optionsPanel, "Draw Sphere", &showSphere);
	new GLUI_Checkbox(optionsPanel, "Draw Axes", &showAxes);
	// new GLUI_Checkbox(optionsPanel, "Draw Text", &showText);

	new GLUI_StaticText(gluiLeftSubWindow, "");

	// 
	// Create Buttons
	// 
	
	new GLUI_Button(gluiLeftSubWindow, "Disable movement", DISABLE_ID, gluiControlCB );
	new GLUI_Button(gluiLeftSubWindow, "Enable movement", ENABLE_ID, gluiControlCB );
	new GLUI_Button(gluiLeftSubWindow, "Hide", HIDE_ID, gluiControlCB );
	new GLUI_Button(gluiLeftSubWindow, "Show", SHOW_ID, gluiControlCB );
	
	new GLUI_StaticText(gluiLeftSubWindow, "");
	
	new GLUI_Button(gluiLeftSubWindow, "Quit", 0, (GLUI_Update_CB)(exit));

	//
	// Create Bottom Sub Window
	// 
	
	gluiBottomSubWindow = GLUI_Master.create_glui_subwindow(hMainWndInstance, GLUI_SUBWINDOW_BOTTOM);
	gluiBottomSubWindow -> set_main_gfx_window(hMainWndInstance);

	GLUI_Rotation * sphere_rotation = new GLUI_Rotation(gluiBottomSubWindow, "Sphere", sphere_rotate);
	sphere_rotation -> set_spin(0.98f);

	new GLUI_Column(gluiBottomSubWindow, false);
	GLUI_Translation * translation_xy = new GLUI_Translation(gluiBottomSubWindow, "Objects XY", GLUI_TRANSLATION_XY, obj_pos );
	translation_xy -> set_speed( 0.01f );

	new GLUI_Column( gluiBottomSubWindow, false );
	GLUI_Translation * translation_x = new GLUI_Translation(gluiBottomSubWindow, "Objects X", GLUI_TRANSLATION_X, &obj_pos[0] );
	translation_x -> set_speed( 0.01f );

	new GLUI_Column( gluiBottomSubWindow, false );
	GLUI_Translation * translation_y = new GLUI_Translation( gluiBottomSubWindow, "Objects Y", GLUI_TRANSLATION_Y, &obj_pos[1] );
	translation_y -> set_speed( 0.01f );

	new GLUI_Column( gluiBottomSubWindow, false );
	GLUI_Translation * translation_z = new GLUI_Translation( gluiBottomSubWindow, "Objects Z", GLUI_TRANSLATION_Z, &obj_pos[2] );
	translation_z -> set_speed( 0.01f );

	// 
	// Regular GLUT Main Loop
	// 
	glutMainLoop();

	return EXIT_SUCCESS;
}

void drawAxes( float scale )
{
	glDisable( GL_LIGHTING );

	glPushMatrix();
	glScalef( scale, scale, scale );

	glBegin( GL_LINES );
 
	glColor3f( 1.0, 0.0, 0.0 );
	glVertex3f( .8f, 0.05f, 0.0 );  glVertex3f( 1.0, 0.25f, 0.0 );		// Letter X
	glVertex3f( 0.8f, .25f, 0.0 );  glVertex3f( 1.0, 0.05f, 0.0 );

	glColor3f( 1.0, 0.0, 0.0 );
	glVertex3f( 0.0, 0.0, 0.0 );  glVertex3f( 1.0, 0.0, 0.0 );			// X axis

	glColor3f( 0.0, 1.0, 0.0 );
	glVertex3f( 0.0, 0.0, 0.0 );  glVertex3f( 0.0, 1.0, 0.0 );			// Y axis

	glColor3f( 0.0, 0.0, 1.0 );
	glVertex3f( 0.0, 0.0, 0.0 );  glVertex3f( 0.0, 0.0, 1.0 );			// Z axis
	
	glEnd();

	glPopMatrix();

	glEnable( GL_LIGHTING );
}

void gluiControlCB(int control_id)
{
	if (control_id == HIDE_ID) { 
		gluiBottomSubWindow -> hide(); 
	}
	else if (control_id == SHOW_ID) { 
		gluiBottomSubWindow -> show(); 
	}
	else if ( control_id == ENABLE_ID ) { 
		gluiBottomSubWindow -> enable(); 
	}
	else if ( control_id == DISABLE_ID ) { 
		gluiBottomSubWindow -> disable(); 
	}
}

void gluiMenuCB(int value)
{
	// Nothing to do
	;
}

void glutIdleCB(void)
{
	if ( glutGetWindow() != hMainWndInstance ) { 
		glutSetWindow(hMainWndInstance);  
	}

	glutPostRedisplay();
}

void glutKeyboardCB(unsigned char key, int x, int y)
{
	switch (key)
	{
	
	case 27: 
	case 'q':

		exit(0);
		break;
	
	};
  
	glutPostRedisplay();
}

void glutMouseCB(int button, int button_state, int x, int y)
{
	printf("void glutMouseCB(int button, int button_state, int x, int y)\n");
	glutPostRedisplay();
}

void glutMotionCB(int x, int y)
{
	printf("void glutMotionCB(int x, int y)\n");
	glutPostRedisplay();
}

void glutPassiveMotionCB(int x, int y)
{
	printf("void glutPassiveMotionCB(int x, int y)\n");
	glutPostRedisplay();
}

void glutReshapeCB(int x, int y)
{
	int tx, ty, tw, th;
	GLUI_Master.get_viewport_area( &tx, &ty, &tw, &th );
	glViewport( tx, ty, tw, th );

	xy_aspect = (float)(tw) / (float)(th);

	glutPostRedisplay();

	return;
}

void glutDisplayCB(void)
{
	glClearColor(0.9f, 0.9f, 0.9f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-xy_aspect * 0.04f, xy_aspect * 0.04f, -0.04f, 0.04f, 0.1f, 15.0f);

	glMatrixMode(GL_MODELVIEW);

	glLoadIdentity();
	glTranslatef(0.0, 0.0, -2.6f);
	glTranslatef(obj_pos[0], obj_pos[1], -obj_pos[2]); 
	// glMultMatrixf( view_rotate );

	glScalef(scale, scale, scale);

	/*** Now we render object, using the variables 'obj_type', 'segments', and
		 'wireframe'.  These are _live_ variables, which are transparently 
		 updated by GLUI ***/

	glPushMatrix();
	glTranslatef( -0.5f, 0.0f, 0.0f );
	glMultMatrixf( sphere_rotate );
	
	if ( wireframe && showSphere) {
		glutWireSphere( 0.4f, sphereSegments, sphereSegments );
	}
	else if ( showSphere ) {
		glutSolidSphere( 0.4f, sphereSegments, sphereSegments );
	}

	if ( showAxes ) {
		drawAxes(0.60f);
	}
	
	glPopMatrix();

	glutSwapBuffers();
}
