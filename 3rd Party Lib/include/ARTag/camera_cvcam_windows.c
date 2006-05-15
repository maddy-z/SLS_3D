//my wrapper around OpenCV's cvcam functions - author Mark Fiala
//-CVCam was in OpenCV 4, but is becoming obsolete, use the newer VFW cameras for input
//-I have trouble setting the camera input size, it appears as 320x240 on some computers, 160x120 for others.  If you
//know how to fix this or create a working Windows camera interface please let me know: mark.fiala@nrc-cnrc.gc.ca  
//Some users have found that if they run another program where you can set the camera resolution first, then it will run 
//with a higher resolution when you run the ARTag demos.  For some, the programs downloaded from 
//http://www.shrinkwrapvb.com/ezvidcap.htm  seem to do the trick on some computers.

//Here's OpenCV's license statement - which you must agree to before using or distributing - from "license.txt"
/*
IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING. 

By downloading, copying, installing or using the software you agree to this license.
If you do not agree to this license, do not download, install,
copy or use the software.


                       Intel License Agreement 
               For Open Source Computer Vision Library 

Copyright (C) 2000, 2001, Intel Corporation, all rights reserved.
Third party copyrights are property of their respective owners. 

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

  * Redistribution's of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.

  * Redistribution's in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

  * The name of Intel Corporation may not be used to endorse or promote products
    derived from this software without specific prior written permission.

This software is provided by the copyright holders and contributors "as is" and
any express or implied warranties, including, but not limited to, the implied
warranties of merchantability and fitness for a particular purpose are disclaimed.
In no event shall the Intel Corporation or contributors be liable for any direct,
indirect, incidental, special, exemplary, or consequential damages
(including, but not limited to, procurement of substitute goods or services;
loss of use, data, or profits; or business interruption) however caused
and on any theory of liability, whether in contract, strict liability,
or tort (including negligence or otherwise) arising in any way out of
the use of this software, even if advised of the possibility of such damage.  
*/






#define _CAMERA_CVCAM_C_



#include "cv.h"
#include "highgui.h"


CvCapture* cvcam_capture = 0;
unsigned char *cvcam_flip_image;
unsigned char *cvcam_rgb_cam_image;

//if(init_camera(cam_num,desired_width,desired_height,greybar_rgb,&cam_width,&cam_height)) problem starting...
char init_camera(int cam_num, int desired_width, int desired_height, char greybar_rgb,
                 int *cam_width, int *cam_height)
{
int width,height;
IplImage *frame;


cvcam_capture = cvCaptureFromCAM(cam_num);
//setting image size correctly dosen't seem to work - to fix
//cvSetCaptureProperty(cvcam_capture,CV_CAP_PROP_FRAME_WIDTH,desired_width);
//cvSetCaptureProperty(cvcam_capture,CV_CAP_PROP_FRAME_HEIGHT,desired_height);

//Initialize camera
if(cvGrabFrame(cvcam_capture))
   {
   frame = cvRetrieveFrame( cvcam_capture );
   if(frame!=NULL) {width = frame->width;height = frame->height;}
   cvcam_flip_image=(unsigned char*)malloc(width*height*3);
   if(cvcam_flip_image==NULL) {printf("ERROR: can't malloc cvcam_flip_image\n");return-1;}
   }

//allocate temporary memory for RGB image when only wanting greyscale
cvcam_rgb_cam_image=(unsigned char*)malloc(width*height*4+100);
if(cvcam_rgb_cam_image==NULL) {printf("failed malloc cvcam_rgb_cam_image\n");exit(1);}

*cam_width=width;  *cam_height=height;
return 0;
}



//close_camera(cam_num);
void close_camera(int cam_num)
{
cvReleaseCapture( &cvcam_capture );
}







//camera_grab_bgr_blocking(cam_num,rgb_cam_image,cam_width,cam_height);
void camera_grab_bgr_blocking(int cam_num, unsigned char *rgb_cam_image, int cam_width, int cam_height)
{
IplImage *frame;
int read_ptr,write_ptr,line_size,j;


if( !cvGrabFrame( cvcam_capture )) return;
frame = cvRetrieveFrame( cvcam_capture );
if( !frame ) return;
//flip image - cvCam provides up-side-down image
read_ptr=(cam_height-1)*cam_width*3;
write_ptr=0;
line_size=cam_width*3;
for(j=0;j<cam_height;j++)
   {
   memcpy(rgb_cam_image+write_ptr,frame->imageData+read_ptr,line_size);
   read_ptr-=line_size;
   write_ptr+=line_size;
   }
}





//camera_grab_greyscale_blocking(cam_num,cam_image,cam_width,cam_height);
//kludge version just calls RGB and converts to greyscale
void camera_grab_greyscale_blocking(int cam_num, unsigned char *cam_image, int cam_width, int cam_height)
{
int i,j=0;
//first grab BGR image and then average pixels
camera_grab_bgr_blocking(cam_num,cvcam_rgb_cam_image,cam_width,cam_height);

//convert to grey
for(i=0;i<cam_width*cam_height*3;i+=3)
   {
   int pixel=cvcam_rgb_cam_image[i+0]+cvcam_rgb_cam_image[i+1]+cvcam_rgb_cam_image[i+2];
   cam_image[j++]=(unsigned char)(pixel/3);
   }

}
