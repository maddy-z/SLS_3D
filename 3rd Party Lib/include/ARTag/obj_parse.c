//© 2006, National Research Council Canada

//OBJ_PARSE.C - reads Wavefront .OBJ files (ascii format from Maya) and puts into mesh_management.c structures
//author:Mark Fiala - Sept 2005 - National Research Council of Canada - IIT/CVG group -
//
//credits:  thanks to http://www.eg-models.de/formats/Format_Obj.html
//          and http://www.nacse.org/~moorchri/obj_format_specs.html for the description of the OBJ format
//


//------------------------------------------------------------------------------------------------------------------------------------------
#ifndef _OBJ_PARSE_C_
 #define _OBJ_PARSE_C_

 
#ifndef _MESH_MANAGEMENT_C_
 #include "mesh_management.c"
#endif

#ifndef _READ_LINE_C_
 #include "read_line.c"
#endif


#define OBJ_PARSE_DEBUG_ON 0

#define OBJ_MAX_NUM_NORMALS 65536  //temporary storage of normal vectors when decoding OBJ format
typedef struct obj_normal_type_s {
                                 float nx,ny,nz;
                                 } obj_normal_type;

#define OBJ_MAX_NUM_MATERIALS 500  //temporary storage of material names
typedef struct obj_material_type_s {
                                   char name[256];
                                   int mesh_num;  //mesh to which this material is mapped
                                   } obj_material_type;

char obj_texture_coords_defined,obj_normal_vectors_defined;   //flags that tell obj_split_face() how to split up 1,2,3 elements

void obj_split_face(char *in, int *vertex_num, int *texvertex_num, int *normal_num);


