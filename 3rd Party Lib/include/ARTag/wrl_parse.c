//© 2006, National Research Council Canada

//WRL_PARSE.C - reads VRML (.WRL) files from VRML 1.0 & 2.0 - poly geometry info only - and puts into mesh_management.c structures
//author:Mark Fiala - Sept 2005 - National Research Council of Canada - IIT/CVG group - 
//
//notes:
//-needs "mesh_management.c"
//-Incomplete coverage of WRL format, only a few keywords are parsed for polygon meshes


//-----------------------------------------------------------------------------------------------------------------------------------------------
#ifndef _WRL_PARSE_C_
 #define _WRL_PARSE_C_


#ifndef _MESH_MANAGEMENT_C_
 #include "mesh_management.c"
#endif

#define WRL_PARSE_DEBUG_ON 0
#define WRL_PARSE_DISPLAY_STATUS_ON 0

//file parsing functions, variables, and constants
char wp_get_char(void);
int wp_get_type(char cc);
int wp_get_token(void);
int wp_get_token_both_cases(void);
int wp_expect_token(char *expected);
int wp_wait_for_token(char *expected);
int wp_get_string_token(char *string_token);

#define WRLPARSE_CHAR_IS_NUMBER       0
#define WRLPARSE_CHAR_IS_LETTER       1
#define WRLPARSE_CHAR_IS_WHITESPACE   2
#define WRLPARSE_CHAR_IS_UNKNOWN      3
#define WRLPARSE_CHAR_IS_COMMA	     4
#define WRLPARSE_CHAR_IS_PERIOD	     5
#define WRLPARSE_CHAR_IS_QUOTES	     6
#define WRLPARSE_CHAR_IS_SQUIGGLY_BRACKET   7

#define WRLPARSE_TOKEN_IS_UNKNOWN  0
#define WRLPARSE_TOKEN_IS_NUMBER   1
#define WRLPARSE_TOKEN_IS_STRING   2
#define WRLPARSE_TOKEN_IS_NAME     3

#define WRLPARSE_MODE_3DGEO  0
#define WRLPARSE_MODE_TEXTURE  1
#define WRLPARSE_MODE_NORMALS  2

char wp_char,wp_token[256],wp_token_type;
int wp_parse_line_number;
FILE *WRL_PARSE_IN;

#define WRLPARSE_MAX_NUM_FACES 300000   //faces have to be stored temporarily until we figure out the number of vertices for the mesh






