#include "CalcShape.h"

#include	<OpenCV_2.3.1\opencv2\opencv.hpp>
#include	<OpenCV_2.3.1\opencv\cv.h>

#include	<OpenGL\Glui\glui.h>
#include	<OpenGL\Glut\glut.h>

CCalcShape::CCalcShape ( int cameraW, int cameraH, int projectorW, int projectorH )
{
	_cameraW = cameraW;
	_cameraH = cameraH;
	_projectorW = projectorW;
	_projectorH = projectorH;
	_pos3d = new float[1];
	_pos2d = new int[1];
}

CCalcShape::~CCalcShape()
{
	delete[] _pos3d;
	delete[] _pos2d;
}

void 
CCalcShape::CalcShape( double *C2P, bool *mask, CvMat *cam_intr, CvMat* cam_extr, CvMat* pro_intr, CvMat *pro_extr )
{
	CvMat *F = cvCreateMat( 3, 1, CV_64F );
	CvMat *Q = cvCreateMat( 3, 3, CV_64F );
	CvMat *invQ = cvCreateMat( 3, 3, CV_64F );
	CvMat *matPos3d = cvCreateMat( 3, 1, CV_64F );
	CvMat *cam_proj = cvCreateMat(3, 4, CV_64F);
	CvMat *pro_proj = cvCreateMat(3, 4, CV_64F);

	cvmMul( cam_intr, cam_extr, cam_proj );
	cvmMul( pro_intr, pro_extr, pro_proj );

	_validPixNum = 0;
	float* tmpPos3d = new float[_cameraW*_cameraH*3];

	/*
	*	calc shape based on "??"
	*/

	for( int y = 0; y < _cameraH; y++ )
	for( int x = 0; x < _cameraW; x++ )
	{
		int pos = x+y*_cameraW;
		if( mask[pos] && C2P[pos] )
		{
			cvmSet( F, 0, 0, x*cvmGet(cam_proj,2,3)-cvmGet(cam_proj,0,3) );
			cvmSet( F, 1, 0, y*cvmGet(cam_proj,2,3)-cvmGet(cam_proj,1,3) );
			cvmSet( F, 2, 0, C2P[pos]*cvmGet(pro_proj,2,3)-cvmGet(pro_proj,1,3) );

			cvmSet( Q, 0, 0, cvmGet(cam_proj,0,0)-x*cvmGet(cam_proj,2,0) );
			cvmSet( Q, 0, 1, cvmGet(cam_proj,0,1)-x*cvmGet(cam_proj,2,1) );
			cvmSet( Q, 0, 2, cvmGet(cam_proj,0,2)-x*cvmGet(cam_proj,2,2) );
			cvmSet( Q, 1, 0, cvmGet(cam_proj,1,0)-y*cvmGet(cam_proj,2,0) );
			cvmSet( Q, 1, 1, cvmGet(cam_proj,1,1)-y*cvmGet(cam_proj,2,1) ); 
			cvmSet( Q, 1, 2, cvmGet(cam_proj,1,2)-y*cvmGet(cam_proj,2,2) );
			cvmSet( Q, 2, 0, cvmGet(pro_proj,1,0)-C2P[pos]*cvmGet(pro_proj,2,0) );
			cvmSet( Q, 2, 1, cvmGet(pro_proj,1,1)-C2P[pos]*cvmGet(pro_proj,2,1) );
			cvmSet( Q, 2, 2, cvmGet(pro_proj,1,2)-C2P[pos]*cvmGet(pro_proj,2,2) );

			cvInv( Q, invQ, CV_LU );
			cvmMul( invQ, F, matPos3d );

			_validPixNum++;
			
			tmpPos3d[pos*3+0] = cvmGet(matPos3d,0,0);
			tmpPos3d[pos*3+1] = cvmGet(matPos3d,1,0);
			tmpPos3d[pos*3+2] = cvmGet(matPos3d,2,0);
		}
	}

	cvReleaseMat( &F );
	cvReleaseMat( &Q );
	cvReleaseMat( &invQ );
	cvReleaseMat( &matPos3d );

	/*
	*	store shape data
	*		-> _pos3d: 3d data
	*		-> _pos2d: corresponding 2d data (position in camera coord)
	*/

	delete[] _pos3d;
	delete[] _pos2d;
	_pos3d = new float[_validPixNum*3];
	_pos2d = new int[_validPixNum*2];
	int num = 0;
	for( int y = 0; y < _cameraH; y++ )
	for( int x = 0; x < _cameraW; x++ )
	{
		int pos = x+y*_cameraW;
		if( mask[pos] && C2P[pos] )
		{
			_pos3d[num*3+0] = tmpPos3d[pos*3+0];
			_pos3d[num*3+1] = tmpPos3d[pos*3+1];
			_pos3d[num*3+2] = tmpPos3d[pos*3+2];
			_pos2d[num*2+0] = x;
			_pos2d[num*2+1] = y;
			num++;
		}
	}
}