//object_type_num=obj_parse(ase_filename,1000,&num_meshes,&num_vtxs,&num_triangles,&num_normals,&bitmaps_loaded); //expect <1000 materials in this file
int obj_parse(char *obj_filename, int max_num_materials, int *num_meshes,
              int *num_vtxs, int *num_triangles,  int *num_normals, int *num_bitmaps)
{
int material_num,mesh_num;
int num_geomobjects_loaded=0,num_meshes_loaded=0,num_wireframe_meshes_loaded=0;
int num_3dvertices_loaded=0,num_texvertices_loaded=0,num_normalvectors_loaded=0;
int num_faces_loaded=0,num_texfaces_loaded=0;
int num_bitmaps_present=0;  //none will be loaded for OBJ file
int object_num;
int num_2sided_faces=0,num_3sided_faces=0,num_4sided_faces=0;
obj_normal_type *obj_normal;
int obj_normal_ptr;
char obj_group_finished=0;
obj_material_type *obj_material;
int obj_material_ptr;
char mtl_filename[256]="";

//create storage of normal vectors (vectors must be re-ordered from OBJ format)
obj_normal=(obj_normal_type*)malloc(OBJ_MAX_NUM_NORMALS*sizeof(obj_normal_type));
if(obj_normal==NULL) {printf("ERROR: Can't malloc *obj_normal.  Aborting\n");return -1;}
obj_normal_ptr=0;

//create storage of materials
obj_material=(obj_material_type*)malloc(OBJ_MAX_NUM_MATERIALS*sizeof(obj_material_type));
if(obj_material==NULL) {printf("ERROR: Can't malloc *obj_material.  Aborting\n");return -1;}
obj_material_ptr=0;

//start meshman_object
object_num=meshman_free_object_model_ptr;
meshman_free_object_model_ptr++;  //increment for next object
strcpy(meshman_object_model[object_num].filename,obj_filename);
meshman_object_model[object_num].start_mesh=meshman_free_mesh_ptr;
meshman_object_model[object_num].end_mesh=meshman_free_mesh_ptr;

//start first mesh
mesh_num=meshman_free_mesh_ptr;
meshman_free_mesh_ptr++;  //increment mesh pointer:
meshman_mesh[mesh_num].start_face=meshman_free_face_ptr;
meshman_mesh[mesh_num].start_face_t=meshman_free_facetexture_ptr;
meshman_mesh[mesh_num].num_sides=4;  //to handle triangles and quads - triangles just have last vertex repeated
meshman_mesh[mesh_num].num_faces=0;
meshman_mesh[mesh_num].base_vertex3d_ptr=meshman_free_3dpoint_ptr;
meshman_mesh[mesh_num].num_3d_vertices=0;
meshman_mesh[mesh_num].base_vertextex_ptr=meshman_free_texpoint_ptr;
meshman_mesh[mesh_num].num_tex_vertices=0;
meshman_mesh[mesh_num].base_facecolour_ptr=meshman_free_facecolour_ptr;
meshman_mesh[mesh_num].num_face_colours=0;
meshman_mesh[mesh_num].base_normal_ptr=meshman_free_normal_ptr;
meshman_mesh[mesh_num].num_normals=0;
meshman_mesh[mesh_num].normal_per_vertex=1;  //OBJ files (as understood when writing this) only have normals per vertex
meshman_mesh[mesh_num].material=0;



obj_texture_coords_defined=0; obj_normal_vectors_defined=0;

READ_LINE_IN=fopen(obj_filename,"rb");
if(READ_LINE_IN==NULL) {printf("Can't open mesh file <%s> for reading\n",obj_filename); return -1;}
line_number=0;

while(read_line(line)==0)
   {
   int num_entries=break_line(line,el1,el2,el3,el4,el5,el6,el7,el8,el9);
   //printf("read line <%s>\n",line);
   //make sure we have enough tokens per line for the different line types
   if( ((strcmp(el1,"F")==0)||(strcmp(el1,"f")==0))
       &&(num_entries<2) )
        printf("WARNING: line %d invalid.  Expecting at least 1 vertex (2 tokens in line - found %d).  Line ignored\n"
               "line reads <%s>\n",line_number,num_entries,line);
   else if( ((strcmp(el1,"VT")==0)||(strcmp(el1,"vt")==0))
       &&(num_entries!=3)&&(num_entries!=4) )
        printf("WARNING: line %d invalid.  Expecting 3 or 4 tokens in line (found %d).  Line ignored\n"
               "line reads <%s>\n",line_number,num_entries,line);
   else if( ((strcmp(el1,"V")==0)||(strcmp(el1,"v")==0)
           ||(strcmp(el1,"VN")==0)||(strcmp(el1,"vn")==0))
          &&(num_entries!=4) )
        printf("WARNING: line %d invalid.  Expecting 4 tokens in line (found %d).  Line ignored\n"
               "line reads <%s>\n",line_number,num_entries,line);
   //materials file
   if((strcmp(el1,"MTLLIB")==0)||(strcmp(el1,"mtllib")==0))
      {strcpy(mtl_filename,el2); printf("OBJ file references material file <%s>\n",mtl_filename);}
   //material reference
   if((strcmp(el1,"USEMTL")==0)||(strcmp(el1,"usemtl")==0))
      {
      //remember which mesh had this material name
      strcpy(obj_material[obj_material_ptr].name,el2);
      obj_material[obj_material_ptr].mesh_num=mesh_num;
      obj_material_ptr++;
      }
   //read vertex 3D coords
   if((strcmp(el1,"V")==0)||(strcmp(el1,"v")==0))
      {
      float x,y,z;
      sscanf(el2,"%f",&x); sscanf(el3,"%f",&y); sscanf(el4,"%f",&z);
      if(obj_group_finished)
         {
         //--- start new mesh ---
         //start first mesh
         meshman_object_model[object_num].end_mesh=meshman_free_mesh_ptr;
         mesh_num=meshman_free_mesh_ptr;
         meshman_free_mesh_ptr++;  //increment mesh pointer:
         num_meshes_loaded++;
         //populate next mesh entry
         meshman_mesh[mesh_num].start_face=meshman_free_face_ptr;
         meshman_mesh[mesh_num].start_face_t=meshman_free_facetexture_ptr;
         meshman_mesh[mesh_num].num_sides=4;  //to handle triangles and quads - triangles just have last vertex repeated
         meshman_mesh[mesh_num].num_faces=0;
         meshman_mesh[mesh_num].base_vertex3d_ptr=meshman_free_3dpoint_ptr;
         meshman_mesh[mesh_num].num_3d_vertices=0;
         meshman_mesh[mesh_num].base_vertextex_ptr=meshman_free_texpoint_ptr;
         meshman_mesh[mesh_num].num_tex_vertices=0;
         meshman_mesh[mesh_num].base_facecolour_ptr=meshman_free_facecolour_ptr;
         meshman_mesh[mesh_num].num_face_colours=0;
         meshman_mesh[mesh_num].base_normal_ptr=meshman_free_normal_ptr;
         meshman_mesh[mesh_num].num_normals=0;
         meshman_mesh[mesh_num].normal_per_vertex=1;  //OBJ files (as understood when writing this) only have normals per vertex
         meshman_mesh[mesh_num].material=0;
         obj_texture_coords_defined=0; obj_normal_vectors_defined=0;
         obj_group_finished=0;
         }
      //import 3D vector coords - note that with Wavefront OBJ files, Y,Z axis are rotated
      meshman_3dpoint[meshman_free_3dpoint_ptr].x=x;
      meshman_3dpoint[meshman_free_3dpoint_ptr].y=-z;
      meshman_3dpoint[meshman_free_3dpoint_ptr].z=y;
      meshman_mesh[mesh_num].num_3d_vertices++;
      meshman_free_3dpoint_ptr++;
      num_3dvertices_loaded++;
      }
   //read vertex texture coords
   else if((strcmp(el1,"VT")==0)||(strcmp(el1,"vt")==0))
      {
      float tx,ty;
      sscanf(el2,"%f",&tx); sscanf(el3,"%f",&ty);
      meshman_texpoint[meshman_free_texpoint_ptr].u=tx;
      meshman_texpoint[meshman_free_texpoint_ptr].v=1.0-ty;  //OBJ files, like VRML file, map from the bottom up
      meshman_mesh[mesh_num].num_tex_vertices++;
      meshman_free_texpoint_ptr++;
      num_texvertices_loaded++;
      obj_texture_coords_defined=1;
      }
   //read vertex normal vector
   else if((strcmp(el1,"VN")==0)||(strcmp(el1,"vn")==0))
      {
      float nx,ny,nz;
      sscanf(el2,"%f",&nx); sscanf(el3,"%f",&ny); sscanf(el4,"%f",&nz);
      if(obj_normal_ptr<OBJ_MAX_NUM_NORMALS)
         {
         obj_normal[obj_normal_ptr].nx=nx;
         obj_normal[obj_normal_ptr].ny=ny;
         obj_normal[obj_normal_ptr].nz=nz;
         obj_normal_ptr++;
         }
      obj_normal_vectors_defined=1;
      }
   //read face description
   else if((strcmp(el1,"F")==0)||(strcmp(el1,"f")==0))
      {
      //only support 4 vertices per poly, repeat last vertex for triangles (hack for meshman)
      //each vertex group contains a 3D vertex, and optionally a texture vertex, and optionally a normal vector
      int vertex_num,texvertex_num,normal_num;
      float nx,ny,nz;
      //first vertex group
      obj_split_face(el2,&vertex_num,&texvertex_num,&normal_num);
      if(vertex_num>0) {vertex_num--; texvertex_num--; normal_num--;}  //.OBJ files start indexing at 1 not 0
      else
         {
         //backwards indexing used
         vertex_num+=meshman_mesh[mesh_num].num_3d_vertices;
         texvertex_num+=meshman_mesh[mesh_num].num_3d_vertices;
         normal_num+=meshman_mesh[mesh_num].num_3d_vertices;
         }
      meshman_face[meshman_free_face_ptr].vertex3d_num=(unsigned short int)vertex_num;
      meshman_free_face_ptr++;
      if(obj_texture_coords_defined)
         {
         meshman_facetexture[meshman_free_facetexture_ptr].texvertex_num=(unsigned short int)texvertex_num;
         meshman_free_facetexture_ptr++;
         }
      if((obj_normal_vectors_defined)&&(normal_num>=0)&&(normal_num<OBJ_MAX_NUM_NORMALS))
         {
         nx=obj_normal[normal_num].nx,ny=obj_normal[normal_num].ny,nz=obj_normal[normal_num].nz;
         meshman_normal[meshman_free_normal_ptr].nx=nx;
         meshman_normal[meshman_free_normal_ptr].ny=ny;
         meshman_normal[meshman_free_normal_ptr].nz=nz;
         meshman_free_normal_ptr++;
         meshman_mesh[mesh_num].num_normals++;
         num_normalvectors_loaded++;
         }
      //second vertex group
      if(num_entries>=3)
         {
         obj_split_face(el3,&vertex_num,&texvertex_num,&normal_num);
         if(vertex_num>0) {vertex_num--; texvertex_num--; normal_num--;}  //.OBJ files start indexing at 1 not 0
         else
            {
            //backwards indexing used
            vertex_num+=meshman_mesh[mesh_num].num_3d_vertices;
            texvertex_num+=meshman_mesh[mesh_num].num_3d_vertices;
            normal_num+=meshman_mesh[mesh_num].num_3d_vertices;
            }
         }
      //enter 2nd vertex, or repeat previous vertex
      meshman_face[meshman_free_face_ptr].vertex3d_num=(unsigned short int)vertex_num;
      meshman_free_face_ptr++;
      if(obj_texture_coords_defined)
         {
         meshman_facetexture[meshman_free_facetexture_ptr].texvertex_num=(unsigned short int)texvertex_num;
         meshman_free_facetexture_ptr++;
         }
      if((obj_normal_vectors_defined)&&(normal_num>=0)&&(normal_num<OBJ_MAX_NUM_NORMALS))
         {
         nx=obj_normal[normal_num].nx,ny=obj_normal[normal_num].ny,nz=obj_normal[normal_num].nz;
         meshman_normal[meshman_free_normal_ptr].nx=nx;
         meshman_normal[meshman_free_normal_ptr].ny=ny;
         meshman_normal[meshman_free_normal_ptr].nz=nz;
         meshman_free_normal_ptr++;
         meshman_mesh[mesh_num].num_normals++;
         num_normalvectors_loaded++;
         }
      //third vertex group
      if(num_entries>=4)
         {
         obj_split_face(el4,&vertex_num,&texvertex_num,&normal_num);
         if(vertex_num>0) {vertex_num--; texvertex_num--; normal_num--;}  //.OBJ files start indexing at 1 not 0
         else
            {
            //backwards indexing used
            vertex_num+=meshman_mesh[mesh_num].num_3d_vertices;
            texvertex_num+=meshman_mesh[mesh_num].num_3d_vertices;
            normal_num+=meshman_mesh[mesh_num].num_3d_vertices;
            }
         }
      //enter 3rd vertex, or repeat previous vertex
      meshman_face[meshman_free_face_ptr].vertex3d_num=(unsigned short int)vertex_num;
      meshman_free_face_ptr++;
      if(obj_texture_coords_defined)
         {
         meshman_facetexture[meshman_free_facetexture_ptr].texvertex_num=(unsigned short int)texvertex_num;
         meshman_free_facetexture_ptr++;
         }
      if((obj_normal_vectors_defined)&&(normal_num>=0)&&(normal_num<OBJ_MAX_NUM_NORMALS))
         {
         nx=obj_normal[normal_num].nx,ny=obj_normal[normal_num].ny,nz=obj_normal[normal_num].nz;
         meshman_normal[meshman_free_normal_ptr].nx=nx;
         meshman_normal[meshman_free_normal_ptr].ny=ny;
         meshman_normal[meshman_free_normal_ptr].nz=nz;
         meshman_free_normal_ptr++;
         meshman_mesh[mesh_num].num_normals++;
         num_normalvectors_loaded++;
         }
      //fourth vertex group
      if(num_entries>=5)
         {
         obj_split_face(el5,&vertex_num,&texvertex_num,&normal_num);
         if(vertex_num>0) {vertex_num--; texvertex_num--; normal_num--;}  //.OBJ files start indexing at 1 not 0
         else
            {
            //backwards indexing used
            vertex_num+=meshman_mesh[mesh_num].num_3d_vertices;
            texvertex_num+=meshman_mesh[mesh_num].num_3d_vertices;
            normal_num+=meshman_mesh[mesh_num].num_3d_vertices;
            }
         }
      //enter 4th vertex, or repeat previous vertex
      meshman_face[meshman_free_face_ptr].vertex3d_num=(unsigned short int)vertex_num;
      meshman_free_face_ptr++;
      if(obj_texture_coords_defined)
         {
         meshman_facetexture[meshman_free_facetexture_ptr].texvertex_num=(unsigned short int)texvertex_num;
         meshman_free_facetexture_ptr++;
         }
      if((obj_normal_vectors_defined)&&(normal_num>=0)&&(normal_num<OBJ_MAX_NUM_NORMALS))
         {
         nx=obj_normal[normal_num].nx,ny=obj_normal[normal_num].ny,nz=obj_normal[normal_num].nz;
         meshman_normal[meshman_free_normal_ptr].nx=nx;
         meshman_normal[meshman_free_normal_ptr].ny=ny;
         meshman_normal[meshman_free_normal_ptr].nz=nz;
         meshman_free_normal_ptr++;
         meshman_mesh[mesh_num].num_normals++;
         num_normalvectors_loaded++;
         }
      if(num_entries==3) num_2sided_faces++;
      else if(num_entries==4) num_3sided_faces++;
      else if(num_entries==5) num_4sided_faces++;

      meshman_mesh[mesh_num].num_faces++;
      num_faces_loaded++;
      //activate flag so next vertex declaration creates a new mesh
      obj_group_finished=1;
      }
   }
fclose(READ_LINE_IN);

if(OBJ_PARSE_DEBUG_ON)
   printf("num_2sided_faces,num_3sided_faces,num_4sided_faces=%d,%d,%d\n",num_2sided_faces,num_3sided_faces,num_4sided_faces);

if(obj_normal_ptr>=OBJ_MAX_NUM_NORMALS)
   printf("WARNING: more than %d normal vectors in OBJ file, only first %d kept\n",OBJ_MAX_NUM_NORMALS);

if(OBJ_PARSE_DEBUG_ON)
   for(material_num=0;material_num<obj_material_ptr;material_num++)
      printf("material <%s> from mesh %d\n",obj_material[material_num].name,obj_material[material_num].mesh_num);


//--------------- parse material file --------------
if(strlen(mtl_filename)>0)
   {
   char temp_line[512];
   int j,current_material_num=0;
   char *mtl_name_storage[OBJ_MAX_NUM_MATERIALS];

   READ_LINE_IN=fopen(mtl_filename,"rb");
   if(READ_LINE_IN==NULL) printf("Can't open material file <%s> for reading\n",mtl_filename);
   else
      {
      printf("Parsing material file <%s>\n",mtl_filename);

      for(j=0;j<OBJ_MAX_NUM_MATERIALS;j++)
         {
         mtl_name_storage[j]=(char*)malloc(256*sizeof(char));
         if(mtl_name_storage[j]==NULL) {printf("ERROR: can't malloc mtl_name_storage\n");return-1;}
         }
      while(read_line(temp_line)==0)
         {
         int i,num_entries;
         if(temp_line[0]=='<')
            {
            i=1;
            while((i<strlen(temp_line))&&(temp_line[i]!='>')) i++;
            i++;
            if(i<strlen(temp_line)) strcpy(line,&(temp_line[i]));
            }
         else strcpy(line,temp_line);
         //printf("read line [%s]\n",line);
         num_entries=break_line(line,el1,el2,el3,el4,el5,el6,el7,el8,el9);
         if( ((strcmp(el1,"NEWMTL")==0)||(strcmp(el1,"newmtl")==0))&&(num_entries>=2) )
            {
            //start material
            current_material_num=meshman_material_ptr;
            if(meshman_material_ptr<meshman_max_num_materials)
               {
               meshman_material_ptr++;
               meshman_material[current_material_num].material_number_in_file=current_material_num;
               meshman_material[current_material_num].bitmap_on=0;
               meshman_material[current_material_num].wireframe_on=0;
               meshman_material[current_material_num].diffuse_on=0;
               meshman_material[current_material_num].specular_on=0;
               meshman_material[current_material_num].ambient_on=0;
               }
            //store name in material file
            strcpy(mtl_name_storage[current_material_num],el2);
            }
         else if( ((strcmp(el1,"KD")==0)||(strcmp(el1,"Kd")==0)||(strcmp(el1,"Kd")==0))&&(num_entries>=4) )
            {
            float red,green,blue;
            sscanf(el2,"%f",&red);
            sscanf(el3,"%f",&green);
            sscanf(el4,"%f",&blue);
            //replace diffuse with bright white if all dark
            if((red<0.1)&&(green<0.1)&&(blue<0.1)) {red=1.0;green=1.0;blue=1.0;}
            meshman_material[current_material_num].diffuse_red=red;
            meshman_material[current_material_num].diffuse_green=green;
            meshman_material[current_material_num].diffuse_blue=blue;
            meshman_material[current_material_num].diffuse_on=1;
            }
         else if( ((strcmp(el1,"KA")==0)||(strcmp(el1,"Ka")==0)||(strcmp(el1,"ka")==0))&&(num_entries>=4) )
            {
            sscanf(el2,"%f",&meshman_material[current_material_num].ambient_red);
            sscanf(el3,"%f",&meshman_material[current_material_num].ambient_green);
            sscanf(el4,"%f",&meshman_material[current_material_num].ambient_blue);
            meshman_material[current_material_num].ambient_on=1;
            }
         else if( ((strcmp(el1,"KS")==0)||(strcmp(el1,"Ks")==0)||(strcmp(el1,"ks")==0))&&(num_entries>=4) )
            {
            sscanf(el2,"%f",&meshman_material[current_material_num].specular_red);
            sscanf(el3,"%f",&meshman_material[current_material_num].specular_green);
            sscanf(el4,"%f",&meshman_material[current_material_num].specular_blue);
            meshman_material[current_material_num].specular_on=1;
            }
         else if( ((strcmp(el1,"MAP_KD")==0)||(strcmp(el1,"map_Kd")==0)||(strcmp(el1,"map_kd")==0))&&(num_entries>=2) )
            {
            int bitmap_id=mesh_management_add_bitmap(el2);  //returns -1 if problem, meshman_bitmap pointer otherwise
            meshman_material[current_material_num].bitmap_num=bitmap_id;
            meshman_material[current_material_num].bitmap_on=1;
            num_bitmaps_present++;
            }
         }
      fclose(READ_LINE_IN);
      if(OBJ_PARSE_DEBUG_ON)
         {
         printf("material mappings from <%s>\n",mtl_filename);
         for(j=0;j<=current_material_num;j++)
            printf("mtl_name_storage[%d]=<%s>\n",j,mtl_name_storage[j]);
         }
      //match material in mtl file to those in OBJ file and set 'material_id' in meshman
      //j=material_id
      for(j=0;j<=current_material_num;j++)
         for(material_num=0;material_num<obj_material_ptr;material_num++)
            if(strcmp(obj_material[material_num].name,mtl_name_storage[j])==0)
               {
               mesh_num=obj_material[material_num].mesh_num;
               meshman_mesh[mesh_num].material=j;
               if(OBJ_PARSE_DEBUG_ON)
                  printf("  Associated material %d with OBJ file material <%s> to mesh %d\n",j,obj_material[material_num].name,mesh_num);
               }
      //free memory
      for(j=0;j<OBJ_MAX_NUM_MATERIALS;j++)
         if(mtl_name_storage[j]!=NULL) free(mtl_name_storage[j]);
      }
   }


//free up temporary normal memory
if(obj_normal!=NULL) free(obj_normal);
if(obj_material!=NULL) free(obj_material);

*num_meshes=num_meshes_loaded;
*num_vtxs=num_3dvertices_loaded;
*num_triangles=num_faces_loaded;
*num_normals=num_normalvectors_loaded;
*num_bitmaps=num_bitmaps_present;
return object_num;
}







