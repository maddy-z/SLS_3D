//© 2006, National Research Council Canada

//ASE_PARSE.C - reads .ASE files (ascii format in 3DStudio export menu) and puts into mesh_management.c structures
//author:Mark Fiala - Sept 2005 - National Research Council of Canada - IIT/CVG group - 
//
//credits:  thanks for http://www.solosnake.com/main/ase.htm for the partial description of the ASE format
//
//notes:  
//-needs "mesh_management.c"
//-Incomplete coverage of ASE format, only diffuse bitmaps are loaded.  I'm sure someone can write a better parser
//-only the last submaterial in a material will have its diffuse bitmap loaded (multiple diffuse bitmaps in a material
// just overwrite the last one).


//-----------------------------------------------------------------------------------------------------------------------------------------------
#ifndef _ASE_PARSE_C_
 #define _ASE_PARSE_C_


#ifndef _MESH_MANAGEMENT_C_
 #include "mesh_management.c"
#endif

#define ASE_PARSE_DEBUG_ON 0
#define ASE_PARSE_DISPLAY_STATUS_ON 0

//file parsing functions, variables, and constants
char ap_get_char(void);
int ap_get_type(char cc);
int ap_get_token(void);
int ap_expect_token(char *expected);
int ap_get_string_token(char *string_token);

#define ASEPARSE_CHAR_IS_NUMBER       0
#define ASEPARSE_CHAR_IS_LETTER       1
#define ASEPARSE_CHAR_IS_WHITESPACE   2
#define ASEPARSE_CHAR_IS_UNKNOWN      3
#define ASEPARSE_CHAR_IS_COMMA	     4
#define ASEPARSE_CHAR_IS_PERIOD	     5
#define ASEPARSE_CHAR_IS_QUOTES	     6
#define ASEPARSE_CHAR_IS_SQUIGGLY_BRACKET   7

#define ASEPARSE_TOKEN_IS_UNKNOWN  0
#define ASEPARSE_TOKEN_IS_NUMBER   1
#define ASEPARSE_TOKEN_IS_STRING   2
#define ASEPARSE_TOKEN_IS_NAME     3


char ap_char,ap_token[256],ap_token_type;
int ap_parse_line_number;
FILE *ASE_PARSE_IN;




FILE *ase_parse_log_file;

//parse error functions
void parse_error(char *message)
{
#ifdef ASE_LOGFILE_ON
if(ase_parse_logfile_on)
   {
   fprintf(ase_parse_log_file,"PARSE.C error line %d\n%s\n",ap_parse_line_number,message);
   fclose(ase_parse_log_file);
   }
#endif
printf("PARSE.C error line %d\n%s\n",ap_parse_line_number,message);
//exit(1);
}
//{printf("ALE_PARSE.C error line %d\n%s\n",ap_parse_line_number,message);}

//parse error functions
void parse_warning(char *message)
{
#ifdef ASE_LOGFILE_ON
if(ase_parse_logfile_on)
   {
   fprintf(ase_parse_log_file,"PARSE.C warning line %d\n%s\n",ap_parse_line_number,message);
   fclose(ase_parse_log_file);
   }
#endif
printf("PARSE.C warning line %d\n%s\n",ap_parse_line_number,message);
}



