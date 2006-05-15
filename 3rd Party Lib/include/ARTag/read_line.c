//© 2006, National Research Council Canada

int read_line(char*);
int break_line(char*,char*,char*,char*,char*,char*,char*,char*,char*,char*);
int break_line_first_element(char*,char*);

#define _READ_LINE_C_


int line_number;
FILE *READ_LINE_IN;
char line[1024],el1[64],el2[64],el3[64],el4[64],el5[64],el6[64],el7[64],el8[64],el9[64];

int read_line(char *line)
{
int in_char,i,j;
char only_whitespace;

start_read_line:
i=0;
do {
   in_char=fgetc(READ_LINE_IN);
   if((in_char==0x0a)||(in_char==0x0d)) line_number++;
   if((in_char!=0x0a)&&(in_char!=0x0d)&&(in_char!=-1)) *(line+i++)=in_char;
   } while((in_char!=0x0a)&&(in_char!=0x0d)&&(in_char!=-1));
*(line+i)=0;
//printf("line#%d line{%s} i=%d\n",line_number,line,i);

//reject line if blank
if((in_char!=-1)&&(i==0)) goto start_read_line;

//reject line if comment
if((in_char!=-1)&&(*line=='#'))
   goto start_read_line;
if((in_char!=-1)&&(*line=='/')&&(*(line+1)=='/'))
   goto start_read_line;

//reject line if all whitespace
if(i>0)
   {
   only_whitespace=0;
   for(j=0;j<i;j++) if((line[j]!=' ')&&(line[j]!=0x09)) only_whitespace=1;
   if(only_whitespace==0) goto start_read_line;
   }

//for(j=0;j<=i;j++) printf("%x ",*(line+j));
//printf("\n");

if((in_char==-1)&&(i==0)) return 1;
else	                  return 0;
}


int break_line(char *line,
               char *element_1, char *element_2, char *element_3,
               char *element_4, char *element_5, char *element_6,
               char *element_7, char *element_8, char *element_9)
{
int i,j;
i=0;

//get element_1;
//skip over whitespace
while((*(line+i)==' ')||(*(line+i)==0x09))  i++;
//copy text to element_1
j=0;
while((*(line+i)!=' ')&&(*(line+i)!=0x09)&&(*(line+i)!=0))
   *(element_1+j++)=*(line+i++);
*(element_1+j)=0;
if(*(line+i)==0) return 1;

//get element_2;
//skip over whitespace
while((*(line+i)==' ')||(*(line+i)==0x09))  i++;
if(*(line+i)==0) return 1;
//copy text to element_2
j=0;
while((*(line+i)!=' ')&&(*(line+i)!=0x09)&&(*(line+i)!=0))
   *(element_2+j++)=*(line+i++);
*(element_2+j)=0;
if(*(line+i)==0) return 2;

//get element_3;
//skip over whitespace
while((*(line+i)==' ')||(*(line+i)==0x09))  i++;
if(*(line+i)==0) return 2;
//copy text to element_3
j=0;
while((*(line+i)!=' ')&&(*(line+i)!=0x09)&&(*(line+i)!=0))
   *(element_3+j++)=*(line+i++);
*(element_3+j)=0;
if(*(line+i)==0) return 3;

//get element_4;
//skip over whitespace
while((*(line+i)==' ')||(*(line+i)==0x09))  i++;
if(*(line+i)==0) return 3;
//copy text to element_4
j=0;
while((*(line+i)!=' ')&&(*(line+i)!=0x09)&&(*(line+i)!=0))
   *(element_4+j++)=*(line+i++);
*(element_4+j)=0;
if(*(line+i)==0) return 4;

//get element_5;
//skip over whitespace
while((*(line+i)==' ')||(*(line+i)==0x09))  i++;
if(*(line+i)==0) return 4;
//copy text to element_5
j=0;
while((*(line+i)!=' ')&&(*(line+i)!=0x09)&&(*(line+i)!=0))
   *(element_5+j++)=*(line+i++);
*(element_5+j)=0;
if(*(line+i)==0) return 5;

//get element_6;
//skip over whitespace
while((*(line+i)==' ')||(*(line+i)==0x09))  i++;
if(*(line+i)==0) return 5;
//copy text to element_6
j=0;
while((*(line+i)!=' ')&&(*(line+i)!=0x09)&&(*(line+i)!=0))
   *(element_6+j++)=*(line+i++);
*(element_6+j)=0;
if(*(line+i)==0) return 6;

//get element_7;
//skip over whitespace
while((*(line+i)==' ')||(*(line+i)==0x09))  i++;
if(*(line+i)==0) return 6;
//copy text to element_7
j=0;
while((*(line+i)!=' ')&&(*(line+i)!=0x09)&&(*(line+i)!=0))
   *(element_7+j++)=*(line+i++);
*(element_7+j)=0;
if(*(line+i)==0) return 7;


//get element_8;
//skip over whitespace
while((*(line+i)==' ')||(*(line+i)==0x09))  i++;
if(*(line+i)==0) return 7;
//copy text to element_8
j=0;
while((*(line+i)!=' ')&&(*(line+i)!=0x09)&&(*(line+i)!=0))
   *(element_8+j++)=*(line+i++);
*(element_8+j)=0;
if(*(line+i)==0) return 8;


//get element_9;
//skip over whitespace
while((*(line+i)==' ')||(*(line+i)==0x09))  i++;
if(*(line+i)==0) return 8;
//copy rest of text to element_9
j=0;
while(*(line+i)!=0)
   *(element_9+j++)=*(line+i++);
*(element_9+j)=0;
if(*(line+i)==0) return 9;

//printf("more than 9 elements in line %s\n",line);
return 10;
}


//finds first element, and over-writes line with that element missing
//returns 1 if it found an element, 0 if it didn't (i.e. hit end)
int break_line_first_element(char *line, char *first_element)
{
int i,j;
char c;
i=0;
//get element_1;
//skip over whitespace
while((*(line+i)==' ')||(*(line+i)==0x09))  i++;
//check for end of line
if(*(line+i)==0) return 0;
//copy text to first_element
j=0;
while((*(line+i)!=' ')&&(*(line+i)!=0x09)&&(*(line+i)!=0))
   *(first_element+j++)=*(line+i++);
*(first_element+j)=0;
//move over line in memory
j=0;
do {
   c=line[i++];
   line[j++]=c;
   } while(c!=0);
return 1;
}


//finds first element, and over-writes line with that element missing
//returns 1 if it found an element, 0 if it didn't (i.e. hit end)
int break_line_first_char(char *line, char *cout)
{
int i,j;
char c;
i=0;
//get element_1;
//skip over whitespace
while((*(line+i)==' ')||(*(line+i)==0x09))  i++;
//check for end of line
if(*(line+i)==0) return 0;
//copy text to first_element
*cout=line[i]; i++;
//move over line in memory
j=0;
do {
   c=line[i++];
   line[j++]=c;
   } while(c!=0);
return 1;
}



