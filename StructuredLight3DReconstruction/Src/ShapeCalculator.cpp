#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <cmath>

#include <OpenCV_2.3.1\opencv2\opencv.hpp>

#include "ShapeCalculator.h"

ShapeCalculator::
ShapeCalculator (	int cameraW, 
							int cameraH, 
							int projectorW, 
							int projectorH )
{
	m_CameraWidth = cameraW;
	m_CameraHeight = cameraH;
	m_ProjectorWidth = projectorW;
	m_ProjectorHeight = projectorH;

	m_Pos2d = NULL;
	m_Pos3d = NULL;
	
	m_ValidPixNum = 0;
}

ShapeCalculator::
~ShapeCalculator()
{
	if ( m_Pos2d ) { delete [] m_Pos2d; }
	if ( m_Pos3d ) { delete [] m_Pos3d; }
}

void 
ShapeCalculator::CalcShape ( double * C2P, 
											bool * mask, 
											const cv::Mat & camIntr, 
											const cv::Mat & camExtr, 
											const cv::Mat & proIntr, 
											const cv::Mat & proExtr )
{
	cv::Mat F ( 3, 1, CV_64FC1 );
	cv::Mat Q ( 3, 3, CV_64FC1 );
	cv::Mat invQ ( 3, 3, CV_64FC1 );
	cv::Mat matPos3d ( 3, 1, CV_64FC1 );
	cv::Mat camProj ( 3, 4, CV_64FC1 );
	cv::Mat proProj ( 3, 4, CV_64FC1 );
	
	camProj = camIntr * camExtr;
	proProj = proIntr * proExtr;
	
	m_ValidPixNum = 0;
	float * tmpPos3d = new float[m_CameraWidth * m_CameraHeight * 3];

	// 
	// Calc Shape
	// 

	for ( int y = 0; y < m_CameraHeight; ++y )
	for ( int x = 0; x < m_CameraWidth; ++x )
	{
		int pos = x + y * m_CameraWidth;
		
		if ( mask[pos] && C2P[pos] )
		{
			F.at<double>(0, 0) = x * camProj.at<double>(2, 3) - camProj.at<double>(0, 3);
			F.at<double>(1, 0) = y * camProj.at<double>(2, 3) - camProj.at<double>(1, 3);
			// F.at<double>(2, 0) = C2P[pos] * proProj.at<double>(2, 3) - proProj.at<double>(1, 3);
			F.at<double>(2, 0) = C2P[pos] * proProj.at<double>(2, 3) - proProj.at<double>(0, 3);
			
			// cvmSet( F, 0, 0, x*cvmGet(cam_proj,2,3) - cvmGet(cam_proj,0,3) );
			// cvmSet( F, 1, 0, y*cvmGet(cam_proj,2,3) - cvmGet(cam_proj,1,3) );
			// cvmSet( F, 2, 0, C2P[pos]*cvmGet(pro_proj,2,3) - cvmGet(pro_proj,1,3) );

			Q.at<double>(0, 0) = camProj.at<double>(0, 0) - x * camProj.at<double>(2, 0);
			Q.at<double>(0, 1) = camProj.at<double>(0, 1) - x * camProj.at<double>(2, 1);
			Q.at<double>(0, 2) = camProj.at<double>(0, 2) - x * camProj.at<double>(2, 2);
			Q.at<double>(1, 0) = camProj.at<double>(1, 0) - y * camProj.at<double>(2, 0);
			Q.at<double>(1, 1) = camProj.at<double>(1, 1) - y * camProj.at<double>(2, 1);
			Q.at<double>(1, 2) = camProj.at<double>(1, 2) - y * camProj.at<double>(2, 2);
			// Q.at<double>(2, 0) = proProj.at<double>(1, 0) - C2P[pos] * proProj.at<double>(2, 0);
			// Q.at<double>(2, 1) = proProj.at<double>(1, 1) - C2P[pos] * proProj.at<double>(2, 1);
			// Q.at<double>(2, 2) = proProj.at<double>(1, 2) - C2P[pos] * proProj.at<double>(2, 2);
			Q.at<double>(2, 0) = proProj.at<double>(0, 0) - C2P[pos] * proProj.at<double>(2, 0);
			Q.at<double>(2, 1) = proProj.at<double>(0, 1) - C2P[pos] * proProj.at<double>(2, 1);
			Q.at<double>(2, 2) = proProj.at<double>(0, 2) - C2P[pos] * proProj.at<double>(2, 2);
	
			invQ = Q.inv(cv::DECOMP_LU);
			matPos3d = invQ * F;

			m_ValidPixNum++;
			
			tmpPos3d[pos*3+0] = matPos3d.at<double>(0, 0);
			tmpPos3d[pos*3+1] = matPos3d.at<double>(1, 0);
			tmpPos3d[pos*3+2] = matPos3d.at<double>(2, 0);
		}
	}

	if ( m_Pos3d ) { delete [] m_Pos3d; }
	if ( m_Pos2d ) { delete [] m_Pos2d; }
	
	m_Pos3d = new float[m_ValidPixNum * 3];
	m_Pos2d = new int[m_ValidPixNum * 2];
	
	int num = 0;

	for ( int y = 0; y < m_CameraHeight; ++y )
	for ( int x = 0; x < m_CameraWidth; ++x )
	{
		int pos = x + y * m_CameraWidth;
		
		if ( mask[pos] && C2P[pos] )
		{
			m_Pos3d[num*3+0] = tmpPos3d[pos*3+0];
			m_Pos3d[num*3+1] = tmpPos3d[pos*3+1];
			m_Pos3d[num*3+2] = tmpPos3d[pos*3+2];
			
			m_Pos2d[num*2+0] = x;
			m_Pos2d[num*2+1] = y;
	
			++num;
		}
	}

	assert ( num == m_ValidPixNum );

	if ( tmpPos3d ) { delete [] tmpPos3d; }

	return;
}