//object_type_num=wrl_parse(wrl_filename,1000,&num_meshes,&num_vtxs,&num_triangles,&num_normals,&bitmaps_loaded);//expect <1000 materials in this file
int wrl_parse(char *wrl_filename, int max_num_materials, int *num_meshes,
              int *num_vtxs, int *num_triangles,  int *num_normals, int *num_bitmaps)
{
char message[1024];
char end;
char base_loaded=0,pointer_loaded=0;
int i,*material_lookup;
int num_geomobjects_loaded=0,num_meshes_loaded=0,num_wireframe_meshes_loaded=0;
int num_3dvertices_loaded=0,num_texvertices_loaded=0,num_normalvectors_loaded=0;
int num_faces_loaded=0,num_texfaces_loaded=0;
int num_bitmaps_present=0,num_meshes_without_materials=0;
int object_num;
char mesh_started;
int *node_storage;
float *nx_storage,*ny_storage,*nz_storage;
int normal_storage_ptr;
char array_mode=WRLPARSE_MODE_3DGEO;

node_storage=(int*)malloc(WRLPARSE_MAX_NUM_FACES*sizeof(int));
if(node_storage==NULL)   {printf("ERROR: Can't malloc %d temporary bytes in wrl_parse(), cannot process WRL file <%s>\n",
                                 WRLPARSE_MAX_NUM_FACES,wrl_filename);return-1;}

nx_storage=(float*)malloc(WRLPARSE_MAX_NUM_FACES*sizeof(float));
ny_storage=(float*)malloc(WRLPARSE_MAX_NUM_FACES*sizeof(float));
nz_storage=(float*)malloc(WRLPARSE_MAX_NUM_FACES*sizeof(float));
if((nx_storage==NULL)||(ny_storage==NULL)||(nz_storage==NULL))
   {printf("ERROR: Can't malloc 3x%d temporary floats in wrl_parse(), cannot process WRL file <%s>\n",
                                 WRLPARSE_MAX_NUM_FACES,wrl_filename);return-1;}


if(WRL_PARSE_DEBUG_ON) printf("WRL_PARSE_DEBUG_ON\n");


*num_meshes=0; *num_vtxs=0; *num_triangles=0;
material_lookup=(int*)malloc(max_num_materials*sizeof(int));
if(material_lookup==NULL) {printf("ERROR mallocing *material_lookup\n");return -1;}
for(i=0;i<max_num_materials;i++) material_lookup[i]=-1;


//----------
wp_parse_line_number=1;
WRL_PARSE_IN=fopen(wrl_filename,"rb");
if(WRL_PARSE_IN==NULL)
   {printf("ERROR: Could not load WRL file <%s>\n",wrl_filename);return -1;}

//start meshman_object
object_num=meshman_free_object_model_ptr;
meshman_free_object_model_ptr++;
strcpy(meshman_object_model[object_num].filename,wrl_filename);
meshman_object_model[object_num].start_mesh=meshman_free_mesh_ptr;
meshman_object_model[object_num].end_mesh=meshman_free_mesh_ptr;

wp_char=wp_get_char();   //wp_get_token needs a char to already be in 'wp_char'
end=0;
//in main mode, expect tokens:
//     'separator' (VRML 1.0), or 'shape' (VRML 2.0) to start mesh
//     'coordinate3' (VRML 1.0), or 'coord' (VRML 2.0) to start 3Dvertex and face lists
//     'indexedfaceset' (VRML 1.0), or 'texcoord' (VRML 2.0) to start 3Dvertex and face lists
//after shape has started, expect tokens:
//     'point' (VRML 1.0 and 2.0) to start either 3d vertex list or texvertex list (depending on if
//                        'indexedfaceset' (VRML 1.0), or 'texcoord' (VRML 2.0) has been called yet
//     'indexedfaceset' (VRML 1.0) or 'coordindex' (VRML 2.0) to start face list
//     'indexedlineset' (VRML 2.0) to start line list
//     'vector' (VRML 2.0) to start normal list (assume no other type of vector)
//     'imagetexture' (VRML 2.0) followed by:  { url "bitmap.jpg"
//     'material' (VRML 2.0) followed by:  { diffuseColor 1 1 1
mesh_started=0;
while(end==0)
   {
   if(wp_get_token()) end=1;
//   if(WRL_PARSE_DEBUG_ON) printf("main token=%s\n",wp_token);
   if((strcmp(wp_token,"separator")==0)||(strcmp(wp_token,"shape")==0))
      {
      if(mesh_started)
         {
         //--- start next mesh ---
         //check last mesh if faces and texvertices were declared, but no textvertes order, in which case they are copied for face_t
         if((meshman_mesh[meshman_free_mesh_ptr].num_faces>0)
          &&(meshman_mesh[meshman_free_mesh_ptr].start_face_t==meshman_free_facetexture_ptr))
            {
            int face,face_ptr=meshman_mesh[meshman_free_mesh_ptr].start_face;
            int num_faces=meshman_mesh[meshman_free_mesh_ptr].num_faces*meshman_mesh[meshman_free_mesh_ptr].num_sides;
            for(face=0;face<num_faces;face++)
               {
               meshman_facetexture[meshman_free_facetexture_ptr].texvertex_num=meshman_face[face_ptr].vertex3d_num;
               meshman_free_facetexture_ptr++;
               face_ptr++;
               }
            }
         //check last mesh if faces and normals were declared, but no normals order, in which case they are copied for face_t
         if((meshman_mesh[meshman_free_mesh_ptr].num_faces>0)&&(normal_storage_ptr>0))
            {
            int face,face_ptr=meshman_mesh[meshman_free_mesh_ptr].start_face;
            int num_faces=meshman_mesh[meshman_free_mesh_ptr].num_faces*meshman_mesh[meshman_free_mesh_ptr].num_sides;
            for(face=0;face<num_faces;face++)
               {
               int normal_num=meshman_face[face_ptr].vertex3d_num;
                //import normal vectors
               meshman_normal[meshman_free_normal_ptr].nx=nx_storage[normal_num];
               meshman_normal[meshman_free_normal_ptr].ny=ny_storage[normal_num];
               meshman_normal[meshman_free_normal_ptr].nz=nz_storage[normal_num];
               meshman_free_normal_ptr++;
               meshman_mesh[meshman_free_mesh_ptr].num_normals++;
               num_normalvectors_loaded++;
               face_ptr++;
               }
            }
         //start new mesh only if mesh is not empty
         if(meshman_mesh[meshman_free_mesh_ptr].num_faces>0)
            {
            meshman_free_mesh_ptr++;
            if(WRL_PARSE_DEBUG_ON) printf("mesh %d started\n",meshman_free_mesh_ptr);
            meshman_object_model[object_num].end_mesh=meshman_free_mesh_ptr;
            meshman_mesh[meshman_free_mesh_ptr].start_face=meshman_free_face_ptr;
            meshman_mesh[meshman_free_mesh_ptr].start_face_t=meshman_free_facetexture_ptr;
            meshman_mesh[meshman_free_mesh_ptr].num_faces=0;
            meshman_mesh[meshman_free_mesh_ptr].base_vertex3d_ptr=meshman_free_3dpoint_ptr;
            meshman_mesh[meshman_free_mesh_ptr].num_3d_vertices=0;
            meshman_mesh[meshman_free_mesh_ptr].base_vertextex_ptr=meshman_free_texpoint_ptr;
            meshman_mesh[meshman_free_mesh_ptr].num_tex_vertices=0;
            meshman_mesh[meshman_free_mesh_ptr].base_facecolour_ptr=meshman_free_facecolour_ptr;
            meshman_mesh[meshman_free_mesh_ptr].num_face_colours=0;
            meshman_mesh[meshman_free_mesh_ptr].base_normal_ptr=meshman_free_normal_ptr;
            meshman_mesh[meshman_free_mesh_ptr].num_normals=0;
            meshman_mesh[meshman_free_mesh_ptr].normal_per_vertex=1;  //hard-coded, this version of WRL_PARSE.C only uses vertex normals
            meshman_mesh[meshman_free_mesh_ptr].material=meshman_default_material;
            normal_storage_ptr=0;
            num_meshes_loaded++;
            }
         }//if(mesh_started)
      else
         {
         //--- start first mesh ---
         meshman_object_model[object_num].end_mesh=meshman_free_mesh_ptr;
         meshman_mesh[meshman_free_mesh_ptr].start_face=meshman_free_face_ptr;
         meshman_mesh[meshman_free_mesh_ptr].start_face_t=meshman_free_facetexture_ptr;
         meshman_mesh[meshman_free_mesh_ptr].num_faces=0;
         meshman_mesh[meshman_free_mesh_ptr].base_vertex3d_ptr=meshman_free_3dpoint_ptr;
         meshman_mesh[meshman_free_mesh_ptr].num_3d_vertices=0;
         meshman_mesh[meshman_free_mesh_ptr].base_vertextex_ptr=meshman_free_texpoint_ptr;
         meshman_mesh[meshman_free_mesh_ptr].num_tex_vertices=0;
         meshman_mesh[meshman_free_mesh_ptr].base_facecolour_ptr=meshman_free_facecolour_ptr;
         meshman_mesh[meshman_free_mesh_ptr].num_face_colours=0;
         meshman_mesh[meshman_free_mesh_ptr].base_normal_ptr=meshman_free_normal_ptr;
         meshman_mesh[meshman_free_mesh_ptr].num_normals=0;
         meshman_mesh[meshman_free_mesh_ptr].normal_per_vertex=1;  //hard-coded, this version of WRL_PARSE.C only uses vertex normals
         meshman_mesh[meshman_free_mesh_ptr].material=meshman_default_material;
         normal_storage_ptr=0;
         num_meshes_loaded++;
         mesh_started=1;
         if(WRL_PARSE_DEBUG_ON) printf("first mesh %d started\n",meshman_free_mesh_ptr);
         }//start first mesh
      }
   //
   if((strcmp(wp_token,"coordinate3")==0)||(strcmp(wp_token,"coord")==0)
    ||(strcmp(wp_token,"point")==0)
    ||(strcmp(wp_token,"coordindex")==0)||(strcmp(wp_token,"indexedfaceset")==0))
      mesh_started=1;
   //--- following point array will be for 3D vertex list
   if((strcmp(wp_token,"coordinate3")==0)||(strcmp(wp_token,"coord")==0)) array_mode=WRLPARSE_MODE_3DGEO;
   //--- following point will be used for texture vertex list
   else if((strcmp(wp_token,"fillin")==0)||(strcmp(wp_token,"texcoord")==0)) array_mode=WRLPARSE_MODE_TEXTURE;
   else if(strcmp(wp_token,"point")==0)
      {
      if(array_mode==WRLPARSE_MODE_3DGEO)
         {
         //----------- 3D point list ---------
         char point_end;
         float x,y,z;
         int num_3dvertices_this_mesh=0;
         //wait for '[' to start 3D point list
         while((wp_wait_for_token("["))&&(end==0));
         //read in 3D coords
         point_end=0;
         while((end==0)&&(point_end==0))
            {
            if(wp_get_token()) end=1;
            if(strcmp(wp_token,"]")==0) point_end=1;
            else
               {
               sscanf(wp_token,"%f",&x);
               if(wp_get_token()) end=1;
               sscanf(wp_token,"%f",&y);
               if(wp_get_token()) end=1;
               sscanf(wp_token,"%f",&z);
               //printf("3D point %d =  %f,%f,%f\n",meshman_free_3dpoint_ptr,x,y,z);
               //import 3D vector coords -  Y,Z axis are rotated
               meshman_3dpoint[meshman_free_3dpoint_ptr].x=x;
               meshman_3dpoint[meshman_free_3dpoint_ptr].y=y;  //-z;  //-z; y=-z for fortress.wrl, axis flipped like OBJ
               meshman_3dpoint[meshman_free_3dpoint_ptr].z=z;  //y; z=y for fortress.wrl, axis flipped like OBJ
               meshman_mesh[meshman_free_mesh_ptr].num_3d_vertices++;
               meshman_free_3dpoint_ptr++;
               num_3dvertices_loaded++;
               num_3dvertices_this_mesh++;
               }
            }//while ((end==0)&&(point_end==0))
         if(WRL_PARSE_DEBUG_ON) printf("--- %d 3D vertices loaded\n",num_3dvertices_this_mesh);
         }//if(array_mode==WRLPARSE_MODE_3DGEO)
      else if(array_mode==WRLPARSE_MODE_TEXTURE)
         {
         //----------- texture point list ---------
         char point_end;
         float tx,ty;
         int num_texvertices_this_mesh=0;
         //wait for '[' to start 3D point list
         while((wp_wait_for_token("["))&&(end==0));
         //read in 3D coords
         point_end=0;
         while((end==0)&&(point_end==0))
            {
            if(wp_get_token()) end=1;
            if(strcmp(wp_token,"]")==0) point_end=1;
            else
               {
               sscanf(wp_token,"%f",&tx);
               if(wp_get_token()) end=1;
               if(strcmp(wp_token,"]")==0) point_end=1;
               else
                  {
                  sscanf(wp_token,"%f",&ty);
                  //import 2D texture vector coords
                  meshman_texpoint[meshman_free_texpoint_ptr].u=tx;
                  meshman_texpoint[meshman_free_texpoint_ptr].v=1.0-ty;  //vrml starts lower left of image for tx,ty=0,0
                  meshman_mesh[meshman_free_mesh_ptr].num_tex_vertices++;
                  meshman_free_texpoint_ptr++;
                  num_texvertices_loaded++;
                  num_texvertices_this_mesh++;
                  }
               }
            }//while ((end==0)&&(point_end==0))
         if(WRL_PARSE_DEBUG_ON) printf("--- %d texture vertices loaded\n",num_texvertices_this_mesh);
         }//if(array_mode==WRLPARSE_MODE_3DGEO)
      }//else if(strcmp(wp_token,"point")==0)
   else if(strcmp(wp_token,"vector")==0)
      {
      //----------- normal vector list ---------
      char vector_end;
      float nx,ny,nz;
      int num_normalvectors_this_mesh=0;
      normal_storage_ptr=0;
      //wait for '[' to start normal vector list
      while((wp_wait_for_token("["))&&(end==0));
      //read in 3D coords
      vector_end=0;
      while((end==0)&&(vector_end==0))
         {
         if(wp_get_token()) end=1;
         if(strcmp(wp_token,"]")==0) vector_end=1;
         else
            {
            sscanf(wp_token,"%f",&nx);
            if(wp_get_token()) end=1;
            if(strcmp(wp_token,"]")==0) vector_end=1;
            else
               {
               sscanf(wp_token,"%f",&ny);
               if(wp_get_token()) end=1;
               if(strcmp(wp_token,"]")==0) vector_end=1;
               else
                  {
                  sscanf(wp_token,"%f",&nz);
                  //store normal vector in temp arrays
                  nx_storage[normal_storage_ptr]=nx;
                  ny_storage[normal_storage_ptr]=ny;
                  nz_storage[normal_storage_ptr]=nz;
                  normal_storage_ptr++;
                  num_normalvectors_this_mesh++;
                  }
               }
            }
         }//while ((end==0)&&(vector_end==0))
      if(WRL_PARSE_DEBUG_ON) printf("--- %d normal vectors loaded\n",num_normalvectors_this_mesh);
      }//else if(strcmp(wp_token,"vector")==0)
   //----------- face list ---------
   else if((strcmp(wp_token,"coordindex")==0)||(strcmp(wp_token,"vrml1indexedfaceset")==0))
      {
      //----------- vertex list ---------
      char face_end,use_minus_flag;
      int num_vertices_per_face=0,max_num_vertices_per_face=0;
      int node_storage_ptr=0,num_faces_this_mesh=0;
      int num_2sided=0,num_3sided=0,num_4sided=0,num_minus1=0;
      int temp_face[100],temp_face_ptr=0,last_vertex_num;
      //wait for '[' to start vertex list
      while((wp_wait_for_token("["))&&(end==0));
      //read in vertex numbers
      face_end=0;
      while((end==0)&&(face_end==0))
         {
         if(wp_get_token()) end=1;
         //   printf("token inside face <%s>\n",wp_token);
         if(strcmp(wp_token,"]")==0) face_end=1;
         else
            {
            int vertex_num;
            sscanf(wp_token,"%d",&vertex_num);
            //store face vertex pointer in temporary array
            if(node_storage_ptr<WRLPARSE_MAX_NUM_FACES)
               {
               node_storage[node_storage_ptr]=vertex_num;
               node_storage_ptr++;
               }
            if(vertex_num==-1)
               {
               num_minus1++;
               //printf("num_vertices_per_face=%d\n",num_vertices_per_face);
               if(num_vertices_per_face==2) num_2sided++;
               else if(num_vertices_per_face==3) num_3sided++;
               else if(num_vertices_per_face==4) num_4sided++;
               num_vertices_per_face=0;
               }
            else
               {
               num_vertices_per_face++;
               if(num_vertices_per_face>max_num_vertices_per_face)
                  max_num_vertices_per_face=num_vertices_per_face;
               }
            }
         }//while ((end==0)&&(face_end==0))
      if(WRL_PARSE_DEBUG_ON)
         printf("end of face list node_storage_ptr=%d max_num_vertices_per_face=%d\n",node_storage_ptr,max_num_vertices_per_face);
      //decide if vertex list did not have -1's as end markers of face vertex pointers
      use_minus_flag=1;
      if(max_num_vertices_per_face>8)
         {
         if(num_minus1==0) use_minus_flag=0;
         if(WRL_PARSE_DEBUG_ON)
            printf("max_num_vertices_per_face is too big. num_2sided=%d num_3sided=%d num_4sided=%d\n",num_2sided,num_3sided,num_4sided);
         if((num_2sided>num_3sided)&&(num_2sided>num_4sided)) max_num_vertices_per_face=2;
         if((num_3sided>num_2sided)&&(num_3sided>num_4sided)) max_num_vertices_per_face=3;
         else max_num_vertices_per_face=4;  //assume quads
         if(WRL_PARSE_DEBUG_ON)
            printf("modified num_sides: node_storage_ptr=%d max_num_vertices_per_face=%d\n",node_storage_ptr,max_num_vertices_per_face);
         }
      meshman_mesh[meshman_free_mesh_ptr].num_sides=max_num_vertices_per_face;
      //enter face vertices into mesh_manager
      i=0; temp_face_ptr=0; last_vertex_num=0;
      while(i<node_storage_ptr)
         {
         int j,vertex_num=node_storage[i];
         if((vertex_num==-1)||((use_minus_flag==0)&&(temp_face_ptr>=max_num_vertices_per_face)))
            {
            if(temp_face_ptr>4) temp_face_ptr=4;  //only keep the first 4 of a face with >4 vertices
            for(j=0;j<temp_face_ptr;j++)
               {
               meshman_face[meshman_free_face_ptr].vertex3d_num=(unsigned short int)temp_face[j];
               meshman_free_face_ptr++;
               last_vertex_num=temp_face[j];
               }
            while(j<meshman_mesh[meshman_free_mesh_ptr].num_sides)
               {
               //repeat last good vertex to fill up - eg for quad mesh, repeat last vertex of triangle
               meshman_face[meshman_free_face_ptr].vertex3d_num=(unsigned short int)last_vertex_num;
               meshman_free_face_ptr++;
               j++;
               }
            //finish face
            meshman_mesh[meshman_free_mesh_ptr].num_faces++;
            num_faces_loaded++;
            num_faces_this_mesh++;
            temp_face_ptr=0;
            }
         else temp_face[temp_face_ptr++]=vertex_num;
         i++;
         }//while(i<node_storage_ptr)
      if(WRL_PARSE_DEBUG_ON) printf("--- %d faces loaded\n",num_faces_this_mesh);
      }//else if((strcmp(wp_token,"coordindex")
   //----------- normal face list ---------
   else if(strcmp(wp_token,"normalindex")==0)
      {
      //----------- normal list ---------
      char normal_index_end,use_minus_flag;
      int num_normals_per_face=0,max_num_normals_per_face=0;
      int node_storage_ptr=0,num_normals_this_mesh=0;
      int num_2sided=0,num_3sided=0,num_4sided=0,num_minus1=0;
      int temp_face[100],temp_face_ptr=0,last_normal_num;
      //wait for '[' to start normal list
      while((wp_wait_for_token("["))&&(end==0));
      //read in vertex numbers
      normal_index_end=0;
      while((end==0)&&(normal_index_end==0))
         {
         if(wp_get_token()) end=1;
         //   printf("token inside face <%s>\n",wp_token);
         if(strcmp(wp_token,"]")==0) normal_index_end=1;
         else
            {
            int normal_num;
            sscanf(wp_token,"%d",&normal_num);
            //store normal pointer in temporary array
            if(node_storage_ptr<WRLPARSE_MAX_NUM_FACES)
               {
               node_storage[node_storage_ptr]=normal_num;
               node_storage_ptr++;
               }
            if(normal_num==-1)
               {
               num_minus1++;
               //printf("num_vertices_per_face=%d\n",num_vertices_per_face);
               if(num_normals_per_face==2) num_2sided++;
               else if(num_normals_per_face==3) num_3sided++;
               else if(num_normals_per_face==4) num_4sided++;
               num_normals_per_face=0;
               }
            else
               {
               num_normals_per_face++;
               if(num_normals_per_face>max_num_normals_per_face)
                  max_num_normals_per_face=num_normals_per_face;
               }
            }
         }//while ((end==0)&&(normal_index_end==0))
      if(WRL_PARSE_DEBUG_ON)
         printf("end of face list node_storage_ptr=%d max_num_normals_per_face=%d\n",node_storage_ptr,max_num_normals_per_face);
      //decide if vertex list did not have -1's as end markers of face vertex pointers
      use_minus_flag=1;
      if(max_num_normals_per_face>8)
         {
         if(num_minus1==0) use_minus_flag=0;
         if(WRL_PARSE_DEBUG_ON)
            printf("max_num_vertices_per_face is too big. num_2sided=%d num_3sided=%d num_4sided=%d\n",num_2sided,num_3sided,num_4sided);
         if((num_2sided>num_3sided)&&(num_2sided>num_4sided)) max_num_normals_per_face=2;
         if((num_3sided>num_2sided)&&(num_3sided>num_4sided)) max_num_normals_per_face=3;
         else max_num_normals_per_face=4;  //assume quads
         if(WRL_PARSE_DEBUG_ON)
            printf("modified num_sides: node_storage_ptr=%d max_num_vertices_per_face=%d\n",node_storage_ptr,max_num_normals_per_face);
         }
      //enter face normals into mesh_manager
      i=0; temp_face_ptr=0; last_normal_num=0;
      while(i<node_storage_ptr)
         {
         int j,normal_num=node_storage[i];
         if((normal_num==-1)||((use_minus_flag==0)&&(temp_face_ptr>=max_num_normals_per_face)))
            {
            if(temp_face_ptr>4) temp_face_ptr=4;  //only keep the first 4 of a face with >4 vertices
            for(j=0;j<temp_face_ptr;j++)
               {
               meshman_normal[meshman_free_normal_ptr].nx=nx_storage[temp_face[j]];
               meshman_normal[meshman_free_normal_ptr].ny=ny_storage[temp_face[j]];
               meshman_normal[meshman_free_normal_ptr].nz=nz_storage[temp_face[j]];
               meshman_free_normal_ptr++;
               meshman_mesh[meshman_free_mesh_ptr].num_normals++;
               num_normalvectors_loaded++;
               num_normals_this_mesh++;
               last_normal_num=temp_face[j];
               }
            while(j<meshman_mesh[meshman_free_mesh_ptr].num_sides)
               {
               //repeat last good vertex to fill up - eg for quad mesh, repeat last vertex of triangle
               meshman_normal[meshman_free_normal_ptr].nx=nx_storage[last_normal_num];
               meshman_normal[meshman_free_normal_ptr].ny=ny_storage[last_normal_num];
               meshman_normal[meshman_free_normal_ptr].nz=nz_storage[last_normal_num];
               meshman_free_normal_ptr++;
               meshman_mesh[meshman_free_mesh_ptr].num_normals++;
               num_normalvectors_loaded++;
               j++;
               }
            //finish face
            temp_face_ptr=0;
            }
         else temp_face[temp_face_ptr++]=normal_num;
         i++;
         }//for(i=0;i<node_storage_ptr;i++)
      if(WRL_PARSE_DEBUG_ON) printf("--- %d normals loaded\n",num_normals_this_mesh);
      }//else if((strcmp(wp_token,"normalindex")
   //----------- line list ---------
   else if(strcmp(wp_token,"indexedlineset")==0)
      {
      //----------- 3D point list ---------
      char line_end,first_vertex_loaded;
      int num_lines_this_mesh=0,first_vertex,last_vertex;
      //wait for '[' to start vertex list
      while((wp_wait_for_token("["))&&(end==0));
      //read in vertex numbers
      line_end=0; first_vertex_loaded=0;
      while((end==0)&&(line_end==0))
         {
         if(wp_get_token()) end=1;
         //   printf("token inside line <%s>\n",wp_token);
         if(strcmp(wp_token,"]")==0) line_end=1;
         else
            {
            int vertex_num;
            sscanf(wp_token,"%d",&vertex_num);
            if(vertex_num==-1) first_vertex_loaded=0;
            else if(first_vertex_loaded==0)
               {first_vertex=vertex_num; first_vertex_loaded=1;}  //don't draw line for first vertex listed
            else
               {//draw a "line" between this one and last
               meshman_face[meshman_free_face_ptr].vertex3d_num=(unsigned short int)last_vertex;
               meshman_free_face_ptr++;
               meshman_face[meshman_free_face_ptr].vertex3d_num=(unsigned short int)vertex_num;
               meshman_free_face_ptr++;
               meshman_mesh[meshman_free_mesh_ptr].num_faces++;
               num_faces_loaded+=2;
               num_lines_this_mesh++;
               }
            last_vertex=vertex_num;
            }
         }//while ((end==0)&&(face_end==0))
      meshman_mesh[meshman_free_mesh_ptr].num_sides=2;
      if(WRL_PARSE_DEBUG_ON) printf("--- %d lines loaded\n",num_lines_this_mesh);
      }//else if(strcmp(wp_token,"indexedlineset")==0)
   //----------- material ---------
   else if(strcmp(wp_token,"material")==0)
      {
      char material_end=0;
      int mesh_material=meshman_mesh[meshman_free_mesh_ptr].material;
      //start new material if needed
      if(mesh_material==meshman_default_material)
         {
         mesh_material=meshman_material_ptr;
         meshman_mesh[meshman_free_mesh_ptr].material=meshman_material_ptr;
         //create new material
         meshman_material[meshman_material_ptr].material_number_in_file=meshman_material_ptr;
         meshman_material[meshman_material_ptr].bitmap_on=0;
         meshman_material[meshman_material_ptr].diffuse_on=0;
         meshman_material[meshman_material_ptr].specular_on=0;
         meshman_material[meshman_material_ptr].ambient_on=0;
         meshman_material_ptr++;
         }
      //in material mode, expect tokens 'diffuseColor','specularColor','shininess','ambientIntensity' (VRML 2.0)
      while ((end==0)&&(material_end==0))
         {
         if(wp_get_token()) end=1;
         if(WRL_PARSE_DEBUG_ON) printf("material token=<%s>\n",wp_token);
         if(strcmp(wp_token,"diffusecolor")==0)
            {
            float red,green,blue;
            char valid=1;
            //red
            if(wp_get_token()) end=1;
            if(wp_token_type!=WRLPARSE_TOKEN_IS_NUMBER) valid=0;
            sscanf(wp_token,"%f",&red);
            //green
            if(wp_get_token()) end=1;
            if(wp_token_type!=WRLPARSE_TOKEN_IS_NUMBER) valid=0;
            sscanf(wp_token,"%f",&green);
            //blue
            if(wp_get_token()) end=1;
            if(wp_token_type!=WRLPARSE_TOKEN_IS_NUMBER) valid=0;
            sscanf(wp_token,"%f",&blue);
            //
            if(valid)
               {
               if(WRL_PARSE_DEBUG_ON) printf("material %d had diffuse r,g,b=%f,%f,%f\n",mesh_material,red,green,blue);
               meshman_material[mesh_material].diffuse_on=1;
               meshman_material[mesh_material].diffuse_red=red;
               meshman_material[mesh_material].diffuse_green=green;
               meshman_material[mesh_material].diffuse_blue=blue;
               }
            }
         else if(strcmp(wp_token,"specularcolor")==0)
            {
            float red,green,blue;
            char valid=1;
            //red
            if(wp_get_token()) end=1;
            if(wp_token_type!=WRLPARSE_TOKEN_IS_NUMBER) valid=0;
            sscanf(wp_token,"%f",&red);
            //green
            if(wp_get_token()) end=1;
            if(wp_token_type!=WRLPARSE_TOKEN_IS_NUMBER) valid=0;
            sscanf(wp_token,"%f",&green);
            //blue
            if(wp_get_token()) end=1;
            if(wp_token_type!=WRLPARSE_TOKEN_IS_NUMBER) valid=0;
            sscanf(wp_token,"%f",&blue);
            //
            if(valid)
               {
               if(WRL_PARSE_DEBUG_ON) printf("material %d had specular r,g,b=%f,%f,%f\n",mesh_material,red,green,blue);
               meshman_material[mesh_material].specular_on=1;
               meshman_material[mesh_material].specular_red=red;
               meshman_material[mesh_material].specular_green=green;
               meshman_material[mesh_material].specular_blue=blue;
               }
            }
         else if(strcmp(wp_token,"ambientintensity")==0)
            {
            if(wp_get_token()) end=1;
            if(wp_token_type==WRLPARSE_TOKEN_IS_NUMBER)
               {
               float ambient;
               sscanf(wp_token,"%f",&ambient);
               if(WRL_PARSE_DEBUG_ON) printf("material %d has ambient =%f\n",mesh_material,ambient);
               meshman_material[mesh_material].ambient_on=1;
               meshman_material[mesh_material].ambient_red=ambient;
               meshman_material[mesh_material].ambient_green=ambient;
               meshman_material[mesh_material].ambient_blue=ambient;
               }
            }
         else if(strcmp(wp_token,"ambientcolor")==0)
            {
            float red,green,blue;
            char valid=1;
            //red
            if(wp_get_token()) end=1;
            if(wp_token_type!=WRLPARSE_TOKEN_IS_NUMBER) valid=0;
            sscanf(wp_token,"%f",&red);
            //green
            if(wp_get_token()) end=1;
            if(wp_token_type!=WRLPARSE_TOKEN_IS_NUMBER) valid=0;
            sscanf(wp_token,"%f",&green);
            //blue
            if(wp_get_token()) end=1;
            if(wp_token_type!=WRLPARSE_TOKEN_IS_NUMBER) valid=0;
            sscanf(wp_token,"%f",&blue);
            //
            if(valid)
               {
               if(WRL_PARSE_DEBUG_ON) printf("material %d had ambient r,g,b=%f,%f,%f\n",mesh_material,red,green,blue);
               meshman_material[mesh_material].ambient_on=1;
               meshman_material[mesh_material].ambient_red=red;
               meshman_material[mesh_material].ambient_green=green;
               meshman_material[mesh_material].ambient_blue=blue;
               }
            }
         else if(strcmp(wp_token,"}")==0)  material_end=1;
         }//while ((end==0)&&(material_end==0))
      }//else if(strcmp(wp_token,"material")==0)
   //----------- bitmap ---------
   else if(strcmp(wp_token,"imagetexture")==0)
      {
      char temp_bitmap_filename[512],bitmap_filename[512];
      char bitmap_found=0;
      int string_success;
      //wait for '[' to start 3D point list
      while((wp_wait_for_token("url"))&&(end==0));
      string_success=wp_get_string_token(temp_bitmap_filename);
      if(string_success==0) bitmap_found=1;
      else if(string_success==1) end=1;
      else if(string_success==2)
         {
         string_success=wp_get_string_token(temp_bitmap_filename);
         if(string_success==0) bitmap_found=1;
         else if(string_success==1) end=1;
         }
      if(WRL_PARSE_DEBUG_ON)  printf("bitmap_found=%d\n",temp_bitmap_filename);
      if(bitmap_found)
         {
         int bitmap_id,material_found=-1;
         // ------- found bitmap name -------
         if(WRL_PARSE_DEBUG_ON)  printf("Found temp_bitmap <%s>\n",temp_bitmap_filename);
         //remove path
         i=strlen(temp_bitmap_filename);
         if(i>0)
            {
            i--;
            while((temp_bitmap_filename[i]!='\\')&&(temp_bitmap_filename[i]!='/')&&(i>0)) i--;
            if(i>0) i++;
            strcpy(bitmap_filename,&temp_bitmap_filename[i]);
            }
         if(WRL_PARSE_DEBUG_ON)
            printf("removed path bitmap <%s>\n",bitmap_filename);
         //add bitmap
         bitmap_id=mesh_management_add_bitmap(bitmap_filename);  //returns -1 if problem, meshman_bitmap pointer otherwise
         printf("added bitmap <%s> to bitmap_id %d\n",bitmap_filename,bitmap_id);
         if(bitmap_id==-1)
            printf("WARNING: can't add bitmap <%s>, meshman max materials reached",bitmap_filename);
         else
            {
            int matnum;
            int mesh_material=meshman_mesh[meshman_free_mesh_ptr].material;
            //has material already been started?  If so just add this bitmap to it, if not create a new material
            if(mesh_material==meshman_default_material)
               {
               //see if material already exists using this bitmap
               for(matnum=0;matnum<meshman_material_ptr;matnum++)
                  if(meshman_material[matnum].bitmap_on)
                     if(meshman_material[matnum].bitmap_num==bitmap_id) mesh_material=matnum;
               if(mesh_material==meshman_default_material)
                  {
                  //create material just for bitmap
                  meshman_material[meshman_material_ptr].material_number_in_file=meshman_material_ptr;
                  meshman_material[meshman_material_ptr].diffuse_on=0;
                  meshman_material[meshman_material_ptr].specular_on=0;
                  meshman_material[meshman_material_ptr].ambient_on=0;
                  mesh_material=meshman_material_ptr;
                  meshman_material_ptr++;
                  }
               }
            //assign bitmap
            meshman_material[mesh_material].bitmap_num=bitmap_id;
            meshman_material[mesh_material].bitmap_on=1;
            meshman_mesh[meshman_free_mesh_ptr].material=mesh_material;
            num_bitmaps_present++;
            if(WRL_PARSE_DEBUG_ON)
               printf("Assigned bitmap#%d <%s> to material %d to mesh %d\n",bitmap_id,bitmap_filename,mesh_material,meshman_free_mesh_ptr);
            }
         }//if(bitmap_found)
      }
   }
fclose(WRL_PARSE_IN);







if(mesh_started)
   {
   //check last mesh if faces and texvertices were declared, but no textvertes order, in which case they are copied for face_t
   if((meshman_mesh[meshman_free_mesh_ptr].num_faces>0)
    &&(meshman_mesh[meshman_free_mesh_ptr].start_face_t==meshman_free_facetexture_ptr))
      {
      int face,face_ptr=meshman_mesh[meshman_free_mesh_ptr].start_face;
      int num_faces=meshman_mesh[meshman_free_mesh_ptr].num_faces*meshman_mesh[meshman_free_mesh_ptr].num_sides;
      for(face=0;face<num_faces;face++)
         {
         meshman_facetexture[meshman_free_facetexture_ptr].texvertex_num=meshman_face[face_ptr].vertex3d_num;
         meshman_free_facetexture_ptr++;
         face_ptr++;
         }
      }
   //check last mesh if faces and normals were declared, but no normals order, in which case they are copied for face_t
   if((meshman_mesh[meshman_free_mesh_ptr].num_faces>0)&&(normal_storage_ptr>0))
      {
      int face,face_ptr=meshman_mesh[meshman_free_mesh_ptr].start_face;
      int num_faces=meshman_mesh[meshman_free_mesh_ptr].num_faces*meshman_mesh[meshman_free_mesh_ptr].num_sides;
      for(face=0;face<num_faces;face++)
         {
         int normal_num=meshman_face[face_ptr].vertex3d_num;
         meshman_normal[meshman_free_normal_ptr].nx=nx_storage[normal_num];
         meshman_normal[meshman_free_normal_ptr].ny=ny_storage[normal_num];
         meshman_normal[meshman_free_normal_ptr].nz=nz_storage[normal_num];
         meshman_free_normal_ptr++;
         meshman_mesh[meshman_free_mesh_ptr].num_normals++;
         num_normalvectors_loaded++;
         face_ptr++;
         }
      }
   //
   if(meshman_mesh[meshman_free_mesh_ptr].num_faces>0)
      {
      num_meshes_loaded++;
      meshman_free_mesh_ptr++;
      }
   }//if(mesh_started)


//validate meshes loaded
for(i=meshman_object_model[object_num].start_mesh;i<=meshman_object_model[object_num].end_mesh;i++)
   {
   //invalidate meshes with no 3d vertices
   if(meshman_mesh[i].num_3d_vertices==0)  meshman_mesh[i].num_faces=0;
   }


//increment for next object
meshman_free_object_model_ptr++;







if(WRL_PARSE_DISPLAY_STATUS_ON)
   {
   printf("-------- %d Materials in meshman ---------\n",meshman_material_ptr);
   for(i=0;i<meshman_material_ptr;i++)
      {
      printf("%d:  material_number in file=%d\n",i,meshman_material[i].material_number_in_file);
      if(meshman_material[i].bitmap_on)
      printf("  bitmap %d\n",meshman_material[i].bitmap_num);
      if(meshman_material[i].bitmap_on)
      printf("  bitmap %d = <%s>\n",meshman_material[i].bitmap_num,
                                    meshman_bitmap[meshman_material[i].bitmap_num].filename);
      if(meshman_material[i].diffuse_on)
      printf("  diffuse %f  %f  %f\n",meshman_material[i].diffuse_red,meshman_material[i].diffuse_green,
                                      meshman_material[i].diffuse_blue);
      if(meshman_material[i].specular_on)
      printf("  specular %f  %f  %f\n",meshman_material[i].specular_red,meshman_material[i].specular_green,
                                       meshman_material[i].specular_blue);
      if(meshman_material[i].ambient_on)
      printf("  ambient %f  %f  %f\n",meshman_material[i].ambient_red,meshman_material[i].ambient_green,
                                      meshman_material[i].ambient_blue);
       }
   printf("-------- %d bitmaps in meshman ---------\n",meshman_bitmap_ptr);
   for(i=0;i<meshman_bitmap_ptr;i++)
      printf("%d: <%s>\n",i,meshman_bitmap[i].filename);

   printf("%d geomobjects and %d meshes loaded\n",num_geomobjects_loaded,num_meshes_loaded);
   printf("%d of %d meshes are wire-frame\n",num_wireframe_meshes_loaded,num_meshes_loaded);
   if(num_geomobjects_loaded!=num_meshes_loaded)
      printf(message,"WARNING <%s> has a differing number of geomobjects and meshes\n",wrl_filename);
   printf("%d 3D and %d texture vertices loaded. %d normal vectors loaded. %d faces and %d texture faces loaded\n",
          num_3dvertices_loaded,num_texvertices_loaded,num_normalvectors_loaded,num_faces_loaded,num_texfaces_loaded);
   }


if(num_meshes_without_materials>0)
   printf("%WARNING: %d geomobjects (meshes) had no material: default white material applied\n",
           num_meshes_without_materials);








*num_meshes=num_meshes_loaded;
*num_vtxs=num_3dvertices_loaded;
*num_triangles=num_faces_loaded;
*num_normals=num_normalvectors_loaded;
*num_bitmaps=num_bitmaps_present;
//clean up memory used
if(material_lookup!=NULL) free(material_lookup);
if(node_storage!=NULL) free(node_storage);
if(nx_storage!=NULL) free(nx_storage);
if(ny_storage!=NULL) free(ny_storage);
if(nz_storage!=NULL) free(nz_storage);

return object_num;
}















