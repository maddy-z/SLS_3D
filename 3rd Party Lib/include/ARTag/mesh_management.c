//© 2006, National Research Council Canada

//MESH_MANAGEMENT.C - rending, storage, and manipulation of 3D meshes
//author:Mark Fiala - Aug 2005 - National Research Council of Canada - IIT/CVG group -
//
//
//init_mesh_management(100,10000,1000,3000000,3000000); //# object_models,# meshes,# materials,# polys,# points
//-------------------------------------------- Mesh files --------------------------------------------
//mesh_management_write_mesh(object_type_num,"iss.mesh"); //dump object object_type_num (probably 0) to a .MESH file
//object_type_num=mesh_management_read_mesh(mesh_in_filename,1000,&num_meshes,&num_vtxs,&num_triangles,&num_normals,&bitmaps_loaded);
//----------------------------------------- OpenGL rendering --------------------------------------------
//meshman_render_opengl(meshman_object_model_num,object_draw_bitmap,wireframe_mode);
//----------------------------------------- Normal vectors --------------------------------------------
//meshman_calculate_missing_normals(0); //add face normals (not vertex normals) to meshes with no normals
//-------------------------------------------- Bitmaps --------------------------------------------
//bitmap_id=mesh_management_add_bitmap(bitmap_filename);  //returns -1 if problem, meshman_bitmap pointer otherwise
//bitmaps_loaded=mesh_management_load_bitmaps();
//----------------------------------------- Scale/Translate meshes --------------------------------------------
//meshman_fit_to_limits(0,-80.0,80.0,-80.0,80.0,50.0,0.0); //fit to panel_set_toolbars3.cf
//meshman_move_xy_centroid(0, 0.0,0.0); //move object #0 to have an X,Y centroid of 0,0
//meshman_move_xyz_centroid(0, 0.0,0.0,0.0); //move object #0 to have an X,Y,Z centroid of 0,0,0





//------------------------------------------------------------------------------------------------------------------------------------------
#ifndef _MESH_MANAGEMENT_C_
 #define _MESH_MANAGEMENT_C_



#define MESHMAN_DEBUG_TEXTURE_PPM 0    //1=output debug texture PPM images, 0=don't (default)


//Data structures - an object model contains a set of meshes
//
//                - each mesh contains a list of faces (<65536)
//                - each face contains a list (2,3,4) of 3d vertices - each vertex pointer has it's own meshman_face[] entry
//                - each 3d vertex contains an X,Y,Z (3D coordinates)
//
//                - if a mesh is textured, each face also has a corresponding facetexture
//                - each facetexture contains a list of texture vertices (texpoints)- each tex pointer has it's own meshman_facetexture[] entry
//                - each texpoint contains a U,V (texture coordinates) - there is a texpoint for every 3d vertex
//
//                - a mesh either has as many normal vectors as faces (normal_per_vertex=0), or as vertices (normal_per_vertex=1)
//                - normal vectors are simply in a list, no indexing of them is done
//
//                - if a mesh is not textured, it is coloured, each face contains a single meshman_facecolour
//                - each meshman_facecolour contains an R,G,B in 8-bit format each (total 24 bit)
//
//                - each mesh uses one of a set of materials (num_materials <= num_meshes)
//                - each material uses one of a set of bitmaps (num_bitmaps <= num_materials)

//------------------------ Objects ----------------------------
typedef struct meshman_object_model_s {
                                      char filename[256];  //file loaded for this object model
                                      int start_mesh,end_mesh;  //start and end meshes for this object definition
                                      } meshman_object_model_type;
meshman_object_model_type *meshman_object_model;
int meshman_free_object_model_ptr,meshman_max_num_object_models;


//------------------------ Meshes ----------------------------
typedef struct meshman_mesh_s {
                              int material;  //pointer to meshman_material[] array
                       //
                              char wireframe_on;  //1=wireframe, use wireframe_red,green,blue as line colours
                              float wireframe_red,wireframe_green,wireframe_blue;  //valid if wireframe_on=1
                              //
                              int num_sides;  //per poly =2 for line, =3 for triangle, =4 for quad
                              int start_face,num_faces;
                              int start_face_t;
                              int base_vertex3d_ptr,num_3d_vertices;
                              int base_vertextex_ptr,num_tex_vertices;
                              int base_facecolour_ptr,num_face_colours;
                              //normal vectors
                              int base_normal_ptr,num_normals;
                              char normal_per_vertex;  //0=only one per face, 1=one per vertex (3 per face)
                              } meshman_mesh_type;
meshman_mesh_type *meshman_mesh;
int meshman_free_mesh_ptr,meshman_max_num_meshes;

//------------------------ Face (polygon) 3D point list ----------------------------
typedef struct meshman_face_s {
                              unsigned short int vertex3d_num;
                              } meshman_face_type;
meshman_face_type *meshman_face;
int meshman_free_face_ptr,meshman_max_num_faces;

//------------------------ Vertices (3D Points X,Y,Z) ----------------------------
typedef struct meshman_3dpoint_s {
                                 float x,y,z;
                                 } meshman_3dpoint_type;
meshman_3dpoint_type *meshman_3dpoint;
int meshman_free_3dpoint_ptr,meshman_max_num_3dpoints;

//------------------------ Normal vectors (either 1 per face or 1 per vertex) ----------------------------
typedef struct meshman_normal_s {
                                 float nx,ny,nz;
                                 } meshman_normal_type;
meshman_normal_type *meshman_normal;
int meshman_free_normal_ptr,meshman_max_num_normals;

//------------------------ Face (polygon) texture point list ----------------------------
//if a mesh is textured, there is a meshman_facetexture[] entry for each meshman_face[] entry
typedef struct meshman_facetexture_s {
                                     unsigned short int texvertex_num;
                                     } meshman_facetexture_type;
meshman_facetexture_type *meshman_facetexture;
int meshman_free_facetexture_ptr,meshman_max_num_facetexture;

//------------------------ Texture Vertices (U,V) ----------------------------
typedef struct meshman_texpoint_s {
                                 float u,v;
                                 } meshman_texpoint_type;
meshman_texpoint_type *meshman_texpoint;
int meshman_free_texpoint_ptr,meshman_max_num_texpoints;

//------------------------ Face (polygon) Colour point list ----------------------------
//if a mesh is coloured, there is a meshman_facecolour[] entry for each meshman_face[] entry
typedef struct meshman_facecolour_s {
                                    unsigned char red,green,blue;
                                    } meshman_facecolour_type;
meshman_facecolour_type *meshman_facecolour;
int meshman_free_facecolour_ptr,meshman_max_num_facecolour;



//------------------------ Materials ----------------------------
typedef struct meshman_material_s {
                                  //bitmap
                                  char bitmap_on;  //1=bitmap texture, 0=no texture
                                  int bitmap_num;  //pointer into meshman_bitmap[] array
                                  //wireframe mode flag
                                  char wireframe_on;  //1=only use diffuse_r,g,b as wireframe colour
                                  //diffuse colour
                                  char diffuse_on;  //1=diffuse_r,g,b valid
                                  float diffuse_red,diffuse_green,diffuse_blue;
                                  //specular
                                  char specular_on;  //1=specular_r,g,b valid
                                  float specular_red,specular_green,specular_blue;
                                  //ambient
                                  char ambient_on;  //1=ambient_r,g,b valid
                                  float ambient_red,ambient_green,ambient_blue;
                                  //
                                  int material_number_in_file;
                                  } meshman_material_type;
meshman_material_type *meshman_material;
int meshman_max_num_materials;
int meshman_material_ptr;

int meshman_default_material;  //default diffuse white material (=0 in first version)

//------------------------ Bitmaps ---------------------------------
typedef struct meshman_bitmap_s {
                                char filename[256];
                                unsigned char *image;
                                unsigned int opengl_handle; //typedef unsigned int GLuint;
                                char rgb_greybar;  //1=RGB, 0=greyscale
                                int original_width,original_height;
                                int width,height;  //width and height are 2^N
                                } meshman_bitmap_type;
meshman_bitmap_type *meshman_bitmap;
int meshman_bitmap_ptr;


//prototypes of internal functions
char meshman_split_filename(char *filename, char *root, char *suffix);
unsigned char* meshman_read_pgm(char *file_name, int *width, int *height);
unsigned char* meshman_read_ppm(char *file_name, int *width, int *height);
void meshman_opengl_bind_texture(int bitmap_num);
int mesh_management_add_bitmap(char *filename);
void meshman_write_ppm(char *file_name, char *comment, unsigned char *image,int width,int height);