void ShapeCalculator::CalcShape (	double *C2P_H,
													double *C2P_V,
													bool * mask, 
													const cv::Mat & camIntr, 
													const cv::Mat & camExtr, 
													const cv::Mat & proIntr, 
													const cv::Mat & proExtr )
{
	m_ValidPixNum = 0;

	for ( int i = 0; i < m_CameraHeight; ++i ) 
	for ( int j = 0; j < m_CameraWidth; ++j ) 
	{
		int pos = i * m_CameraWidth + j;
		
		if (	C2P_H[pos] >= 0.0f && C2P_H[pos] < m_ProjectorHeight &&
				C2P_V[pos] >= 0.0f && C2P_V[pos] < m_ProjectorWidth && 
				mask[pos] ) 
		{ 
			++m_ValidPixNum; 
		}
	}

	CvMat * camPts = cvCreateMat ( 2, m_ValidPixNum, CV_64FC1 );
	CvMat * proPts = cvCreateMat ( 2, m_ValidPixNum, CV_64FC1 );
	CvMat * posPts = cvCreateMat ( 4, m_ValidPixNum, CV_64FC1 );

	int count = 0;
	
	for ( int y = 0; y < m_CameraHeight; ++y ) 
	for ( int x = 0; x < m_CameraWidth; ++x )
	{
		int pos = y * m_CameraWidth + x;

		if (	C2P_H[pos] < 0.0f || C2P_H[pos] >= m_ProjectorHeight 
			||	C2P_V[pos] < 0.0f || C2P_V[pos] >= m_ProjectorWidth 
			||	! ( mask[pos] ) ) 
		{ 
			continue; 
		}

		cvmSet ( camPts, 0, count, x );
		cvmSet ( camPts, 1, count, y );

		cvmSet ( proPts, 0, count, C2P_V[pos] );
		cvmSet ( proPts, 1, count, C2P_H[pos] );

		++count;
	}

	assert ( count == m_ValidPixNum );

	cv::Mat camProj = camIntr * camExtr;
	cv::Mat proProj = proIntr * proExtr;

	CvMat * cProj = cvCreateMat(3, 4, CV_64FC1);
	CvMat * pProj = cvCreateMat(3, 4, CV_64FC1);
	
	for ( int i = 0; i < 3; ++i )
	for ( int j = 0; j < 4; ++j ) {
		cvmSet ( cProj, i, j, camProj.at<double>(i, j) );
		cvmSet ( pProj, i, j, proProj.at<double>(i, j) );
	}

	// 
	// Calculate Triangle 3d Points
	// 

	cvTriangulatePoints ( cProj, pProj, camPts, proPts, posPts );

	if ( m_Pos3d ) { delete [] m_Pos3d; }
	if ( m_Pos2d ) { delete [] m_Pos2d; }
	
	m_Pos3d = new float[m_ValidPixNum * 3];
	m_Pos2d = new int[m_ValidPixNum * 2];
	
	int num = 0;

	for ( int y = 0; y < m_CameraHeight; ++y )
	for ( int x = 0; x < m_CameraWidth; ++x )
	{
		int pos = x + y * m_CameraWidth;
		
		if (	C2P_H[pos] >= 0.0f && C2P_H[pos] < m_ProjectorHeight &&
				C2P_V[pos] >= 0.0f && C2P_V[pos] < m_ProjectorWidth && 
				mask[pos] )
		{
			double z = cvmGet(posPts, 3, num);

			m_Pos3d[num*3+0] = cvmGet(posPts, 0, num) / z;
			m_Pos3d[num*3+1] = cvmGet(posPts, 1, num) / z;
			m_Pos3d[num*3+2] = cvmGet(posPts, 2, num) / z;
			
			m_Pos2d[num*2+0] = x;
			m_Pos2d[num*2+1] = y;
	
			++num;
		}
	}

	assert ( num == m_ValidPixNum );

	cvReleaseMat ( &camPts );
	cvReleaseMat ( &proPts );
	cvReleaseMat ( &posPts );
	cvReleaseMat ( &cProj );
	cvReleaseMat ( &pProj );

	return;
}