//The following functions return 0 for ok, 1 for problem
// get_token()           returns 1 if EOF
// expect_token(char*)   retrns 1 if next token is not the expected one
// get_string_token()        returns 1 if not a proper filename with "file" or "file.pgm" format

char wp_get_char(void)
{
char it;

it=fgetc(WRL_PARSE_IN);
if(it==0x0a) wp_parse_line_number++;
return it;
}



int wp_get_type(char cc)
{
if((cc>='0')&&(cc<='9')||(cc=='-')) return WRLPARSE_CHAR_IS_NUMBER;

if(((cc>='a')&&(cc<='z'))
 ||((cc>='A')&&(cc<='Z'))
 ||(cc=='_')||(cc=='/')||(cc=='\\')||(cc==':')||(cc=='#')||(cc=='$')||(cc=='^')||(cc=='*')
 ||(cc=='+')
 ||(cc=='%')||(cc=='&')||(cc=='~')
 ||(cc=='<')||(cc=='>')||(cc=='!')
 ||(cc==0x27)) return WRLPARSE_CHAR_IS_LETTER;

if((cc==' ')||(cc==0x0a)||(cc==0x0d)||(cc==0x09)) return WRLPARSE_CHAR_IS_WHITESPACE;

if(cc==',') return WRLPARSE_CHAR_IS_COMMA;

if(cc=='.') return WRLPARSE_CHAR_IS_PERIOD;

if(cc=='"') return WRLPARSE_CHAR_IS_QUOTES;

if((cc=='{')||(cc=='}')) return WRLPARSE_CHAR_IS_SQUIGGLY_BRACKET;

return WRLPARSE_CHAR_IS_UNKNOWN;
}