void obj_split_face(char *in, int *vertex_num, int *texvertex_num, int *normal_num)
{
char first[256],second[256],third[256];  //number in group
int i=0,ibegin=0;
int number;
//find first number
while((in[i]!='/')&&(i<strlen(in))) i++;
strcpy(first,&(in[ibegin])); first[i]=0;
sscanf(first,"%d",vertex_num);
i++;  //advance past '/'
if(i>=strlen(in)) return;  //only one number
//find second number
ibegin=i;
while((in[i]!='/')&&(i<strlen(in))) i++;
strcpy(second,&(in[ibegin])); second[i]=0;
sscanf(second,"%d",&number);
if(i>=strlen(in))
   {
   //only two numbers, decide if the second is the texture or normal vector
   if((obj_texture_coords_defined)&&(obj_normal_vectors_defined))
     {printf("WARNING: only two numbers in group <%s> when both texture and normal vectors were defined\n",in);return;}
   if((obj_texture_coords_defined==0)&&(obj_normal_vectors_defined==0))
     {printf("WARNING: two numbers in group <%s> when neither texture and normal vectors were defined\n",in);return;}
   if(obj_texture_coords_defined) *texvertex_num=number;
   else                           *normal_num=number;
   return;
   }
i++;  //advance past '/'
if(i>=strlen(in)) return;  //only two numbers
*texvertex_num=number; //second number must be texture vertex
//find third number
ibegin=i;
while((in[i]!='/')&&(i<strlen(in))) i++;
strcpy(third,&(in[ibegin])); third[i]=0;
sscanf(third,"%d",normal_num);
}



