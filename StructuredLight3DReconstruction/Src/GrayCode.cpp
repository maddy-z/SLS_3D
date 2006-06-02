#include <cstdio>
#include <cstdlib>
#include <cmath>

#include <iostream>
#include <windows.h>

#include <OpenGL\Glui\glui.h>
#include <OpenGL\Glut\glut.h>
#include <OpenCV_2.3.1\opencv2\opencv.hpp>

#include "GrayCode.h"
#include "SlsUtils.h"

// =====================
// Constructor & Destructor
// =====================

GrayCode::GrayCode ( int cameraW, int cameraH, int projectorW, int projectorH, bool bSlit, const char * dirname )
{
	// Code covers 1024 ( = 2 ^ m_CodeDepth ) pixels.
	// This should be greater than projector resolutions.

	m_CodeDepth			= 10;
	m_CodeResolution		= 1;						// 1

	m_SlitWidth				= 3;
	m_SlitInterval				= 16;					// 16
	m_bSlit						= bSlit;

	m_NormThres			= 70;
	m_ProjSeqNum			= 0;
		
	// skip_graycode = true;

	m_DispMode				= DISP_IDLE;
	
	m_CameraWidth		= cameraW;
	m_CameraHeight		= cameraH;
	m_ProjectorWidth		= projectorW;
	m_ProjectorHeight	= projectorH;

	m_Diff						= new unsigned char[m_CameraWidth * m_CameraHeight];
	m_White					= new unsigned char[m_CameraWidth * m_CameraHeight * 3];
	m_Black					=	new unsigned char[m_CameraWidth * m_CameraHeight * 3];
	m_bSlitImg				= new unsigned char[m_CameraWidth * m_CameraHeight];
	
	for ( int i = 0; i < 2; ++i )
	{
		m_C2P[i]				= new unsigned int[m_CameraWidth * m_CameraHeight];
		m_C2P_UC[i]			= new unsigned char[m_CameraWidth * m_CameraHeight];
		m_C2P_DB[i]			= new double[m_CameraWidth * m_CameraHeight];
	}

	m_Sum						= new double[m_CameraWidth * m_CameraHeight];
	m_Num						= new double[m_CameraWidth * m_CameraHeight];
	m_Mask						= new bool[m_CameraWidth * m_CameraHeight];

	m_Illuminance			= new unsigned char[m_CameraWidth * m_CameraHeight * 3];
	m_Compensate			= new unsigned char[m_CameraWidth * m_CameraHeight * 3];

	memset ( m_Mask, true, m_CameraWidth * m_CameraHeight );

	sprintf ( m_DirName, "%s", dirname );
}
GrayCode::~GrayCode ()
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

void GrayCode::InitDispCode ( int nProjSeq, int dispMode, int hvMode )
{
	switch ( dispMode )
	{
	case DISP_IDLE:					m_DispMode = DISP_IDLE;				break;
	case DISP_GRAYCODE:		m_DispMode = DISP_GRAYCODE;	break;
	case DISP_SLIT:					m_DispMode = DISP_SLIT;				break;
	case DISP_ILLUMI:				m_DispMode = DISP_ILLUMI;			break;
	}

	switch ( hvMode )
	{
	case HORI:							m_HVMode = HORI;						break;
	case VERT:							m_HVMode = VERT;						break;
	}

	InitDispCode ( nProjSeq );
}
void GrayCode::InitDispCode ( int nProjSeq )
{
	m_NPMode = POSITIVE;
	m_ProjSeqNum = nProjSeq;
	
	switch ( m_DispMode )
	{

	case DISP_GRAYCODE:
		
		m_GBit = m_CodeDepth - 1;

		for ( int i = 0; i < m_CameraWidth * m_CameraHeight; ++i ) {
			m_C2P[m_HVMode][i] = 0;
		}

		break;

	case DISP_SLIT:
		
		m_CurrSlitNum = 0;

		for ( int i = 0; i < m_CameraWidth * m_CameraHeight; ++i )
		{
			m_C2P_DB[m_HVMode][i] = 0.0;
			m_Sum[i] = 0.0;
			m_Num[i] = 0.0;
		}

		break;
	}
}

