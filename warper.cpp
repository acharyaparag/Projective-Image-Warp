/*
 Parag Acharya  pachary@clemson.edu 11/04/2013
 
 Program to perform Projective Warping
*/
#include<iostream>
#include <cstdlib>
#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdlib.h>
#include<stdio.h>
#include<string.h>
#include <OpenImageIO/imageio.h>
#include<string>
#include<math.h>
#include"Matrix.h"


#define MIN(a, b) ((a < b) ? a : b)
#define MAX(a, b) ((a > b) ? a : b)

OIIO_NAMESPACE_USING

using namespace std;

struct pixel
{
	 float r,g,b;
};
  
Matrix3x3 M(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0);

pixel *imgData = NULL , **finData = NULL,**warpImage = NULL;
string writeFile;
int xres = 0,yres = 0,image_size = 0,channels = 0,winWidth = 0,winHeight = 0,writeFlag = 0,newxPos = 0,newyPos = 0,newxres = 0,newyres = 0;
float Left = 0,Right = 0,Bottom = 0,Top = 0;  

void warping();  
void forwardMap();         

float minimum(float a,float b,float c,float d)
{
  float min1 = 0,min2 = 0,finmin = 0;
  min1 = MIN(a,b);
  min2 = MIN(c,d);
  finmin = MIN(min1,min2);
  return finmin;   
}

float maximum(float a,float b,float c,float d)
{
  float max1 = 0,max2 = 0,finmax = 0;
  max1 = MAX(a,b);
  max2 = MAX(c,d);
  finmax = MAX(max1,max2);
  return finmax;   
}

//Function to read an Image file.Takes Filename to read as Input
void loadImage(char * filename)  
{

  ImageInput *in = ImageInput::open(filename);
  if (!in)
     {
      std::cerr<<"Could not open "<<filename<<", error = "<<geterror()<<endl;
      exit(0);
     }
 
  // Get Image spec
  const ImageSpec &spec = in->spec();     
  xres = spec.width;
  yres = spec.height;
  channels = spec.nchannels;
  imgData= new pixel[xres*yres];
  
    if(!in->read_image(TypeDesc::FLOAT,&imgData[0]))
   {
     std::cerr<<"Could not read pixels from "<<filename<<", error="<<in->geterror()<<endl;
     delete in;
     exit(0);
   }

  //Display Image Info
  cout<<"Image Info:"<<endl;
  cout<<"Dimensions:"<<xres<<"*"<<yres<<endl;
  cout<<"Number of Channels:"<<channels<<endl;
  if (channels == 4)
  cout<<"Name of channels:"<<spec.channelnames[0]<<","<<spec.channelnames[1]<<","<<spec.channelnames[2]<<","<<spec.channelnames[3]<<endl;
  else
  cout<<"Name of channels:"<<spec.channelnames[0]<<","<<spec.channelnames[1]<<","<<spec.channelnames[2]<<endl;
  const size_t bpp = spec.pixel_bytes();
  cout<<"Color Depth:"<<bpp<<"(bytes per pixel)"<<","<<bpp*8<<"(bits per pixel)"<<endl;

  finData = new pixel*[yres]; 
  finData[0] = new pixel[xres*yres]; 
  
  int image_size = xres*yres ;
  image_size--;
  for(int i = 1; i < yres; i++)
     {
       finData[i] = finData[i - 1] + xres;	
     }

  for(int row = 0; row < yres; row++)
     {
        for(int col = xres-1; col >=0; col--)
	     {
	      finData[row][col].r = imgData[image_size].r;
	      finData[row][col].g = imgData[image_size].g;
	      finData[row][col].b = imgData[image_size].b; 
	      image_size--;
	     }
      } 

if (!in->close())
  {
     std::cerr<<"Error closing "<<filename<<", error="<<in->geterror()<<endl;
     delete in;
     exit(0);
   }

  delete in;
 } 