void ShapeCalculator::WriteOBJ ( const char * fileName )
{
	/*
	*	Initialize Buffers
	*/

	float * tmpVertex = new float[m_CameraWidth * m_CameraHeight * 3];
	bool * inlier = new bool[m_CameraWidth * m_CameraHeight];

	for ( int i = 0; i < m_CameraWidth * m_CameraHeight; ++i ) 
	{
		tmpVertex[i*3+0] = 0.0;
		tmpVertex[i*3+1] = 0.0;
		tmpVertex[i*3+2] = 0.0;
		
		inlier[i] = false;
	}
	for ( int i = 0; i < m_ValidPixNum; ++i ) 
	{
		int pos = m_Pos2d[i*2+0] + m_CameraWidth * m_Pos2d[i*2+1];
		
		tmpVertex[pos*3 + 0] = m_Pos3d[i*3 + 0];
		tmpVertex[pos*3 + 1] = m_Pos3d[i*3 + 1];
		tmpVertex[pos*3 + 2] = m_Pos3d[i*3 + 2];
		
		inlier[pos] = true;
	}

	/*
	*	Remove inliers ( Filtered by distance )
	*	
	*	1. Average 3d position data of the neighboring pixels of a focused pixel
	*	2. If the distance between the average point and the focused pixel is larger than a threshold, remove it as an outlier
	*		(neighboring pixels are choosen in camera's 2d coordinate system)
	*/

    for ( int y = 1; y < m_CameraHeight - 1; ++y )
	for ( int x = 1; x < m_CameraWidth - 1; ++x )
	{
		double meanx = 0.0f, meany = 0.0f, meanz = 0.0f, dist;
		
		int num = 0;
		int pos = x + y * m_CameraWidth;
		
		if ( ! inlier[pos] ) { continue; }
		
		for ( int dy = -1; dy <= 1; ++dy )
		for ( int dx = -1; dx <= 1; ++dx )
		{
			int posd = (x+dx) + m_CameraWidth * (y+dy);
	
			if ( inlier[posd] )
			{
				meanx += tmpVertex[posd * 3 + 0];
				meany += tmpVertex[posd * 3 + 1];
				meanz += tmpVertex[posd * 3 + 2];

				++num;
			}
		}
		
		meanx /= (double)(num);
		meany /= (double)(num);
		meanz /= (double)(num);

		dist = sqrt (	( meanx - tmpVertex[0+pos*3] ) * ( meanx - tmpVertex[0+pos*3] ) +
							( meany - tmpVertex[1+pos*3] ) * ( meany - tmpVertex[1+pos*3] ) +
							( meanz - tmpVertex[2+pos*3] ) * ( meanz - tmpVertex[2+pos*3] ) );
		
		if ( dist > 10 ) { inlier[pos] = false;	}				// Threshold = 10 ... hardcoded, but it works quite well all the time !
	}

	/*
	*	Store Valid Pixels Only
	*/

	int vertexNum = 0;
	
	for ( int i = 0; i < m_CameraWidth * m_CameraHeight; ++i ) {
		if ( inlier[i] ) { vertexNum++; }
	}

	float * vertex = new float[vertexNum*3];
	int * index = new int[m_CameraWidth * m_CameraHeight];
	int * invIndex = new int[vertexNum];

	int num = 0;

    for ( int i = 0; i < m_CameraWidth * m_CameraHeight; ++i )
	{
		if ( inlier[i] )
		{
			vertex[num*3+0] = tmpVertex[i*3+0];
			vertex[num*3+1] = tmpVertex[i*3+1];
			vertex[num*3+2] = tmpVertex[i*3+2];
			
			index[i] = num+1;
			invIndex[num] = i;
			
			++num;
		}
	}

	assert ( num == vertexNum );

	/*
	*	Triangluration based on camera's 2d coordinate system
	*/

	int triangleNum = 0;
	
	for ( int y = 0; y < m_CameraHeight - 1; ++y )
	for ( int x = 0; x < m_CameraWidth - 1; ++x )
	{
		int pos[3];

		pos[0] = x + m_CameraWidth * y;
		pos[1] = (x+1) + m_CameraWidth * y;
		pos[2] = x + m_CameraWidth * (y+1);
		
		if ( inlier[pos[0]] && inlier[pos[1]] && inlier[pos[2]] ) {
			triangleNum++;
		}

		pos[0] = (x+1) + m_CameraWidth * (y+1);
		pos[1] = x + m_CameraWidth * (y+1);
		pos[2] = (x+1) + m_CameraWidth * y;
		
		if ( inlier[pos[0]] && inlier[pos[1]] && inlier[pos[2]] ) {
			triangleNum++;
		}
	}

	int * triangle = new int[triangleNum * 3];

	int t = 0;

	for ( int y = 0; y < m_CameraHeight-1; ++y )
	for ( int x = 0; x < m_CameraWidth-1; ++x )
	{
		int pos[3];

		pos[0] = x + m_CameraWidth * y;
		pos[1] = (x+1) + m_CameraWidth * y;
		pos[2] = x + m_CameraWidth * (y+1);

		if ( inlier[pos[0]] && inlier[pos[1]] && inlier[pos[2]] )
		{
			triangle[t*3+0] = index[pos[0]];
			triangle[t*3+1] = index[pos[1]];
			triangle[t*3+2] = index[pos[2]];
			
			t++;
		}
		
		pos[0] = (x+1) + m_CameraWidth * (y+1);
		pos[1] = (x) + m_CameraWidth * (y+1);
		pos[2] = (x+1) + m_CameraWidth * (y);

		if ( inlier[pos[0]] && inlier[pos[1]] && inlier[pos[2]] )
		{
			triangle[t*3+0] = index[pos[0]];
			triangle[t*3+1] = index[pos[1]];
			triangle[t*3+2] = index[pos[2]];
			
			t++;
		}
	}

	/*
	*	Save
	*/

	FILE * fp;

	fp = fopen ( fileName, "w" );
	fprintf ( fp,"# Wavefront OBJ generated by Daisuke Iwai\n\n" );
	fprintf ( fp,"# %d Vertexes\n", vertexNum );

	for ( int i = 0; i < vertexNum; ++i ) {
		fprintf ( fp,"v %f %f %f\n", vertex[i*3+0], vertex[i*3+1], vertex[i*3+2] );
	}

	fprintf ( fp,"\n# Texture coord\n" );
	
	for ( int i = 0; i < vertexNum; ++i )
	{
		float x = (float)( invIndex[i] % m_CameraWidth ) / (float)( m_CameraWidth );
		//	float y = (_cameraH-1) - (float)(inv_index[i]/_cameraW) / (float)_cameraH;
		float y = ( (m_CameraHeight-1) - (float)( invIndex[i] / m_CameraWidth ) ) / (float)( m_CameraHeight );
		
		fprintf (fp, "vt %f %f\n", x, y );
	}

	fprintf ( fp,"\n# 1 groups\n");
	fprintf ( fp,"# %d faces (triangles)\n\n", triangleNum);
	fprintf ( fp,"g group\n" );													// group name

	for ( int i = 0; i < triangleNum; ++i ) {
		fprintf ( fp, "f %d %d %d\n", triangle[i*3+0], triangle[i*3+1], triangle[i*3+2] );
	}
	fprintf ( fp, "\n" );

	fclose(fp);

	delete[] tmpVertex;
	delete[] inlier;
	delete[] index;
	delete[] vertex;
	delete[] triangle;

	return;
}