//init_mesh_management(100,10000,1000,3000000,3000000); //# object_models,# meshes,# materials,# polys,# points
char init_mesh_management(int max_num_object_types, int max_num_meshes,
                          int max_num_materials, int max_num_faces, int max_num_points)
{
//objects
meshman_object_model=(meshman_object_model_type*)malloc(max_num_object_types*sizeof(meshman_object_model_type));
if(meshman_object_model==NULL) {printf("ERROR: Can't malloc %d meshman_object_model[] elements\n",max_num_object_types);return -1;}
meshman_max_num_object_models=max_num_object_types;
meshman_free_object_model_ptr=0;
//meshes
meshman_mesh=(meshman_mesh_type*)malloc(max_num_meshes*sizeof(meshman_mesh_type));
if(meshman_mesh==NULL) {printf("ERROR: Can't malloc %d meshman_mesh[] elements\n",max_num_meshes);return -1;}
meshman_max_num_meshes=max_num_meshes;
meshman_free_mesh_ptr=0;
//faces - each face has a set of 3d vertices
meshman_face=(meshman_face_type*)malloc(max_num_faces*sizeof(meshman_face_type));
if(meshman_face==NULL) {printf("ERROR: Can't malloc %d meshman_face[] elements\n",max_num_faces);return -1;}
meshman_max_num_faces=max_num_faces;
meshman_free_face_ptr=0;
//3d vertices
meshman_3dpoint=(meshman_3dpoint_type*)malloc(max_num_points*sizeof(meshman_3dpoint_type));
if(meshman_3dpoint==NULL) {printf("ERROR: Can't malloc %d meshman_3dpoint[] elements\n",max_num_points);return -1;}
meshman_max_num_3dpoints=max_num_points;
meshman_free_3dpoint_ptr=0;
//vertex normals - up to 4 per face
meshman_normal=(meshman_normal_type*)malloc(4*max_num_faces*sizeof(meshman_normal_type));
if(meshman_normal==NULL) {printf("ERROR: Can't malloc %d meshman_3dpoint[] elements\n",4*max_num_faces);return -1;}
meshman_max_num_normals=4*max_num_faces;
meshman_free_normal_ptr=0;
//facetextures - each face has a set of texture vertices, each corresponding to a 3D vertex
meshman_facetexture=(meshman_facetexture_type*)malloc(max_num_faces*sizeof(meshman_facetexture_type));
if(meshman_facetexture==NULL) {printf("ERROR: Can't malloc %d meshman_facetexture[] elements\n",max_num_faces);return -1;}
meshman_max_num_facetexture=max_num_faces;
meshman_free_facetexture_ptr=0;
//texture vertices
meshman_texpoint=(meshman_texpoint_type*)malloc(max_num_points*sizeof(meshman_texpoint_type));
if(meshman_texpoint==NULL) {printf("ERROR: Can't malloc %d meshman_texpoint[] elements\n",max_num_points);return -1;}
meshman_max_num_texpoints=max_num_points;
meshman_free_texpoint_ptr=0;
//facecolour  - each face has a single 24 bit colour
meshman_facecolour=(meshman_facecolour_type*)malloc(max_num_faces*sizeof(meshman_facecolour_type));
if(meshman_facecolour==NULL) {printf("ERROR: Can't malloc %d meshman_facecolour[] elements\n",max_num_faces);return -1;}
meshman_max_num_facecolour=max_num_faces;
meshman_free_facecolour_ptr=0;
//materials: zero or one bitmap per material
meshman_material=(meshman_material_type*)malloc(max_num_materials*sizeof(meshman_material_type));
if(meshman_material==NULL) {printf("ERROR: Can't malloc meshman_material[%d]\n",max_num_materials);return -1;}
meshman_max_num_materials=max_num_materials;
meshman_material_ptr=0;
//bitmaps: zero or one bitmap per material - therefore make as many bitmap entries as materials
meshman_bitmap=(meshman_bitmap_type*)malloc(max_num_materials*sizeof(meshman_bitmap_type));
if(meshman_bitmap==NULL) {printf("ERROR: Can't malloc %d meshman_bitmap[] elements\n",max_num_materials);return -1;}
meshman_bitmap_ptr=0;
//create default diffuse white material for meshes without colour, bitmap, or wireframe applied
meshman_material[meshman_material_ptr].diffuse_on=1;  //1=diffuse_r,g,b valid
meshman_material[meshman_material_ptr].diffuse_red=255;
meshman_material[meshman_material_ptr].diffuse_green=255;
meshman_material[meshman_material_ptr].diffuse_blue=255;
meshman_material[meshman_material_ptr].bitmap_on=0;
meshman_material[meshman_material_ptr].wireframe_on=0;
meshman_default_material=meshman_material_ptr;
if(meshman_material_ptr<meshman_max_num_materials) meshman_material_ptr++;

return 0;
}







//----------------------------------------- Scale/Translate meshes --------------------------------------------

//meshman_fit_to_limits(0,-80.0,80.0,-80.0,80.0,50.0,0.0); //fit to panel_set_toolbars3.cf
void meshman_fit_to_limits(int object_type_num,
                           float xmin, float xmax,
                           float ymin, float ymax,
                           float zmin, float zmax)
{
int mesh_num;
float minx,maxx,miny,maxy,minz,maxz;  //extents found in meshes
char minx_on=0,maxx_on=0,miny_on=0,maxy_on=0,minz_on=0,maxz_on=0;  //have above been set yet?
float scalex,scaley,scalez,scale;
char scalex_valid=1,scaley_valid=1,scalez_valid=1;  //avoid considering dimensions for planar objects
float offsetx,offsety,offsetz;

//find extents of model
for(mesh_num=meshman_object_model[object_type_num].start_mesh;mesh_num<=meshman_object_model[object_type_num].end_mesh;mesh_num++)
 if(meshman_mesh[mesh_num].num_sides>0)  //don't include junk meshes that aren't rendered
   {
   int v,vertex3d_ptr=meshman_mesh[mesh_num].base_vertex3d_ptr;
   for(v=0;v<meshman_mesh[mesh_num].num_3d_vertices;v++)
      {
      if(minx_on==0)  {minx=meshman_3dpoint[vertex3d_ptr].x;minx_on=1;}
      else if(minx>meshman_3dpoint[vertex3d_ptr].x) minx=meshman_3dpoint[vertex3d_ptr].x;
      if(maxx_on==0)  {maxx=meshman_3dpoint[vertex3d_ptr].x;maxx_on=1;}
      else if(meshman_3dpoint[vertex3d_ptr].x>maxx) maxx=meshman_3dpoint[vertex3d_ptr].x;
      if(miny_on==0)  {miny=meshman_3dpoint[vertex3d_ptr].y;miny_on=1;}
      else if(miny>meshman_3dpoint[vertex3d_ptr].y) miny=meshman_3dpoint[vertex3d_ptr].y;
      if(maxy_on==0)  {maxy=meshman_3dpoint[vertex3d_ptr].y;maxy_on=1;}
      else if(meshman_3dpoint[vertex3d_ptr].y>maxy) maxy=meshman_3dpoint[vertex3d_ptr].y;
      if(minz_on==0)  {minz=meshman_3dpoint[vertex3d_ptr].z;minz_on=1;}
      else if(minz>meshman_3dpoint[vertex3d_ptr].z) minz=meshman_3dpoint[vertex3d_ptr].z;
      if(maxz_on==0)  {maxz=meshman_3dpoint[vertex3d_ptr].z;maxz_on=1;}
      else if(meshman_3dpoint[vertex3d_ptr].z>maxz) maxz=meshman_3dpoint[vertex3d_ptr].z;
      vertex3d_ptr++;
      }
   }
printf("original range (%f to %f),(%f to %f),(%f to %f)  desired range=(%f to %f),(%f to %f),(%f to %f)\n",
       minx,maxx,miny,maxy,minz,maxz,xmin,xmax,ymin,ymax,zmin,zmax);
//choose appropriate scale factor
if(maxx-minx==0.0) scalex_valid=0;
if(scalex_valid) scalex=(xmax-xmin)/(maxx-minx);
if(maxy-miny==0.0) scaley_valid=0;
if(scaley_valid) scaley=(ymax-ymin)/(maxy-miny);
if(maxz-minz==0.0) scalez_valid=0;
if(scalez_valid) scalez=(zmax-zmin)/(maxz-minz);
if((scalex_valid==0)&&(scaley_valid==0)&&(scalez_valid==0)) return;  //degenerate - meshes have zero or same 3D points
//printf("scalex_valid=%d scalex=%f  :  scaley_valid=%d scaley=%f  :  scalez_valid=%d scalez=%f\n",
//       scalex_valid,scalex,scaley_valid,scaley,scalez_valid,scalez);
if(scalex_valid) scale=scalex;
else if(scaley_valid) scale=scaley;
else if(scalez_valid) scale=scalez;
if((scaley_valid)&&(scaley<scale)) scale=scaley;
if((scalez_valid)&&(scalez<scale)) scale=scalez;
//calculate offsets
offsetx=xmin-scale*minx;  offsety=ymin-scale*miny;  offsetz=zmin-scale*minz;
//
printf("scale=%f  offsets=%f,%f,%f\n",scale,offsetx,offsety,offsetz);
//adjust 3D vertices
for(mesh_num=meshman_object_model[object_type_num].start_mesh;mesh_num<=meshman_object_model[object_type_num].end_mesh;mesh_num++)
   {
   int v,vertex3d_ptr=meshman_mesh[mesh_num].base_vertex3d_ptr;
   for(v=0;v<meshman_mesh[mesh_num].num_3d_vertices;v++)
      {
      meshman_3dpoint[vertex3d_ptr].x*=scale;
      meshman_3dpoint[vertex3d_ptr].x+=offsetx;
      meshman_3dpoint[vertex3d_ptr].y*=scale;
      meshman_3dpoint[vertex3d_ptr].y+=offsety;
      meshman_3dpoint[vertex3d_ptr].z*=scale;
      meshman_3dpoint[vertex3d_ptr].z+=offsetz;
      vertex3d_ptr++;
      }
   }
}




//meshman_move_xy_centroid(0, 0.0,0.0); //move object #0 to have an X,Y centroid of 0,0
void meshman_move_xy_centroid(int object_type_num, float xdesired, float ydesired)
{
int mesh_num;
float sumx=0.0,sumy=0.0,num=0.0;  //centroid stats
float offsetx,offsety;

for(mesh_num=meshman_object_model[object_type_num].start_mesh;mesh_num<=meshman_object_model[object_type_num].end_mesh;mesh_num++)
 if(meshman_mesh[mesh_num].num_sides>0)  //don't include junk meshes that aren't rendered
   {
   int v,vertex3d_ptr=meshman_mesh[mesh_num].base_vertex3d_ptr;
   for(v=0;v<meshman_mesh[mesh_num].num_3d_vertices;v++)
      {
      sumx+=meshman_3dpoint[vertex3d_ptr].x;
      sumy+=meshman_3dpoint[vertex3d_ptr].y;
      num+=1.0;
      vertex3d_ptr++;
      }
   }
//calculate offsets
offsetx=xdesired-sumx/num;  offsety=ydesired-sumy/num;
//
//adjust 3D vertices
for(mesh_num=meshman_object_model[object_type_num].start_mesh;mesh_num<=meshman_object_model[object_type_num].end_mesh;mesh_num++)
   {
   int v,vertex3d_ptr=meshman_mesh[mesh_num].base_vertex3d_ptr;
   for(v=0;v<meshman_mesh[mesh_num].num_3d_vertices;v++)
      {
      meshman_3dpoint[vertex3d_ptr].x+=offsetx;
      meshman_3dpoint[vertex3d_ptr].y+=offsety;
      vertex3d_ptr++;
      }
   }
}