/*
//for testing obj_split_face()
{
char temp[256];
int vertex_num,texvertex_num,normal_num;
strcpy(temp,"-123");
obj_split_face(temp,&vertex_num,&texvertex_num,&normal_num);
printf("temp=<%s>   vertex_num=%d  texvertex_num=%d  normal_num=%d\n",temp,vertex_num,texvertex_num,normal_num);
//
strcpy(temp,"-123/-34");
obj_texture_coords_defined=1; obj_normal_vectors_defined=0;
obj_split_face(temp,&vertex_num,&texvertex_num,&normal_num);
printf("temp=<%s>   vertex_num=%d  texvertex_num=%d  normal_num=%d\n",temp,vertex_num,texvertex_num,normal_num);
//
strcpy(temp,"-123/-34");
obj_texture_coords_defined=0; obj_normal_vectors_defined=1;
obj_split_face(temp,&vertex_num,&texvertex_num,&normal_num);
printf("temp=<%s>   vertex_num=%d  texvertex_num=%d  normal_num=%d\n",temp,vertex_num,texvertex_num,normal_num);
//
strcpy(temp,"-123/34/-56");
obj_texture_coords_defined=1; obj_normal_vectors_defined=1;
obj_split_face(temp,&vertex_num,&texvertex_num,&normal_num);
printf("temp=<%s>   vertex_num=%d  texvertex_num=%d  normal_num=%d\n",temp,vertex_num,texvertex_num,normal_num);
//
exit(1);
}
*/




//------------------------------------------------------------------------------------------------------------------------------------------
#endif //#ifndef _OBJ_PARSE_C_