//Function to write an Image file.Takes filename to write as Input
void writeImage(string writeFile) 
{ 
  pixel * writeImage = new pixel[newxres*newyres];
  image_size = newxres*newyres;
  image_size--;
  
   for(int row = 0; row < newyres; row++)
     {
        for(int col = newxres-1; col >=0; col--)
	     {
	      writeImage[image_size].r = warpImage[row][col].r ;
	      writeImage[image_size].g = warpImage[row][col].g ;
	      writeImage[image_size].b = warpImage[row][col].b ; 
	      image_size--;
	     }
      }   
   

  ImageOutput *out = ImageOutput::create(writeFile);
  if(!out){
           std::cerr<<"Could not create:"<<writeFile<<","<<geterror()<<endl;
           exit(0);
          }

  ImageSpec spec(newxres,newyres,channels,TypeDesc::FLOAT);
  if (! out->open (writeFile, spec)) 
     {
	std::cerr << "Could not open " << writeFile << ", error = " << out->geterror() << endl;
        delete out;
        exit(0);
     }

if(!out->write_image(TypeDesc::FLOAT,writeImage))
     {
        std::cerr << "Could not write pixels to " << writeFile << ", error = " << out->geterror() << endl;
        delete out;
        exit(0);
     }
 
  out->close();
  delete out;
  delete[] writeImage;
	
}

void displayImage()  //Function to display image
 
 {
  
  glClearColor(0, 0, 0, 0);
  glClear(GL_COLOR_BUFFER_BIT);  // clear window to background color
  glRasterPos2i(0,0);
  if (writeFlag)
  {
   glDrawPixels(newxres,newyres,GL_RGB,GL_FLOAT,*warpImage);   // Display the warped Image
  } 
  else
  {
   glDrawPixels(xres,yres,GL_RGB,GL_FLOAT,*finData);  // Display the original Image
  }
  glFlush();
}


/*
  Keyboard Callback Routine:
  This routine is called every time a key is pressed on the keyboard
*/
void handleKey(unsigned char key, int x, int y){
  
  switch(key){
    case 'q':		// 'quit the program'
    case 'Q':
    case 27:
        {
          exit(-1);
        }
      
    default:		// not a valid key -- just ignore it
      return;
  }
}


//Function to Perform warping
void warping()
{
  Vector3d origin(Left,Bottom,0); 
  Matrix3x3 invM = M.inv();
  
  warpImage = new pixel*[newyres]; 
  warpImage[0] = new pixel[newxres*newyres];  
 
  for(int i = 1; i < newyres; i++)
     {
       warpImage[i] = warpImage[i - 1] + newxres;	
     }

   for(int row = 0; row < newyres; row++)
     {
        for(int col = 0; col < newxres; col++)
	     {
              Vector3d pixel_out(col,row,1);
              pixel_out = pixel_out + origin;
              Vector3d pixel_in = invM*pixel_out;

              float u = pixel_in[0]/pixel_in[2];
              float v = pixel_in[1]/pixel_in[2];
 
	      if(u >= xres || u < 0 || v >= yres || v < 0)  //u and v should be within the bounding box of the input image
                 {
		   warpImage[row][col].r = 0;
		   warpImage[row][col].g = 0;
		   warpImage[row][col].b = 0;
                 }
            else
                 {
                   warpImage[row][col].r = finData[int(v)][int(u)].r;   
                   warpImage[row][col].g = finData[int(v)][int(u)].g;  
                   warpImage[row][col].b = finData[int(v)][int(u)].b;   
                 }                          
	     }
      }  
}

/*
   Convert the string s to lower case
*/
void lowercase(char *s){
   int i;

   if(s != NULL) {
      for(i = 0; s[i] != '\0'; i++) {
         if(s[i] >= 'A' && s[i] <= 'Z')
            s[i] += ('a' - 'A');
      }
   }
}


/* 
   Multiply M by a rotation matrix of angle theta
*/

void Rotate(float theta){
   Matrix3x3 R(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0);
   double rad, c, s;

   rad = PI * theta / 180.0;
   c = cos(rad);
   s = sin(rad);

   R[0][0] = c;
   R[0][1] = -s;
   R[1][0] = s;
   R[1][1] = c;

   Matrix3x3 Prod = R * M;

   for(int row = 0; row < 3; row++) {
      for(int col = 0; col < 3; col++) {
         M[row][col] = Prod[row][col];
      }
   }

}