//meshman_move_xyz_centroid(0, 0.0,0.0,0.0); //move object #0 to have an X,Y,Z centroid of 0,0,0
void meshman_move_xyz_centroid(int object_type_num, float xdesired, float ydesired, float zdesired)
{
int mesh_num;
float sumx=0.0,sumy=0.0,sumz=0.0,num=0.0;  //centroid stats
float offsetx,offsety,offsetz;

for(mesh_num=meshman_object_model[object_type_num].start_mesh;mesh_num<=meshman_object_model[object_type_num].end_mesh;mesh_num++)
 if(meshman_mesh[mesh_num].num_sides>0)  //don't include junk meshes that aren't rendered
   {
   int v,vertex3d_ptr=meshman_mesh[mesh_num].base_vertex3d_ptr;
   for(v=0;v<meshman_mesh[mesh_num].num_3d_vertices;v++)
      {
      sumx+=meshman_3dpoint[vertex3d_ptr].x;
      sumy+=meshman_3dpoint[vertex3d_ptr].y;
      sumz+=meshman_3dpoint[vertex3d_ptr].z;
      num+=1.0;
      vertex3d_ptr++;
      }
   }
//calculate offsets
offsetx=xdesired-sumx/num;  offsety=ydesired-sumy/num;  offsetz=zdesired-sumz/num;
//
//adjust 3D vertices
for(mesh_num=meshman_object_model[object_type_num].start_mesh;mesh_num<=meshman_object_model[object_type_num].end_mesh;mesh_num++)
   {
   int v,vertex3d_ptr=meshman_mesh[mesh_num].base_vertex3d_ptr;
   for(v=0;v<meshman_mesh[mesh_num].num_3d_vertices;v++)
      {
      meshman_3dpoint[vertex3d_ptr].x+=offsetx;
      meshman_3dpoint[vertex3d_ptr].y+=offsety;
      meshman_3dpoint[vertex3d_ptr].z+=offsetz;
      vertex3d_ptr++;
      }
   }
}







//----------------------------------------- Normal vectors --------------------------------------------

//meshman_calculate_missing_normals(0); //add face normals (not vertex normals) to meshes with no normals
void meshman_calculate_missing_normals(int object_type_num)
{
int mesh_num;

int np=meshman_free_normal_ptr;
int fp=meshman_free_face_ptr;
int add=0;


for(mesh_num=meshman_object_model[object_type_num].start_mesh;mesh_num<=meshman_object_model[object_type_num].end_mesh;mesh_num++)
   if((meshman_mesh[mesh_num].num_normals==0)&&(meshman_mesh[mesh_num].num_sides>=3))
      {
      int face_ptr=meshman_mesh[mesh_num].start_face;
      int base_vertex3d_ptr=meshman_mesh[mesh_num].base_vertex3d_ptr;
      int face_num,vertex_num;
      float x0,y0,z0,x1,y1,z1,x2,y2,z2,nx,ny,nz,mag;
      //set up mesh params
      meshman_mesh[mesh_num].base_normal_ptr=meshman_free_normal_ptr;
      meshman_mesh[mesh_num].normal_per_vertex=0;  //only face normals
      //loop through mesh, find cross product of vertices (1-0 cross 1-2 = outwards facing for cw polys)
      for(face_num=0;face_num<meshman_mesh[mesh_num].num_faces;face_num++)
         for(vertex_num=0;vertex_num<meshman_mesh[mesh_num].num_sides;vertex_num++)
            {
            switch(vertex_num)
               {
               int vertex3d_ptr;
               case 0: 
                  vertex3d_ptr=base_vertex3d_ptr+meshman_face[face_ptr].vertex3d_num;
                  x0=meshman_3dpoint[vertex3d_ptr].x; y0=meshman_3dpoint[vertex3d_ptr].y; z0=meshman_3dpoint[vertex3d_ptr].z;
                  face_ptr++; 
               break; 
               case 1: 
                  vertex3d_ptr=base_vertex3d_ptr+meshman_face[face_ptr].vertex3d_num;
                  x1=meshman_3dpoint[vertex3d_ptr].x; y1=meshman_3dpoint[vertex3d_ptr].y; z1=meshman_3dpoint[vertex3d_ptr].z;
                  face_ptr++; 
               break; 
               case 2: 
                  vertex3d_ptr=base_vertex3d_ptr+meshman_face[face_ptr].vertex3d_num;
                  x2=meshman_3dpoint[vertex3d_ptr].x; y2=meshman_3dpoint[vertex3d_ptr].y; z2=meshman_3dpoint[vertex3d_ptr].z;
                  face_ptr++; 
                  nx=(y0-y1)*(z2-z1)-(z0-z1)*(y2-y1);
                  ny=(z0-z1)*(x2-x1)-(x0-x1)*(z2-z1);
                  nz=(x0-x1)*(y2-y1)-(y0-y1)*(x2-x1);
                  mag=(float)sqrt(nx*nx+ny*ny+nz*nz);
                  //make normal into unit vector (may not be necessary)
                  if(mag>0.0)
                     {
                     nx/=mag; ny/=mag; nz/=mag;
                     //add normal to face
                     meshman_normal[meshman_free_normal_ptr].nx=nx;
                     meshman_normal[meshman_free_normal_ptr].ny=ny;
                     meshman_normal[meshman_free_normal_ptr].nz=nz;
                     if(meshman_free_normal_ptr<meshman_max_num_normals)
                        {
                        meshman_free_normal_ptr++;
                        meshman_mesh[mesh_num].num_normals++;
                        }
                     }
               break; 
               default: 
                  face_ptr++; 
               break; 
               }
         }//for(vertex_num=0;...
      }
}







//-------------------------------------------- Mesh files --------------------------------------------

//mesh_management_write_mesh(object_type_num,"iss.mesh"); //dump object object_type_num (probably 0) to a .MESH file
//printf("Wrote mesh file <%s>\n","iss.mesh");
char mesh_management_write_mesh(int object_type_num, char *filename)
{
int material_num,mesh_num;
FILE *mesh_out;

mesh_out=fopen(filename,"wb");
if(mesh_out==NULL) {printf("WARNING: cannot open <%s> for writing\n",filename);return -1;}
fprintf(mesh_out,"//object_type #%d from <%s>\n",object_type_num,meshman_object_model[object_type_num].filename);
fprintf(mesh_out,"//--%d materials--\n",meshman_material_ptr-1);  //1 default material

//write out materials
for(material_num=0;material_num<meshman_material_ptr;material_num++)
   if(material_num!=meshman_default_material)
      {
      if(material_num!=meshman_default_material)
         fprintf(mesh_out,"MATERIAL %d\n",meshman_material[material_num].material_number_in_file);
      if(meshman_material[material_num].bitmap_on)
         fprintf(mesh_out,"   bitmap %s\n",meshman_bitmap[meshman_material[material_num].bitmap_num].filename);
      if(meshman_material[material_num].wireframe_on)
         fprintf(mesh_out,"   wire_frame %f %f %f\n",meshman_material[material_num].diffuse_red,
                 meshman_material[material_num].diffuse_green,meshman_material[material_num].diffuse_blue);
      if(meshman_material[material_num].diffuse_on)
         fprintf(mesh_out,"   diffuse %f %f %f\n",meshman_material[material_num].diffuse_red,
                 meshman_material[material_num].diffuse_green,meshman_material[material_num].diffuse_blue);
      if(meshman_material[material_num].specular_on)
         fprintf(mesh_out,"   specular %f %f %f\n",meshman_material[material_num].specular_red,
                 meshman_material[material_num].specular_green,meshman_material[material_num].specular_blue);
      if(meshman_material[material_num].ambient_on)
         fprintf(mesh_out,"   ambient %f %f %f\n",meshman_material[material_num].ambient_red,
                 meshman_material[material_num].ambient_green,meshman_material[material_num].ambient_blue);
      }
//write out meshes
for(mesh_num=meshman_object_model[object_type_num].start_mesh;mesh_num<=meshman_object_model[object_type_num].end_mesh;mesh_num++)
 if(meshman_mesh[mesh_num].num_faces>0)
   {
   int meshman_material_num=meshman_mesh[mesh_num].material;
   int original_material_num=meshman_material[meshman_material_num].material_number_in_file;
   int face,face_ptr=meshman_mesh[mesh_num].start_face;
   int texface_ptr=meshman_mesh[mesh_num].start_face_t;
   int v,vertex3d_ptr=meshman_mesh[mesh_num].base_vertex3d_ptr;
   int n,vtxnormal_ptr=meshman_mesh[mesh_num].base_normal_ptr;
   int vertextex_ptr=meshman_mesh[mesh_num].base_vertextex_ptr;
   int f,facecolour_ptr=meshman_mesh[mesh_num].base_facecolour_ptr;
   fprintf(mesh_out,"MESH START\n");
   if(meshman_material_num!=meshman_default_material)
      fprintf(mesh_out,"material_id %d\n",original_material_num);
   fprintf(mesh_out,"num_sides %d\n",meshman_mesh[mesh_num].num_sides);
   //output faces
   for(face=0;face<meshman_mesh[mesh_num].num_faces;face++)
      {
      int side;
      fprintf(mesh_out,"   face  ");
      for(side=0;side<meshman_mesh[mesh_num].num_sides;side++)
         {fprintf(mesh_out,"%d  ",meshman_face[face_ptr].vertex3d_num);face_ptr++;}
      fprintf(mesh_out,"\n");
      }
   //output 3d vertices
   for(v=0;v<meshman_mesh[mesh_num].num_3d_vertices;v++)
      {
      fprintf(mesh_out,"   vertex %f %f %f\n",meshman_3dpoint[vertex3d_ptr].x,
              meshman_3dpoint[vertex3d_ptr].y,meshman_3dpoint[vertex3d_ptr].z);
      vertex3d_ptr++;
      }
   //output mesh normal vectors
   if(meshman_mesh[mesh_num].normal_per_vertex==0)
      for(n=0;n<meshman_mesh[mesh_num].num_normals;n++)
         {
         fprintf(mesh_out,"   facenormal %f %f %f\n",meshman_normal[vtxnormal_ptr].nx,
                 meshman_normal[vtxnormal_ptr].ny,meshman_normal[vtxnormal_ptr].nz);
         vtxnormal_ptr++;
         }
   //output vertex normal vectors
   else
      for(n=0;n<meshman_mesh[mesh_num].num_normals;n++)
         {
         fprintf(mesh_out,"   vtxnormal %f %f %f\n",meshman_normal[vtxnormal_ptr].nx,
                 meshman_normal[vtxnormal_ptr].ny,meshman_normal[vtxnormal_ptr].nz);
         vtxnormal_ptr++;
         }
   //output texture faces
   if(meshman_mesh[mesh_num].num_tex_vertices>0)
      for(face=0;face<meshman_mesh[mesh_num].num_faces;face++)
         {
         int side;
         fprintf(mesh_out,"   texface  ");
         for(side=0;side<meshman_mesh[mesh_num].num_sides;side++)
            {fprintf(mesh_out,"%d  ",meshman_facetexture[texface_ptr].texvertex_num);texface_ptr++;}
         fprintf(mesh_out,"\n");
         }
   //output texture vertices
   for(v=0;v<meshman_mesh[mesh_num].num_tex_vertices;v++)
      {
      fprintf(mesh_out,"   texvertex %f %f\n",meshman_texpoint[vertextex_ptr].u,meshman_texpoint[vertextex_ptr].v);
      vertextex_ptr++;
      }
   //output face colours
   for(f=0;f<meshman_mesh[mesh_num].num_face_colours;f++)
      {
      fprintf(mesh_out,"   facecolor %d %d %d\n",meshman_facecolour[facecolour_ptr].red,
              meshman_facecolour[facecolour_ptr].green,meshman_facecolour[facecolour_ptr].blue);
      facecolour_ptr++;
      }
   fprintf(mesh_out,"MESH END\n");
   }
fclose(mesh_out);
return 0;  //all ok
}





