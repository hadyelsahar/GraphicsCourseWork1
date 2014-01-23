#include "screencasts.h"

using namespace std ;

struct Point
{
    float x;
    float y;
    float z;
    float u;
    float v;
};

struct Triangle
{
  int id1;
  int id2;
  int id3;
  Point p1;
  Point p2;
  Point p3;
  Point norm;
};

Point *points;
Triangle *triangles;
int allPointsno;
int allPolygonsNo;
Point center ; 
float _angle = 0 ;
float _vangle = 0 ;
GLuint texture;
bool motion = false ; 

bool loadfile(string filename)
{
  if(filename == "") return false;
  
  ifstream filestream(filename.c_str());

  if(!filestream.is_open())
    return false;

  while(filestream.good() )
  {
    string line;
    getline(filestream,line);
    std:: cout << line << endl;
    if (line.size() ==0)
      continue;

    //get number of points 
    if(line.find("POINTS") != string::npos)
    {
      string allPointsStr;
      for(int i=0;i<line.length();i++)
      {
        if(isdigit(line[i])) //Make sure it is a number
        {          
          allPointsStr +=line[i];
        }
        
      }

      allPointsno = atoi(allPointsStr.c_str());
      points = new Point[allPointsno];

      for(int i=0;i<allPointsno;i++)
      {
        filestream>>points[i].x>>points[i].y>>points[i].z;
        center.x += points[i].x;
        center.y += points[i].y;
        center.z += points[i].z;
      }

      //calculating the center
      float x =0;
      float y =0;
      float z =0;
      for(int i=0;i<allPointsno;i++)
      {
        x += points[i].x;
        y += points[i].y;
        z += points[i].z;
      }

      center.x = (float) x / allPointsno;
      center.y = (float) y / allPointsno;
      center.z = (float) z / allPointsno;
     
    }

    //Find the Polygons Data
    if(line.find("POLYGONS") != string::npos)
    {
      string allPolygonsStr;


      for(int i=0;i<line.length();i++)
      {
        if(isdigit(line[i])) //Make sure it is a number
        {          
          allPolygonsStr +=line[i];
        }
      }
      int pos1 = line.find(" ");
      int pos2 = line.find(" ",pos1);
      allPolygonsNo = atoi(line.substr(pos1+1,pos2-pos1-1).c_str());
      cout << "all polygons number = "<< allPolygonsNo << endl;
      triangles = new Triangle[allPolygonsNo];

      //loading triangles
      for(int i=0;i<allPolygonsNo;i++)
      {
        int numberOfSides;
        filestream>>numberOfSides;
        
        int np1;
        int np2;
        int np3;
        
        filestream>>np1>>np2>>np3;
        triangles[i].id1 = np1;
        triangles[i].id2 = np2;
        triangles[i].id3 = np3;
        // triangles[i].p1 = points[np1];
        // triangles[i].p2 = points[np2];
        // triangles[i].p3 = points[np3];
      }

      //calculating normals 
      for(int i=0;i<allPolygonsNo;i++)
      {
        float v1x = triangles[i].p1.x - triangles[i].p2.x;
        float v1y = triangles[i].p1.y - triangles[i].p2.y;
        float v1z = triangles[i].p1.z - triangles[i].p2.z;

        float v2x = triangles[i].p1.x - triangles[i].p3.x;
        float v2y = triangles[i].p1.y - triangles[i].p3.y;
        float v2z = triangles[i].p1.z - triangles[i].p3.z;

        triangles[i].norm.x = v1y*v2z - v2y*v1z;
        triangles[i].norm.y =v1z*v2x - v2z*v1y;
        triangles[i].norm.z = v1x*v2y - v2x*v1y;

        // //Now we have to normalize that vector
        // // we dont need it anymore because of glEnable(GL_NORMALIZE);
        // float length = sqrt(pow(triangles[i].norm.x,2) + pow(triangles[i].norm.y,2)+ pow(triangles[i].norm.z,2));

        // triangles[i].norm.x = triangles[i].norm.x / (float)length;
        // triangles[i].norm.y =triangles[i].norm.y /(float)length;
        // triangles[i].norm.z = triangles[i].norm.z /(float)length;

        //todo: average sharing 

      }



    }

   if(line.find("TEXTURE_COORDINATES") != string::npos)
    {    
      for(int i=0;i<allPointsno;i++)
        filestream>>points[i].u>>points[i].v;
    }



  }


  for (int i =0 ; i < allPolygonsNo ; i++)
  {
    triangles[i].p1 = points[triangles[i].id1];
    triangles[i].p2 = points[triangles[i].id2];
    triangles[i].p3 = points[triangles[i].id3];
  }



  return true;
}

