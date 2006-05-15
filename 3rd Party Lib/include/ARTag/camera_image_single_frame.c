//© 2006, National Research Council Canada

//CAMERA_IMAGE_SINGLE_FRAME.C - Mark Fiala June 2006
//
//-Simple camera emulator that just provides a single image, expects greyscale <camera_image.pgm>
//to be present in the directory.  Otherwise a black-to-white ramp image is created.

#define _CAMERA_IMAGE_SINGLE_FRAME_C_



unsigned char *camera_image_single_frame_greyscale;
unsigned char *camera_image_single_frame_rgb;

unsigned char* camera_single_image_frame_read_pgm(char *file_name, int *width, int *height);


//if(init_camera(cam_num,desired_width,desired_height,greybar_rgb,&cam_width,&cam_height)) problem starting...
char init_camera(int cam_num, int desired_width, int desired_height, char greybar_rgb,
                 int *cam_width, int *cam_height)
{
int i,width,height;


camera_image_single_frame_greyscale=
   camera_single_image_frame_read_pgm("camera_image.pgm",&width,&height);
if(camera_image_single_frame_greyscale==NULL)
   {
   width=desired_width;
   height=desired_height;
   camera_image_single_frame_greyscale=(unsigned char*)malloc(width*height);
   if(camera_image_single_frame_greyscale==NULL) 
      {printf("ERROR mallocing camera_image_single_frame_greyscale in camera_image_single_frame.c\n");exit(1);}
   //create default test pattern of black-to-white ramp
   for(i=0;i<width*height;i++) camera_image_single_frame_greyscale[i]=((i/2)%width)%256;
   }

//create RGB version
camera_image_single_frame_rgb=(unsigned char*)malloc(width*height*3);
if(camera_image_single_frame_rgb==NULL) 
  {printf("ERROR mallocing camera_image_single_frame_rgb in camera_image_single_frame.c\n");exit(1);}
for(i=0;i<width*height*3;i+=3)
   {
   camera_image_single_frame_rgb[i+0]=camera_image_single_frame_greyscale[i/3];
   camera_image_single_frame_rgb[i+1]=camera_image_single_frame_greyscale[i/3];
   camera_image_single_frame_rgb[i+2]=camera_image_single_frame_greyscale[i/3];
   }

*cam_width=width;  *cam_height=height;
return 0;
}



//close_camera(cam_num);
void close_camera(int cam_num)
{
if(camera_image_single_frame_greyscale!=NULL) free(camera_image_single_frame_greyscale);
if(camera_image_single_frame_rgb!=NULL) free(camera_image_single_frame_rgb);
}




//camera_grab_greyscale_blocking(cam_num,cam_image,cam_width,cam_height);
void camera_grab_greyscale_blocking(int cam_num, unsigned char *cam_image, int cam_width, int cam_height)
{
memcpy(cam_image,camera_image_single_frame_greyscale,cam_width*cam_height);
}





//camera_grab_bgr_blocking(cam_num,rgb_cam_image,cam_width,cam_height);
void camera_grab_bgr_blocking(int cam_num, unsigned char *rgb_cam_image, int cam_width, int cam_height)
{
memcpy(rgb_cam_image,camera_image_single_frame_rgb,cam_width*cam_height*3);
}




unsigned char* camera_single_image_frame_read_pgm(char *file_name, int *width, int *height)
{
int i,size;
int max_grey;
FILE *in_file;
char c,comment[256];
unsigned char uc,*image;

in_file=fopen(file_name,"rb");
if(in_file==NULL)  return NULL;

c=fgetc(in_file); if(c!='P') {printf("%s Not a PGM file\n",file_name);exit(1);}
c=fgetc(in_file); if(c!='5') {printf("%s Not a PGM file\n",file_name);exit(1);}
c=fgetc(in_file); if(c!='\n') {printf("%s Not a PGM file\n",file_name);exit(1);}
while(1)
    {
    fgets(comment,256,in_file);
    if(comment[0]!='#') break;
    }
sscanf(comment,"%d %d",width,height);
fgets(comment,256,in_file);
sscanf(comment,"%d",&max_grey);

size=(*width)*(*height);
image=(unsigned char*)malloc(size);
if(image==NULL) {printf("READ_PGM() error: Couldn't malloc image\n");exit(1);}

for(i=0;i<size;i++)
   {
   uc=fgetc(in_file);
   *(image+i)=uc;
   }
fclose(in_file);
return image;
}