//-requires readline.c to be included first
#ifndef _READ_LINE_C_
 #include "read_line.c"
#endif


//object_type_num=mesh_management_read_mesh(mesh_in_filename,1000,&num_meshes,&num_vtxs,&num_triangles,&num_normals,&bitmaps_loaded);
//if(object_type_num==-1) problem...
int mesh_management_read_mesh(char *filename, int max_num_materials, int *num_meshes,
                              int *num_vtxs, int *num_triangles, int *num_normals, int *num_bitmaps)
{
int mesh_num=-1,material_num_in_file,material_num,material_ref=-1,num_sides=-1,bitmap_id=-1;
int num_meshes_loaded=0;
int num_3dvertices_loaded=0,num_texvertices_loaded=0,num_normalvectors_loaded=0;
int num_faces_loaded=0,num_texfaces_loaded=0;
int num_faces_this_mesh,num_3dvtx_this_mesh,num_texfaces_this_mesh,num_texvtx_this_mesh;
int num_facenormals_this_mesh,num_normalvtx_this_mesh;
int i,*material_lookup;  //maps from file material# to pointer in memory
int num_bitmaps_present=0,num_meshes_without_materials=0;
int object_num;

object_num=meshman_free_object_model_ptr;

*num_meshes=0; *num_vtxs=0; *num_triangles=0;
material_lookup=(int*)malloc(max_num_materials*sizeof(int));
if(material_lookup==NULL) {printf("ERROR mallocing *material_lookup\n");return -1;}
for(i=0;i<max_num_materials;i++) material_lookup[i]=-1;

//start meshman_object
strcpy(meshman_object_model[object_num].filename,filename);
meshman_object_model[object_num].start_mesh=meshman_free_mesh_ptr;
meshman_object_model[object_num].end_mesh=meshman_free_mesh_ptr;


READ_LINE_IN=fopen(filename,"rb");
if(READ_LINE_IN==NULL) {printf("Can't open mesh file <%s> for reading\n",filename); return -1;}
line_number=0;

while(read_line(line)==0)
   {
   int num_entries=break_line(line,el1,el2,el3,el4,el5,el6,el7,el8,el9);
   //printf("read line <%s>\n",line);

   //make sure we have enough tokens per line for the different line types
   if( ((strcmp(el1,"MATERIAL")==0)||(strcmp(el1,"material")==0)
      ||(strcmp(el1,"MESH")==0)||(strcmp(el1,"mesh")==0)
      ||(strcmp(el1,"MATERIAL_ID")==0)||(strcmp(el1,"material_id")==0)
      ||(strcmp(el1,"NUM_SIDES")==0)||(strcmp(el1,"num_sides")==0)
      ||(strcmp(el1,"BITMAP")==0)||(strcmp(el1,"bitmap")==0))
     &&(num_entries!=2) )
        printf("WARNING: line %d invalid.  Expecting 2 tokens in line (found %d).  Line ignored\n"
               "line reads <%s>\n",line_number,num_entries,line);
   else if( ((strcmp(el1,"WIRE_FRAME")==0)||(strcmp(el1,"wire_frame")==0)
          ||(strcmp(el1,"DIFFUSE")==0)||(strcmp(el1,"diffuse")==0)
          ||(strcmp(el1,"SPECULAR")==0)||(strcmp(el1,"specular")==0)
          ||(strcmp(el1,"AMBIENT")==0)||(strcmp(el1,"ambient")==0)
          ||(strcmp(el1,"VERTEX")==0)||(strcmp(el1,"vertex")==0)
          ||(strcmp(el1,"FACENORMAL")==0)||(strcmp(el1,"facenormal")==0)
          ||(strcmp(el1,"VTXNORMAL")==0)||(strcmp(el1,"vtxnormal")==0))
         &&(num_entries<4) )
            printf("WARNING: line %d invalid.  Expecting 4 tokens in line (found %d).  Line ignored\n"
                   "line reads <%s>\n",line_number,num_entries,line);
   else if( ((strcmp(el1,"FACE")==0)||(strcmp(el1,"face")==0)
           ||(strcmp(el1,"TEXFACE")==0)||(strcmp(el1,"texface")==0))
     &&(num_entries!=num_sides+1) )
        printf("WARNING: line %d invalid.  Expecting %d tokens in line (found %d).  Line ignored\n"
               "line reads <%s>\n",num_sides+1,line_number,num_entries,line);
   else if( (strcmp(el1,"TEXVERTEX")==0)||(strcmp(el1,"texvertex")==0)
     &&(num_entries!=3) )
        printf("WARNING: line %d invalid.  Expecting 3 tokens in line (found %d).  Line ignored\n"
               "line reads <%s>\n",line_number,num_entries,line);
   //check that we are in the correct mode for the following tokens
   if( ((strcmp(el1,"WIRE_FRAME")==0)||(strcmp(el1,"wire_frame")==0)
      ||(strcmp(el1,"DIFFUSE")==0)||(strcmp(el1,"diffuse")==0)
      ||(strcmp(el1,"SPECULAR")==0)||(strcmp(el1,"specular")==0)
      ||(strcmp(el1,"AMBIENT")==0)||(strcmp(el1,"ambient")==0)
      ||(strcmp(el1,"BITMAP")==0)||(strcmp(el1,"bitmap")==0))
     &&((material_num<0)||(material_num>=meshman_max_num_materials)) )
        printf("WARNING: Material number has not been started, or most recent material number is out of range\n"
               "line %d invalid, ignored.  material_num=%d\n"
               "line reads <%s>\n",line_number,material_num,line);
   if( ((strcmp(el1,"MATERIAL_ID")==0)||(strcmp(el1,"material_id")==0)
      ||(strcmp(el1,"NUM_SIDES")==0)||(strcmp(el1,"num_sides")==0)
      ||(strcmp(el1,"FACE")==0)||(strcmp(el1,"face")==0)
      ||(strcmp(el1,"VERTEX")==0)||(strcmp(el1,"vertex")==0)
      ||(strcmp(el1,"TEXFACE")==0)||(strcmp(el1,"texface")==0)
      ||(strcmp(el1,"FACENORMAL")==0)||(strcmp(el1,"facenormal")==0)
      ||(strcmp(el1,"VTXNORMAL")==0)||(strcmp(el1,"vtxnormal")==0)
      ||(strcmp(el1,"TEXVERTEX")==0)||(strcmp(el1,"texvertex")==0))
     &&(mesh_num==-1) )
        printf("WARNING: Mesh has not been started, token needs to be between a MESH START and MESH END line\n"
               "line %d invalid, ignored\n"
               "line reads <%s>\n",line_number,line);
   //materials
   if((strcmp(el1,"MATERIAL")==0)||(strcmp(el1,"material")==0))
      {
      sscanf(el2,"%d",&material_num_in_file);
      //associate number in file (material_number) with number in meshman (meshman_material_ptr)
      material_num=meshman_material_ptr;
      if(meshman_material_ptr<meshman_max_num_materials)
         {
         meshman_material_ptr++;
         material_lookup[material_num_in_file]=material_num;
         meshman_material[material_num].material_number_in_file=material_num_in_file;
         meshman_material[material_num].bitmap_on=0;
         meshman_material[material_num].wireframe_on=0;
         meshman_material[material_num].diffuse_on=0;
         meshman_material[material_num].specular_on=0;
         meshman_material[material_num].ambient_on=0;
         }
      }
   else if ((strcmp(el1,"WIRE_FRAME")==0)||(strcmp(el1,"wire_frame")==0))
      {
      meshman_material[material_num].wireframe_on=1;  //1=wireframe, use diffuse_red,green,blue as wireframe colours
      sscanf(el2,"%f",&meshman_material[material_num].diffuse_red);
      sscanf(el3,"%f",&meshman_material[material_num].diffuse_green);
      sscanf(el4,"%f",&meshman_material[material_num].diffuse_blue);
      }
   else if ((strcmp(el1,"DIFFUSE")==0)||(strcmp(el1,"diffuse")==0))
      {
      meshman_material[material_num].diffuse_on=1;  //1=wireframe, use wireframe_red,green,blue as line colours
      sscanf(el2,"%f",&meshman_material[material_num].diffuse_red);
      sscanf(el3,"%f",&meshman_material[material_num].diffuse_green);
      sscanf(el4,"%f",&meshman_material[material_num].diffuse_blue);
      }
   else if ((strcmp(el1,"SPECULAR")==0)||(strcmp(el1,"specular")==0))
      {
      meshman_material[material_num].specular_on=1;  //1=wireframe, use wireframe_red,green,blue as line colours
      sscanf(el2,"%f",&meshman_material[material_num].specular_red);
      sscanf(el3,"%f",&meshman_material[material_num].specular_green);
      sscanf(el4,"%f",&meshman_material[material_num].specular_blue);
      }
   else if ((strcmp(el1,"AMBIENT")==0)||(strcmp(el1,"ambient")==0))
      {
      meshman_material[material_num].ambient_on=1;  //1=wireframe, use wireframe_red,green,blue as line colours
      sscanf(el2,"%f",&meshman_material[material_num].ambient_red);
      sscanf(el3,"%f",&meshman_material[material_num].ambient_green);
      sscanf(el4,"%f",&meshman_material[material_num].ambient_blue);
      }
   //start/end meshes
   else if ((strcmp(el1,"BITMAP")==0)||(strcmp(el1,"bitmap")==0))
      {
      int bitmap_id=mesh_management_add_bitmap(el2);  //returns -1 if problem, meshman_bitmap pointer otherwise
      meshman_material[material_num].bitmap_num=bitmap_id;
      meshman_material[material_num].bitmap_on=1;
      num_bitmaps_present++;
      }
   else if((strcmp(el1,"MESH")==0)||(strcmp(el1,"mesh")==0))
      {
      if((strcmp(el2,"START")==0)||(strcmp(el2,"start")==0))
         {
         mesh_num=meshman_free_mesh_ptr;
         meshman_mesh[mesh_num].wireframe_on=0;
         meshman_mesh[mesh_num].start_face=meshman_free_face_ptr;
         meshman_mesh[mesh_num].start_face_t=meshman_free_facetexture_ptr;
         meshman_mesh[mesh_num].num_faces=0;
         meshman_mesh[mesh_num].base_vertex3d_ptr=meshman_free_3dpoint_ptr;
         meshman_mesh[mesh_num].num_3d_vertices=0;
         meshman_mesh[mesh_num].base_vertextex_ptr=meshman_free_texpoint_ptr;
         meshman_mesh[mesh_num].num_tex_vertices=0;
         meshman_mesh[mesh_num].base_facecolour_ptr=meshman_free_facecolour_ptr;
         meshman_mesh[mesh_num].num_face_colours=0;
         meshman_mesh[mesh_num].base_normal_ptr=meshman_free_normal_ptr;
         meshman_mesh[mesh_num].num_normals=0;
         meshman_mesh[mesh_num].normal_per_vertex=-1;  
         meshman_mesh[mesh_num].num_sides=3;   //assume triangle faces until otherwise stated
         num_faces_this_mesh=0; num_3dvtx_this_mesh=0; num_texfaces_this_mesh=0; num_texvtx_this_mesh=0;
         num_facenormals_this_mesh=0; num_normalvtx_this_mesh=0;
         }
      if((strcmp(el2,"END")==0)||(strcmp(el2,"end")==0)) 
         {
         //assign material
         if(material_ref!=-1)
            {
            int temp_meshman_mat_num;
            //printf("Material %d applied - ",material_ref);
            temp_meshman_mat_num=material_lookup[material_ref];
            meshman_mesh[mesh_num].material=temp_meshman_mat_num;
            }
         else
            {
            //printf("geomobject has no material or wireframe colour attached: default white material applied\n");
            num_meshes_without_materials++;
            //assign default white colour to mesh
            meshman_mesh[mesh_num].material=meshman_default_material;
            }
         //advance mesh count
         if(mesh_num!=-1) meshman_object_model[object_num].end_mesh=mesh_num;
         if(meshman_free_mesh_ptr<meshman_max_num_meshes) {meshman_free_mesh_ptr++; num_meshes_loaded++;}
         mesh_num=-1; material_ref=-1; bitmap_id=-1;
         }
      }
   //configure meshes
   else if((strcmp(el1,"NUM_SIDES")==0)||(strcmp(el1,"num_sides")==0))
      {
      sscanf(el2,"%d",&num_sides);
      if((num_sides<0)||(num_sides>8))
          printf("WARNING: %d=invalid number of sides per poly.  Only supports 1,2,3, or 4 sides/poly but can store up to 8\n"
                 "line %d invalid, ignored.  line reads <%s>\n",num_sides,line_number,line);
      else meshman_mesh[mesh_num].num_sides=num_sides;
      }
   else if((strcmp(el1,"MATERIAL_ID")==0)||(strcmp(el1,"material_id")==0))
      sscanf(el2,"%d",&material_ref);
   //enter mesh data
   else if((strcmp(el1,"FACE")==0)||(strcmp(el1,"face")==0))
      {
      int p;   //only support <=4 sides per poly
      sscanf(el2,"%d",&p);
      meshman_face[meshman_free_face_ptr].vertex3d_num=(unsigned short int)p;
      if(meshman_free_face_ptr<meshman_max_num_faces) meshman_free_face_ptr++;
      if(num_sides>=2)
         {
         sscanf(el3,"%d",&p);
         meshman_face[meshman_free_face_ptr].vertex3d_num=(unsigned short int)p;
         if(meshman_free_face_ptr<meshman_max_num_faces) meshman_free_face_ptr++;
         }
      if(num_sides>=3)
         {
         sscanf(el4,"%d",&p);
         meshman_face[meshman_free_face_ptr].vertex3d_num=(unsigned short int)p;
         if(meshman_free_face_ptr<meshman_max_num_faces) meshman_free_face_ptr++;
         }
      if(num_sides>=4)
         {
         sscanf(el5,"%d",&p);
         meshman_face[meshman_free_face_ptr].vertex3d_num=(unsigned short int)p;
         if(meshman_free_face_ptr<meshman_max_num_faces) meshman_free_face_ptr++;
         }
      meshman_mesh[mesh_num].num_faces++;
      num_faces_loaded++;
      num_faces_this_mesh++;
      }
   else if((strcmp(el1,"VERTEX")==0)||(strcmp(el1,"vertex")==0))
      {
      float x,y,z;
      sscanf(el2,"%f",&x); sscanf(el3,"%f",&y); sscanf(el4,"%f",&z);
      meshman_3dpoint[meshman_free_3dpoint_ptr].x=x;
      meshman_3dpoint[meshman_free_3dpoint_ptr].y=y;
      meshman_3dpoint[meshman_free_3dpoint_ptr].z=z;
      meshman_mesh[mesh_num].num_3d_vertices++;
      if(meshman_free_3dpoint_ptr<meshman_max_num_3dpoints)
         {
         meshman_free_3dpoint_ptr++;
         num_3dvertices_loaded++;
         num_3dvtx_this_mesh++;
         }
      }
   else if((strcmp(el1,"TEXFACE")==0)||(strcmp(el1,"texface")==0))
      {
      int p;   //only support <=4 sides per poly
      sscanf(el2,"%d",&p);
      meshman_facetexture[meshman_free_facetexture_ptr].texvertex_num=(unsigned short int)p;
      if(meshman_free_facetexture_ptr<meshman_max_num_facetexture) meshman_free_facetexture_ptr++;
      if(num_sides>=2)
         {
         sscanf(el3,"%d",&p);
         meshman_facetexture[meshman_free_facetexture_ptr].texvertex_num=(unsigned short int)p;
         if(meshman_free_facetexture_ptr<meshman_max_num_facetexture) meshman_free_facetexture_ptr++;
         }
      if(num_sides>=3)
         {
         sscanf(el4,"%d",&p);
         meshman_facetexture[meshman_free_facetexture_ptr].texvertex_num=(unsigned short int)p;
         if(meshman_free_facetexture_ptr<meshman_max_num_facetexture) meshman_free_facetexture_ptr++;
         }
      if(num_sides>=4)
         {
         sscanf(el5,"%d",&p);
         meshman_facetexture[meshman_free_facetexture_ptr].texvertex_num=(unsigned short int)p;
         if(meshman_free_facetexture_ptr<meshman_max_num_facetexture) meshman_free_facetexture_ptr++;
         }
      num_texfaces_loaded++;
      num_texfaces_this_mesh++;
      }
   else if((strcmp(el1,"TEXVERTEX")==0)||(strcmp(el1,"texvertex")==0))
      {
      float tx,ty;
      sscanf(el2,"%f",&tx); sscanf(el3,"%f",&ty);
      meshman_texpoint[meshman_free_texpoint_ptr].u=tx;
      meshman_texpoint[meshman_free_texpoint_ptr].v=ty;
      meshman_mesh[mesh_num].num_tex_vertices++;
      if(meshman_free_texpoint_ptr<meshman_max_num_texpoints)
         {
         meshman_free_texpoint_ptr++;
         num_texvertices_loaded++;
         num_texvtx_this_mesh++;
         }
      }
   else if((strcmp(el1,"FACENORMAL")==0)||(strcmp(el1,"facenormal")==0))
      {
      float nx,ny,nz;
      sscanf(el2,"%f",&nx); sscanf(el3,"%f",&ny); sscanf(el4,"%f",&nz);
      meshman_normal[meshman_free_normal_ptr].nx=nx;
      meshman_normal[meshman_free_normal_ptr].ny=ny;
      meshman_normal[meshman_free_normal_ptr].nz=nz;
      if(meshman_mesh[mesh_num].normal_per_vertex==1)
          printf("WARNING: Only either 'facenormal' or 'vtxnormal' tokens valid in a mesh\n"
               "line %d invalid, ignored.  line reads <%s>\n",line_number,line);
      meshman_mesh[mesh_num].normal_per_vertex=0;
      if(meshman_free_normal_ptr<meshman_max_num_normals)
         {
         meshman_free_normal_ptr++;
         meshman_mesh[mesh_num].num_normals++;
         num_normalvectors_loaded++;
         num_facenormals_this_mesh++;
         }
      }
   else if((strcmp(el1,"VTXNORMAL")==0)||(strcmp(el1,"vtxnormal")==0))
      {
      float nx,ny,nz;
      sscanf(el2,"%f",&nx); sscanf(el3,"%f",&ny); sscanf(el4,"%f",&nz);
      meshman_normal[meshman_free_normal_ptr].nx=nx;
      meshman_normal[meshman_free_normal_ptr].ny=ny;
      meshman_normal[meshman_free_normal_ptr].nz=nz;
      if(meshman_mesh[mesh_num].normal_per_vertex==0)
          printf("WARNING: Only either 'facenormal' or 'vtxnormal' tokens valid in a mesh\n"
               "line %d invalid, ignored.  line reads <%s>\n",line_number,line);
      meshman_mesh[mesh_num].normal_per_vertex=1;
      if(meshman_free_normal_ptr<meshman_max_num_normals)
         {
         meshman_free_normal_ptr++;
         meshman_mesh[mesh_num].num_normals++;
         num_normalvectors_loaded++;
         num_facenormals_this_mesh++;
         }
      }
   }
fclose(READ_LINE_IN);

if(mesh_num!=-1) printf("Warning: <%s> ends inside a mesh, no MESH END line found before file ended\n",filename);

if(num_meshes_without_materials>0)
   printf("WARNING: %d meshes had no material: default white material applied\n",num_meshes_without_materials);


//printf("%d 3D and %d texture vertices loaded. %d normal vectors loaded. %d faces and %d texture faces loaded\n",
//       num_3dvertices_loaded,num_texvertices_loaded,num_normalvectors_loaded,num_faces_loaded,num_texfaces_loaded);

//increment for next object
if(meshman_free_object_model_ptr<meshman_max_num_object_models) meshman_free_object_model_ptr++;

//warnings for memory filling up
if(meshman_free_object_model_ptr>=meshman_max_num_object_models)
   printf("MEMORY WARNING: limit of %d objects was reached, some will not be loaded\n",meshman_max_num_object_models);
if(meshman_free_mesh_ptr>=meshman_max_num_meshes)
   printf("MEMORY WARNING: limit of %d meshes was reached, some will not be loaded\n",meshman_max_num_meshes);
if(meshman_free_face_ptr>=meshman_max_num_faces)
   printf("MEMORY WARNING: limit of %d face_pointers (one per vertex) was reached, some will not be loaded\n",meshman_free_face_ptr);
if(meshman_free_3dpoint_ptr>=meshman_max_num_3dpoints)
   printf("MEMORY WARNING: limit of %d 3D vertices was reached, some will not be loaded\n",meshman_max_num_3dpoints);
if(meshman_free_normal_ptr>=meshman_max_num_normals)
   printf("MEMORY WARNING: limit of %d normal vectors was reached, some will not be loaded\n",meshman_free_normal_ptr);
if(meshman_free_facetexture_ptr>=meshman_max_num_facetexture)
   printf("MEMORY WARNING: limit of %d face_texture_pointers (one per vertex) was reached, some will not be loaded\n",meshman_max_num_facetexture);
if(meshman_free_texpoint_ptr>=meshman_max_num_texpoints)
   printf("MEMORY WARNING: limit of %d texture vertices (one per vertex) was reached, some will not be loaded\n",meshman_max_num_texpoints);
if(meshman_material_ptr>=meshman_max_num_materials)
   printf("MEMORY WARNING: limit of %d texture vertices (one per vertex) was reached, some will not be loaded\n",meshman_material_ptr);

//return stats to calling functions
*num_meshes=num_meshes_loaded;
*num_vtxs=num_3dvertices_loaded;
*num_triangles=num_faces_loaded;
*num_normals=num_normalvectors_loaded;
*num_bitmaps=num_bitmaps_present;
if(material_lookup!=NULL) free(material_lookup);  //clean up memory used
return object_num;
}






