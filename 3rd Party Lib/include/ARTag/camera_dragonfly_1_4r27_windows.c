//© 2006, National Research Council Canada

//Mark's common camera interface - Feb 2/2004


//Windows PGR Dragonfly greyscale version
//- cam_num,desired_width,desired_height,and greybar_rgb are ignored

//not very elegant grab routine, copies entire memory

//if(init_camera(cam_num,desired_width,desired_height,greybar_rgb,&cam_width,&cam_height)) problem starting...
//camera_grab_greyscale_blocking(cam_num,cam_image,cam_width,cam_height);
//camera_grab_bgr_blocking(cam_num,rgb_cam_image,cam_width,cam_height);
//close_camera(cam_num);



#define _CAMERA_DRAGONFLY_640_480_WINDOWS_C_


#include <assert.h>
#include <memory.h>
#include "pgrflycapture.h"


#define FLYCAPTURE_MAX_CAMS 32

FlyCaptureError   flycapture_error;
FlyCaptureContext flycapture_context[FLYCAPTURE_MAX_CAMS];   


int camera_dragonfly_grab_ms,camera_dragonfly_convert8torgb_ms,camera_dragonfly_colour_correct_ms;


//if(init_camera(cam_num,desired_width,desired_height,greybar_rgb,&cam_width,&cam_height)) problem starting...
char init_camera(int cam_num, int desired_width, int desired_height, char greybar_rgb,
                 int *cam_width, int *cam_height)
{


//
// Enumerate the cameras on the bus
//
FlyCaptureInfo arInfo[ FLYCAPTURE_MAX_CAMS ];
unsigned int	 uiSize = FLYCAPTURE_MAX_CAMS;

flycapture_error = flycaptureBusEnumerateCameras( arInfo, &uiSize );
flycaptureErrorToString( flycapture_error ); 
if(flycapture_error!=NULL) {printf("flycaptureBusEnumerateCameras() flycapture_error=<%s>\n",flycapture_error);return -1;}
   //;HANDLE_ERROR( flycapture_error, "flycaptureBusEnumerateCameras()" );

for( unsigned int uiBusIndex = 0; uiBusIndex < uiSize; uiBusIndex++ )
{
  printf( 
     "Bus index %u: %s (%u)\n",
     uiBusIndex,
     arInfo[ uiBusIndex ].pszModelString,
     arInfo[ uiBusIndex ].SerialNumber );
}

//
// create the flycapture context.
//
flycapture_error = flycaptureCreateContext( &flycapture_context[cam_num] );
flycaptureErrorToString( flycapture_error ); 
if(flycapture_error!=NULL) {printf("flycaptureCreateContext() flycapture_error=<%s>\n",flycapture_error);return -1;}
//   _HANDLE_ERROR( flycapture_error, "flycaptureCreateContext()" );

//
// Initialize the camera.
//
printf( "Initializing camera %u.\n", cam_num );
flycapture_error = flycaptureInitialize( flycapture_context[cam_num], cam_num );
flycaptureErrorToString( flycapture_error ); 
if(flycapture_error!=NULL) {printf("flycaptureInitialize() flycapture_error=<%s>\n",flycapture_error);return -1;}
//   _HANDLE_ERROR( flycapture_error, "flycaptureInitialize()" );

//
// Start grabbing images in 8-bit greyscale (or stippled, if this is a 
// colour camera) 640x480 mode with a frame rate  of 15 fps.
//
printf( "Starting camera.\n\n" );
if(greybar_rgb==0)
   {
   //greyscale camera
   flycapture_error = flycaptureStart( 
                            flycapture_context[cam_num], FLYCAPTURE_VIDEOMODE_640x480Y8, FLYCAPTURE_FRAMERATE_15 );
   }
else
   {
   //RGB camera
   flycapture_error = flycaptureStart( 
                            flycapture_context[cam_num], FLYCAPTURE_VIDEOMODE_640x480RGB, FLYCAPTURE_FRAMERATE_15 );
   }
flycaptureErrorToString( flycapture_error ); 
if(flycapture_error!=NULL) {printf("flycaptureStart() flycapture_error=<%s>\n",flycapture_error);return -1;}
//   _HANDLE_ERROR( flycapture_error, "flycaptureStart()" );

*cam_width=640;  *cam_height=480;
return 0;
}



//close_camera(cam_num);
void close_camera(int cam_num)
{
flycaptureDestroyContext(flycapture_context[cam_num]);
}




//camera_grab_greyscale_blocking(cam_num,cam_image,cam_width,cam_height);
void camera_grab_greyscale_blocking(int cam_num, unsigned char *cam_image, int cam_width, int cam_height)
{
/*unsigned char *junk;
if(grab_ptgrey_dragonfly(cam_image,junk,cam_width,cam_height)==0)
   printf("camera_dragonfly_640_480_linux.c: grab_ptgrey_dragonfly() returned NULL\n");
*/

// Time the grabbing of 30 images.
//
FlyCaptureImage image;
//
// Initialize the image structure to sane values
//
image.iCols = 0;
image.iRows = 0;

flycapture_error = flycaptureGrabImage2( flycapture_context[cam_num], &image );
flycaptureErrorToString( flycapture_error ); 
if(flycapture_error!=NULL) printf("flycaptureGrabImage2() flycapture_error=<%s>\n",flycapture_error);
//      _HANDLE_ERROR( error, "flycaptureGrabImage2()" );

memcpy(cam_image,image.pData,cam_width*cam_height);
}





//camera_grab_bgr_blocking(cam_num,rgb_cam_image,cam_width,cam_height);
void camera_grab_bgr_blocking(int cam_num, unsigned char *rgb_cam_image, int cam_width, int cam_height)
{
#ifdef TIMER_LOADED
 int seconds,begin,now;
 get_time(&seconds,&begin);
#endif

FlyCaptureImage image;
image.iCols = 0;
image.iRows = 0;

flycapture_error = flycaptureGrabImage2( flycapture_context[cam_num], &image );
flycaptureErrorToString( flycapture_error ); 
if(flycapture_error!=NULL) printf("flycaptureGrabImage2() flycapture_error=<%s>\n",flycapture_error);
//      _HANDLE_ERROR( error, "flycaptureGrabImage2()" );

#ifdef TIMER_LOADED
 get_time(&seconds,&now);   camera_dragonfly_grab_ms=now-begin;
#endif


// Convert the 8-bit data to 24-bit RGB data
#ifdef TIMER_LOADED
 get_time(&seconds,&begin);
#endif

flycapture_error = flycaptureConvertImage(flycapture_context[cam_num],&image,
                                          FLYCAPTURE_OUTPUT_BGR,rgb_cam_image);
if(flycapture_error!=NULL) printf("flycaptureConvertImage() flycapture_error=<%s>\n",flycapture_error);

#ifdef TIMER_LOADED
 get_time(&seconds,&now);   camera_dragonfly_convert8torgb_ms=now-begin;
#endif



camera_dragonfly_colour_correct_ms=0;

//memcpy(rgb_cam_image,image.pData,cam_width*cam_height*3);
}