bool loadTexture(string filename)
{          
  GLubyte *texturedata; // The texture image
  int textureWidth;
  int textureHeight;
  // load texture

  if(filename == ""){
    cout << "\n Error texture file name is empty";
    cout.flush(); 
    exit(0);
  }

  ifstream textureFile(filename.c_str());

  if(!textureFile.is_open()){
    cout << "\n Error loading the texture";
    cout.flush(); 
    exit(0);
  }

  string line;
  getline(textureFile,line); getline(textureFile,line);
  textureFile >> textureWidth;
  textureFile >> textureHeight;
  int maxRGBValue; textureFile >> maxRGBValue;
  texturedata = new GLubyte[textureWidth*textureHeight*4];

  int m,n,c;

  for(m=0;m<textureHeight;m++)
  for(n=0;n<textureWidth;n++)
  {
          textureFile >> c;
          texturedata[(m*textureWidth+n)*4]=(GLubyte) c;
          textureFile >> c;
          texturedata[(m*textureWidth+n)*4+1]=(GLubyte) c;
          textureFile >> c;
          texturedata[(m*textureWidth+n)*4+2]=(GLubyte) c;
          texturedata[(m*textureWidth+n)*4+3]=(GLubyte) 255;
  }
  textureFile.close();

  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexImage2D(GL_TEXTURE_2D,                
                 0,                            
                 GL_RGBA,                       //Format OpenGL uses for image
                 textureWidth, textureHeight,  //Width and height
                 0,                            //The border of the image
                 GL_RGBA, //GL_RGB, because pixels are stored in RGB format
                 GL_UNSIGNED_BYTE, //GL_UNSIGNED_BYTE, because pixels are stored
                                   //as unsigned numbers
                 texturedata);               //The actual pixel data

  //Set texture parameterstextureHeight
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,GL_REPEAT);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  //Define the texture
  // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureWidth, textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, texturedata);

  delete[]texturedata;
 
  return true;
}




//Initializes 3D rendering
void initRendering() {
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_NORMALIZE);
  glShadeModel(GL_SMOOTH);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_LIGHT1);
  glEnable(GL_TEXTURE_2D) ;
  glEnable(GL_COLOR_MATERIAL);
  glutSetKeyRepeat(GLUT_KEY_REPEAT_ON);
}