/* 
   Multiply M by a scale matrix of with scaling in X direction : x and scaling in Y direction :y
*/

void Scale(float x,float y){
   Matrix3x3 S(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0);

   S[0][0] = x;
   S[1][1] = y;
	if(x == 0)
	{
	 S[0][0] = 1;
	 cout<<"Invalid input,divide by zero error"<<endl;
	 cout<<"Changing to scale by 1 in x direction"<<endl;
	}
	if(y == 0)
	{
	 S[1][1] = 1;
	 cout<<"Invalid input, divide by zero error"<<endl;
	 cout<<"Changing to scale by 1 in y direction"<<endl;
	}

   Matrix3x3 Prod = S * M;

   for(int row = 0; row < 3; row++) {
      for(int col = 0; col < 3; col++) {
         M[row][col] = Prod[row][col];
      }
   }
}

/* 
   Multiply M by a Translate matrix of with translate in X direction : x and translate in Y direction :y
*/

void Translate(float x,float y){
   Matrix3x3 T(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0);

   T[0][2] = x;
   T[1][2] = y;

   Matrix3x3 Prod = T * M;

   for(int row = 0; row < 3; row++) {
      for(int col = 0; col < 3; col++) {
         M[row][col] = Prod[row][col];
      }
   }
}

/* 
   Multiply M by a Shear matrix of with shear in X direction : x and shear in Y direction :y
*/

void Shear(float x,float y){
   Matrix3x3 Sh(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0);

   Sh[0][1] = x;
   Sh[1][0] = y;

   Matrix3x3 Prod = Sh * M;

   for(int row = 0; row < 3; row++) {
      for(int col = 0; col < 3; col++) {
         M[row][col] = Prod[row][col];
      }
   }

}

/* 
   Multiply M by a Perspective Matrix
*/

void Perspective(float x,float y){
   Matrix3x3 P(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0);

   P[2][0] = x;
   P[2][1] = y;

   Matrix3x3 Prod = P * M;

   for(int row = 0; row < 3; row++) {
      for(int col = 0; col < 3; col++) {
         M[row][col] = Prod[row][col];
      }
   }
}

/* 
   Multiply M by a Flip matrix
   x = 1 flip x coordinates
   y = 1 flip y coordinates
*/
void Flip(int x,int y){
   Matrix3x3 F(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0);

   if ( x== 1)
      {
       F[0][0] = -1;
      }
   if( y == 1)
      {
       F[1][1] =  -1;
      } 
   if ( x == 1 and y == 1)
      {
       F[0][0] = -1;
       F[1][1] = -1;
      }  

   Matrix3x3 Prod = F * M;

   for(int row = 0; row < 3; row++) {
      for(int col = 0; col < 3; col++) {
         M[row][col] = Prod[row][col];
      }
   }
}
/*
   Routine to build a projective transform from input text, display, or
   write transformed image to a file
*/
void process_input(){
   char command[1024];
   bool done;
   float theta;
   float x,y;
   int a,b;


   /* build identity matrix */
   M.identity();

   for(done = false; !done;) {

      /* prompt and accept input, converting text to lower case */
      printf("> ");
      scanf("%s", command);
      lowercase(command);

      /* parse the input command, and read parameters as needed */
      if(strcmp(command, "d") == 0) {
         done = true;
         forwardMap();
         warping();
         writeFlag = 1;
      } else if(strlen(command) != 1) {
         printf("invalid command, enter r, s, t, f, h, p, d\n");
      } else {
         switch(command[0]) {

            case 'r':		/* Rotation, accept angle in degrees */
               if(scanf("%f", &theta) == 1)
                  Rotate(theta);
               else
                  fprintf(stderr, "invalid rotation angle\n");
               break;
            case 's':		/* Scale, accept scale factors */
                      if(scanf("%f", &x) == 1)
			{
				if(scanf("%f", &y) == 1)
				{
					Scale(x, y);

				}
				else	
				fprintf(stderr, "INVALID Scale in Y direction\n");
			}
		       else	
		       fprintf(stderr, "INVALID Scale in X direction\n");
               break;
            case 't':		/* Translation, accept translations */
                      if(scanf("%f", &x) == 1)
			{
				if(scanf("%f", &y) == 1)
				{
					Translate(x, y);

				}
				else	
				fprintf(stderr, "INVALID Translate in Y direction\n");
			}
		       else	
		       fprintf(stderr, "INVALID Translate in X direction\n");
               break;
            case 'f':		/* Mirror, accept 0/1, 0/1 for x mirror, ymirror */
                      if(scanf("%d", &a) == 1)
			{
				if(scanf("%d", &b) == 1)
				{
					Flip(a, b);

				}
				else	
				fprintf(stderr, "INVALID Flip Y coordinate value\n");
			}
		       else	
		       fprintf(stderr, "INVALID Flip X coordinate value\n");
               break;
            case 'h':		/* Shear, accept shear factors */
                      if(scanf("%f", &x) == 1)
			{
				if(scanf("%f", &y) == 1)
				{
					Shear(x, y);

				}
				else	
				fprintf(stderr, "INVALID y Shear Term\n");
			}
		       else	
		       fprintf(stderr, "INVALID X Shear Term\n");
               break;
            case 'p':		/* Perspective, accept perspective factors */
                      if(scanf("%f", &x) == 1)
			{
				if(scanf("%f", &y) == 1)
				{
					Perspective(x, y);

				}
				else	
				fprintf(stderr, "INVALID y Perspective term\n");
			}
		       else	
		       fprintf(stderr, "INVALID X Perspective term\n");
               break;
            case 'd':		/* Done, that's all for now */
               done = true;
               break;
            default:
               printf("invalid command, enter r, s, t, f, h, p, d\n");
         }
      }
   }
}