//get lower case version of token and disregard commas
int wp_get_token(void)
{
int ret=wp_get_token_both_cases();
int i;
char c,cdiff='A'-'a';

if(strcmp(wp_token,",")==0)
   ret=wp_get_token_both_cases();

if(ret==0)
   for(i=0;i<strlen(wp_token);i++)
      {
      c=wp_token[i];
      if((c>='A')&&(c<='Z')) wp_token[i]-=cdiff;
      }
return ret;
}




// wp_get_token()           returns 1 if EOF
int wp_get_token_both_cases(void)
{
int done,w,type;
int num_quotes;
char clast;

// Set default token type
wp_token_type=WRLPARSE_TOKEN_IS_UNKNOWN;

//first skip over whitespace
whitespace_jump:
done=0;
while(done==0)
   {
   if(wp_char==EOF) return 1;
   if((wp_char!=' ')&&(wp_char!=0x0a)&&(wp_char!=0x0d)&&(wp_char!=0x09)) done=1;
   else wp_char=wp_get_char();
   }

w=0;
//skip over comments
if(wp_char=='/')
   {
   wp_token[0]='/';  w=1;
   wp_char=wp_get_char();
   if(wp_char=='*')
      {
      done=0; clast=0;
      while(done==0)
         {
         wp_char=wp_get_char(); if(wp_char==EOF) return 1;
         if((wp_char=='/')&&(clast=='*')) done=1;
         clast=wp_char;
         }
      wp_char=wp_get_char(); if(wp_char==EOF) return 1;
      goto whitespace_jump;
      }
   else if(wp_char=='/')
      {
      while(wp_char!=0x0a)
         {
         wp_char=wp_get_char(); if(wp_char==EOF) return 1;
         }
      goto whitespace_jump;
      }
   }
//skip over # comments
if(wp_char=='#')
   {
   while(wp_char!=0x0a)
      {
      wp_char=wp_get_char(); if(wp_char==EOF) return 1;
      }
   goto whitespace_jump;
   }


wp_token[w++]=wp_char;
type=wp_get_type(wp_char);
switch(type)
   {
   case WRLPARSE_CHAR_IS_LETTER:
   while((type==WRLPARSE_CHAR_IS_LETTER)||(type==WRLPARSE_CHAR_IS_NUMBER))
      {
      wp_char=wp_get_char(); if(wp_char==EOF) {wp_token[w]=0; return 1;}
      type=wp_get_type(wp_char);
      if((type==WRLPARSE_CHAR_IS_LETTER)||(type==WRLPARSE_CHAR_IS_NUMBER)) wp_token[w++]=wp_char;
      wp_token_type=WRLPARSE_TOKEN_IS_NAME;
      }
   break;

   case WRLPARSE_CHAR_IS_NUMBER:
   wp_token_type=WRLPARSE_TOKEN_IS_NUMBER;
   while((type==WRLPARSE_CHAR_IS_NUMBER)||(type==WRLPARSE_CHAR_IS_PERIOD))
      {
      wp_char=wp_get_char(); if(wp_char==EOF) {wp_token[w]=0; return 1;}
      type=wp_get_type(wp_char);
      if((type==WRLPARSE_CHAR_IS_NUMBER)||(type==WRLPARSE_CHAR_IS_PERIOD)) wp_token[w++]=wp_char;
      }
   break;

   case WRLPARSE_CHAR_IS_SQUIGGLY_BRACKET:
   wp_char=wp_get_char(); if(wp_char==EOF) {wp_token[w]=0; return 1;}
   break;

   case WRLPARSE_CHAR_IS_COMMA:
   wp_char=wp_get_char(); if(wp_char==EOF) {wp_token[w]=0; return 1;}
   break;

   case WRLPARSE_CHAR_IS_PERIOD:
   wp_char=wp_get_char(); if(wp_char==EOF) {wp_token[w]=0; return 1;}
   break;

   case WRLPARSE_CHAR_IS_QUOTES:		//used for filenames
   wp_token_type=WRLPARSE_TOKEN_IS_STRING;
   num_quotes=1;
   while(num_quotes<2)
      {
      wp_char=wp_get_char(); if(wp_char==EOF) {wp_token[w]=0; return 1;}
      type=wp_get_type(wp_char);
      if(w<256)
         wp_token[w++]=wp_char;
      if(type==WRLPARSE_CHAR_IS_QUOTES) num_quotes++;
      }
   wp_token[w]=0; //printf("string wp_token<%s>\n",wp_token);
   wp_char=wp_get_char(); if(wp_char==EOF) {wp_token[w]=0; return 1;}
   break;

   default:
      if(WRL_PARSE_DISPLAY_STATUS_ON) printf("unknown character %wp_char = 0x%x\n",wp_char,wp_char);
      wp_char=wp_get_char();
   break;
   }
wp_token[w]=0;
return 0;
}



int wp_expect_token(char *expected)
{
if(wp_get_token()) return 1;
if(strcmp(wp_token,expected)!=0) return 1;
else return 0;
}


int wp_wait_for_token(char *expected)
{
char ret=0,end=0;
while((ret==0)&&(end==0))
   {
   if(wp_get_token()) ret=1;
   if(strcmp(wp_token,expected)==0)  end=1;
   }////while ((ret==0)&&(end==0))
return ret;
}



int wp_get_string_token(char *string_token)
{
int i,j;

if(wp_get_token()) return 1;
if(wp_token_type!=WRLPARSE_TOKEN_IS_STRING) return 2;
i=1;j=0;
while((wp_token[i]!='"')&&(wp_token[i]!=0))
   string_token[j++]=wp_token[i++];
string_token[j]=0;
return 0;
}




#endif //#ifndef _WRL_PARSE_C_
//-----------------------------------------------------------------------------------------------------------------------------------------------