//-------------------------------------------- Bitmaps --------------------------------------------

//bitmap_id=mesh_management_add_bitmap(bitmap_filename);  //returns -1 if problem, meshman_bitmap pointer otherwise
int mesh_management_add_bitmap(char *filename)
{
char bitmap_exists=0;
int i,bitmap_id;

//check for existing bitmap names
for(i=0;i<meshman_bitmap_ptr;i++)
   {
   if(strcmp(filename,meshman_bitmap[i].filename)==0)
   {bitmap_id=i;bitmap_exists=1;}
   }
if(bitmap_exists==0)
   {
   if(meshman_bitmap_ptr>=meshman_max_num_materials) return -1;  //ran out of bitmap slots
   strcpy(meshman_bitmap[meshman_bitmap_ptr].filename,filename);
   meshman_bitmap[meshman_bitmap_ptr].image=NULL;
   bitmap_id=meshman_bitmap_ptr;
   meshman_bitmap_ptr++;
   }
//material2bitmapnum_lookup[material_number]=bitmap_id;
return bitmap_id;
}



//bitmaps_loaded=mesh_management_load_bitmaps();
int mesh_management_load_bitmaps(void)
{
int i,num_bitmaps_successfully_loaded=0;
unsigned char *temp;

for(i=0;i<meshman_bitmap_ptr;i++)
   {
   //check if bitmap has been loaded yet by looking for a NULL image pointer
   if(meshman_bitmap[i].image==NULL)
      {
      char root[256],suffix[256];
      meshman_split_filename(meshman_bitmap[i].filename,root,suffix);
      temp=NULL;
      if((strcmp(suffix,"PGM")==0)||(strcmp(suffix,"pgm")==0))
         {
         temp=meshman_read_pgm(meshman_bitmap[i].filename,
                               &meshman_bitmap[i].original_width,&meshman_bitmap[i].original_height);
         meshman_bitmap[i].rgb_greybar=0;
         }
      else if((strcmp(suffix,"PPM")==0)||(strcmp(suffix,"ppm")==0))
         {
         temp=meshman_read_ppm(meshman_bitmap[i].filename,
                               &meshman_bitmap[i].original_width,&meshman_bitmap[i].original_height);
         meshman_bitmap[i].rgb_greybar=1;
         }
#ifdef USE_HIGHGUI
         //-- Other image types such as JPG, GIF, BMP, etc loaded with OpenCV's highgui
      //-- WARNING:  TGA file type not supported (with OpenCV 3.1)
      //-- WARNING:  image width should be multiple of 4 - problems with highgui experienced otherwise (with OpenCV 3.1)
      else 
         {
         IplImage *opencv_temp;
         opencv_temp=cvLoadImage(meshman_bitmap[i].filename, 3);  //load as RGB
         if(opencv_temp==NULL)
            printf("WARNING: Can't load <%s>\n",meshman_bitmap[i].filename);
         else
            {
            int j;
            temp=(unsigned char*)opencv_temp->imageData;
            meshman_bitmap[i].original_width=opencv_temp->width;
            meshman_bitmap[i].original_height=opencv_temp->height;
            meshman_bitmap[i].rgb_greybar=1;
            //flip R,B colours
            for(j=0;j<meshman_bitmap[i].original_width*meshman_bitmap[i].original_height*3;j+=3)
                  {
               unsigned char red=temp[j+2],blue=temp[j+0];
               temp[j+0]=red; temp[j+2]=blue;
               }
            }
         }
#endif
      if(temp!=NULL) //if bitmap was just loaded, it will be in *temp
         {
         int wbit,hbit,width,height,mem_size;
         //debug - see what was actually loaded
         if(MESHMAN_DEBUG_TEXTURE_PPM)
            {
            char root[512],suffix[256],debug_ppm_filename[256];
            meshman_split_filename(meshman_bitmap[i].filename,root,suffix);
            sprintf(debug_ppm_filename,"%s_loaded.ppm",root);
            meshman_write_ppm(debug_ppm_filename,"",temp,meshman_bitmap[i].original_width,meshman_bitmap[i].original_height);
            printf("Wrote %s\n",debug_ppm_filename);
            }
         //find appropriate texture image size
         wbit=1; width=1<<wbit;
         while((wbit<=13)&&(width<meshman_bitmap[i].original_width)) {wbit++; width=1<<wbit;}
         hbit=1; height=1<<hbit;
         while((hbit<=13)&&(height<meshman_bitmap[i].original_height)) {hbit++; height=1<<hbit;}
         if((wbit<=13)&&(hbit<=13))  //suitable size was found
            {
            if(MESHMAN_DEBUG_TEXTURE_PPM)
               printf("original_width,height=%d,%d  chose width,height=%d,%d\n",
                      meshman_bitmap[i].original_width,meshman_bitmap[i].original_height,width,height);
            //allocate memory for texture
            mem_size=width*height;
            if(meshman_bitmap[i].rgb_greybar) mem_size*=3;
            meshman_bitmap[i].image=(unsigned char*)malloc(mem_size);
            if(meshman_bitmap[i].image==NULL)
               printf("meshman: WARNING: Can't malloc %d bytes for <%s>\n",mem_size,meshman_bitmap[i].filename);
            else
               {
               //memory allocated, now either resize or copy in image
               if((meshman_bitmap[i].original_width!=width)||(meshman_bitmap[i].original_height!=height))
                  {
                  //resize texture using bi-linear interpolation
                  float scalex=(float)meshman_bitmap[i].original_width/(float)width;
                  float scaley=(float)meshman_bitmap[i].original_height/(float)height;
                  int x,y,u,v;   //x,y output coords, u,v original texture coords
                  float uu,vv,aa,bb,cc,dd;
                  unsigned char p00,p01,p10,p11,pixel;
                  if(meshman_bitmap[i].rgb_greybar)
                     {
                     //resize RGB texture
                     int row_width=meshman_bitmap[i].original_width*3;
                     int rgb_offset;  //three colour channels, =0,1,2
                     for(y=0;y<height;y++)
                        for(x=0;x<width;x++)
                           {
                           uu=(float)x*scalex; vv=(float)y*scaley;
                           u=(int)uu; v=(int)vv;
                           uu-=(float)u; vv-=(float)v;
                           if(v>=meshman_bitmap[i].original_height-1) v--;  //stop reading beyond source image memory
                           for(rgb_offset=0;rgb_offset<3;rgb_offset++)
                                 {
                              //perform bilinear interpolation
                              p00=*(temp+v*row_width+u*3+rgb_offset);
                              p01=*(temp+v*row_width+(u+1)*3+rgb_offset);
                              p10=*(temp+(v+1)*row_width+u*3+rgb_offset);
                              p11=*(temp+(v+1)*row_width+(u+1)*3+rgb_offset);
                              aa=(float)p00;  bb=(float)(p01-p00);
                              cc=(float)(p10-p00); dd=(float)(p00+p11-p01-p10);
                              pixel=(unsigned char)(aa+bb*uu+cc*vv+dd*uu*vv);
                              *(meshman_bitmap[i].image+y*width*3+x*3+rgb_offset)=pixel;
                              }
                           }
                     }
                   else
                     {
                     //resize greyscale texture
                     int row_width=meshman_bitmap[i].original_width*3;
                     for(y=0;y<height;y++)
                        for(x=0;x<height;x++)
                           {
                           uu=(float)x*scalex; vv=(float)y*scaley;
                           u=(int)uu; v=(int)vv;
                           uu-=(float)u; vv-=(float)v;
                           //perform bilinear interpolation
                           p00=*(temp+v*row_width+u);
                           p01=*(temp+v*row_width+u+1);
                           p10=*(temp+(v+1)*row_width+u);
                           p11=*(temp+(v+1)*row_width+u+1);
                           aa=(float)p00;  bb=(float)(p01-p00);
                           cc=(float)(p10-p00); dd=(float)(p00+p11-p01-p10);
                           pixel=(unsigned char)(aa+bb*uu+cc*vv+dd*uu*vv);
                           *(meshman_bitmap[i].image+y*width+x)=pixel;
                           }
                     }
                  }
               else
                  {
                  //no resize needed - simple copy of texture image data
                  memcpy(meshman_bitmap[i].image,temp,mem_size);
                  }
               meshman_bitmap[i].width=width;
               meshman_bitmap[i].height=height;
               num_bitmaps_successfully_loaded++;
#ifdef OPENGL_PROJECT
               //--- bind to OpenGL ---
               meshman_opengl_bind_texture(i);
#endif //#ifdef OPENGL_PROJECT
               //debug - see result of resizing
               if(MESHMAN_DEBUG_TEXTURE_PPM)
                  {
                  char root[512],suffix[256],debug_ppm_filename[256];
                  meshman_split_filename(meshman_bitmap[i].filename,root,suffix);
                  sprintf(debug_ppm_filename,"%s_resized.ppm",root);
                  meshman_write_ppm(debug_ppm_filename,"",meshman_bitmap[i].image,meshman_bitmap[i].width,meshman_bitmap[i].height);
                  printf("Wrote %s\n",debug_ppm_filename);
                  }
               }
            }//if((wbit<=13)&&(hbit<=13))  //suitable size was found
         //if(temp!=NULL) free(temp);
         }//   if(temp!=NULL) //bitmap was just loaded into *temp
      } //if(meshman_bitmap[i].image==NULL)
   }//for(i=0;i<meshman_bitmap_ptr;i++)


return num_bitmaps_successfully_loaded;
}





