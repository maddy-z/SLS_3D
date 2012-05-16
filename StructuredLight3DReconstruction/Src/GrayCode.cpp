#include <cstdio>
#include <cstdlib>
#include <cmath>

#include <iostream>
#include <windows.h>

#include <OpenGL\Glui\glui.h>
#include <OpenGL\Glut\glut.h>

#include <OpenCV_2.3.1\opencv2\opencv.hpp>

#include "GrayCode.h"

// 
// Constructor & Destructor
// 

GrayCode::GrayCode ( int cameraW, int cameraH, int projectorW, int projectorH, bool bSlit, char * dirname )
{
	// Code covers 1024 ( = 2 ^ m_CodeDepth ) pixels.
	// This should be greater than projector resolution.

	m_CodeDepth = 10;
	m_CodeResolution = 1;				// 1

	m_SlitWidth = 3;
	m_SlitInterval = 16;				// 16
	m_bSlit = bSlit;

	m_NormThres = 70;

	m_ProjSeqNum = 0;

	// skip_graycode = true;

	m_DispMode = DISP_IDLE;

	m_CameraWidth = cameraW;
	m_CameraHeight = cameraH;
	m_ProjectorWidth = projectorW;
	m_ProjectorHeight = projectorH;

	m_Diff = new unsigned char[m_CameraWidth * m_CameraHeight];
	m_White = new unsigned char[m_CameraWidth * m_CameraHeight * 3];
	m_Black = new unsigned char[m_CameraWidth * m_CameraHeight * 3];
	m_bSlitImg = new unsigned char[m_CameraWidth * m_CameraHeight];
	
	for ( int i = 0; i < 2; i++ )
	{
		m_C2P[i] = new int[m_CameraWidth * m_CameraHeight];
		m_C2P_UC[i] = new unsigned char[m_CameraWidth * m_CameraHeight];
		m_C2P_DB[i] = new double[m_CameraWidth * m_CameraHeight];
	}

	m_Sum = new double[m_CameraWidth * m_CameraHeight];
	m_Num = new double[m_CameraWidth * m_CameraHeight];
	m_Mask = new bool[m_CameraWidth * m_CameraHeight];

	m_Illuminance = new unsigned char[m_CameraWidth * m_CameraHeight * 3];
	m_Compensate  = new unsigned char[m_CameraWidth * m_CameraHeight * 3];

	sprintf( m_DirName, "%s", dirname );
}

GrayCode::~GrayCode()
{
	delete [] m_Diff;
	delete [] m_White;
	delete [] m_Black;
	delete [] m_bSlitImg;

	for ( int i = 0; i < 2; i++ )
	{
		delete [] m_C2P[i];
		delete [] m_C2P_UC[i];
		delete [] m_C2P_DB[i];
	}

	delete [] m_Sum;
	delete [] m_Num;
	delete [] m_Mask;
	delete [] m_Illuminance;
	delete [] m_Compensate;
}

void GrayCode::InitDispCode( int nProjSeq )
{
	m_DispMode = GrayCode::DISP_GRAYCODE;
	m_GBit = m_CodeDepth - 1;
	m_NPMode = GrayCode::POSITIVE;
}

unsigned int GrayCode::Gray2Binary ( unsigned int g )
{
	unsigned int ans = (g >> 31);

	for (int i = 30; i >= 0; --i) {
		ans <<= 1;
		ans |= ((g >> i) ^ (ans >> 1)) & 0x1;
	}

	return ans;
}

unsigned int GrayCode::Binary2Gray ( unsigned int b )
{
	unsigned int ans = (b >> 31);
	
	for (int i = 30; i >= 0 ; --i) {
		ans <<= 1;
		ans |= ((b >> i) ^ (b >> (i+1))) & 0x1;
	}

	return ans;
}

void GrayCode::DispCode ( int hv_mode )
{
	std::cout << "Start:\tvoid GrayCode::DispCode ( int )" << std::endl;
	// std::cout << "GBit = " << m_GBit << std::endl;

	unsigned g, b, on;

	if ( hv_mode == HORI )	{ m_HVMode = HORI; }
	else					{ m_HVMode = VERT; }

	glColor3f(0.0f, 0.0f, 0.0f);

	glBegin(GL_POLYGON);
		glVertex3f(0.0f, 0.0f, 0.0f);
		glVertex3f(m_ProjectorWidth, 0.0f, 0.0f);
		glVertex3f(m_ProjectorWidth, m_ProjectorHeight, 0.0f);
		glVertex3f(0.0f, m_ProjectorHeight, 0.0f);
	glEnd();
	
	glColor3f(1.0f, 1.0f, 1.0f);

	switch (m_DispMode)
	{

	case GrayCode::DISP_ILLUMI:
	
		glBegin(GL_POLYGON);
			glVertex3f(0.0f, 0.0f, 0.0f);
			glVertex3f(m_ProjectorWidth, 0.0f, 0.0f);
			glVertex3f(m_ProjectorWidth, m_ProjectorHeight, 0.0f);
			glVertex3f(0.0f, m_ProjectorHeight, 0.0f);
		glEnd();

		break;

	case GrayCode::DISP_GRAYCODE:

		switch (m_HVMode)
		{

		case GrayCode::HORI:

			for (int i = 0; i < m_ProjectorHeight; i += m_CodeResolution) 
			{
				b = i / m_CodeResolution;
				g = Binary2Gray(b);
				on = (g >> m_GBit) & 0x1;
				
				if (m_NPMode == GrayCode::NEGATIVE) { on = 1 - on; }
				if (on) {
					glBegin(GL_POLYGON);
						glVertex3f(0.0f, i, 0.0f);
						glVertex3f(m_ProjectorWidth, i, 0.0f);
						glVertex3f(m_ProjectorWidth, i + m_CodeResolution, 0.0f);
						glVertex3f(0.0f, i + m_CodeResolution, 0.0f);
					glEnd();
				}
			}

			break;

		case GrayCode::VERT:

			// std::cout << "GrayCode Display: Vertical" << std::endl;

			for (int i = 0; i < m_ProjectorWidth; i += m_CodeResolution) 
			{
				b = i / m_CodeResolution;
				g = Binary2Gray(b);
				on = (g >> m_GBit) & 0x1;
				
				if (m_NPMode == GrayCode::NEGATIVE) { on = 1 - on; }
				if (on) {
					glBegin(GL_POLYGON);
						glVertex3f(i, 0.0f, 0.0f);
						glVertex3f(i + m_CodeResolution, 0.0f, 0.0f);
						glVertex3f(i + m_CodeResolution, m_ProjectorHeight, 0.0f);
						glVertex3f(i, m_ProjectorHeight, 0.0f);
					glEnd();
				}
			}

			break;
		}

		break;
	}

	// std::cout << "End:\tvoid GrayCode::DispCode ( int )" << std::endl;
}