//object_type_num=ase_parse(ase_filename,1000,&num_meshes,&num_vtxs,&num_triangles,&num_normals,&bitmaps_loaded);//expect <1000 materials in this file
int ase_parse(char *ase_filename, int max_num_materials, int *num_meshes,
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

if(ASE_PARSE_DEBUG_ON) printf("ASE_PARSE_DEBUG_ON\n"); 

object_num=meshman_free_object_model_ptr;

*num_meshes=0; *num_vtxs=0; *num_triangles=0;
material_lookup=(int*)malloc(max_num_materials*sizeof(int));
if(material_lookup==NULL) {printf("ERROR mallocing *material_lookup\n");return -1;}
for(i=0;i<max_num_materials;i++) material_lookup[i]=-1;

#ifdef ASE_LOGFILE_ON
 ase_parse_log_file=fopen("ase_parse.log","wb");
 if(ase_parse_log_file==NULL) {printf("ERROR opening ase_parse_log_file for writing\n");ase_parse_logfile_on=0;}
 else  ase_parse_logfile_on=1;
#endif

//----------
ap_parse_line_number=1;
ASE_PARSE_IN=fopen(ase_filename,"rb");
if(ASE_PARSE_IN==NULL)
   {sprintf(message,"ERROR: Could not load ASE file <%s>\n",ase_filename);
    parse_error(message);return -1;}

//start meshman_object
strcpy(meshman_object_model[object_num].filename,ase_filename);
meshman_object_model[object_num].start_mesh=meshman_free_mesh_ptr;
meshman_object_model[object_num].end_mesh=meshman_free_mesh_ptr;

ap_char=ap_get_char();   //ap_get_token needs a char to already be in 'ap_char'
end=0;
//in main mode, expect tokens *MATERIAL_LIST,*GEOMOBJECT
while (end==0)
   {
   if(ap_get_token()) end=1;
   if(ASE_PARSE_DEBUG_ON) printf("main token=%s\n",ap_token);
   if((strcmp(ap_token,"*MATERIAL_LIST")==0)||(strcmp(ap_token,"*material_list")==0))
      {
      char material_list_end=0;
      int num_materials=0;
      //in *material_list mode, expect tokens *MATERIAL_COUNT,*MATERIAL
      while ((end==0)&&(material_list_end==0))
         {
         if(ap_get_token()) end=1;
         if(ASE_PARSE_DEBUG_ON) printf("material list token=<%s>\n",ap_token);
         if((strcmp(ap_token,"*MATERIAL_COUNT")==0)||(strcmp(ap_token,"*material_count")==0))
            {
            if(ap_get_token()) end=1;
            if(ap_token_type!=ASEPARSE_TOKEN_IS_NUMBER) parse_error("'*MATERIAL_COUNT number' format expected");
            sscanf(ap_token,"%d",&num_materials);
            }
         else if((strcmp(ap_token,"*MATERIAL")==0)||(strcmp(ap_token,"*material")==0))
            {
            char material_end=0;
            char temp_bitmap_loaded=0,temp_diffuse_loaded=0,temp_specular_loaded=0,temp_ambient_loaded=0;
            int bitmap_id;
            float temp_diffuse_red,temp_diffuse_green,temp_diffuse_blue;
            float temp_specular_red,temp_specular_green,temp_specular_blue;
            float temp_ambient_red,temp_ambient_green,temp_ambient_blue;
            int material_number;
            //get material number
            if(ap_get_token()) end=1;
            if(ap_token_type!=ASEPARSE_TOKEN_IS_NUMBER) parse_error("number expected after *MATERIAL");
            sscanf(ap_token,"%d",&material_number);
            if(ASE_PARSE_DEBUG_ON) printf("in material #%d\n",material_number);
            //in *material mode, expect tokens *MATERIAL_AMBIENT,*MATERIAL_DIFFUSE,*MATERIAL_SPECULAR,
            //                                 *MAP_DIFFUSE,*SUBMATERIAL
            //       and token with brackets to skip over  *MAP_BUMP,*MAP_REFLECT,*MAP_SELFILLUM,*MAP_SPECULAR,*MAP_AMBIENT
            //       and tokens with ""  *MATERIAL_NAME, *MATERIAL_CLASS, *MAP_REFRACT
            while ((end==0)&&(material_end==0))
               {
               if(ap_get_token()) end=1;
               if(ASE_PARSE_DEBUG_ON) printf("material token=<%s>\n",ap_token);
               if((strcmp(ap_token,"*MATERIAL_DIFFUSE")==0)||(strcmp(ap_token,"*material_diffuse")==0))
                  {
                  temp_diffuse_loaded=1;
                  //red
                  if(ap_get_token()) end=1;
                  if(ap_token_type!=ASEPARSE_TOKEN_IS_NUMBER) parse_error("RGB numbers expected after *MATERIAL_DIFFUSE");
                  sscanf(ap_token,"%f",&temp_diffuse_red);
                  //green
                  if(ap_get_token()) end=1;
                  if(ap_token_type!=ASEPARSE_TOKEN_IS_NUMBER) parse_error("RGB numbers expected after *MATERIAL_DIFFUSE");
                  sscanf(ap_token,"%f",&temp_diffuse_green);
                  //blue
                  if(ap_get_token()) end=1;
                  if(ap_token_type!=ASEPARSE_TOKEN_IS_NUMBER) parse_error("RGB numbers expected after *MATERIAL_DIFFUSE");
                  sscanf(ap_token,"%f",&temp_diffuse_blue);
                  }
               else if((strcmp(ap_token,"*MATERIAL_SPECULAR")==0)||(strcmp(ap_token,"*material_specular")==0))
                  {
                  temp_specular_loaded=1;
                  //red
                  if(ap_get_token()) end=1;
                  if(ap_token_type!=ASEPARSE_TOKEN_IS_NUMBER) parse_error("RGB numbers expected after *MATERIAL_SPECULAR");
                  sscanf(ap_token,"%f",&temp_specular_red);
                  //green
                  if(ap_get_token()) end=1;
                  if(ap_token_type!=ASEPARSE_TOKEN_IS_NUMBER) parse_error("RGB numbers expected after *MATERIAL_SPECULAR");
                  sscanf(ap_token,"%f",&temp_specular_green);
                  //blue
                  if(ap_get_token()) end=1;
                  if(ap_token_type!=ASEPARSE_TOKEN_IS_NUMBER) parse_error("RGB numbers expected after *MATERIAL_SPECULAR");
                  sscanf(ap_token,"%f",&temp_specular_blue);
                  }
               if((strcmp(ap_token,"*MATERIAL_AMBIENT")==0)||(strcmp(ap_token,"*material_ambient")==0))
                  {
                  temp_ambient_loaded=1;
                  //red
                  if(ap_get_token()) end=1;
                  if(ap_token_type!=ASEPARSE_TOKEN_IS_NUMBER) parse_error("RGB numbers expected after *MATERIAL_AMBIENT");
                  sscanf(ap_token,"%f",&temp_ambient_red);
                  //green
                  if(ap_get_token()) end=1;
                  if(ap_token_type!=ASEPARSE_TOKEN_IS_NUMBER) parse_error("RGB numbers expected after *MATERIAL_AMBIENT");
                  sscanf(ap_token,"%f",&temp_ambient_green);
                  //blue
                  if(ap_get_token()) end=1;
                  if(ap_token_type!=ASEPARSE_TOKEN_IS_NUMBER) parse_error("RGB numbers expected after *MATERIAL_AMBIENT");
                  sscanf(ap_token,"%f",&temp_ambient_blue);
                  }
               else if((strcmp(ap_token,"*MATERIAL_NAME")==0)||(strcmp(ap_token,"*material_name")==0))
                  {
                  char temp_material_name[512];
                  if(ap_get_string_token(temp_material_name)) end=1;
                  }
               else if((strcmp(ap_token,"*MATERIAL_CLASS")==0)||(strcmp(ap_token,"*material_class")==0))
                  {
                  char temp_material_classname[512];
                  if(ap_get_string_token(temp_material_classname)) end=1;
                  }
               else if((strcmp(ap_token,"*MAP_DIFFUSE")==0)||(strcmp(ap_token,"*map_diffuse")==0))
                  {
                  char map_diffuse_end=0;
                  //in *map_diffuse mode, expect tokens *BITMAP
                  while ((end==0)&&(map_diffuse_end==0))
                     {
                     if(ap_get_token()) end=1;
                     //printf("map_diffuse token=<%s>\n",ap_token);
                     if((strcmp(ap_token,"*BITMAP")==0)||(strcmp(ap_token,"*bitmap")==0))
                        {
                        char temp_bitmap_filename[512],bitmap_filename[512];
                        int i;
                        if(ap_get_string_token(temp_bitmap_filename)) end=1;
                        else
                           {
                           /// ------- found bitmap name -------
                           if(ASE_PARSE_DEBUG_ON) 
                              printf("----mat------bitmap <%s> for material #%d\n",temp_bitmap_filename,material_number);
                           //remove path
                           i=strlen(temp_bitmap_filename);
                           if(i>0)
                              {
                              i--;
                              while((temp_bitmap_filename[i]!='\\')&&(i>0)) i--;
                              if(i>0) i++;
                              strcpy(bitmap_filename,&temp_bitmap_filename[i]);
                              }
                           if(ASE_PARSE_DEBUG_ON) 
                              printf("removed path bitmap <%s> for material #%d\n",bitmap_filename,material_number);
                           //add bitmap
                           bitmap_id=mesh_management_add_bitmap(bitmap_filename);  //returns -1 if problem, meshman_bitmap pointer otherwise
                           if(bitmap_id==-1)
                              {
                              sprintf(message,"can't add bitmap <%s>, meshman max materials reached",bitmap_filename);
                              parse_error(message);
                              }
                           else temp_bitmap_loaded=1; 
                           }
                        }//if((strcmp(ap_token,"*BITMAP")==0)||(strcmp(ap_token,"*bitmap")==0))
                     else if((strcmp(ap_token,"*MAP_GENERIC")==0)||(strcmp(ap_token,"*map_generic")==0))
                        {
                        char map_generic_end=0;
                        //printf("starting map_generic\n");
                        while ((end==0)&&(map_generic_end==0))
                           {
                           if(ap_get_token()) end=1;
                           if((strcmp(ap_token,"*BITMAP")==0)||(strcmp(ap_token,"*bitmap")==0))
                              {
                              char temp_bitmap_filename[512],bitmap_filename[512];
                              int i;
                              if(ap_get_string_token(temp_bitmap_filename)) end=1;
                              else
                                 {
                                 /// ------- found bitmap name -------
                                 if(ASE_PARSE_DEBUG_ON) 
                                    printf("----sub/generic--bitmap <%s> for material #%d\n",temp_bitmap_filename,material_number);
                                 //remove path
                                 i=strlen(temp_bitmap_filename);
                                 if(i>0)
                                    {
                                    i--;
                                    while((temp_bitmap_filename[i]!='\\')&&(i>0)) i--;
                                    if(i>0) i++;
                                    strcpy(bitmap_filename,&temp_bitmap_filename[i]);
                                    }
                                 if(ASE_PARSE_DEBUG_ON) 
                                    printf("removed path bitmap <%s> for material #%d\n",bitmap_filename,material_number);
                                 //add bitmap
                                 bitmap_id=mesh_management_add_bitmap(bitmap_filename);  //returns -1 if problem, meshman_bitmap pointer otherwise
                                 if(bitmap_id==-1)
                                    {
                                    sprintf(message,"can't add bitmap <%s>, meshman max materials reached",bitmap_filename);
                                    parse_error(message);
                                    }
                                 else temp_bitmap_loaded=1;
                                 }
                              }//if((strcmp(ap_token,"*BITMAP")==0)||(strcmp(ap_token,"*bitmap")==0))
                           else if(strcmp(ap_token,"}")==0)  map_generic_end=1;
                           }////while ((end==0)&&(map_generic_end==0))
                        //printf("ending map_generic_end\n");
                        }
                     else if(strcmp(ap_token,"}")==0)  map_diffuse_end=1;
                     }//while ((end==0)&&(map_diffuse_end==0))
                  }
               else if((strcmp(ap_token,"*SUBMATERIAL")==0)||(strcmp(ap_token,"*submaterial")==0))
                  {
                  char submaterial_end=0;
                  if(ap_get_token()) end=1;  //get past submaterial number
                  if(ASE_PARSE_DEBUG_ON) printf("in submaterial %s\n",ap_token);
                  if(ap_get_token()) end=1;  //get past {
                  //printf("in submaterial token should be {  is <%s>\n",ap_token);
                  //in *submaterial mode, expect tokens *MAP_DIFFUSE
                  //       and token with brackets to skip over  *MAP_BUMP,*MAP_REFLECT,*MAP_SELFILLUM,*MAP_SPECULAR,*MAP_AMBIENT
				  //       and tokens with ""  *MATERIAL_NAME, *MATERIAL_CLASS
                  while ((end==0)&&(submaterial_end==0))
                     {
                     if(ap_get_token()) end=1;
                     if(ASE_PARSE_DEBUG_ON) printf("submaterial list token=<%s>\n",ap_token);
                     if((strcmp(ap_token,"*MAP_DIFFUSE")==0)||(strcmp(ap_token,"*map_diffuse")==0))
                        {
                        char map_diffuse_end=0;
                        //in *map_diffuse mode, expect tokens *BITMAP
                        while ((end==0)&&(map_diffuse_end==0))
                           {
                           if(ap_get_token()) end=1;
                           if((strcmp(ap_token,"*BITMAP")==0)||(strcmp(ap_token,"*bitmap")==0))
                              {
                              char temp_bitmap_filename[512],bitmap_filename[512];
                              int i;
                              if(ap_get_string_token(temp_bitmap_filename)) end=1;
                              else
                                 {
                                 /// ------- found bitmap name -------
                                 if(ASE_PARSE_DEBUG_ON) 
								    printf("----sub------bitmap <%s> for material #%d\n",temp_bitmap_filename,material_number);
                                 //remove path
                                 i=strlen(temp_bitmap_filename);
                                 if(i>0)
                                    {
                                    i--;
                                    while((temp_bitmap_filename[i]!='\\')&&(i>0)) i--;
                                    if(i>0) i++;
                                    strcpy(bitmap_filename,&temp_bitmap_filename[i]);
                                    }
                                 if(ASE_PARSE_DEBUG_ON) 
								    printf("removed path bitmap <%s> for material #%d\n",bitmap_filename,material_number);
                                 //add bitmap
                                 bitmap_id=mesh_management_add_bitmap(bitmap_filename);  //returns -1 if problem, meshman_bitmap pointer otherwise
                                 if(bitmap_id==-1)
                                    {
                                    sprintf(message,"can't add bitmap <%s>, meshman max materials reached",bitmap_filename);
                                    parse_error(message);
                                    }
                                 else temp_bitmap_loaded=1;
                                 }
                              }//if((strcmp(ap_token,"*BITMAP")==0)||(strcmp(ap_token,"*bitmap")==0))
                           else if(strcmp(ap_token,"}")==0)  map_diffuse_end=1;
                           }//while ((end==0)&&(map_diffuse_end==0))
                        }
                     else if((strcmp(ap_token,"*MAP_BUMP")==0)||(strcmp(ap_token,"*map_bump")==0))
                        {
                        char map_bump_end=0;
                        //printf("starting map_bump\n");
                        while ((end==0)&&(map_bump_end==0))
                           {
                           if(ap_get_token()) end=1;
                           if(strcmp(ap_token,"}")==0)  map_bump_end=1;
                           }////while ((end==0)&&(map_bump_end==0))
                        //printf("ending map_bump\n");
                        }
                     else if((strcmp(ap_token,"*MAP_REFLECT")==0)||(strcmp(ap_token,"*map_reflect")==0))
                        {
                        char map_reflect_end=0;
                        //printf("starting map_reflect\n");
                        while ((end==0)&&(map_reflect_end==0))
                           {
                           if(ap_get_token()) end=1;
                           if(strcmp(ap_token,"}")==0)  map_reflect_end=1;
                           }////while ((end==0)&&(map_reflect_end==0))
                        //printf("ending map_reflect\n");
                        }
                     else if((strcmp(ap_token,"*MAP_SELFILLUM")==0)||(strcmp(ap_token,"*map_selfillum")==0))
                        {
                        char map_selfillum_end=0;
                        //printf("starting map_selfillumt\n");
                        while ((end==0)&&(map_selfillum_end==0))
                           {
                           if(ap_get_token()) end=1;
                           if(strcmp(ap_token,"}")==0)  map_selfillum_end=1;
                           }////while ((end==0)&&(map_selfillum_end==0))
                        //printf("ending map_selfillum\n");
                        }
                     else if((strcmp(ap_token,"*MAP_SPECULAR")==0)||(strcmp(ap_token,"*map_specular")==0))
                        {
                        char map_specular_end=0;
                        //printf("starting map_specular\n");
                        while ((end==0)&&(map_specular_end==0))
                           {
                           if(ap_get_token()) end=1;
                           if(strcmp(ap_token,"}")==0)  map_specular_end=1;
                           }////while ((end==0)&&(map_selfillum_end==0))
                        //printf("ending map_specular\n");
                        }
                     else if((strcmp(ap_token,"*MAP_REFRACT")==0)||(strcmp(ap_token,"*map_refract")==0))
                        {
                        char map_refract_end=0;
                        //printf("starting map_refract\n");
                        while ((end==0)&&(map_refract_end==0))
                           {
                           if(ap_get_token()) end=1;
                           if(strcmp(ap_token,"}")==0)  map_refract_end=1;
                           }////while ((end==0)&&(map_refract_end==0))
                        //printf("ending map_refract\n");
                        }
                     else if((strcmp(ap_token,"*MAP_AMBIENT")==0)||(strcmp(ap_token,"*map_ambient")==0))
                        {
                        char map_ambient_end=0;
                        //printf("starting map_ambient\n");
                        while ((end==0)&&(map_ambient_end==0))
                           {
                           if(ap_get_token()) end=1;
                           if(strcmp(ap_token,"}")==0)  map_ambient_end=1;
                           }////while ((end==0)&&(map_ambient_end==0))
                        //printf("ending map_refract\n");
                        }
                     else if((strcmp(ap_token,"*MATERIAL_NAME")==0)||(strcmp(ap_token,"*material_name")==0))
                        {
                        char temp_material_name[512];
                        if(ap_get_string_token(temp_material_name)) end=1;
                        }
                     else if((strcmp(ap_token,"*MATERIAL_CLASS")==0)||(strcmp(ap_token,"*material_class")==0))
                        {
                        char temp_material_classname[512];
                        if(ap_get_string_token(temp_material_classname)) end=1;
                        }
                     else if(strcmp(ap_token,"}")==0)  submaterial_end=1;
                     }////while ((end==0)&&(submaterial_end==0))
                  }
               else if((strcmp(ap_token,"*MAP_BUMP")==0)||(strcmp(ap_token,"*map_bump")==0))
                  {
                  char map_bump_end=0;
                  //printf("starting map_bump\n");
                  while ((end==0)&&(map_bump_end==0))
                     {
                     if(ap_get_token()) end=1;
                     if(strcmp(ap_token,"}")==0)  map_bump_end=1;
                     }////while ((end==0)&&(map_bump_end==0))
                  //printf("ending map_bump\n");
                  }
               else if((strcmp(ap_token,"*MAP_REFLECT")==0)||(strcmp(ap_token,"*map_reflect")==0))
                  {
                  char map_reflect_end=0;
                  //printf("starting map_reflect\n");
                  while ((end==0)&&(map_reflect_end==0))
                     {
                     if(ap_get_token()) end=1;
                     if(strcmp(ap_token,"}")==0)  map_reflect_end=1;
                     }////while ((end==0)&&(map_reflect_end==0))
                  //printf("ending map_reflect\n");
                  }
               else if((strcmp(ap_token,"*MAP_SELFILLUM")==0)||(strcmp(ap_token,"*map_selfillum")==0))
                  {
                  char map_selfillum_end=0;
                  //printf("starting map_selfillumt\n");
                  while ((end==0)&&(map_selfillum_end==0))
                     {
                     if(ap_get_token()) end=1;
                     if(strcmp(ap_token,"}")==0)  map_selfillum_end=1;
                     }////while ((end==0)&&(map_selfillum_end==0))
                  //printf("ending map_selfillum\n");
                  }
               else if((strcmp(ap_token,"*MAP_SPECULAR")==0)||(strcmp(ap_token,"*map_specular")==0))
                  {
                  char map_specular_end=0;
                  //printf("starting map_specular\n");
                  while ((end==0)&&(map_specular_end==0))
                     {
                     if(ap_get_token()) end=1;
                     if(strcmp(ap_token,"}")==0)  map_specular_end=1;
                     }////while ((end==0)&&(map_specular_end==0))
                  //printf("ending map_specular\n");
                  }
               else if((strcmp(ap_token,"*MAP_REFRACT")==0)||(strcmp(ap_token,"*map_refract")==0))
                  {
                  char map_refract_end=0;
                  //printf("starting map_refract\n");
                  while ((end==0)&&(map_refract_end==0))
                     {
                     if(ap_get_token()) end=1;
                     if(strcmp(ap_token,"}")==0)  map_refract_end=1;
                     }////while ((end==0)&&(map_refract_end==0))
                  //printf("ending map_refract\n");
                  }
               else if((strcmp(ap_token,"*MAP_AMBIENT")==0)||(strcmp(ap_token,"*map_ambient")==0))
                  {
                  char map_ambient_end=0;
                  //printf("starting map_ambient\n");
                  while ((end==0)&&(map_ambient_end==0))
                     {
                     if(ap_get_token()) end=1;
                     if(strcmp(ap_token,"}")==0)  map_ambient_end=1;
                     }////while ((end==0)&&(map_ambient_end==0))
                  //printf("ending map_refract\n");
                  }
               else if(strcmp(ap_token,"}")==0)  material_end=1;
               }//while ((end==0)&&(material_end==0))
            //------------------- file material --------------------
            if(meshman_material_ptr>=meshman_max_num_materials)
               parse_error("Too many materials: limit of -meshman_max_num_materials- reached");
            else
               {
               //material bitmap
               if(temp_bitmap_loaded)
                  {
                  meshman_material[meshman_material_ptr].bitmap_num=bitmap_id;
                  meshman_material[meshman_material_ptr].bitmap_on=1;
				  num_bitmaps_present++;
                  }
               else meshman_material[meshman_material_ptr].bitmap_on=0;
/*            //wireframe
            if()  meshman_material[meshman_material_ptr].wireframe_on=1;
            else  meshman_material[meshman_material_ptr].wireframe_on=0;*/
            //material diffuse properties
               if(temp_diffuse_loaded)
                  {
                  meshman_material[meshman_material_ptr].diffuse_red=temp_diffuse_red;
                  meshman_material[meshman_material_ptr].diffuse_green=temp_diffuse_green;
                  meshman_material[meshman_material_ptr].diffuse_blue=temp_diffuse_blue;
                  meshman_material[meshman_material_ptr].diffuse_on=1;
                  }
               else meshman_material[meshman_material_ptr].diffuse_on=0;
               //material specular properties
               if(temp_specular_loaded)
                  {
                  meshman_material[meshman_material_ptr].specular_red=temp_specular_red;
                  meshman_material[meshman_material_ptr].specular_green=temp_specular_green;
                  meshman_material[meshman_material_ptr].specular_blue=temp_specular_blue;
                  meshman_material[meshman_material_ptr].specular_on=1;
                  }
               else meshman_material[meshman_material_ptr].specular_on=0;
               //material ambient properties
               if(temp_ambient_loaded)
                  {
                  meshman_material[meshman_material_ptr].ambient_red=temp_ambient_red;
                  meshman_material[meshman_material_ptr].ambient_green=temp_ambient_green;
                  meshman_material[meshman_material_ptr].ambient_blue=temp_ambient_blue;
                  meshman_material[meshman_material_ptr].ambient_on=1;
                  }
               else meshman_material[meshman_material_ptr].ambient_on=0;
               //associate number in file (material_number) with number in meshman (meshman_material_ptr)
               material_lookup[material_number]=meshman_material_ptr;
               meshman_material[meshman_material_ptr].material_number_in_file=material_number;
               meshman_material_ptr++;
               }
            if(ASE_PARSE_DEBUG_ON) 
			   printf("filed material %d into meshman material %d\n",material_number,meshman_material_ptr-1);
            }
         else if(strcmp(ap_token,"}")==0)  material_list_end=1;
         }//while ((end==0)&&(material_list_end==0))
      if(ASE_PARSE_DEBUG_ON) printf("Made it to end of *MATERIAL_LIST - %d materials loaded\n",num_materials);
//      printf("Made it to end of *MATERIAL_LIST - %d materials loaded\n",num_materials);
//      for(i=0;i<meshman_bitmap_ptr;i++) printf("%d: <%s>\n",i,meshman_bitmap[i].filename);
//      exit(1);
      }
   else if((strcmp(ap_token,"*GEOMOBJECT")==0)||(strcmp(ap_token,"*geomobject")==0))
      {
      char geomobject_end=0;
      int num_3dvtx_this_mesh=0,num_texvtx_this_mesh=0,num_normalvtx_this_mesh=0;  //count vertex elements in this mesh
      int num_faces_this_mesh=0,num_texfaces_this_mesh=0;  //count face elements in this mesh
      char temp_wireframe_mode=0;
      float temp_wireframe_red,temp_wireframe_green,temp_wireframe_blue;
      int mesh_num_vertices=-1,mesh_numfaces=-1,mesh_numtexvertices=-1,mesh_numtexfaces=-1;
      int material_ref=-1;
      //in *geomobject mode, expect tokens *NODE_TM,*MESH,*WIREFRAME_COLOR,*MATERIAL_REF
      while ((end==0)&&(geomobject_end==0))
         {
         if(ap_get_token()) end=1;
         //printf("geomobject token=%s\n",ap_token);
         if((strcmp(ap_token,"*NODE_TM")==0)||(strcmp(ap_token,"*node_tm")==0))
            {
            while((ap_expect_token("}"))&&(end==0));
            if(end==1) parse_error("file ended inside *NODE_TM  -*NODE_TM never ended with }");
            //printf("Made it to end of *NODE_TM\n");
            }
         else if((strcmp(ap_token,"*WIREFRAME_COLOR")==0)||(strcmp(ap_token,"*wireframe_color")==0))
            {
            temp_wireframe_mode=1;
            //red
            if(ap_get_token()) end=1;
            if(ap_token_type!=ASEPARSE_TOKEN_IS_NUMBER) parse_error("RGB numbers expected after *WIREFRAME_COLOR");
            sscanf(ap_token,"%f",&temp_wireframe_red);
            //green
            if(ap_get_token()) end=1;
            if(ap_token_type!=ASEPARSE_TOKEN_IS_NUMBER) parse_error("RGB numbers expected after *WIREFRAME_COLOR");
            sscanf(ap_token,"%f",&temp_wireframe_green);
            //blue
            if(ap_get_token()) end=1;
            if(ap_token_type!=ASEPARSE_TOKEN_IS_NUMBER) parse_error("RGB numbers expected after *WIREFRAME_COLOR");
            sscanf(ap_token,"%f",&temp_wireframe_blue);
            }
         else if((strcmp(ap_token,"*MATERIAL_REF")==0)||(strcmp(ap_token,"*material_ref")==0))
            {
            if(ap_get_token()) end=1;
            if(ap_token_type!=ASEPARSE_TOKEN_IS_NUMBER) parse_error("material number expected after *MATERIAL_REF");
            sscanf(ap_token,"%d",&material_ref);
            }
      //in *geomobject mode, expect tokens *NODE_TM,*MESH,*WIREFRAME_COLOR,*MATERIAL_REF

         else if((strcmp(ap_token,"*MESH")==0)||(strcmp(ap_token,"*mesh")==0))
            {
            char mesh_end=0;
            //in *mesh mode, expect tokens *MESH_NUMVERTEX,*MESH_NUMFACES,*MESH_NUMTVERTEX,*MESH_NUMTVFACES,
            //                             *MESH_VERTEX_LIST,*MESH_TVERTLIST,*MESH_FACE_LIST,*MESH_TFACELIST,
            //                             *MESH_NORMALS
            if(ASE_PARSE_DEBUG_ON) printf("in MESH mode\n");
            if(ASE_PARSE_DEBUG_ON) printf("object_num=%d meshman_free_mesh_ptr=%d\n",object_num,meshman_free_mesh_ptr);
            //
	    //--- start new mesh ---
            meshman_object_model[object_num].end_mesh=meshman_free_mesh_ptr;
            meshman_mesh[meshman_free_mesh_ptr].start_face=meshman_free_face_ptr;
            meshman_mesh[meshman_free_mesh_ptr].start_face_t=meshman_free_facetexture_ptr;
            meshman_mesh[meshman_free_mesh_ptr].num_sides=3;  //ASE files have triangles
            meshman_mesh[meshman_free_mesh_ptr].num_faces=0;
            meshman_mesh[meshman_free_mesh_ptr].base_vertex3d_ptr=meshman_free_3dpoint_ptr;
            meshman_mesh[meshman_free_mesh_ptr].num_3d_vertices=0;
            meshman_mesh[meshman_free_mesh_ptr].base_vertextex_ptr=meshman_free_texpoint_ptr;
            meshman_mesh[meshman_free_mesh_ptr].num_tex_vertices=0;
            meshman_mesh[meshman_free_mesh_ptr].base_facecolour_ptr=meshman_free_facecolour_ptr;
            meshman_mesh[meshman_free_mesh_ptr].num_face_colours=0;
            meshman_mesh[meshman_free_mesh_ptr].base_normal_ptr=meshman_free_normal_ptr;
            meshman_mesh[meshman_free_mesh_ptr].num_normals=0;
            meshman_mesh[meshman_free_mesh_ptr].normal_per_vertex=1;  //hard-coded, this version of ASE_PARSE.C only uses vertex normals
            //printf("in MESH modee\n");exit(1);
	    //parse mesh entry
            while ((end==0)&&(mesh_end==0))
               {
               if(ap_get_token()) end=1;
               if(ASE_PARSE_DEBUG_ON) printf("mesh token=%s\n",ap_token);
               if((strcmp(ap_token,"*MESH_NUMVERTEX")==0)||(strcmp(ap_token,"*mesh_numvertex")==0))
                  {
                  if(ap_get_token()) end=1;
                  if(ap_token_type!=ASEPARSE_TOKEN_IS_NUMBER) parse_error("'*mesh_numvertex number' format expected");
                  sscanf(ap_token,"%d",&mesh_num_vertices);
                  }
               else if((strcmp(ap_token,"*MESH_NUMFACES")==0)||(strcmp(ap_token,"*mesh_numfaces")==0))
                  {
                  if(ap_get_token()) end=1;
                  if(ap_token_type!=ASEPARSE_TOKEN_IS_NUMBER) parse_error("'*mesh_numfaces number' format expected");
                  sscanf(ap_token,"%d",&mesh_numfaces);
                  }
               else if((strcmp(ap_token,"*MESH_NUMTVERTEX")==0)||(strcmp(ap_token,"*mesh_numtvertex")==0))
                  {
                  if(ap_get_token()) end=1;
                  if(ap_token_type!=ASEPARSE_TOKEN_IS_NUMBER) parse_error("'*numtvertex number' format expected");
                  sscanf(ap_token,"%d",&mesh_numtexvertices);
                  }
               else if((strcmp(ap_token,"*MESH_NUMTVFACES")==0)||(strcmp(ap_token,"*mesh_numtvfaces")==0))
                  {
                  if(ap_get_token()) end=1;
                  if(ap_token_type!=ASEPARSE_TOKEN_IS_NUMBER) parse_error("'*mesh_numtvfaces number' format expected");
                  sscanf(ap_token,"%d",&mesh_numtexfaces);
                  }
               else if((strcmp(ap_token,"*MESH_VERTEX_LIST")==0)||(strcmp(ap_token,"*mesh_vertex_list")==0))
                  {
                  //------------------------------ 3D VERTEX COORDS ----------------------------------
                  char vtxlist_end=0;
                  //get {
                  if(ap_get_token()) end=1;
                  if(strcmp(ap_token,"{")!=0)
                     parse_error("{ expected after *MESH_VERTEX_LIST");
                  //in *vertex list mode, expect tokens *MESH_VERTEX only
                  while ((end==0)&&(vtxlist_end==0))
                     {
                     if(ap_get_token()) end=1;
                     //printf("vtx list token=%s\n",ap_token);
                     if((strcmp(ap_token,"*MESH_VERTEX")!=0)&&(strcmp(ap_token,"*mesh_vertex")!=0)&&(strcmp(ap_token,"}")!=0))
                        parse_error("*MESH_VERTEX token expected only inside MESH_VERTEX_LIST");
                     if((strcmp(ap_token,"*MESH_VERTEX")==0)||(strcmp(ap_token,"*mesh_vertex")==0))
                        {
                        int id;
                        float x,y,z;
                        //get id
                        if(ap_get_token()) end=1;
                        if(ap_token_type!=ASEPARSE_TOKEN_IS_NUMBER) parse_error("'*MESH_VERTEX number' format expected");
                        sscanf(ap_token,"%d",&id);
                        //get x,y,z
                        if(ap_get_token()) end=1;
                        sscanf(ap_token,"%f",&x);
                        if(ap_get_token()) end=1;
                        sscanf(ap_token,"%f",&y);
                        if(ap_get_token()) end=1;
                        sscanf(ap_token,"%f",&z);
                        //
                        if((id>=0)&&(id<mesh_num_vertices))
                           {
                           meshman_3dpoint[meshman_free_3dpoint_ptr].x=x;
                           meshman_3dpoint[meshman_free_3dpoint_ptr].y=y;
                           meshman_3dpoint[meshman_free_3dpoint_ptr].z=z;
                           meshman_mesh[meshman_free_mesh_ptr].num_3d_vertices++;
                           meshman_free_3dpoint_ptr++;
                           num_3dvertices_loaded++;
                           num_3dvtx_this_mesh++;
                           }
                        else {
                             sprintf(message,"*MESH_VERTEX id=%d not in range 0-%d",id,mesh_num_vertices);
                             parse_error(message);
                             }
                        }
                     else if(strcmp(ap_token,"}")==0)  vtxlist_end=1;
                     }
                  }
               else if((strcmp(ap_token,"*MESH_TVERTLIST")==0)||(strcmp(ap_token,"*mesh_tvertlist")==0))
                  {
                  //------------------------------ 2D VERTEX TEXTURE COORDS ----------------------------------
                  char tvtxlist_end=0;
                  //get {
                  if(ap_get_token()) end=1;
                  if(strcmp(ap_token,"{")!=0)
                     parse_error("{ expected after *MESH_VERTEX_LIST");
                  //in *vertex list mode, expect tokens *MESH_TVERT only
                  while ((end==0)&&(tvtxlist_end==0))
                     {
                     if(ap_get_token()) end=1;
                     //printf("vtx list token=%s\n",ap_token);
                     if((strcmp(ap_token,"*MESH_TVERT")!=0)&&(strcmp(ap_token,"*mesh_tvert")!=0)&&(strcmp(ap_token,"}")!=0))
                        parse_error("*MESH_TVERT token expected only inside MESH_VERTEX_LIST");
                     if((strcmp(ap_token,"*MESH_TVERT")==0)||(strcmp(ap_token,"*mesh_tvert")==0))
                        {
                        int id;
                        float tx,ty,tz;
                        //get id
                        if(ap_get_token()) end=1;
                        if(ap_token_type!=ASEPARSE_TOKEN_IS_NUMBER) parse_error("'*MESH_VERTEX number' format expected");
                        sscanf(ap_token,"%d",&id);
                        //get tx,ty,tz
                        if(ap_get_token()) end=1;
                        sscanf(ap_token,"%f",&tx);
                        if(ap_get_token()) end=1;
                        sscanf(ap_token,"%f",&ty);
                        if(ap_get_token()) end=1;
                        sscanf(ap_token,"%f",&tz);
                        //
                        if((id>=0)&&(id<mesh_numtexvertices))
                           {
                           meshman_texpoint[meshman_free_texpoint_ptr].u=tx;
                           meshman_texpoint[meshman_free_texpoint_ptr].v=ty;
                           meshman_mesh[meshman_free_mesh_ptr].num_tex_vertices++;
                           meshman_free_texpoint_ptr++;
                           num_texvertices_loaded++;
                           num_texvtx_this_mesh++;
                          }
                        }
                     else if(strcmp(ap_token,"}")==0)  tvtxlist_end=1;
                     }
                  }
               else if((strcmp(ap_token,"*MESH_NORMALS")==0)||(strcmp(ap_token,"*mesh_normals")==0))
                  {
                  //------------------------------ Normal Vectors for each Vertex ----------------------------------
                  char normallist_end=0;
                  //get {
                  if(ap_get_token()) end=1;
                  if(strcmp(ap_token,"{")!=0)
                     parse_error("{ expected after *MESH_NORMALS");
                  //in *face_list list mode, expect tokens *MESH_FACENORMAL,*MESH_VERTEXNORMAL - allow others due to extra items after C:
                  while ((end==0)&&(normallist_end==0))
                     {
                     if(ap_get_token()) end=1;
                     if((strcmp(ap_token,"*MESH_FACENORMAL")==0)||(strcmp(ap_token,"*mesh_facenormal")==0))
                        {
                        //face normals not currently used
                        int face_num;
                        float nx,ny,nz;
                        if(ap_get_token()) end=1;
                        sscanf(ap_token,"%d",&face_num);
                        if(ap_get_token()) end=1;
                        sscanf(ap_token,"%f",&nx);
                        if(ap_get_token()) end=1;
                        sscanf(ap_token,"%f",&ny);
                        if(ap_get_token()) end=1;
                        sscanf(ap_token,"%f",&nz);
                        }
                     else if((strcmp(ap_token,"*MESH_VERTEXNORMAL")==0)||(strcmp(ap_token,"*mesh_vertexnormal")==0))
                        {
                        int id; //id unused
                        float nx,ny,nz;
                        //get id
                        if(ap_get_token()) end=1;
                        if(ap_token_type!=ASEPARSE_TOKEN_IS_NUMBER) parse_error("'*MESH_VERTEXNORMAL number' format expected");
                        sscanf(ap_token,"%d",&id);
                        //get normal vector (nx,ny,nz)
                        if(ap_get_token()) end=1;
                        sscanf(ap_token,"%f",&nx);
                        if(ap_get_token()) end=1;
                        sscanf(ap_token,"%f",&ny);
                        if(ap_get_token()) end=1;
                        sscanf(ap_token,"%f",&nz);
                        //
                        if((id>=0)&&(id<mesh_numtexvertices))
                           {
                           meshman_normal[meshman_free_normal_ptr].nx=nx;
                           meshman_normal[meshman_free_normal_ptr].ny=ny;
                           meshman_normal[meshman_free_normal_ptr].nz=nz;
                           meshman_free_normal_ptr++;
                           meshman_mesh[meshman_free_mesh_ptr].num_normals++;
                           num_normalvectors_loaded++;
                           num_normalvtx_this_mesh++;
                          }
                        }//if((strcmp(ap_token,"*MESH_VERTEXNORMAL")==0)||(strcmp(ap_token,"*mesh_vertexnormal")==0))
                     else if(strcmp(ap_token,"}")==0) normallist_end=1;
                     //if(normallist_end) printf("normallist_end\n");
                     }//while ((end==0)&&(normallist_end==0))
                  }
               else if((strcmp(ap_token,"*MESH_FACE_LIST")==0)||(strcmp(ap_token,"*mesh_face_list")==0))
                  {
                  //------------------------------ TRIANGLES - VERTEX MEMBERS ----------------------------------
                  char facelist_end=0;
                  //get {
                  if(ap_get_token()) end=1;
                  if(strcmp(ap_token,"{")!=0)
                     parse_error("{ expected after *MESH_FACE_LIST");
                  //in *face_list list mode, expect tokens *MESH_FACE - allow others due to extra items after C:
                  while ((end==0)&&(facelist_end==0))
                     {
                     if(ap_get_token()) end=1;
                     //printf("face list token=%s\n",ap_token);
                     //don't check for other tokens
                     //if((strcmp(ap_token,"*MESH_FACE")!=0)&&(strcmp(ap_token,"*mesh_face")!=0)&&(strcmp(ap_token,"}")!=0))
                     //   parse_error("*MESH_FACE token expected only inside MESH_FACE_LIST");
                     if((strcmp(ap_token,"*MESH_FACE")==0)||(strcmp(ap_token,"*mesh_face")==0))
                        {
                        int face,a,b,c;
                        //get face number
                        if(ap_get_token()) end=1;
                        //printf("inside list token=%s\n",ap_token);
                        if(ap_token_type==ASEPARSE_TOKEN_IS_NUMBER)  //face id and : must be separate tokens
                           {
                           sscanf(ap_token,"%d",&face);
                           if(ap_get_token()) end=1;
                           if(strcmp(ap_token,":")!=0)
                              {sprintf(message,": expected after face number. token=<%s>",ap_token);parse_error(message);return -1;}
                           }
                        else
                           {
                           if(ap_token[strlen(ap_token)-1]!=':')
                              {sprintf(message,": expected after face number. ap_token=<%s>",ap_token);parse_error(message);return -1;}
                           ap_token[strlen(ap_token)-1]=0; sscanf(ap_token,"%d",&face);
                           }
                        //get a: face
                        if(ap_get_token()) end=1;
                        if((strcmp(ap_token,"a:")!=0)&&(strcmp(ap_token,"A:")!=0))
                           {parse_error("a: expected after face number");return -1;}
                        if(ap_get_token()) end=1;
                        sscanf(ap_token,"%d",&a);
                        //get b: face
                        if(ap_get_token()) end=1;
                        if((strcmp(ap_token,"b:")!=0)&&(strcmp(ap_token,"B:")!=0))
                           {parse_error("b: expected after face number");return -1;}
                        if(ap_get_token()) end=1;
                        sscanf(ap_token,"%d",&b);
                        //get c: face
                        if(ap_get_token()) end=1;
                        if((strcmp(ap_token,"c:")!=0)&&(strcmp(ap_token,"C:")!=0))
                           {parse_error("c: expected after face number");return -1;}
                        if(ap_get_token()) end=1;
                        sscanf(ap_token,"%d",&c);
                        //
                        if((a>=65536)||(b>=65536)||(c>=65536))
                           parse_error("3D vertex pointer out of range, limited to 0-65535");
                        if((face>=0)&&(face<mesh_numfaces)&&(a<65536)&&(b<65536)&&(c<65536))
                           {
                           //--- Add 3 vertex pointers ---
                           meshman_face[meshman_free_face_ptr].vertex3d_num=(unsigned short int)a;
                           meshman_free_face_ptr++;
                           meshman_face[meshman_free_face_ptr].vertex3d_num=(unsigned short int)b;
                           meshman_free_face_ptr++;
                           meshman_face[meshman_free_face_ptr].vertex3d_num=(unsigned short int)c;
                           meshman_free_face_ptr++;
                           meshman_mesh[meshman_free_mesh_ptr].num_faces++;
                           num_faces_loaded++;
                           num_faces_this_mesh++;
                           }
                        else {
                             sprintf(message,"*MESH_FACE id=%d not in range 0-%d",face,mesh_numfaces);
                             parse_error(message);
                             }
                        //printf("loaded num_faces_this_mesh=%d\n",num_faces_this_mesh);
                        }//if((strcmp(ap_token,"*MESH_FACE")==0)||(strcmp(ap_token,"*mesh_face")==0))
                     else if(strcmp(ap_token,"}")==0) facelist_end=1;
                     //if(facelist_end) printf("facelist_end\n");
                     }//while ((end==0)&&(facelist_end==0))
                  }
               else if((strcmp(ap_token,"*MESH_TFACELIST")==0)||(strcmp(ap_token,"*mesh_tfacelist")==0))
                  {
                  //------------------------------ TRIANGLES - TEXTURE COORDS ----------------------------------
                  char texfacelist_end=0;
                  //get {
                  if(ap_get_token()) end=1;
                  if(strcmp(ap_token,"{")!=0)
                     parse_error("{ expected after *MESH_TFACELIST");
                  //in *face_list list mode, expect tokens *MESH_TFACE only
                  while ((end==0)&&(texfacelist_end==0))
                     {
                     if(ap_get_token()) end=1;
                     //printf("texface list token=%s\n",ap_token);
                     if((strcmp(ap_token,"*MESH_TFACE")!=0)&&(strcmp(ap_token,"*mesh_tface")!=0)&&(strcmp(ap_token,"}")!=0))
                        parse_error("*MESH_TFACE token expected only inside MESH_TFACELIST");
                     if((strcmp(ap_token,"*MESH_TFACE")==0)||(strcmp(ap_token,"*mesh_tface")==0))
                        {
                        int tface,ta,tb,tc;
                        //get face number
                        if(ap_get_token()) end=1;
                        //printf("inside list token=%s\n",ap_token);
                        if(ap_token_type!=ASEPARSE_TOKEN_IS_NUMBER)
                              {sprintf(message,"number expected after *MESH_TFACE, token=<%s>",ap_token);parse_error(message);return -1;}
                        sscanf(ap_token,"%d",&tface);
                        //get a: face
                        if(ap_get_token()) end=1;
                        sscanf(ap_token,"%d",&ta);
                        //get b: face
                        if(ap_get_token()) end=1;
                        sscanf(ap_token,"%d",&tb);
                        //get c: face
                        if(ap_get_token()) end=1;
                        sscanf(ap_token,"%d",&tc);
                        //
                         if((ta>=65536)||(tb>=65536)||(tc>=65536))
                           parse_error("texture vertex pointer out of range, limited to 0-65535");
                        if((ta<65536)&&(tb<65536)&&(tc<65536))
                           {
                           //--- Add 3 vertex pointers ---
                           meshman_facetexture[meshman_free_facetexture_ptr].texvertex_num=(unsigned short int)ta;
                           meshman_free_facetexture_ptr++;
                           meshman_facetexture[meshman_free_facetexture_ptr].texvertex_num=(unsigned short int)tb;
                           meshman_free_facetexture_ptr++;
                           meshman_facetexture[meshman_free_facetexture_ptr].texvertex_num=(unsigned short int)tc;
                           meshman_free_facetexture_ptr++;
                           num_texfaces_loaded++;
                           num_texfaces_this_mesh++;
                           }
                       else {
                            sprintf(message,"*MESH_TFACE id=%d not in range 0-%d",tface,mesh_numtexfaces);
                            parse_error(message);
                            }
                        }
                     else if(strcmp(ap_token,"}")==0) texfacelist_end=1;
                     }
                  }
               else if(strcmp(ap_token,"}")==0) mesh_end=1;
               }
            if(end==1) parse_error("file ended inside *MESH  - *MESH never ended with }");
            //printf("mesh_num_vertices=%d   mesh_numfaces=%d\n",mesh_num_vertices,mesh_numfaces);
            //printf("Made it to end of *MESH\n");
            //--- mesh data ---
            meshman_mesh[meshman_free_mesh_ptr].wireframe_on=temp_wireframe_mode;
            num_meshes_loaded++;
            //increment meshman_free_mesh_ptr at end of geomobject assuming only one mesh/geomobject
            }
         else if(strcmp(ap_token,"}")==0) geomobject_end=1;
         }//while ((end==0)&&(geomobject_end==0))
      //------------------ Loaded a GEOMOBJECT ----------------
      if(ASE_PARSE_DEBUG_ON) printf("Made it to end of *GEOMOBJECT\n");
      //check various vertex counts add up
      if(num_3dvtx_this_mesh!=mesh_num_vertices)
         {
         sprintf(message,"%d vertices expected, whereas %d were listed",mesh_num_vertices,num_3dvtx_this_mesh);
         parse_error(message);
         }
         //check texture info if in texture mode
      if(temp_wireframe_mode==0)
         {
         if((num_texvtx_this_mesh!=mesh_numtexvertices)&&(mesh_numtexvertices!=-1))
            {
            sprintf(message,"%d texture vertices expected, whereas %d were listed",mesh_numtexvertices,num_texvtx_this_mesh);
            parse_error(message);
            }
         if((num_texfaces_this_mesh!=mesh_numtexfaces)&&(mesh_numtexfaces!=-1))
            {
            sprintf(message,"%d textured faces expected, whereas %d were listed",mesh_numtexfaces,num_texfaces_this_mesh);
            parse_error(message);
            }
         }
      //check various face counts add up
      if(num_faces_this_mesh!=mesh_numfaces)
         {
         sprintf(message,"%d faces expected, whereas %d were listed",mesh_numfaces,num_faces_this_mesh);
         parse_error(message);
         }
      if(ASE_PARSE_DEBUG_ON) 
	     {
		 printf("%d VERTICES\n",num_3dvtx_this_mesh);
//         for(i=0;i<num_3dvtx_this_mesh;i++)
//            printf("%d: %f,%f,%f \n",temp_vtx_id[i],temp_vtx_x[i],temp_vtx_y[i],temp_vtx_z[i]);
         if(temp_wireframe_mode==0)
            {
            printf("%d TEXTURE VERTICES\n",num_texvtx_this_mesh);
//           for(i=0;i<num_texvtx_this_mesh;i++)
//               printf("%d: %f,%f\n",temp_vtx_tex_id[i],temp_vtx_tex_x[i],temp_vtx_tex_y[i]);
            }
         printf("%d FACES\n",num_faces_this_mesh);
//         for(i=0;i<num_faces_this_mesh;i++)
//            printf("%d: %d,%d,%d\n",temp_face_id[i],temp_face_a[i],temp_face_b[i],temp_face_c[i]);
         if(temp_wireframe_mode==0)
            {
            printf("%d TEXTURE FACES\n",num_texfaces_this_mesh);
//            for(i=0;i<num_texfaces_this_mesh;i++)
//               printf("%d: %d,%d,%d\n",temp_texface_id[i],temp_texface_a[i],temp_texface_b[i],temp_texface_c[i]);
            }
         }
      //-------------------- all data is loaded - now put in mesh structure --------------------
      if(temp_wireframe_mode)
         {
         //--- geomobject has a wireframe colour applied ---
         if(ASE_PARSE_DEBUG_ON) 
		    printf("Wireframe mode RED=%f  GREEN=%f  BLUE=%f\n",temp_wireframe_red,temp_wireframe_green,temp_wireframe_blue);
		 num_wireframe_meshes_loaded++;
         }
      if(material_ref!=-1)
         {
         int temp_meshman_mat_num;
         if(ASE_PARSE_DEBUG_ON) printf("Material %d applied - ",material_ref);
         temp_meshman_mat_num=material_lookup[material_ref];
         meshman_mesh[meshman_free_mesh_ptr].material=temp_meshman_mat_num;
         if((meshman_material[temp_meshman_mat_num].bitmap_on)&&(ASE_PARSE_DEBUG_ON))
            {
            //geomobject has a texure image applied
            printf("  bitmap %d = <%s>\n",meshman_material[temp_meshman_mat_num].bitmap_num,
                   meshman_bitmap[meshman_material[temp_meshman_mat_num].bitmap_num].filename);
            }
         else if(ASE_PARSE_DEBUG_ON) 
            {
            //geomobject has a single colour
            printf("not a bitmap\n");
            }
         }
      else
         {
         if(ASE_PARSE_DEBUG_ON)
            parse_warning("geomobject has no material or wireframe colour attached: default white material applied\n");
         num_meshes_without_materials++;
         //assign default white colour to mesh
         meshman_mesh[meshman_free_mesh_ptr].material=meshman_default_material;
         }
      meshman_free_mesh_ptr++;
      num_geomobjects_loaded++;
      //fclose(ase_parse_log_file); exit(1);
      }//if(strcmp(ap_token,"*GEOMOBJECT")==0)
   }
fclose(ASE_PARSE_IN);

/*
if(base_loaded==0)
   {sprintf(message,"ERROR:need to have a 'base' section in setup file <%s>\n",ase_filename);
    parse_error(message);}
if(pointer_loaded==0)
   {sprintf(message,"ERROR:need to have a 'pointer' section in setup file <%s>\n",ase_filename);
    parse_error(message);}
*/

if(ASE_PARSE_DISPLAY_STATUS_ON)
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
      {sprintf(message,"WARNING <%s> has a differing number of geomobjects and meshes\n",ase_filename);
       parse_error(message);}
   printf("%d 3D and %d texture vertices loaded. %d normal vectors loaded. %d faces and %d texture faces loaded\n",
          num_3dvertices_loaded,num_texvertices_loaded,num_normalvectors_loaded,num_faces_loaded,num_texfaces_loaded);
   }