char meshman_split_filename(char *filename, char *root, char *suffix)
{
int i;
//find .
i=strlen(filename)-1;
if(i<3) return -1;
while((filename[i]!='.')&&(i>=0)) i--;
//{printf("%d <%c>\n",i,filename[i]);i--;}
if(i==0) return -1;
//make root and suffix
strcpy(root,filename);
root[i]=0;
strcpy(suffix,&(filename[i+1]));

return 0;
}


unsigned char* meshman_read_pgm(char *file_name, int *width, int *height)
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
if(image==NULL) {printf("meshman_read_pgm() error: Couldn't malloc image\n");exit(1);}

for(i=0;i<size;i++)
   {
   uc=fgetc(in_file);
   *(image+i)=uc;
   }
fclose(in_file);
return image;
}




unsigned char* meshman_read_ppm(char *file_name, int *width, int *height)
{
int i,size;
int max_grey;
FILE *in_file;
char c,comment[256]; 
unsigned char uc,*image;

in_file=fopen(file_name,"rb");
if(in_file==NULL)  return NULL;

c=fgetc(in_file); if(c!='P') {printf("%s Not a PPM file\n",file_name);exit(1);}
c=fgetc(in_file); if(c!='6') {printf("%s Not a PPM file\n",file_name);exit(1);}
c=fgetc(in_file); if(c!='\n') {printf("%s Not a PPM file\n",file_name);exit(1);}
while(1)
    {
    fgets(comment,256,in_file);
    if((comment[0]!='#')&&(comment[0]!=0x0a)&&(comment[0]!=0x0d)) break;
    }
sscanf(comment,"%d %d",width,height);
fgets(comment,256,in_file);
sscanf(comment,"%d",&max_grey);

size=(*width)*(*height);
image=(unsigned char*)malloc(size*3+10);
if(image==NULL) {printf("meshman_read_ppm() error: Couldn't malloc image\n");exit(1);}

for(i=0;i<size*3;i++)
   {
   uc=fgetc(in_file);
   *(image+i)=uc;
   }
fclose(in_file);
return image;
}











