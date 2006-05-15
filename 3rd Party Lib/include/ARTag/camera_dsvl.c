//© 2006, National Research Council Canada
//
//Dec 2007 - modified init_camera() so it uses the desired width,height - previous version camera default was used
//
//USB video input can run a lot faster with DSVL instead of OpenCV's cvcam
//camera_dsvl.c - wrapper for Thomas Pintaric's DSVL (DirectShow Video Processing Library)
//uses DSVL Version: 0.0.8 (05/13/2005) -Thomas Pintaric, Vienna University of Technology
//
//to get DSVL, get the file DSVL-0.0.8b.zip from Sourceforge, it's a choice in a list at
//http://sourceforge.net/project/showfiles.php?group_id=116280&package_id=126242
//
//you need the DSVL.h, DSVL_PixelFormatTypes.h, and you need to link in DSVL.lib to ccmpile 
//you need DSVL.dll (or DSVLd.dll for debug) in the same directory, or in your C:\WINNT\SYSTEM32 dir

#ifndef _CAMERA_DSVL_C_
#define _CAMERA_DSVL_C_

#include "DSVL.h"

//Thanks to Levi Lister from Columbia University for first applying DSVL to ARTag,
//and thanks to Kris Woodbeck from the NRC who wrote this wrapper

DSVL_VideoSource* dsvl_vs = NULL;
BYTE* g_pPixelBuffer = NULL;
MemoryBufferHandle g_mbHandle;

char init_camera(int cam_num, int desired_width, int desired_height, char greybar_rgb,
                 int *cam_width, int *cam_height)
{
	CoInitialize(NULL);
	PIXELFORMAT pxf;
	double cap_fps=0;
	long w=0, h=0;
   char xmlstring[512];

	dsvl_vs = new DSVL_VideoSource();

   sprintf(xmlstring,"<?xml version=\"1.0\" encoding=\"UTF-8\"?><dsvl_input xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"><camera show_format_dialog=\"false\" frame_width=\"%d\" frame_height=\"%d\"><pixel_format><RGB24/></pixel_format></camera></dsvl_input>",desired_width,desired_height);

	if (FAILED(dsvl_vs->BuildGraphFromXMLString(xmlstring)))
		return 1;
	if (FAILED(dsvl_vs->GetCurrentMediaFormat(&w, &h,&cap_fps, &pxf)))
		return(false);
	if (FAILED(dsvl_vs->EnableMemoryBuffer(3))) 
		return 1;
	if (FAILED(dsvl_vs->Run())) 
		return 1;
	
	*cam_width = w;
	*cam_height = h;

	return 0;
}

void close_camera(int cam_num) {
	dsvl_vs->Stop();
	dsvl_vs->ReleaseGraph();
	delete dsvl_vs;
}

void camera_grab_bgr_blocking(int cam_num, unsigned char *rgb_cam_image, int cam_width, int cam_height) {
	DWORD wait_result = dsvl_vs->WaitForNextSample();

	if (SUCCEEDED(dsvl_vs->CheckoutMemoryBuffer(&g_mbHandle, &g_pPixelBuffer))) {
		int read_ptr=(cam_height-1)*cam_width*3;
		int write_ptr=0;
		int line_size=cam_width*3;
		for (int j=0;j<cam_height;j++) {
			memcpy(rgb_cam_image+write_ptr,g_pPixelBuffer+read_ptr,line_size);
			read_ptr-=line_size;
			write_ptr+=line_size;
		}
		dsvl_vs->CheckinMemoryBuffer(g_mbHandle);
	}
}

#endif