void 
CCalcShape::writeOBJ( char *fname )
{
	/*
	*	init buffers
	*/

	float *tmp_vertex = new float[_cameraW * _cameraH * 3];
	bool *inlier = new bool[_cameraW * _cameraH];

	for( int i = 0; i < _cameraW*_cameraH; i++ )
	{
		tmp_vertex[0+i*3] = 0.0;
		tmp_vertex[1+i*3] = 0.0;
		tmp_vertex[2+i*3] = 0.0;
		inlier[i] = false;
	}
	for( int i = 0; i < _validPixNum; i++ )
	{
		int pos = _pos2d[i*2+0]+_cameraW*_pos2d[i*2+1];
		tmp_vertex[ 0+pos*3 ] = _pos3d[i*3+0];
		tmp_vertex[ 1+pos*3 ] = _pos3d[i*3+1];
		tmp_vertex[ 2+pos*3 ] = _pos3d[i*3+2];
		inlier[ pos ] = true;
	}

	/*
	*	remove inliers (filtered by distance)
	*		1. average 3d position data of the neighboring pixels of a focused pixel
	*		2. if the distance between the average point and the focused pixel is larger than a threshold, remove it as an outlier
	*		(neighboring pixels are choosen in camera's 2d coordinate system)
	*/

    for( int y = 1; y < _cameraH-1; y++ )
	for( int x = 1; x < _cameraW-1; x++)
	{
		double meanx = 0, meany = 0, meanz = 0, dist;
		int num = 0;
		int pos = x+_cameraW*y;
		if( !inlier[ pos ] )	continue;
		for( int dy = -1; dy <= 1; dy++ )
		for( int dx = -1; dx <= 1; dx++ )
		{
			int posd = (x+dx)+_cameraW*(y+dy);
			if( inlier[ posd ] )
			{
				meanx += tmp_vertex[ 0+posd*3 ];
				meany += tmp_vertex[ 1+posd*3 ];
				meanz += tmp_vertex[ 2+posd*3 ];
				num++;
			}
		}
		meanx /= (double)num;
		meany /= (double)num;
		meanz /= (double)num;
		dist = sqrt(( meanx - tmp_vertex[0+pos*3] ) * ( meanx - tmp_vertex[0+pos*3] ) +
					( meany - tmp_vertex[1+pos*3] ) * ( meany - tmp_vertex[1+pos*3] ) +
					( meanz - tmp_vertex[2+pos*3] ) * ( meanz - tmp_vertex[2+pos*3] ) );
		if( dist > 10 )	inlier[pos] = false;	// threshold = 10 ... hardcoded, but it works quite well all the time
	}

	/*
	*	store valid pixels only
	*/
	int vertexNum = 0;
	for( int i = 0; i < _cameraW*_cameraH; i++ )
	{
		if( inlier[i] )	vertexNum++;
	}

	float *vertex = new float[vertexNum*3];
	int *index = new int[_cameraW*_cameraH];
	int *inv_index = new int[vertexNum];

	int num=0;
    for( int i = 0; i < _cameraW*_cameraH; i++ )
	{
		if( inlier[i] )
		{
			vertex[num*3+0] = tmp_vertex[i*3+0];
			vertex[num*3+1] = tmp_vertex[i*3+1];
			vertex[num*3+2] = tmp_vertex[i*3+2];
			index[i] = num+1;
			inv_index[num] = i;
			num++;
		}
	}

	/*
	*	triangluration based on camera's 2d coordinate system
	*/

	int triangleNum = 0;
	for( int y = 0; y < _cameraH-1; y++)
	for( int x = 0; x < _cameraW-1; x++)
	{
		int pos[3];
		pos[0] = x+_cameraW*y;
		pos[1] = (x+1)+_cameraW*y;
		pos[2] = x+_cameraW*(y+1);
		if( inlier[ pos[0] ] && inlier[ pos[1] ] && inlier[ pos[2] ] )
		{
			triangleNum++;
		}
		pos[0] = (x+1)+_cameraW*(y+1);
		pos[1] = x+_cameraW*(y+1);
		pos[2] = (x+1)+_cameraW*y;
		if( inlier[ pos[0] ] && inlier[ pos[1] ] && inlier[ pos[2] ] )
		{
			triangleNum++;
		}
	}

	int *triangle = new int[triangleNum*3];

	int t=0;
	for( int y = 0; y < _cameraH-1; y++)
	for( int x = 0; x < _cameraW-1; x++)
	{
		int pos[3];
		pos[0] = x+_cameraW*y;
		pos[1] = (x+1)+_cameraW*y;
		pos[2] = x+_cameraW*(y+1);
		if( inlier[ pos[0] ] && inlier[ pos[1] ] && inlier[ pos[2] ] )
		{
			triangle[t*3+0] = index[pos[0]];
			triangle[t*3+1] = index[pos[1]];
			triangle[t*3+2] = index[pos[2]];
			t++;
		}
		pos[0] = (x+1)+_cameraW*(y+1);
		pos[1] = x+_cameraW*(y+1);
		pos[2] = (x+1)+_cameraW*y;
		if( inlier[ pos[0] ] && inlier[ pos[1] ] && inlier[ pos[2] ] )
		{
			triangle[t*3+0] = index[pos[0]];
			triangle[t*3+1] = index[pos[1]];
			triangle[t*3+2] = index[pos[2]];
			t++;
		}
	}

	/*
	*	save
	*/

	FILE *fp;

	fp = fopen( fname, "w" );
	fprintf(fp,"# Wavefront OBJ generated by Daisuke Iwai\n\n");
	fprintf(fp,"# %d vertecies\n",vertexNum);

	for( int i = 0; i < vertexNum; i++ )
	{
		fprintf(fp,"v %f %f %f\n",vertex[i*3+0],vertex[i*3+1],vertex[i*3+2]);
	}

	fprintf(fp,"\n# texture coord\n");
	for( int i = 0; i < vertexNum; i++ )
	{
		float x = (float)(inv_index[i]%_cameraW) / (float)_cameraW;
	//	float y = (_cameraH-1) - (float)(inv_index[i]/_cameraW) / (float)_cameraH;
		float y = ( (_cameraH-1) - (float)(inv_index[i]/_cameraW) ) / (float)_cameraH;
		fprintf(fp,"vt %f %f\n",x,y);
	}

	fprintf(fp,"\n# 1 groups\n");
	fprintf(fp,"# %d faces (triangles)\n\n",triangleNum);
	// group name
	fprintf(fp,"g group\n");

	for( int i = 0; i < triangleNum; i++ )
	{
		fprintf(fp,"f %d %d %d\n",
			triangle[i*3+0],triangle[i*3+1],triangle[i*3+2]);
	}
	fprintf(fp,"\n");

	fclose(fp);

	delete[] tmp_vertex;
	delete[] inlier;
	delete[] index;
	delete[] vertex;
	delete[] triangle;
}