//**************************************************************************************//
//*****************************   OpenGL Routines   ************************************//
//**************************************************************************************//

#ifdef OPENGL_PROJECT

//meshman_render_opengl(meshman_object_model_num,object_draw_bitmap,wireframe_mode);
void meshman_render_opengl(int meshman_object_model_num, char object_draw_bitmap,
                      char wireframe_mode)
{
int mesh_num;
for(mesh_num=meshman_object_model[meshman_object_model_num].start_mesh;
   mesh_num<=meshman_object_model[meshman_object_model_num].end_mesh;
   mesh_num++)
   {
   char draw_bitmap=0;
   int bitmap_num,material_num=meshman_mesh[mesh_num].material;
   int vertex3d_ptr,base_vertex3d_ptr=meshman_mesh[mesh_num].base_vertex3d_ptr;
   int normal_ptr=meshman_mesh[mesh_num].base_normal_ptr;
   int num_normals=meshman_mesh[mesh_num].num_normals;
   char normal_per_vertex=meshman_mesh[mesh_num].normal_per_vertex;
   char set_v2v3v4_normal=(normal_per_vertex)&&(num_normals>0) ? 1:0;
   int vertextex_ptr,base_vertextex_ptr=meshman_mesh[mesh_num].base_vertextex_ptr;
   int face_num;
   int face_ptr=meshman_mesh[mesh_num].start_face;
   int texface_ptr=meshman_mesh[mesh_num].start_face_t;
   int base_facecolour_ptr=meshman_mesh[mesh_num].base_facecolour_ptr;

   //set colour for mesh
//   glColor3f(1.0f, 1.0f, 1.0f);  //default
   glColor3f(meshman_material[material_num].diffuse_red,
             meshman_material[material_num].diffuse_green,
             meshman_material[material_num].diffuse_blue);
/*   if(meshman_material[material_num].diffuse_on)
      {
      GLfloat mat_diffuse[]={meshman_material[material_num].diffuse_red,
                             meshman_material[material_num].diffuse_green,
                             meshman_material[material_num].diffuse_blue,
                             1.0};
      glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
      }
   if(meshman_material[material_num].specular_on)
      {
      GLfloat mat_specular[]={meshman_material[material_num].specular_red,
                                      meshman_material[material_num].specular_green,
                                      meshman_material[material_num].specular_blue,
                                      1.0};
      glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
      }
   if(meshman_material[material_num].ambient_on)
      {
      GLfloat mat_ambient[]={meshman_material[material_num].ambient_red,
                                      meshman_material[material_num].ambient_green,
                                      meshman_material[material_num].ambient_blue,
                                      1.0};
      glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
      }*/
   //decide to draw texture or not
   draw_bitmap=0;
   if((object_draw_bitmap)&&(meshman_material[material_num].bitmap_on))
      {
      bitmap_num=meshman_material[material_num].bitmap_num;
      if(meshman_bitmap[bitmap_num].image!=NULL)  draw_bitmap=1;
      else                                        draw_bitmap=0;
//      if(meshman_bitmap[bitmap_num].image==NULL)  {printf("-------Can't draw bitmap %d\n",bitmap_num);exit(1);}
      }

//        printf("mesh_num=%d num_sides=%d num_faces=%d\n",
//               mesh_num,meshman_mesh[mesh_num].num_sides,meshman_mesh[mesh_num].num_faces);
   //------ draw LINES - partially unrolled to speed up 2-sided polygons ----
   if(meshman_mesh[mesh_num].num_sides==2)
      {
      glBegin(GL_LINES);
      for(face_num=0;face_num<meshman_mesh[mesh_num].num_faces;face_num++)
         {
         vertex3d_ptr=base_vertex3d_ptr+meshman_face[face_ptr].vertex3d_num;
         glVertex3f(meshman_3dpoint[vertex3d_ptr].x,
                    meshman_3dpoint[vertex3d_ptr].y,
                    meshman_3dpoint[vertex3d_ptr].z);
         face_ptr++;
         vertex3d_ptr=base_vertex3d_ptr+meshman_face[face_ptr].vertex3d_num;
         glVertex3f(meshman_3dpoint[vertex3d_ptr].x,
                    meshman_3dpoint[vertex3d_ptr].y,
                    meshman_3dpoint[vertex3d_ptr].z);
         face_ptr++;
         }
      glEnd();
      }
   //------ draw TRIANGLES - partially unrolled to speed up 3-sided polygons ----
   if(meshman_mesh[mesh_num].num_sides==3)
      {
      if(draw_bitmap)
         {
         bitmap_num=meshman_material[material_num].bitmap_num;
         glEnable(GL_TEXTURE_2D);
         glBindTexture(GL_TEXTURE_2D,meshman_bitmap[bitmap_num].opengl_handle);
         //printf("drawing ISS material=%d  with handle=%d\n",
         //       material_num,meshman_bitmap[bitmap_num].opengl_handle);
         glBegin(GL_TRIANGLES);
         //render textured triangles
         for(face_num=0;face_num<meshman_mesh[mesh_num].num_faces;face_num++)
            {
            //first vertex
            if(num_normals>0)
               {
               glNormal3f(meshman_normal[normal_ptr].nx,
                          meshman_normal[normal_ptr].ny,meshman_normal[normal_ptr].nz);
               normal_ptr++;
               }
            vertextex_ptr=base_vertextex_ptr+meshman_facetexture[texface_ptr].texvertex_num;
            glTexCoord2f(meshman_texpoint[vertextex_ptr].u,
                         meshman_texpoint[vertextex_ptr].v);
            vertex3d_ptr=base_vertex3d_ptr+meshman_face[face_ptr].vertex3d_num;
            glVertex3f(meshman_3dpoint[vertex3d_ptr].x,
                       meshman_3dpoint[vertex3d_ptr].y,
                       meshman_3dpoint[vertex3d_ptr].z);
            texface_ptr++; face_ptr++;
            //second vertex
            if(set_v2v3v4_normal)
               {
               glNormal3f(meshman_normal[normal_ptr].nx,
                          meshman_normal[normal_ptr].ny,meshman_normal[normal_ptr].nz);
               normal_ptr++;
               }
            vertextex_ptr=base_vertextex_ptr+meshman_facetexture[texface_ptr].texvertex_num;
            glTexCoord2f(meshman_texpoint[vertextex_ptr].u,
                         meshman_texpoint[vertextex_ptr].v);
            vertex3d_ptr=base_vertex3d_ptr+meshman_face[face_ptr].vertex3d_num;
            glVertex3f(meshman_3dpoint[vertex3d_ptr].x,
                       meshman_3dpoint[vertex3d_ptr].y,
                       meshman_3dpoint[vertex3d_ptr].z);
            texface_ptr++; face_ptr++;
            //third vertex
            if(set_v2v3v4_normal)
               {
               glNormal3f(meshman_normal[normal_ptr].nx,
                          meshman_normal[normal_ptr].ny,meshman_normal[normal_ptr].nz);
               normal_ptr++;
               }
            vertextex_ptr=base_vertextex_ptr+meshman_facetexture[texface_ptr].texvertex_num;
            glTexCoord2f(meshman_texpoint[vertextex_ptr].u,
                         meshman_texpoint[vertextex_ptr].v);
            vertex3d_ptr=base_vertex3d_ptr+meshman_face[face_ptr].vertex3d_num;
            glVertex3f(meshman_3dpoint[vertex3d_ptr].x,
                       meshman_3dpoint[vertex3d_ptr].y,
                       meshman_3dpoint[vertex3d_ptr].z);
            texface_ptr++; face_ptr++; 
            }
         glEnd();
         }
      else //untextured triangles
         {
         glDisable(GL_TEXTURE_2D);
         //start triangles
         glBegin(GL_TRIANGLES);
         //render untextured triangles
         for(face_num=0;face_num<meshman_mesh[mesh_num].num_faces;face_num++)
            {
            //first vertex
            if(num_normals>0)
               {
               glNormal3f(meshman_normal[normal_ptr].nx,
                          meshman_normal[normal_ptr].ny,meshman_normal[normal_ptr].nz);
               normal_ptr++;
               }
            vertex3d_ptr=base_vertex3d_ptr+meshman_face[face_ptr].vertex3d_num;
            glVertex3f(meshman_3dpoint[vertex3d_ptr].x,
                       meshman_3dpoint[vertex3d_ptr].y,
                       meshman_3dpoint[vertex3d_ptr].z);
            face_ptr++;
            //second vertex
            if(set_v2v3v4_normal)
               {
               glNormal3f(meshman_normal[normal_ptr].nx,
                          meshman_normal[normal_ptr].ny,meshman_normal[normal_ptr].nz);
               normal_ptr++;
               }
            vertex3d_ptr=base_vertex3d_ptr+meshman_face[face_ptr].vertex3d_num;
            glVertex3f(meshman_3dpoint[vertex3d_ptr].x,
                       meshman_3dpoint[vertex3d_ptr].y,
                       meshman_3dpoint[vertex3d_ptr].z);
            face_ptr++;
            //third vertex
            if(set_v2v3v4_normal)
               {
               glNormal3f(meshman_normal[normal_ptr].nx,
                          meshman_normal[normal_ptr].ny,meshman_normal[normal_ptr].nz);
               normal_ptr++;
               }
            vertex3d_ptr=base_vertex3d_ptr+meshman_face[face_ptr].vertex3d_num;
            glVertex3f(meshman_3dpoint[vertex3d_ptr].x,
                       meshman_3dpoint[vertex3d_ptr].y,
                          meshman_3dpoint[vertex3d_ptr].z);
            face_ptr++;
            }
         glEnd();
         }  //else //untextured triangles
      }//if(meshman_mesh[mesh_num]==3)
   //------ draw QUADS - partially unrolled to speed up 4-sided polygons ----
   if(meshman_mesh[mesh_num].num_sides==4)
      {
      if(draw_bitmap)
         {
         bitmap_num=meshman_material[material_num].bitmap_num;
         glEnable(GL_TEXTURE_2D);
         glBindTexture(GL_TEXTURE_2D,meshman_bitmap[bitmap_num].opengl_handle);
         //printf("drawing ISS material=%d  with handle=%d\n",
         //         material_num,meshman_bitmap[bitmap_num].opengl_handle);
         glBegin(GL_QUADS);
         //render textured quads
         for(face_num=0;face_num<meshman_mesh[mesh_num].num_faces;face_num++)
            {
            //first vertex
            if(num_normals>0)
               {
               glNormal3f(meshman_normal[normal_ptr].nx,
                          meshman_normal[normal_ptr].ny,meshman_normal[normal_ptr].nz);
               normal_ptr++;
               }
            vertextex_ptr=base_vertextex_ptr+meshman_facetexture[texface_ptr].texvertex_num;
            glTexCoord2f(meshman_texpoint[vertextex_ptr].u,
                         meshman_texpoint[vertextex_ptr].v);
            vertex3d_ptr=base_vertex3d_ptr+meshman_face[face_ptr].vertex3d_num;
            glVertex3f(meshman_3dpoint[vertex3d_ptr].x,
                       meshman_3dpoint[vertex3d_ptr].y,
                       meshman_3dpoint[vertex3d_ptr].z);
            texface_ptr++; face_ptr++;
            //second vertex
            if(set_v2v3v4_normal)
               {
               glNormal3f(meshman_normal[normal_ptr].nx,
                          meshman_normal[normal_ptr].ny,meshman_normal[normal_ptr].nz);
               normal_ptr++;
               }
            vertextex_ptr=base_vertextex_ptr+meshman_facetexture[texface_ptr].texvertex_num;
            glTexCoord2f(meshman_texpoint[vertextex_ptr].u,
                         meshman_texpoint[vertextex_ptr].v);
            vertex3d_ptr=base_vertex3d_ptr+meshman_face[face_ptr].vertex3d_num;
            glVertex3f(meshman_3dpoint[vertex3d_ptr].x,
                       meshman_3dpoint[vertex3d_ptr].y,
                       meshman_3dpoint[vertex3d_ptr].z);
            texface_ptr++; face_ptr++;
            //third vertex
            if(set_v2v3v4_normal)
               {
               glNormal3f(meshman_normal[normal_ptr].nx,
                          meshman_normal[normal_ptr].ny,meshman_normal[normal_ptr].nz);
               normal_ptr++;
               }
            vertextex_ptr=base_vertextex_ptr+meshman_facetexture[texface_ptr].texvertex_num;
            glTexCoord2f(meshman_texpoint[vertextex_ptr].u,
                         meshman_texpoint[vertextex_ptr].v);
            vertex3d_ptr=base_vertex3d_ptr+meshman_face[face_ptr].vertex3d_num;
            glVertex3f(meshman_3dpoint[vertex3d_ptr].x,
                       meshman_3dpoint[vertex3d_ptr].y,
                       meshman_3dpoint[vertex3d_ptr].z);
            texface_ptr++; face_ptr++;
            //fourth vertex
            if(set_v2v3v4_normal)
               {
               glNormal3f(meshman_normal[normal_ptr].nx,
                          meshman_normal[normal_ptr].ny,meshman_normal[normal_ptr].nz);
               normal_ptr++;
               }
            vertextex_ptr=base_vertextex_ptr+meshman_facetexture[texface_ptr].texvertex_num;
            glTexCoord2f(meshman_texpoint[vertextex_ptr].u,
                         meshman_texpoint[vertextex_ptr].v);
            vertex3d_ptr=base_vertex3d_ptr+meshman_face[face_ptr].vertex3d_num;
            glVertex3f(meshman_3dpoint[vertex3d_ptr].x,
                       meshman_3dpoint[vertex3d_ptr].y,
                       meshman_3dpoint[vertex3d_ptr].z);
            texface_ptr++; face_ptr++;
            }
         glEnd();
         }
      else //untextured quads
         {
         glDisable(GL_TEXTURE_2D);
         //start triangles
         glBegin(GL_QUADS);
         //render untextured triangles
         for(face_num=0;face_num<meshman_mesh[mesh_num].num_faces;face_num++)
            {
            //first vertex
            if(num_normals>0)
               {
               glNormal3f(meshman_normal[normal_ptr].nx,
                          meshman_normal[normal_ptr].ny,meshman_normal[normal_ptr].nz);
               normal_ptr++;
               }
            vertex3d_ptr=base_vertex3d_ptr+meshman_face[face_ptr].vertex3d_num;
            glVertex3f(meshman_3dpoint[vertex3d_ptr].x,
                       meshman_3dpoint[vertex3d_ptr].y,
                       meshman_3dpoint[vertex3d_ptr].z);
            face_ptr++;
            //second vertex
            if(set_v2v3v4_normal)
               {
               glNormal3f(meshman_normal[normal_ptr].nx,
                          meshman_normal[normal_ptr].ny,meshman_normal[normal_ptr].nz);
               normal_ptr++;
               }
            vertex3d_ptr=base_vertex3d_ptr+meshman_face[face_ptr].vertex3d_num;
            glVertex3f(meshman_3dpoint[vertex3d_ptr].x,
                       meshman_3dpoint[vertex3d_ptr].y,
                       meshman_3dpoint[vertex3d_ptr].z);
            face_ptr++;
            //third vertex
            if(set_v2v3v4_normal)
               {
               glNormal3f(meshman_normal[normal_ptr].nx,
                          meshman_normal[normal_ptr].ny,meshman_normal[normal_ptr].nz);
               normal_ptr++;
               }
            vertex3d_ptr=base_vertex3d_ptr+meshman_face[face_ptr].vertex3d_num;
            glVertex3f(meshman_3dpoint[vertex3d_ptr].x,
                       meshman_3dpoint[vertex3d_ptr].y,
                          meshman_3dpoint[vertex3d_ptr].z);
            face_ptr++;
            //fourth vertex
            if(set_v2v3v4_normal)
               {
               glNormal3f(meshman_normal[normal_ptr].nx,
                          meshman_normal[normal_ptr].ny,meshman_normal[normal_ptr].nz);
               normal_ptr++;
               }
            vertex3d_ptr=base_vertex3d_ptr+meshman_face[face_ptr].vertex3d_num;
            glVertex3f(meshman_3dpoint[vertex3d_ptr].x,
                       meshman_3dpoint[vertex3d_ptr].y,
                          meshman_3dpoint[vertex3d_ptr].z);
            face_ptr++;
            }
         glEnd();
         }  //else //untextured quads
      }//if(meshman_mesh[mesh_num]==4)
   }//    for(mesh_num=...
}




