/*
 Parag Acharya  pachary@clemson.edu 11/04/2013
 
 Program to perform Interactive Warping
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
int xres = 0,yres = 0,image_size = 0,channels = 0,winWidth = 0,winHeight = 0,writeFlag = 0,newxPos = 0,newyPos = 0,newxres = 0,
    newyres = 0,Count = 0;
int X[4],Y[4],U[4],V[4],tempX[4],tempY[4];
float Left = 0,Right = 0,Bottom = 0,Top = 0;  

void warping();  
void forwardMap();  
void calcM();    
void reshapeNew(int,int);   

float minimum(float a,float b,float c,float d)  //Returns minimum of 4 input values
{
  float min1 = 0,min2 = 0,finmin = 0;
  min1 = MIN(a,b);
  min2 = MIN(c,d);
  finmin = MIN(min1,min2);
  return finmin;   
}

float maximum(float a,float b,float c,float d) //Returns maximum of 4 input values 
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


void displayImage()  //Function to display input image
 
 { 
  glClearColor(0, 0, 0, 0);
  glClear(GL_COLOR_BUFFER_BIT);  // clear window to background color
  glRasterPos2i(0,0);
  glDrawPixels(xres,yres,GL_RGB,GL_FLOAT,*finData);  // Display the original Image
  glFlush();
}

void displayWarp()  //Function to display warped image
 
 { 
  glClearColor(0, 0, 0, 0);
  glClear(GL_COLOR_BUFFER_BIT);  // clear window to background color
  glRasterPos2i(0,0);
  glDrawPixels(newxres,newyres,GL_RGB,GL_FLOAT,*warpImage);   // Display the warped Image
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

	      if(u >= xres || u < 0 || v >= yres || v < 0) {
				warpImage[row][col].r = 0;
				warpImage[row][col].g = 0;
				warpImage[row][col].b = 0;}
            else{
          
              warpImage[row][col].r = finData[int(v)][int(u)].r;   
              warpImage[row][col].g = finData[int(v)][int(u)].g;  
              warpImage[row][col].b = finData[int(v)][int(u)].b;  
                 }                          
	     }
      }  
}

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

   newxres = round(Right) - round(Left); // new width
   newyres = round(Top) - round(Bottom); // new height
}

void mouseButton(int button,int state,int x,int y)
{

  if (state == GLUT_DOWN)
     {
       if (Count <4)
        {       
         tempX[Count] = x;  //Storing the input x coordinate into tempX
         tempY[Count] = y;  //Storing the input y coordinate into tempY
         Count++; 
        } 
     }
  if ( Count == 4)  
     { 

      int index = 0;
      //Swapping the first coordinate with the last and second with second last inorder to prevent image from being flipped 
      for ( int i = 3;i>=0;i--)
        {
          X[index] = tempX[i];   
          Y[index] = tempY[i];
          index++;
        }
       Count++;
       calcM();   
       forwardMap();
       warping();
       // Display the warped image in a new window
       glutInitWindowSize(newxres, newyres);
       int win2 = glutCreateWindow("Projective Warp");
       glutDisplayFunc(displayWarp);
       glMatrixMode(GL_PROJECTION);
       glLoadIdentity();
       gluOrtho2D(0,newxres, 0,newyres);
       glutReshapeFunc(reshapeNew);
       glutKeyboardFunc(handleKey);
     } 
}

void calcM()  // Compute the Projective Warp Matrix
{
  int locCount = 0;

  U[0] = 0;
  V[0] = 0;
  U[1] = xres;
  V[1] = 0;
  U[2] = xres;
  V[2] = yres;
  U[3] = 0;
  V[3] = yres;

  Matrix A(8,8);
  Matrix invA(8,8);
  Vector Out(8);
  Vector Input(8);
  locCount = 0;

  //Initializing the Vector Input
  for ( int i = 0;i<4;i++)
      {
        Input[locCount++] = U[i];
        Input[locCount++]= V[i];
      } 
 
  //Initializing Matrix A
   for(int row = 0;row<8;row++)
     {
       for(int col = 0;col<8;col++)
          {
           A[row][col] = 1;
          }
     }

locCount = -1;

//Populating Matrix A
  for(int row = 0;row<8;row++)
     {
       for(int col = 0;col<8;col++)
          {
           if(row%2==0 and col == 0) locCount++;

           if(row%2 == 0 and col == 0)
              A[row][col] = X[locCount]; 
           
           if(row%2 == 0 and col == 1)
              A[row][col] = Y[locCount]; 
           
           if(row%2 == 0 and col == 6)
              A[row][col] = -U[locCount]*X[locCount] ; 
 
           if(row%2 == 0 and col == 7)
              A[row][col] = -U[locCount]*Y[locCount]; 

           if(row%2!=0 and col == 3)
              A[row][col] = X[locCount];
   
           if(row%2!=0 and col == 4)
              A[row][col] = Y[locCount];
    
           if(row%2!=0 and col == 6)
              A[row][col] = -V[locCount]*X[locCount];


           if(row%2!=0 and col == 7)
               A[row][col] = -V[locCount]*Y[locCount];
 
           if(row%2 == 0 and (col == 3 or col == 4 or col == 5))
              A[row][col] = 0;
           
           if(row%2 != 0 and (col == 0 or col == 1 or col == 2))
               A[row][col] = 0;            
          }
     }
  

 invA = A.inv();
 Out = invA*Input;
 locCount = 0;

//Populating Matrix M from the Vector Out
  for(int row = 0;row<3;row++)
     {
       for(int col = 0;col<3;col++)
          {
           if ( row == 2 and col == 2)
              M[row][col] = 1;
           else
              M[row][col] = Out[locCount++];
          }
     }
}

//Reshape routine
//Shrink the image if the user decreases the size of the original display window
//Keep the image centered if the user increases the size of the original display window
void reshapeOrig(int width,int height)
{ 

  float xPos;
  float yPos;
  glViewport(0, 0, width, height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0, width, 0, height);
  glMatrixMode(GL_MODELVIEW);
  
  newxPos = 0;
  newyPos = 0;
  
  xPos = ((float)width/(float)xres);
  yPos = ((float)height/(float)yres);
  

  if (xPos < 1 or yPos < 1)       //User decreased the size of the display window
  {
	  if(xPos > yPos)
	  {
		  xPos = yPos;
		
	  }
	  else if(yPos > xPos)
	  {
		  yPos = xPos;
		  
	  }
	  glPixelZoom(xPos,yPos); // Zoom the image according to the display window size
}
  
 if ( (xPos > 1 or yPos > 1) and (width > xres or height >yres))  //User increased the size of the display window
  {
	newxPos =  (((float)width - (float)xres)/2.0);
	newyPos =  (((float)height - (float)yres)/2.0);
        glViewport(newxPos, newyPos, width, height);     //Keep the image centered
   }
 
}

//Reshape routine
//Shrink the image if the user decreases the size of the new display window
//Keep the image centered if the user increases the size of the  new display window
void reshapeNew(int width,int height)
{ 

  float xPos;
  float yPos;
  glViewport(0, 0, width, height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0, width, 0, height);
  glMatrixMode(GL_MODELVIEW);
  
  newxPos = 0;
  newyPos = 0;
  
  xPos = ((float)width/(float)newxres);
  yPos = ((float)height/(float)newyres);
  

  if (xPos < 1 or yPos < 1)       //User decreased the size of the display window
  {
	  if(xPos > yPos)
	  {
		  xPos = yPos;
		
	  }
	  else if(yPos > xPos)
	  {
		  yPos = xPos;
		  
	  }
	  glPixelZoom(xPos,yPos); // Zoom the image according to the display window size
}
  
 if ( (xPos > 1 or yPos > 1) and (width > newxres or height >newyres))  //User increased the size of the display window
  {
	newxPos =  (((float)width - (float)newxres)/2.0);
	newyPos =  (((float)height - (float)newyres)/2.0);
        glViewport(newxPos, newyPos, width, height);     //Keep the image centered
   }
 
}
/*
   Main program to read,display and write file
*/
int main(int argc, char* argv[]){

  // start up the glut utilities
  glutInit(&argc, argv);
  string flag,inputLine;
 
  if(argc < 2)
    {
     cout<<"No Image specified"<<endl;
     exit(0);
    }

  loadImage(argv[1]);      
 
  // create the graphics window, giving width, height, and title text
  glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
  
  glutInitWindowSize(xres, yres);
  int win1 = glutCreateWindow("Input Image");
  
  // set up the callback routines to be called when glutMainLoop() detects event  
  glutDisplayFunc(displayImage);  // display  callback
  glutKeyboardFunc(handleKey);	  // keyboard callback

   // define the drawing coordinate system on the viewport
   // lower left is (0, 0), upper right is (WIDTH, HEIGHT)
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   gluOrtho2D(0, xres, 0, yres);
   glutMouseFunc(mouseButton);
   glutReshapeFunc(reshapeOrig);
 
  // Routine that loops forever looking for events. It calls the registered 
  // callback routine to handle each event that is detected
  glutMainLoop();
  return 0;
}
