#include <cstdio>
#include <cstdlib>
#include <cassert>

#include <iostream>

#include <Windows.h>

#include "..\StructuredLight3DReconstruction\Src\GrayCode.h"

#include <OpenGL\Glut\glut.h>

void ShowBinary(unsigned int num)
{
	unsigned int v;

	for (int i = 31; i >= 0; --i) {
		v = (num >> i) & 0x1;

		if ( v ) { std::cout << "1"; }
		else { std::cout << "0"; }
	}

	std::cout << std::endl;
}

/*

unsigned int Binary2Gray(unsigned int b)
{
	unsigned int ans = (b >> 31);
	
	for (int i = 30; i >= 0; --i) {
		ans <<= 1;
		ans |= ( ((b >> i) ^ (b >> (i+1))) & 0x1 );
	}

	return ans;
}

unsigned int Gray2Binary(unsigned int g)
{
	unsigned int ans = (g >> 31);

	for (int i = 30; i >= 0; --i) {
		ans <<= 1;
		ans |= ((g >> i) ^ (ans >> 1)) & 0x1;
	}

	return ans;
}

*/

// 
// Global Variables
// 

bool IsHori = false;

int projectorWidth = 1024;
int projectorHeight = 1024;

int glutMainWndHandler = -1;

GrayCode * gc = new GrayCode(640, 480, projectorWidth, projectorHeight, true, "123");

// 
// Main Loop Display
// 

void Display(void)
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glViewport(0, 0, projectorWidth, projectorHeight);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, projectorWidth, 0, projectorHeight, 0.0f, 1.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	if ( IsHori ) { gc->DispCode(GrayCode::HORI); }
	else { gc->DispCode(GrayCode::VERT); }

	if (gc->m_GBit > 0) { gc->m_GBit = (gc->m_GBit - 1); }
	else { gc->m_GBit = gc->m_CodeDepth - 1; IsHori = !IsHori; }

	Sleep(800);

	glutSwapBuffers();
	glutPostRedisplay();
}

// 
// Main Function
// 

int main(int argc, char ** argv)
{
	/*
	for (int i = 0; i < 16; ++i) 
	{
		std::cout << "Origin:\t" ; ShowBinary(i);
		std::cout << "B2G:\t" ; ShowBinary(Binary2Gray(i));
		std::cout << "G2B:\t"; ShowBinary(Gray2Binary(Binary2Gray(i)));

		assert (i == Gray2Binary(Binary2Gray(i)));
	}
	
	std::cout << std::endl;

	ShowBinary(0xffffffff);
	ShowBinary(Binary2Gray(0xffffffff));
	ShowBinary(Gray2Binary(Binary2Gray(0xffffffff)));

	std::cout << std::endl;
	*/

	gc ->InitDispCode(0);

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(200, 200);
	glutInitWindowSize(projectorWidth, projectorHeight);

	glutMainWndHandler = glutCreateWindow("Display Gray Code");

	glutDisplayFunc(Display);

	//
	// Start to Loop
	// 

	glutMainLoop();

	// 
	// End Loop
	// 
	
	system("pause");
	exit(EXIT_SUCCESS);
}