if(num_meshes_without_materials>0)
   {
   char message[512];
   sprintf(message,"%WARNING: %d geomobjects (meshes) had no material: default white material applied\n",
           num_meshes_without_materials);
   parse_warning(message);
   }


//increment for next object
meshman_free_object_model_ptr++;

*num_meshes=num_meshes_loaded;
*num_vtxs=num_3dvertices_loaded;
*num_triangles=num_faces_loaded;
*num_normals=num_normalvectors_loaded;
*num_bitmaps=num_bitmaps_present;
if(material_lookup!=NULL) free(material_lookup);  //clean up memory used
return object_num;
}















//The following functions return 0 for ok, 1 for problem
// get_token()           returns 1 if EOF
// expect_token(char*)   retrns 1 if next token is not the expected one
// get_string_token()        returns 1 if not a proper filename with "file" or "file.pgm" format

char ap_get_char(void)
{
char it;

it=fgetc(ASE_PARSE_IN);
if(it==0x0a) ap_parse_line_number++;
return it;
}



int ap_get_type(char cc)
{
if((cc>='0')&&(cc<='9')||(cc=='-')) return ASEPARSE_CHAR_IS_NUMBER;

if(((cc>='a')&&(cc<='z'))
 ||((cc>='A')&&(cc<='Z'))
 ||(cc=='_')||(cc=='/')||(cc=='\\')||(cc==':')||(cc=='#')||(cc=='$')||(cc=='^')||(cc=='*')
 ||(cc=='+')
 ||(cc=='%')||(cc=='&')||(cc=='~')
 ||(cc=='<')||(cc=='>')||(cc=='!')
 ||(cc==0x27)) return ASEPARSE_CHAR_IS_LETTER;

if((cc==' ')||(cc==0x0a)||(cc==0x0d)||(cc==0x09)) return ASEPARSE_CHAR_IS_WHITESPACE;

if(cc==',') return ASEPARSE_CHAR_IS_COMMA;

if(cc=='.') return ASEPARSE_CHAR_IS_PERIOD;

if(cc=='"') return ASEPARSE_CHAR_IS_QUOTES;

if((cc=='{')||(cc=='}')) return ASEPARSE_CHAR_IS_SQUIGGLY_BRACKET;

return ASEPARSE_CHAR_IS_UNKNOWN;
}