void GrayCode::EncodeGray2Binary ()
{
	if ( m_DispMode != GrayCode::DISP_GRAYCODE ) { return; }

	for ( int i = 0; i < m_CameraHeight * m_CameraWidth; ++i ) {
		m_C2P[m_HVMode][i] = Gray2Binary(m_C2P[m_HVMode][i]);
	}
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

void GrayCode::DispCode ()
{
	DispCode ( m_HVMode );
}
void GrayCode::DispCode ( int hv_mode )
{
	std::cout << "\nStart:\tvoid GrayCode::DispCode ( int )" << std::endl;

	unsigned int g, b, on;

	if ( hv_mode == HORI )	{ m_HVMode = HORI; }
	else if ( hv_mode == VERT ) { m_HVMode = VERT; }
	
	switch ( m_DispMode )
	{
	case DISP_GRAYCODE:	std::cout << "Disp Mode = DISP_GRAYCODE" << std::endl;		break;
	case DISP_ILLUMI:			std::cout << "Disp Mode = DISP_ILLUMI"		<< std::endl;		break;
	case DISP_SLIT:				std::cout << "Disp Mode = DISP_SLIT"			<< std::endl;		break;
	}

	switch ( m_NPMode )
	{
	case POSITIVE:				std::cout << "NPMode = POSITIVE"				<< std::endl;		break;
	case NEGATIVE:				std::cout << "NPMode = NEGATIVE"				<< std::endl;		break;
	}

	switch ( m_HVMode )
	{	
	case HORI:						std::cout << "HVMode = HORIZONTAL"			<< std::endl;		break;
	case VERT:						std::cout << "HVMode = VERTICAL"				<< std::endl;		break;
	}

	glColor3f(0.0f, 0.0f, 0.0f);

	glBegin(GL_POLYGON);
		glVertex3f(0.0f, 0.0f, 0.0f);
		glVertex3f(m_ProjectorWidth, 0.0f, 0.0f);
		glVertex3f(m_ProjectorWidth, m_ProjectorHeight, 0.0f);
		glVertex3f(0.0f, m_ProjectorHeight, 0.0f);
	glEnd();
	
	glColor3f ( 1.0f, 1.0f, 1.0f );

	switch ( m_DispMode )	
	{
	
	case GrayCode::DISP_ILLUMI:

		if ( m_NPMode == POSITIVE ) 
		{
			glBegin(GL_POLYGON);
				glVertex3f(0.0f, 0.0f, 0.0f);
				glVertex3f(m_ProjectorWidth, 0.0f, 0.0f);
				glVertex3f(m_ProjectorWidth, m_ProjectorHeight, 0.0f);
				glVertex3f(0.0f, m_ProjectorHeight, 0.0f);
			glEnd();
		}

		break;

	case GrayCode::DISP_GRAYCODE:

		std::cout << "GBit = " << m_GBit << std::endl;
		
		switch (m_HVMode)
		{

		case GrayCode::HORI:

			for (int i = 0; i < m_ProjectorHeight; i += m_CodeResolution) 
			{
				b = i / m_CodeResolution;
				g = Binary2Gray(b);
				on = (g >> m_GBit) & 0x1;
				
				if ( m_NPMode == NEGATIVE ) { on = 1 - on; }
				if ( on ) 
				{
					glBegin(GL_POLYGON);
						glVertex3f(0.0f, m_ProjectorHeight - i, 0.0f);
						glVertex3f(m_ProjectorWidth, m_ProjectorHeight - i, 0.0f);
						glVertex3f(m_ProjectorWidth, m_ProjectorHeight - (i + m_CodeResolution), 0.0f);
						glVertex3f(0.0f, m_ProjectorHeight - (i + m_CodeResolution), 0.0f);
					glEnd();
				}
			}

			break;

		case GrayCode::VERT:

			for ( int i = 0; i < m_ProjectorWidth; i += m_CodeResolution ) 
			{
				b = i / m_CodeResolution;
				g = Binary2Gray(b);
				on = (g >> m_GBit) & 0x1;
				
				if ( m_NPMode == NEGATIVE ) { on = 1 - on; }
				if ( on ) 
				{
					glBegin ( GL_POLYGON );
						glVertex3f ( i, 0.0f, 0.0f );
						glVertex3f ( i + m_CodeResolution, 0.0f, 0.0f );
						glVertex3f ( i + m_CodeResolution, m_ProjectorHeight, 0.0f );
						glVertex3f ( i, m_ProjectorHeight, 0.0f );
					glEnd();
				}
			}

			break;
		}

		break;

	case GrayCode::DISP_SLIT:

		switch (m_HVMode)
		{

		case GrayCode::HORI:

			for ( int i = m_CurrSlitNum; i < m_ProjectorHeight; i += m_SlitInterval )
			{
				glBegin(GL_POLYGON);
					glVertex3f(0.0f, i, 0.0f);
					glVertex3f(m_ProjectorWidth, i, 0.0f);
					glVertex3f(m_ProjectorWidth, i+m_SlitWidth, 0.0f);
					glVertex3f(0.0f, i+m_SlitWidth, 0.0f);
				glEnd();
			}

			break;
		
		case GrayCode::VERT:
			
			for ( int i = m_CurrSlitNum; i < m_ProjectorWidth; i += m_SlitInterval )
			{
				glBegin(GL_POLYGON);
					glVertex3f(i, 0.0f, 0.0f);
					glVertex3f(i+m_SlitWidth, 0.0f, 0.0f);
					glVertex3f(i+m_SlitWidth, m_ProjectorHeight, 0.0f);
					glVertex3f(i, m_ProjectorHeight, 0.0f);
				glEnd();
			}

			break;
		}

		break;
	
	}

	std::cout << "End:\tvoid GrayCode::DispCode ( int )\n" << std::endl;
}
void GrayCode::DispCode ( int hv_mode, int np_mode )
{
	if ( np_mode == POSITIVE ) { m_NPMode = POSITIVE; }
	else if ( np_mode == NEGATIVE ) { m_NPMode = NEGATIVE; }

	DispCode ( hv_mode );
}

void GrayCode::Binarize ( const unsigned char * color, int bin_mode )
{
	if ( m_DispMode != DISP_GRAYCODE ) { return; }
	
	for ( int i = 0; i < m_CameraWidth * m_CameraHeight; ++i, color += 3 )
	{
		int val = ( color[0] + color[1] + color[2] ) / 3;	
	
		if ( bin_mode == DIFF_MODE ) {
			val = val - (int)(m_Diff[i]);
			
			if ( val < 0 ) { m_Diff[i] = 255; }
			else { m_Diff[i] = 0; }
		}
		else {
			m_Diff[i] = val;
		}
	}

	return;
}

void GrayCode::CaptureCode ( const unsigned char * image )
{
	CaptureCode ( image, m_DispMode, m_HVMode, m_GBit );
}
void GrayCode::CaptureCode ( const unsigned char * image, int dispMode, int hvMode, unsigned int nthFrame ) 
{
	std::cout << "\nStart:\tvoid GrayCode::CaptureCode ( const unsigned char *, int, int, unsigned int )" << std::endl;

	if ( image == NULL ) { 
		std::cout << "End:\tvoid GrayCode::CaptureCode ( const unsigned char *, int, int, unsigned int )\n" << std::endl; 
		return; 
	}

	switch ( m_DispMode )
	{
	case DISP_GRAYCODE:	std::cout << "Disp Mode = DISP_GRAYCODE" << std::endl;		break;
	case DISP_ILLUMI:			std::cout << "Disp Mode = DISP_ILLUMI" << std::endl;			break;
	case DISP_SLIT:				std::cout << "Disp Mode = DISP_SLIT" << std::endl;				break;
	}

	switch ( m_NPMode )
	{
	case POSITIVE:				std::cout << "NPMode = POSITIVE"	<< std::endl;					break;
	case NEGATIVE:				std::cout << "NPMode = NEGATIVE"	<< std::endl;					break;
	}

	switch ( m_HVMode )
	{
	case HORI:						std::cout << "HVMode = HORIZONTAL" << std::endl;				break;
	case VERT:						std::cout << "HVMode = VERTICAL"		<< std::endl;				break;
	}

	if ( dispMode == GrayCode::DISP_GRAYCODE )
	{
		std::cout << "GBit = " << m_GBit << std::endl;

		if ( m_NPMode == POSITIVE ) {
			Binarize ( image, GrayCode::AVG_MODE );
		}
		else if ( m_NPMode == NEGATIVE ) {
			Binarize ( image, GrayCode::DIFF_MODE );
			
			for ( int i = 0; i < m_CameraHeight * m_CameraWidth; ++i ) {
				// if ( m_Diff[i] ) { m_C2P[m_HVMode][i] |= (1 << m_GBit); }
				if ( m_Diff[i] ) { m_C2P[hvMode][i] |= (1 << nthFrame); }
			}
		}
	}
	else if ( dispMode == GrayCode::DISP_ILLUMI )
	{
		const unsigned char * srcDataPtr = NULL;
		char fileName[256];

		if ( m_NPMode == POSITIVE ) 
		{
			memcpy ( m_White, image, m_CameraWidth * m_CameraHeight * 3 );
			sprintf ( fileName, "%s/White_%dth.bmp", m_DirName, m_ProjSeqNum );
			
			srcDataPtr = m_White;
		}
		else if ( m_NPMode == NEGATIVE ) 
		{
			memcpy ( m_Black, image, m_CameraWidth * m_CameraHeight * 3 );
			sprintf ( fileName, "%s/Black_%dth.bmp", m_DirName, m_ProjSeqNum );

			srcDataPtr = m_Black;
		}
		else { 
			return; 
		}
		
		cv::Mat saveDest(m_CameraHeight, m_CameraWidth, CV_8UC3);
		unsigned char * saveDestRowStart = saveDest.data;
		unsigned char * saveDestDataPtr = NULL;

		for ( int i = 0; i < m_CameraHeight; ++i, saveDestRowStart += saveDest.step ) 
		{
			saveDestDataPtr = saveDestRowStart;

			for ( int j = 0; j < m_CameraWidth; ++j, saveDestDataPtr += 3, srcDataPtr += 3 )
			{
				saveDestDataPtr[0] = srcDataPtr[0];
				saveDestDataPtr[1] = srcDataPtr[1];
				saveDestDataPtr[2] = srcDataPtr[2];
			}
		}
			
		cv::imwrite(fileName, saveDest);
	}
	else if ( dispMode == GrayCode::DISP_SLIT )
	{
		// TODO:
	}
	else {
		printf("Invalid Disp Mode\n");
		return;
	}

	std::cout << "End:\tvoid GrayCode::CaptureCode ( const unsigned char *, int, int, unsigned int )\n" << std::endl;
}

bool GrayCode::GetNextFrame () 
{
	switch ( m_DispMode ) 
	{
	
	case DISP_GRAYCODE:
		
		switch ( m_NPMode )
		{
		case POSITIVE:	
			
			m_NPMode = NEGATIVE; 
			break;

		case NEGATIVE:
			
			m_NPMode = POSITIVE;
			m_GBit = m_GBit - 1;
			if ( m_GBit < 0 ) 
			{
				m_GBit = m_CodeDepth - 1;
				// m_DispMode = GrayCode::DISP_IDLE;
				
				return false;		
			}

			break;
		}

		break;

	case DISP_ILLUMI:
		
		switch ( m_NPMode )
		{
		case POSITIVE:		m_NPMode = NEGATIVE;	break;
		case NEGATIVE:		m_NPMode = POSITIVE;		return false;
		}
		
		break;

	case DISP_SLIT:

		// TODO:
		return false;
		break;
	}

	return true;
}

bool GrayCode::SaveCurrFrame ()
{
	if ( m_DispMode == GrayCode::DISP_GRAYCODE ) 
	{
		if ( m_Diff == NULL ) { return false; }
		if ( m_NPMode == NEGATIVE ) 
		{
			cv::Mat gcImg ( m_CameraHeight, m_CameraWidth, CV_8UC1 );
			CopyRawImageBuf2CvMat ( m_Diff, 1, gcImg );

			char fileName[128];
		
			if ( m_HVMode == VERT ) { sprintf ( fileName, "%s/GrayCode_VERT_%dth.bmp", m_DirName, m_GBit ); }
			else if ( m_HVMode == HORI ) { sprintf ( fileName, "%s/GrayCode_HORI_%dth.bmp", m_DirName, m_GBit ); }
			
			cv::imwrite ( fileName, gcImg );

			return true;
		}
		else if ( m_NPMode == GrayCode::POSITIVE ) {
			return false;
		}
		else {
			return false;
		}
	}

	return false;
}

// 
// Get Spatial Code
// 

double GrayCode::dblCode ( int hv_mode, double cx, double cy )
{
	int ix, iy;
	int c00, c01, c10, c11;

	double fx, fy;
	double c0, c1;

	ix = (int)(cx);
	iy = (int)(cy);

	if ( ix == m_CameraWidth || iy == m_CameraHeight ) {
		return 0.0f;
		// return m_C2P[hv_mode][iy * m_CameraWidth + ix];
	}

	fx = cx - ix;
	fy = cy - iy;

	if ( m_bSlit )
	{
		c00 = m_C2P_DB[hv_mode][(iy) * m_CameraWidth + ix];
		c01 = m_C2P_DB[hv_mode][(iy+1) * m_CameraWidth + ix];
		c10 = m_C2P_DB[hv_mode][(iy) * m_CameraWidth + (ix+1)];
		c11 = m_C2P_DB[hv_mode][(iy+1) * m_CameraWidth + (ix+1)];
	}
	else
	{
		c00 = m_C2P[hv_mode][iy * m_CameraWidth + ix];
		c01 = m_C2P[hv_mode][(iy+1) * m_CameraWidth + ix];
		c10 = m_C2P[hv_mode][iy * m_CameraWidth + (ix+1)];
		c11 = m_C2P[hv_mode][(iy+1) * m_CameraWidth + (ix+1)];
	}

	c0 = c00 * (1-fy) + c01 * fy;
	c1 = c10 * (1-fy) + c11 * fy;

	return (c0 * (1-fx) + c1 * fx);
}
void GrayCode::Mask ( int threshold )
{
	for ( int i = 0; i < m_CameraWidth * m_CameraHeight; ++i )
	{
		int avgWhite = ( m_White[i*3+0] + m_White[i*3+1] + m_White[i*3+2] ) / 3;
		int avgBlack = ( m_Black[i*3+0] + m_Black[i*3+1] + m_Black[i*3+2] ) / 3;
		
		if ( abs( avgWhite - avgBlack ) > threshold ) { m_Mask[i] = true; }
		else { m_Mask[i] = false; }
	}
}

void GrayCode::WriteC2P ()
{
	for ( int i = 0; i < m_CameraWidth * m_CameraHeight; ++i )
	{	
		if ( m_bSlit )																	// m_C2P_DB -> Gray Scale Images
		{
			*(m_C2P_UC[VERT] + i) = m_C2P_DB[VERT][i];
			*(m_C2P_UC[HORI] + i) = m_C2P_DB[HORI][i];
		}
		else																				// m_C2P -> Gray Scale Images
		{
			m_C2P_DB[VERT][i] = m_C2P[VERT][i];
			m_C2P_DB[HORI][i] = m_C2P[HORI][i];
			
			*(m_C2P_UC[VERT] + i) = (float)(m_C2P[VERT][i]);
			*(m_C2P_UC[HORI] + i) = (float)(m_C2P[HORI][i]);
		}
	}

	char filename[256];

	FILE * fpVert;
	FILE * fpHori;
	
	sprintf ( filename, "%s/_C2P_VERT_%dth.txt", m_DirName, m_ProjSeqNum);	fpVert = fopen ( filename, "w" );
	sprintf ( filename, "%s/_C2P_HORI_%dth.txt", m_DirName, m_ProjSeqNum);	fpHori = fopen ( filename, "w" );
	
	for ( int i = 0; i < m_CameraWidth * m_CameraHeight; ++i ) 
	{		
		fprintf(fpVert,"%.15f\n", m_C2P_DB[VERT][i]);
		fprintf(fpHori,"%.15f\n", m_C2P_DB[HORI][i]);
	}

	return;
}

int GrayCode::ShowProjectorIlluminace ()
{
	cv::Mat pIllumi ( m_ProjectorHeight, m_ProjectorWidth, CV_8UC3 );
	
	unsigned char * pDestRowStart = pIllumi.data;
	unsigned int srcP = 0;

	for ( int i = 0; i < m_CameraHeight * m_CameraWidth; ++i ) 
	{
		int pX = m_C2P[VERT][i];
		int pY = m_ProjectorHeight - m_C2P[HORI][i];
			
		pDestRowStart[pY * pIllumi.step + pX * 3]			= 255;
		pDestRowStart[pY * pIllumi.step + pX * 3 + 1]		= 255;
		pDestRowStart[pY * pIllumi.step + pX * 3 + 2]		= 255;
			
		// ilP->imageData[(pY*_projectorW+pX)*3]		= 255;
		// ilP->imageData[(pY*_projectorW+pX)*3+1]	= 255;
		// ilP->imageData[(pY*_projectorW+pX)*3+2]	= 255;
	}

	cv::namedWindow ( "Showing Projector Illumniation", CV_WINDOW_AUTOSIZE );
	cv::imshow ( "Showing Projector Illumniation", pIllumi );

	return 0;
}