void meshman_opengl_bind_texture(int bitmap_num)
{
glGenTextures(1, &meshman_bitmap[bitmap_num].opengl_handle);
//printf("meshman_bitmap[bitmap_num].opengl_handle=%d for file <%s>\n",
//       meshman_bitmap[bitmap_num].opengl_handle,meshman_bitmap[bitmap_num].filename);
glBindTexture(GL_TEXTURE_2D,meshman_bitmap[bitmap_num].opengl_handle);
glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
if(meshman_bitmap[bitmap_num].rgb_greybar)
 glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,meshman_bitmap[bitmap_num].width,meshman_bitmap[bitmap_num].height, 0,
              GL_RGB, GL_UNSIGNED_BYTE,meshman_bitmap[bitmap_num].image);
else
 glTexImage2D(GL_TEXTURE_2D, 0, 1,meshman_bitmap[bitmap_num].width,meshman_bitmap[bitmap_num].height, 0,
              GL_LUMINANCE, GL_UNSIGNED_BYTE,meshman_bitmap[bitmap_num].image);
}




#endif //#ifdef OPENGL_PROJECT












//write_ppm(output_filename,comment,rgbimage,width,height);
void meshman_write_ppm(char *file_name, char *comment, unsigned char *image,int width,int height)
{
FILE *out;
int i,j;

out=(FILE*)fopen(file_name,"wb");
if(out==NULL)
   {printf("PGM_FUNCTIONS.C error: Couldn't open %s for writing\n",file_name);exit(1);}
fprintf(out,"P6\n#%s\n",comment);
fprintf(out,"%d %d\n255\n",width,height);
for(i=0;i<width*height*3;i++)
    {
    j=(int)(*(image+i));
    fputc(j,out);
    }
fclose(out);
}







//------------------------------------------------------------------------------------------------------------------------------------------
#endif//#ifndef _MESH_MANAGEMENT_C_