// ap_get_token()           returns 1 if EOF
int ap_get_token(void)
{
int done,w,type;
int num_quotes;
char clast;

// Set default token type
ap_token_type=ASEPARSE_TOKEN_IS_UNKNOWN;

//first skip over whitespace 
whitespace_jump:
done=0;
while(done==0)
   {
   if(ap_char==EOF) return 1;
   if((ap_char!=' ')&&(ap_char!=0x0a)&&(ap_char!=0x0d)&&(ap_char!=0x09)) done=1;
   else ap_char=ap_get_char();
   }

w=0;
//skip over comments 
if(ap_char=='/')
   {
   ap_token[0]='/';  w=1;
   ap_char=ap_get_char();
   if(ap_char=='*')
      {
      done=0; clast=0;
      while(done==0)
         {
         ap_char=ap_get_char(); if(ap_char==EOF) return 1;
         if((ap_char=='/')&&(clast=='*')) done=1;
         clast=ap_char;
         }
      ap_char=ap_get_char(); if(ap_char==EOF) return 1;
      goto whitespace_jump;
      }
   else if(ap_char=='/')
      {
      while(ap_char!=0x0a)
         {
         ap_char=ap_get_char(); if(ap_char==EOF) return 1;
         }
      goto whitespace_jump;
      }
   }
//skip over # comments
if(ap_char=='#')
   {
   while(ap_char!=0x0a)
      {
      ap_char=ap_get_char(); if(ap_char==EOF) return 1;
      }
   goto whitespace_jump;
   }


ap_token[w++]=ap_char;
type=ap_get_type(ap_char);
switch(type)
   {
   case ASEPARSE_CHAR_IS_LETTER:
   while((type==ASEPARSE_CHAR_IS_LETTER)||(type==ASEPARSE_CHAR_IS_NUMBER))	
      {
      ap_char=ap_get_char(); if(ap_char==EOF) {ap_token[w]=0; return 1;}
      type=ap_get_type(ap_char);
      if((type==ASEPARSE_CHAR_IS_LETTER)||(type==ASEPARSE_CHAR_IS_NUMBER)) ap_token[w++]=ap_char;
      ap_token_type=ASEPARSE_TOKEN_IS_NAME;
      }
   break;

   case ASEPARSE_CHAR_IS_NUMBER:
   ap_token_type=ASEPARSE_TOKEN_IS_NUMBER;
   while((type==ASEPARSE_CHAR_IS_NUMBER)||(type==ASEPARSE_CHAR_IS_PERIOD))	
      {
      ap_char=ap_get_char(); if(ap_char==EOF) {ap_token[w]=0; return 1;}
      type=ap_get_type(ap_char);
      if((type==ASEPARSE_CHAR_IS_NUMBER)||(type==ASEPARSE_CHAR_IS_PERIOD)) ap_token[w++]=ap_char;
      }
   break;

   case ASEPARSE_CHAR_IS_SQUIGGLY_BRACKET:
   ap_char=ap_get_char(); if(ap_char==EOF) {ap_token[w]=0; return 1;}
   break;

   case ASEPARSE_CHAR_IS_COMMA:
   ap_char=ap_get_char(); if(ap_char==EOF) {ap_token[w]=0; return 1;}
   break;

   case ASEPARSE_CHAR_IS_PERIOD:
   ap_char=ap_get_char(); if(ap_char==EOF) {ap_token[w]=0; return 1;}
   break;

   case ASEPARSE_CHAR_IS_QUOTES:		//used for filenames
   ap_token_type=ASEPARSE_TOKEN_IS_STRING;
   num_quotes=1;
   while(num_quotes<2) 
      {
      ap_char=ap_get_char(); if(ap_char==EOF) {ap_token[w]=0; return 1;}
      type=ap_get_type(ap_char);
      if(w<256)
         ap_token[w++]=ap_char;
      if(type==ASEPARSE_CHAR_IS_QUOTES) num_quotes++;
      }
   ap_token[w]=0; //printf("string ap_token<%s>\n",ap_token);
   ap_char=ap_get_char(); if(ap_char==EOF) {ap_token[w]=0; return 1;}
   break;

   default: 
      if(ASE_PARSE_DISPLAY_STATUS_ON) printf("unknown character %ap_char = 0x%x\n",ap_char,ap_char);
      ap_char=ap_get_char();
   break;
   }
ap_token[w]=0;
return 0;
}



int ap_expect_token(char *expected)
{
if(ap_get_token()) return 1;
if(strcmp(ap_token,expected)!=0) return 1;
else return 0;
}


int ap_get_string_token(char *string_token)
{
int i,j;

if(ap_get_token()) return 1;
if(ap_token_type!=ASEPARSE_TOKEN_IS_STRING) {printf("---\n");return 1;}
i=1;j=0;
while((ap_token[i]!='"')&&(ap_token[i]!=0))
   string_token[j++]=ap_token[i++];
string_token[j]=0;
return 0;
}




#endif //#ifndef _ASE_PARSE_C_
//-----------------------------------------------------------------------------------------------------------------------------------------------