//Called when the window is resized
void handleResize(int w, int h) {
  glViewport(0, 0, w, h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(45.0, (double)w / (double)h, 1.0, 200.0);
}

//Called when a key is pressed
void handleKeypress(unsigned char key, int x, int y) {
  switch (key) {  
    case 27: //Escape key
        exit(0);

    case 97:
      cout << "left" << endl;
      cout << _angle << endl;
        _angle += 2.0f;
        if (_angle > 360) {
        _angle -= 360;
      }
      glutPostRedisplay();
      break;    

    case 100:
      cout << "right" << endl;
      cout << _angle << endl;
        _angle -= 2.0f;
        if (_angle < 0) {
        _angle += 360;      
      }
      glutPostRedisplay();
      break;

      case 119:
      cout << "up" << endl;
      cout << _vangle << endl;
        _vangle += 2.0f;
        if (_vangle > 360) {
        _vangle -= 360;        
      }
      glutPostRedisplay();
      break;    

    case 115:
      cout << "down" << endl;
      cout << _vangle << endl;
        _vangle -= 2.0f;
        if (_vangle < 0) {
        _vangle += 360;      
      }
      glutPostRedisplay();
      break;


    case 32:
      motion = !motion ; 

  }
}


void display()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  GLfloat ambientColor[] = {1.0f, 1.0f, 1.0f, 1.0f}; //Color(0.2, 0.2, 0.2)
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientColor);


  //Lighting
  // //Add positioned light
  GLfloat lightColor0[] = {1.0f, 1.0f, 1.0f, 1.0f}; //Color (0.5, 0.5, 0.5)
  GLfloat lightPos0[] = {3.0f, 0.0f, -3.0f, 0.0f}; //Positioned at (4, 0, 8)
  glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor0);
  glLightfv(GL_LIGHT0, GL_POSITION, lightPos0);

  //Add directed light
  GLfloat lightColor1[] = {1.0f, 0.5f, 0.5f, 1.0f}; //Color (0.5, 0.2, 0.2)
  //Coming from the direction (-1, 0.5, 0.5)
  GLfloat lightPos1[] = {-3.0f, 0.0f, -3.0f, 0.0f};
  glLightfv(GL_LIGHT1, GL_DIFFUSE, lightColor1);
  glLightfv(GL_LIGHT1, GL_POSITION, lightPos1);

  //Textures:
  glBindTexture(GL_TEXTURE_2D, texture) ;


  //Transformations 
  glMatrixMode(GL_MODELVIEW); //Switch to the drawing perspective
  glLoadIdentity(); //Reset the drawing perspective

  glTranslatef(-center.x, -center.y, -center.z); 
  glTranslatef(0,0.0f,-1.0f);
  glRotatef(_angle, 0.0f, 1.0f, 0.0f); //Rotate about the z-axis  
  glRotatef(_vangle, 1.0f, 0.0f, 0.0f); //Rotate about the z-axis  

  glPushMatrix();

  glScalef(2.0f,2.0f,2.0f);

  glBegin(GL_TRIANGLES);


  for(int i=0;i<allPolygonsNo;i++)
  {    
    glColor3f(0.3f, 0.3f, 0.3f);

    glNormal3f(-triangles[i].norm.x,-triangles[i].norm.y,-triangles[i].norm.z);
    glTexCoord2f(triangles[i].p1.u,triangles[i].p1.v);
    glVertex3f(triangles[i].p1.x,triangles[i].p1.y,triangles[i].p1.z);

    glTexCoord2f(triangles[i].p2.u,triangles[i].p2.v);
    glNormal3f(-triangles[i].norm.x,-triangles[i].norm.y,-triangles[i].norm.z);  
    glVertex3f(triangles[i].p2.x,triangles[i].p2.y,triangles[i].p2.z);


    glNormal3f(-triangles[i].norm.x,-triangles[i].norm.y,-triangles[i].norm.z);    
    glTexCoord2f(triangles[i].p3.u,triangles[i].p3.v);
    glVertex3f(triangles[i].p3.x,triangles[i].p3.y,triangles[i].p3.z);               
  }

  glEnd();   

  // cout << "center = \t "<< center.x << "\t" << center.y << "\t" << center.z;
  // gluLookAt(0,0,-1.0,center.x, center.y, center.z, 0, 1,0);       
  glutSwapBuffers();
  glFlush();
  glPopMatrix();
}


void update(int value) {
  if (motion) {
    _angle += 2.0f;
  if (_angle > 360) {
    _angle -= 360;
  }
  
  glutPostRedisplay(); //Tell GLUT that the display has changed  
  //Tell GLUT to call update again in 25 milliseconds
  }
  
  glutTimerFunc(25, update, 0);
}

int main(int argc, char** argv)
{
  //Initialize GLUT
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize(800, 600);
  
  loadfile("face.vtk.txt");
  

  //Create the window
  glutCreateWindow("Transformations and Timers - videotutorialsrock.com");
  initRendering();

  loadTexture("face.ppm");
  
  //Set handler functions
  glutDisplayFunc(display);
  glutKeyboardFunc(handleKeypress);
  // glutSpecialFunc(handleSpecialKeypress);
  glutReshapeFunc(handleResize);
  
  glutTimerFunc(25, update, 0); //Add a timer
  glutMainLoop();
  return 0;
}