//Calculate the bounding box size

void forwardMap()
{
   Vector3d leftBottom(0,0,1);
   Vector3d rightBottom(xres,0,1);
   Vector3d rightTop(xres,yres,1);
   Vector3d leftTop(0,yres,1);
   
   Vector3d vec1 = M*leftBottom;
   Vector3d vec2 = M*rightBottom;
   Vector3d vec3 = M*rightTop;
   Vector3d vec4 = M*leftTop;

   vec1 = vec1/vec1[2];
   vec2 = vec2/vec2[2];
   vec3 = vec3/vec3[2];
   vec4 = vec4/vec4[2];

   Left = minimum(vec1[0],vec2[0],vec3[0],vec4[0]);
   Right = maximum(vec1[0],vec2[0],vec3[0],vec4[0]);
   Bottom = minimum(vec1[1],vec2[1],vec3[1],vec4[1]);
   Top = maximum(vec1[1],vec2[1],vec3[1],vec4[1]);

   newxres = round(Right) - round(Left);  // new width 
   newyres = round(Top) - round(Bottom);  // new height
  
}


/*
   Main program to read,display and write file
*/
int main(int argc, char* argv[]){

  // start up the glut utilities
  glutInit(&argc, argv);
  string flag,inputLine;
  int search = 0,searchC = 0;
 
  if(argc < 2)
    {
     cout<<"No Image specified"<<endl;
     exit(0);
    }

  loadImage(argv[1]);

  process_input();

  if(argc == 3)
    {
     writeFile = argv[2];
     writeImage(writeFile);
    }
               
 
  // create the graphics window, giving width, height, and title text
  glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
  glutInitWindowSize(newxres, newyres);
  glutCreateWindow("Projective Warping");

  // set up the callback routines to be called when glutMainLoop() detects event  
  glutDisplayFunc(displayImage);  // display  callback
  glutKeyboardFunc(handleKey);	  // keyboard callback



  // define the drawing coordinate system on the viewport
  // lower left is (0, 0), upper right is (WIDTH, HEIGHT)
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   gluOrtho2D(0, newxres, 0, newyres);
 

  // Routine that loops forever looking for events. It calls the registered 
  // callback routine to handle each event that is detected
  glutMainLoop();
  return 0;
}
