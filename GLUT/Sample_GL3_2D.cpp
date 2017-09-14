#include<bits/stdc++.h>
#include <fstream>
#include<ao/ao.h>
#include <GL/glew.h>
#include <GL/glu.h>
#include <GL/freeglut.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;

struct VAO {
    GLuint VertexArrayID;
    GLuint VertexBuffer;
    GLuint ColorBuffer;

    GLenum PrimitiveMode;
    GLenum FillMode;
    int NumVertices;
};
typedef struct VAO VAO;

struct GLMatrices {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
	GLuint MatrixID;
} Matrices;

struct parameters
{
	string name;
	VAO* object;
	int status;
	float tlx;
	float tly;
	float brx;
       	float bry;	
	float dx;
	float dy;
	float angle;
	int isRotating;
};

float pLeft = -4.0f;
float pRight = 4.0f;
float pTop = 4.0f;
float pBottom = -4.0f;
float pFar = 500.0f;
float pNear = 0.1f;
char myString[50] = "hello world";
int fireStatus = 0;
map<int, parameters> fire;
map<string, parameters> objects;
map<int, parameters> blocks;
int numOfBlocks = 0;
int gameOver = 0;
int Rpoints = 0;
int Gpoints = 0;
int Bpoints = 0;
int totalPoints = 0;
int penalty = 0;
int isBlock[50];
int isFire[100];
int blockLimit = 6;
long long int myClock = 0;

void moveObject(string name, float dx, float dy)
{
	objects[name].dx += dx;
	objects[name].dy += dy;
}

GLuint programID;

/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open())
	{
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::string Line = "";
		while(getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> VertexShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

	// Link the program
	fprintf(stdout, "Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
	glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
	fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
    struct VAO* vao = new struct VAO;
    vao->PrimitiveMode = primitive_mode;
    vao->NumVertices = numVertices;
    vao->FillMode = fill_mode;

    // Create Vertex Array Object
    glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
    glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
    glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

    glBindVertexArray (vao->VertexArrayID); // Bind the VAO 
    glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
    glVertexAttribPointer(
                          0,                  // attribute 0. Vertices
                          3,                  // size (x,y,z)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
    glVertexAttribPointer(
                          1,                  // attribute 1. Color
                          3,                  // size (r,g,b)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
    GLfloat* color_buffer_data = new GLfloat [3*numVertices];
    for (int i=0; i<numVertices; i++) {
        color_buffer_data [3*i] = red;
        color_buffer_data [3*i + 1] = green;
        color_buffer_data [3*i + 2] = blue;
    }

    return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
    // Change the Fill Mode for this object
    glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

    // Bind the VAO to use
    glBindVertexArray (vao->VertexArrayID);

    // Enable Vertex Attribute 0 - 3d Vertices
    glEnableVertexAttribArray(0);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

    // Enable Vertex Attribute 1 - Color
    glEnableVertexAttribArray(1);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

    // Draw the geometry !
    glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

void output(float x, float y, float r, float g, float b,const char *string)
{
  glColor3f( r, g, b );
  glRasterPos2f(x, y);
  int len, i;
  len = (int)strlen(string);
  for (i = 0; i < len; i++) {
	  glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_10, string[i]);
  }
}

void PlaySound()
{
	ao_device *device;
	ao_sample_format format;
	int default_driver;
	char *buffer;
	int buf_size;
	int sample;
	float freq = 200.0;
	int i;

	/* -- Initialize -- */


	ao_initialize();

	/* -- Setup for default driver -- */

	default_driver = ao_default_driver_id();

        memset(&format, 0, sizeof(format));
	format.bits = 16;
	format.channels = 6;
	format.rate = 190000;
	format.byte_format = AO_FMT_LITTLE;

	/* -- Open driver -- */
	device = ao_open_live(default_driver, &format, NULL /* no options */);
	if (device == NULL) {
		fprintf(stderr, "Error opening device.\n");
		return ;
	}

	/* -- Play some stuff -- */
	buf_size = format.bits/8 * format.channels * format.rate;
	buffer = (char* )calloc(buf_size, sizeof(char));

	for (i = 0; i < format.rate; i++) {
		sample = (int)(0.75 * 32768.0 *
			sin(2 * M_PI * freq * ((float) i/format.rate)));

		/* Put the same stuff in left and right channel */
		buffer[4*i] = buffer[4*i+2] = sample & 0xff;
		buffer[4*i+1] = buffer[4*i+3] = (sample >> 8) & 0xff;
	}
	ao_play(device, buffer, buf_size);

	/* -- Close and shutdown -- */
	ao_close(device);

	ao_shutdown();

  return;
}
/**************************
 * Customizable functions *
 **************************/

//float triangle_rot_dir = 1;
//float rectangle_rot_dir = 1;
//bool triangle_rot_status = true;
//bool rectangle_rot_status = true;
//float tdy =0, tdx=0;
//float tmov = 0.1;

void changeZoom()
{
	Matrices.projection = glm::ortho(pLeft, pRight, pBottom, pTop, pNear, pFar);
	return  ;
}

void zoomIn()
{
	if(pRight > 2)
	{
		pLeft *= 0.8;
		pRight *= 0.8;
		pTop *= 0.8;
		pBottom *= 0.8;
	}
}
void zoomOut()
{
	if(pRight < 8)
	{
		pLeft *= 1.25;
		pRight *= 1.25;
		pTop *= 1.25;
		pBottom *= 1.25;
	}
}

void panRight()
{
	if(pRight < 7.5)
	{
		pLeft += 0.5f;
		pRight += 0.5f;
	}
}
void panLeft()
{
	if(pRight > 0.5)
	{
		pLeft -= 1.0f;
		pRight -= 1.0f;
	}
}
void restoreOriginal()
{
	pLeft = -4.0f;
	pRight = 4.0f;
	pTop = 4.0f;
	pBottom = -4.0f;
}
/* Executed when a regular key is pressed */
void keyboardDown (unsigned char key, int x, int y)
{
    switch (key) {
        case 'Q':
        case 'q':
        case 27: //ESC
            exit (0);
        default:
            break;
    }
}

/* Executed when a regular key is released */
void keyboardUp (unsigned char key, int x, int y)
{
	//printf("%c\n",key);
    switch (key) {
	case 'A':
	case 'a':
		objects["bucket1"].dx -= 0.2;
		break;
	case 'S':
	case 's':
		objects["bucket1"].dx += 0.2;
		break;
	case 'D':
	case 'd':
		objects["bucket2"].dx -= 0.2;
		break;
	case 'F':
	case 'f':
		objects["bucket2"].dx += 0.2;
		break;
        case 'c':
        case 'C':
           // rectangle_rot_status = !rectangle_rot_status;
            break;
        case 'p':
        case 'P':
            //triangle_rot_status = !triangle_rot_status;
            break;
	case 'N':
        case 'n':
            blockLimit += 1;
	    blockLimit = min(20, blockLimit);
            break;
	case 'M':
	case 'm':
	    blockLimit -= 1;
	    blockLimit = max(0, blockLimit);
	    break;
	case 32:
	    PlaySound();
	    fireStatus = 1;
	case 'o':
	case 'O':
	    restoreOriginal();
	    break;
	case 'r':
	case 'R':
	    panRight();
	    break;
	case 'l':
	case 'L':
	    panLeft();
	    break;
        default:
            break;
    }
}

/* Executed when a special key is pressed */
void keyboardSpecialDown (int key, int x, int y)
{
}

/* Executed when a special key is released */
void keyboardSpecialUp (int key, int x, int y)
{
	//printf("%c\n",key);
	if(key == GLUT_KEY_UP)
	{
		objects["laserBase"].dy += 0.2;
		objects["laser"].dy += 0.2;
	}
	if(key == GLUT_KEY_DOWN)
	{
		objects["laserBase"].dy -= 0.2;
		objects["laser"].dy -= 0.2;
	}
	if(key == GLUT_KEY_LEFT)
	{
		objects["laser"].angle -= 5;
		//cout << objects["laser"].angle << endl;
		//if(objects["laser"].angle <= -60)
		//	objects["laser"].angle = -60;
	}
	if(key == GLUT_KEY_RIGHT)
	{
		objects["laser"].angle += 5;
		//cout << objects["laser"].angle << endl;
		//if(objects["laser"].angle >= 60)
		//	objects["laser"].angle = 60;
	}
}

/* Executed when a mouse button 'button' is put into state 'state'
 at screen position ('x', 'y')
 */


string objectCaptured;
int objectCapturedBool = 0;
void mouseClick (int button, int state, int x, int y)
{
    int dir = 0;
    int canFire = 1;
    double x1 = ((x-300.0)/300.0)*4;
    double y1 = ((300.0 - y)/300.0)*4;
    //
    if(button == 3)
    {
	    zoomIn();
	    printf("upmouse wheeeel\n");
    }
    else if(button == 4)
    {
	    zoomOut();
	    printf("mouse wheeeel\n");
    }
    //
    if(x1 > (objects["bucket1"].tlx + objects["bucket1"].dx) && x1 < (objects["bucket1"].brx + objects["bucket1"].dx) && y1 < (objects["bucket1"].tly) && y1 > (objects["bucket1"].bry))
    {
	    //cout << x1 << " " << y1 << endl;
	    objectCaptured = "bucket1";
	    objectCapturedBool = 1;
	    //printf("bucket1 captured\n");
    	    canFire = 0;
    }
    else if(x1 > (objects["bucket2"].tlx + objects["bucket2"].dx) && x1 < (objects["bucket2"].brx + objects["bucket2"].dx) && y1 < (objects["bucket2"].tly) && y1 > (objects["bucket2"].bry))
    {
	    //cout << x1 << " " << y1 << endl;
	    objectCaptured = "bucket2";
	    objectCapturedBool = 1;
	    //printf("bucket2 captured\n");
    	    canFire = 0;
    }
    else if(x1 >= (objects["laserBase"].tlx) && x1 <= (objects["laserBase"].brx + 1) && y1 > (objects["laserBase"].bry + objects["laserBase"].dy) && y1 < (objects["laserBase"].tly + objects["laserBase"].dy))
    {
    	objectCaptured = "laserBase";
	objectCapturedBool = 1;
	canFire = 0;
    }
    else objectCapturedBool = 0;

    //cout << objectCapturedBool << endl;
    switch (button) {
        case GLUT_LEFT_BUTTON:
	    if(state == 1 && canFire == 1)
            	fireStatus = 1;
            break;
        case GLUT_RIGHT_BUTTON:
            //if (state == GLUT_UP) {
            //}
            break;
        default:
            break;
    }
   // cout << objectCaptured << endl;
    canFire = 1;
}

/* Executed when the mouse moves to position ('x', 'y') */
void mouseMotion (int x, int y)
{
	double x1 = ((x-300.0)/300.0)*4;
	double y1 = ((300.0 - y)/300.0)*4;
	//cout << objectCapturedBool << endl;
	if((objectCaptured == "bucket1" || objectCaptured == "bucket2") && objectCapturedBool == 1)
		objects[objectCaptured].dx = x1 - (objects[objectCaptured].tlx + objects[objectCaptured].brx)/2;
	else if(objectCaptured == "laserBase" && objectCapturedBool == 1)
	{
		objects["laserBase"].dy = y1 - (objects["laserBase"].tly + objects["laserBase"].bry)/2;
		objects["laser"].dy = y1 - (objects["laser"].tly);
	}
	else
	{
		double x2,y2;
		y2 = objects["laser"].tly + objects["laser"].dy;
		x2 = objects["laser"].tlx;
		double slope = (y1-y2)/(x1-x2);
		//cout << "slope is " <<slope << endl;
		objects["laser"].angle = (atan(slope)/M_PI)*180;
		//cout << objects["laser"].angle;
	}
	//printf("%lf %lf\n",x1,y1);
}


/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (int width, int height)
{
	GLfloat fov = 90.0f;

	// sets the viewport of openGL renderer
	glViewport (0, 0, (GLsizei) width, (GLsizei) height);

	// set the projection matrix as perspective/ortho
	// Store the projection matrix in a variable for future use

    // Perspective projection for 3D views
    // Matrices.projection = glm::perspective (fov, (GLfloat) width / (GLfloat) height, 0.1f, 500.0f);

    // Ortho projection for 2D views
    Matrices.projection = glm::ortho(-4.0f, 4.0f, -4.0f, 4.0f, 0.1f, 500.0f);
}

VAO  *bar, *triangle, *bucket1, *bucket2, *laserBase, *laser, *mirror1 ,*mirror2, *mirror3,*blockarr[50];

int prevX = -4 , prevY = -4;

void createBar()
{
	static const GLfloat vertex_buffer_data [] = {
		-4,3.5, 0,
		-3,3.5, 0,
		-3, 4, 0,

		-3,4,0,
		-4,4,0,
		-4, 3.5,0
	};

	static const GLfloat color_buffer_data [] = {
		1.0, 128.0/255.0, 0,
		1.0, 128.0/255.0, 0,
		1.0, 128.0/255.0, 0,
		
		1.0, 128.0/255.0, 0,
		1.0, 128.0/255.0, 0,
		1.0, 128.0/255.0, 0
	};
  
  bar = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
  parameters pBar = {};
  pBar.name = "bar";
  pBar.tlx = -4;
  pBar.tly = 4;
  pBar.brx = -3;
  pBar.bry = 3.5;
  pBar.dx = 0;
  pBar.dy = 0;
  pBar.angle = 0;
  pBar.object = bar;
  pBar.isRotating = 0;
  objects["bar"] = pBar;
}
void createLaserBase()
{
	static const GLfloat vertex_buffer_data [] = {
		-4, 0, 0, //v1
		-3.8,0.2,0, //v2
		-4, 0.4, 0, //v3

		-4, 0.4, 0, //v3
		-4, 0.4, 0, //v3
		-4, 0.4, 0  //v3
	};

	static const GLfloat color_buffer_data [] = {
		0,0,0,
		0,0,0,
		0,0,0,
		
		0,0,0,
		0,0,0,
		0,0,0
	};

  laserBase = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
  parameters plaserBase = {};
  plaserBase.name = "laserBase";
  plaserBase.tlx = -4;
  plaserBase.tly = 0.4;
  plaserBase.brx = -3.8;
  plaserBase.bry = 0;
  plaserBase.dx = 0;
  plaserBase.dy = 0;
  plaserBase.angle = 0;
  plaserBase.object = laserBase;
  plaserBase.isRotating = 0;
  objects["laserBase"] = plaserBase;
}
void createLaser()
{
	static const GLfloat vertex_buffer_data [] = {
		-3.8, 0.16, 0, // v1
		-3, 0.16, 0, // v2
		-3, 0.24, 0, // v3

		-3, 0.24, 0, //v3
		-3.8, 0.24, 0, //v4
		-3.8, 0.16, 0 //v1
	};

	static const GLfloat color_buffer_data [] = {
		0,1,0,
		0,1,0,
		0,1,0,

		0,1,0,
		0,1,0,
		0,1,0
	};
  laser = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
  parameters plaser = {};
  plaser.name = "laser";
  plaser.tlx = -3.8;
  plaser.tly = 0.2;
  plaser.brx = -3;
  plaser.bry = -0.2;
  plaser.dx = 0;
  plaser.dy = 0;
  plaser.angle = 0;
  plaser.object = laser;
  plaser.isRotating = 0;
  objects["laser"] = plaser;
}

void createBucket1()
{
	static const GLfloat vertex_buffer_data [] = {
		-3, -4, 0, // v1
		-2, -4, 0, // v2
		-2, -3, 0, // v3

		-2, -3, 0, //v3
		-3, -3, 0, //v4
		-3, -4, 0, //v1
	};

	static const GLfloat color_buffer_data [] = {
		1,0,0,
		1,0,0,
		1,0,0,

		1,0,0,
		1,0,0,
		1,0,0,
	};
  bucket1 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
  parameters pbucket1 = {};
  pbucket1.name = "bucket1";
  pbucket1.tlx = -3;
  pbucket1.tly = -3;
  pbucket1.brx = -2;
  pbucket1.bry = -4;
  pbucket1.dx = 0;
  pbucket1.dy = 0;
  pbucket1.angle = 0;
  pbucket1.object = bucket1;
  pbucket1.isRotating = 0;
  objects["bucket1"] = pbucket1;
}

void createBucket2()
{
	static const GLfloat vertex_buffer_data [] = {
		3, -4, 0, //v1
		4, -4, 0, //v2
		4, -3, 0, //v3

		4, -3, 0, // v3
		3, -3, 0, // v4
		3, -4, 0, // v1
	};

	static const GLfloat color_buffer_data [] = {
		0,1,0,
		0,1,0,
		0,1,0,

		0,1,0,
		0,1,0,
		0,1,0,
	};
  bucket2 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
  parameters pbucket2 = {};
  pbucket2.name = "bucket2";
  pbucket2.tlx = 3;
  pbucket2.tly = -3;
  pbucket2.brx = 4;
  pbucket2.bry = -4;
  pbucket2.dx = 0;
  pbucket2.dy = 0;
  pbucket2.angle = 0;
  pbucket2.object = bucket2;
  pbucket2.isRotating = 0;
  objects["bucket2"] = pbucket2;
}

void createBlackRandomBlock(int i)
{
	float x = (rand()%600)/100.0 - 2.5;
	float y = 4;
	//while(x != prevX)
	{
		x = (rand()%600)/100.0 - 2.5;
	}
	prevX = x;
	static const GLfloat vertex_buffer_data [] = {
		0, 0, 0, //v1
		0 + 0.4, 0, 0, //v2
		0 + 0.4, 0-0.7, 0, //v3

		0 + 0.4, 0-0.7, 0, //v3
		0, 0-0.7, 0, //v4
		0, 0, 0  //v1
	};
	int rb=0,gb=0,bb=0;
	static const GLfloat color_buffer_data [] = {
		0,0,0,
		0,0,0,
		0,0,0,
	
		0,0,0,
		0,0,0,
		0,0,0
	};

  blockarr[i] = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
  parameters pblock = {};
  pblock.name = "block";
  pblock.tlx = x;
  pblock.tly = y;
  pblock.brx = x + 0.4;
  pblock.bry = y - 0.7;
  pblock.dx = 0;
  pblock.dy = 0;
  pblock.angle = 0;
  pblock.object = blockarr[i];
  pblock.isRotating = 0;
  pblock.status = 0;
  blocks[i] = pblock;
  numOfBlocks += 1;
}
void createGreenRandomBlock(int i)
{
	float x = (rand()%600)/100.0 - 2.5;
	//while(x != prevX)
	{
		x = (rand()%600)/100.0 - 2.5;
	}
	prevX = x;
	float y = 4;
	static const GLfloat vertex_buffer_data [] = {
		0, 0, 0, //v1
		0 + 0.4, 0, 0, //v2
		0 + 0.4, 0-0.7, 0, //v3

		0 + 0.4, 0-0.7, 0, //v3
		0, 0-0.7, 0, //v4
		0, 0, 0  //v1
	};
	int rb=0,gb=1,bb=0;
	static const GLfloat color_buffer_data [] = {
		0,1,0,
		0,1,0,
		0,1,0,
	
		0,1,0,
		0,1,0,
		0,1,0
	};

  blockarr[i] = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_LINE);
  parameters pblock = {};
  pblock.name = "block";
  pblock.tlx = x;
  pblock.tly = y;
  pblock.brx = x + 0.4;
  pblock.bry = y - 0.7;
  pblock.dx = 0;
  pblock.dy = 0;
  pblock.angle = 0;
  pblock.object = blockarr[i];
  pblock.isRotating = 0;
  pblock.status = 1;
  blocks[i] = pblock;
  numOfBlocks += 1;
}
void createRedRandomBlock(int i)
{
	float x = (rand()%600)/100.0 - 2.5;
	//while(x != prevX)
	{
		x = (rand()%600)/100.0 - 2.5;
	}
	prevX = x;
	float y = 4;
	static const GLfloat vertex_buffer_data [] = {
		0, 0, 0, //v1
		0 + 0.4, 0, 0, //v2
		0 + 0.4, 0-0.7, 0, //v3

		0 + 0.4, 0-0.7, 0, //v3
		0, 0-0.7, 0, //v4
		0, 0, 0  //v1
	};
	int rb=1,gb=0,bb=0;
	static const GLfloat color_buffer_data [] = {
		1,0,0,
		1,0,0,
		1,0,0,
	
		1,0,0,
		1,0,0,
		1,0,0
	};

  blockarr[i] = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_LINE);
  parameters pblock = {};
  pblock.name = "block";
  pblock.tlx = x;
  pblock.tly = y;
  pblock.brx = x + 0.2;
  pblock.bry = y - 0.5;
  pblock.dx = 0;
  pblock.dy = 0;
  pblock.angle = 0;
  pblock.object = blockarr[i];
  pblock.isRotating = 0;
  pblock.status = 2;
  blocks[i] = pblock;
  numOfBlocks += 1;
}

void createFire(int i)
{
	static const GLfloat vertex_buffer_data [] = {
		0, 0, 0, //v1
		0.5, 0, 0, //v2
		0.5, 0.1, 0, //v3

		0.5, 0.1, 0, //v3
		0, 0.1, 0, //v4
		0, 0, 0  //v1
	};
	float rx = (float)21/(float)255,gx = (float)244/(float)255,bx = (float)238/(float)238;
	static const GLfloat color_buffer_data [] = {
		rx,gx,bx,
		rx,gx,bx,
		rx,gx,bx,
		
		rx,gx,bx,
		rx,gx,bx,
		rx,gx,bx
	};

  VAO* fireObject = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
  parameters pfire = {};
  pfire.name = "fire";
  pfire.tlx = objects["laser"].tlx + objects["laser"].dx;
  pfire.tly = objects["laser"].tly + objects["laser"].dy;
  pfire.brx = objects["laser"].brx + objects["laser"].dx;
  pfire.bry = objects["laser"].bry + objects["laser"].dy;
  pfire.dx = 0;
  pfire.dy = 0;
  pfire.angle = objects["laser"].angle;
  pfire.object = fireObject;
  pfire.isRotating = 0;
  fire[i] = pfire;
}

void createMirror1()
{
	static const GLfloat vertex_buffer_data [] = {
		3.5, -1, 0, //v1
		3.6, -1,0, //v2
		3.6, 1, 0, //v3

		3.6, 1, 0, //v3
		3.5, 1, 0, //v4
		3.5, -1, 0  //v1
	};

	float r,g,b;
	r = 0; g = 0; b=0;
	static const GLfloat color_buffer_data [] = {
		r,g,b,
		r,g,b,
		r,g,b,
		
		r,g,b,
		r,g,b,
		r,g,b
	};

  mirror1 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
  parameters pmirror1 = {};
  pmirror1.name = "mirror1";
  pmirror1.tlx = 3.5;
  pmirror1.tly = 1.0;
  pmirror1.brx = 3.6;
  pmirror1.bry = -1.0;
  pmirror1.dx = 0;
  pmirror1.dy = 0;
  pmirror1.angle = 90;
  pmirror1.object = mirror1;
  pmirror1.isRotating = 0;
  objects["mirror1"] = pmirror1;
}
void createMirror2()
{
	float s = 2;
	static const GLfloat vertex_buffer_data [] = {
		-2.5 - sqrt(3)/2 + s, 2.5, 0, //v1
		-2.5 - sqrt(3)/2 + 0.2 + s, 2.5, 0, //v2
		-2.5 + sqrt(3)/2 + 0.2 + s, 1.5, 0, //v3

		-2.5 + sqrt(3)/2 + 0.2 + s, 1.5, 0, //v3
		-2.5 + sqrt(3)/2 + s, 1.5, 0, //v4
		-2.5 - sqrt(3)/2 + s, 2.5, 0  //v1
	};

	float r,g,b;
	r = 0; g = 0; b=0;
	static const GLfloat color_buffer_data [] = {
		r,g,b,
		r,g,b,
		r,g,b,
		
		r,g,b,
		r,g,b,
		r,g,b
	};

  mirror2 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
  parameters pmirror2 = {};
  pmirror2.name = "mirror2";
  pmirror2.tlx = -2.5 - sqrt(3)/2 + s;
  pmirror2.tly = 2.5;
  pmirror2.brx = s + -2.5 + sqrt(3)/2 + 0.2;
  pmirror2.bry = 1.5;
  pmirror2.dx = 0;
  pmirror2.dy = 0;
  pmirror2.angle = 30;
  pmirror2.object = mirror2;
  pmirror2.isRotating = 0;
  objects["mirror2"] = pmirror2;
}
void createMirror3()
{
	float s = 2;
	static const GLfloat vertex_buffer_data [] = {
		-2.5 - sqrt(3)/2 + s, -2.5, 0, //v1
		-2.5 - sqrt(3)/2 + 0.2 + s, -2.5,0, //v2
		-2.5 + sqrt(3)/2 + 0.2 + s, -1.5, 0, //v3

		-2.5 + sqrt(3)/2 + 0.2 + s, -1.5, 0, //v3
		-2.5 + sqrt(3)/2 + s, -1.5, 0, //v4
		-2.5 - sqrt(3)/2 + s, -2.5, 0  //v1
	};

	float r,g,b;
	r = 0; g = 0; b=0;
	static const GLfloat color_buffer_data [] = {
		r,g,b,
		r,g,b,
		r,g,b,
		
		r,g,b,
		r,g,b,
		r,g,b
	};

  mirror3 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
  parameters pmirror3 = {};
  pmirror3.name = "mirror3";
  pmirror3.tlx = s + -2.5 - sqrt(3)/2;
  pmirror3.tly = -2.5;
  pmirror3.brx = s + -2.5 + sqrt(3)/2 + 0.2;
  pmirror3.bry = -1.5;
  pmirror3.dx = 0;
  pmirror3.dy = 0;
  pmirror3.angle = 30;
  pmirror3.object = mirror3;
  pmirror3.isRotating = 0;
  objects["mirror3"] = pmirror3;
}

float camera_rotation_angle = 70;
//long long int timer = 0;

/* Render the scene with openGL */
/* Edit this function according to your assignment */
void draw ()
{
  // clear the color and depth in the frame buffer
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // use the loaded shader program
  // Don't change unless you know what you are doing
  glUseProgram (programID);

  // Eye - Location of camera. Don't change unless you are sure!!
  glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
  // Target - Where is the camera looking at.  Don't change unless you are sure!!
  glm::vec3 target (0, 0, 0);
  // Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
  glm::vec3 up (0, 1, 0);

  // Compute Camera matrix (view)
  // Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
  //  Don't change unless you are sure!!
  Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

  // Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
  //  Don't change unless you are sure!!
  glm::mat4 VP = Matrices.projection * Matrices.view;

  // Send our transformation to the currently bound shader, in the "MVP" uniform
  // For each model you render, since the MVP will be different (at least the M part)
  //  Don't change unless you are sure!!
  glm::mat4 MVP;	// MVP = Projection * View * Model

  //timer = (timer+1)%1000000007;
  myClock = (myClock + 1)%1000000009;
  
  changeZoom();
  
  if(objects["laser"].angle <= -60)
	  objects["laser"].angle = -60;
  if(objects["laser"].angle >= 60)
	  objects["laser"].angle = 60;
  
  // Checking block collection

  
  for(int i=0; i<blockLimit; i++)
  {
  	if(isBlock[i] == 1)
	{
		float blockBRX,blockBRY;
		blockBRX = blocks[i].brx + blocks[i].dx;
		blockBRY = blocks[i].bry + blocks[i].dy;

		if(blocks[i].status == 1)
		{
			float bucket2TLX, bucket2TLY, bucket2BRX, bucket2BRY;
			bucket2TLX = objects["bucket2"].tlx + objects["bucket2"].dx;
			bucket2TLY = objects["bucket2"].tly;
			bucket2BRX = objects["bucket2"].brx + objects["bucket2"].dx;
			bucket2BRY = objects["bucket2"].bry;
			//printf("checking for block %d",i);
			if((blockBRX >= bucket2TLX) && (blockBRX <= bucket2BRX) && (blockBRY <= -3))
			{
				Gpoints += 1;
				blocks[i].dy = 100;
				isBlock[i] = 0;
				printf("Green block captured\n");
  				totalPoints = Bpoints + Rpoints + Gpoints;
  				printf("\t%dTotal Points = %d(Black blocks) + %d(Red blocks) + %d(Green Blocks)\n",totalPoints, Bpoints, Rpoints, Gpoints);
			}
		}
		if(blocks[i].status == 2)
		{
			float bucket1TLX, bucket1TLY, bucket1BRX, bucket1BRY;
			bucket1TLX = objects["bucket1"].tlx + objects["bucket1"].dx;
			bucket1TLY = objects["bucket1"].tly;
			bucket1BRX = objects["bucket1"].brx + objects["bucket1"].dx;
			bucket1BRY = objects["bucket1"].bry;
			//printf("checking for block %d",i);
			if((blockBRX >= bucket1TLX) && (blockBRX <= bucket1BRX) && (blockBRY <= -3))
			{
				Rpoints += 1;
				blocks[i].dy = 100;
				isBlock[i] = 0;
				printf("Red block captured\n");
	 	 		totalPoints = Bpoints + Rpoints + Gpoints;
  				printf("\t%dTotal Points = %d(Black blocks) + %d(Red blocks) + %d(Green Blocks)\n",totalPoints, Bpoints, Rpoints, Gpoints);
			}
		}
		if(blocks[i].status == 0)
		{
			int f1 = 0,f2 = 0;
			float bucket1TLX, bucket1TLY, bucket1BRX, bucket1BRY;
			bucket1TLX = objects["bucket1"].tlx + objects["bucket1"].dx;
			bucket1TLY = objects["bucket1"].tly;
			bucket1BRX = objects["bucket1"].brx + objects["bucket1"].dx;
			bucket1BRY = objects["bucket1"].bry;
			if((blockBRX >= bucket1TLX) && (blockBRX <= bucket1BRX) && (blockBRY <= -3))
			{
				f1 = 1;
				blocks[i].dy = 100;
				isBlock[i] = 0;
			}
			float bucket2TLX, bucket2TLY, bucket2BRX, bucket2BRY;
			bucket2TLX = objects["bucket2"].tlx + objects["bucket2"].dx;
			bucket2TLY = objects["bucket2"].tly;
			bucket2BRX = objects["bucket2"].brx + objects["bucket2"].dx;
			bucket2BRY = objects["bucket2"].bry;
			if((blockBRX >= bucket2TLX) && (blockBRX <= bucket2BRX) && (blockBRY <= -3))
			{
				f2 = 1;
				blocks[i].dy = 100;
				isBlock[i] = 0;
			}
			if(f1 || f2)
			{
				printf("Black box captured\n\t\t\t GAME OVER \n");
				printf("Total points --> %d\n",totalPoints);
				exit(0);
			}
		}
	}
  }
  // Reflecting of ray by mirror 1 
  
  for(int i=0; i<100; i++)
  {
  	if(isFire[i] == 1)
	{
		float laserX = fire[i].tlx + fire[i].dx;
		float laserY = fire[i].tly + fire[i].dy;
		
		//cout << laserX << " " << laserY << endl;
		if(laserX >= 3.5 && laserX <= 3.6  && laserY <= 1 && laserY >= -1)
		{
			fire[i].angle = 180 - fire[i].angle;
			//printf("laser hit mirror1\n");
		}
		
	}
  }
  // reflecting by mirror 2
  for(int i=0; i<100; i++)
  {
  	if(isFire[i] == 1)
	{
		float laserX = fire[i].tlx + fire[i].dx;
		float laserY = fire[i].tly + fire[i].dy;
		float y = objects["mirror2"].tly - objects["mirror2"].bry;
		float x = objects["mirror2"].tlx - objects["mirror2"].brx;
		
		float m = y/x;

		float temp2 = laserY - objects["mirror2"].tly - m*(laserX - objects["mirror2"].tlx);
		//cout << fabs(temp2) << " " << laserY << endl;
		if(fabs(temp2) <= 0.1 && laserY <= 2.65 && laserY >= 1.45)
		{
			fire[i].angle = 180 - (fire[i].angle - 60);
			//printf("laser hit mirror2\n");
		}
		
	}
  }
  
  // reflecting by mirror 3
  for(int i=0; i<100; i++)
  {
  	if(isFire[i] == 1)
	{
		float laserX = fire[i].tlx + fire[i].dx;
		float laserY = fire[i].tly + fire[i].dy;
		float y = objects["mirror3"].tly - objects["mirror3"].bry;
		float x = objects["mirror3"].tlx - objects["mirror3"].brx;
		
		float m = y/x;

		float temp2 = laserY - objects["mirror3"].tly - m*(laserX - objects["mirror3"].tlx);
		//cout << fabs(temp2) << " " << laserY << endl;
		if(fabs(temp2) <= 0.1 && laserY >= -2.45  && laserY <= -1.65)
		{
			fire[i].angle = 180 - (2*330 + fire[i].angle - 180);
			//printf("laser hit mirror3\n");
		}
		
	}
  }
  // mirror1
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translateMirror1 = glm::translate(glm::vec3((float)objects["mirror1"].dx, (float)objects["mirror1"].dy, 0));
  Matrices.model *= translateMirror1;
  MVP = VP * Matrices.model;

  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(mirror1);
  

  // mirror2
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translateMirror2 = glm::translate(glm::vec3((float)objects["mirror2"].dx, (float)objects["mirror2"].dy, 0));
  Matrices.model *= translateMirror2;
  MVP = VP * Matrices.model;

  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(mirror2);
  // mirror3
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translateMirror3 = glm::translate(glm::vec3((float)objects["mirror3"].dx, (float)objects["mirror3"].dy, 0));
  Matrices.model *= translateMirror3;
  MVP = VP * Matrices.model;

  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(mirror3);

  // laser + laserbase constraint

  if(objects["laserBase"].bry+objects["laserBase"].dy <= -3)
  {
	  objects["laserBase"].dy = -3;
	  objects["laser"].dy = -3;
  }
  if(objects["laserBase"].tly + objects["laserBase"].dy >= 4)
  {
	  objects["laserBase"].dy = 3.6;
	  objects["laser"].dy = 3.6;
  }

  // laser

  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translateLaser = glm::translate(glm::vec3((float)objects["laser"].dx, (float)objects["laser"].dy, 0));
 
  glm::mat4 translateLaserR1 = glm::translate(glm::vec3((float)(3.8), (float)(-0.2), 0));
  glm::mat4 rotateLaser = glm::rotate((float)((objects["laser"].angle)*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  glm::mat4 translateLaserR2 = glm::translate(glm::vec3((float)(-3.8), (float)(0.2), 0));
  
  Matrices.model *= translateLaser;
  Matrices.model *= translateLaserR2;
  Matrices.model *= rotateLaser;
  Matrices.model *= translateLaserR1;
  MVP = VP * Matrices.model;

  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(laser);

  // laserbase

  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translateLaserBase = glm::translate(glm::vec3((float)objects["laserBase"].dx, (float)objects["laserBase"].dy, 0));
  Matrices.model *= translateLaserBase;
  MVP = VP * Matrices.model;

  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(laserBase);
	
  // bucket2
  if(objects["bucket2"].brx+objects["bucket2"].dx >= 4)
	  objects["bucket2"].dx = 0;
  if(objects["bucket2"].tlx + objects["bucket2"].dx <= -3)
	  objects["bucket2"].dx = -6;
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translateBucket2 = glm::translate(glm::vec3((float)objects["bucket2"].dx, (float)objects["bucket2"].dy, 0));
  Matrices.model *= translateBucket2;
  MVP = VP * Matrices.model;

  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(bucket2);
  // bucket1
  if(objects["bucket1"].brx + objects["bucket1"].dx >= 4)
	  objects["bucket1"].dx = 6;
  if(objects["bucket1"].tlx + objects["bucket1"].dx <= -3)
	  objects["bucket1"].dx = 0;
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translateBucket1 = glm::translate(glm::vec3((float)objects["bucket1"].dx, (float)objects["bucket1"].dy, 0));
  Matrices.model *= translateBucket1;
  MVP = VP * Matrices.model;

  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(bucket1);
  
  // fire and block collision detection
 
  for(int i=0; i<100; i++)
  {
  	if(isFire[i] == 1)
	{
		for(int j=0; j<blockLimit; j++)
		{
			
			float laserX = fire[i].tlx + fire[i].dx;
			float laserY = fire[i].tly + fire[i].dy;

			float bTLX = blocks[j].tlx + blocks[j].dx;
			float bTLY = blocks[j].tly + blocks[j].dy;
			
			float bBRX = blocks[j].brx + blocks[j].dx;
			float bBRY = blocks[j].bry + blocks[j].dy;

			if((laserX >= bTLX-0.2) && (laserX <= bBRX+0.2) && (laserY <= bTLY+0.2) && (laserY >= bBRY-0.2))
			{
				if(blocks[j].status == 0)
				{
					blocks[j].dx = 100;
					isBlock[j] = 0;
					fire[i].dx = 100;
					isFire[i] = 0;
					Bpoints += 1;
  					totalPoints = Bpoints + Rpoints + Gpoints;
  					printf("\t%d (Total Points) = %d (Black blocks) + %d (Red blocks) + %d (Green Blocks)\n",totalPoints, Bpoints, Rpoints, Gpoints);
				}
				else
				{
					penalty += 1;
					isBlock[j] = 0;
					fire[i].dx = 100;
					isFire[i] = 0;
					blocks[j].dx = 100;
				}
			}
		}
	}
  }

  // making fire
  for(int i=0; i<100; i++)
  {
  	if(myClock%25 == 0 && fireStatus == 1 && isFire[i] == 0)
	{
		createFire(i);
		isFire[i] = 1;
		fireStatus = 0;
	}
  }

  for(int i=0; i<100; i++)
  {
  	if(isFire[i] == 1)
	{
  		Matrices.model = glm::mat4(1.0f);
		fire[i].dx += cos((float)(fire[i].angle)*(M_PI/180.0))/10;
		fire[i].dy += sin((float)(fire[i].angle)*(M_PI/180.0))/10;

 		glm::mat4 translateFire = glm::translate(glm::vec3(fire[i].tlx + fire[i].dx ,fire[i].tly + fire[i].dy, 0));

  		glm::mat4 rotateFire = glm::rotate((float)((fire[i].angle)*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  		
		Matrices.model *= translateFire;
  		Matrices.model *= rotateFire;
  		MVP = VP * Matrices.model;

  		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  		draw3DObject(fire[i].object);
	}
  }
  
  // absorbing fire
  for(int i=0; i<100; i++)
  {
  	if(isFire[i] == 1)
	{
		if(objects["laser"].tlx + objects["laser"].dx + fire[i].dx >= 4)
		{
			fire[i].dx = 100;
			isFire[i] = 0;
		}
		float y = objects["laser"].tly + objects["laser"].dy + fire[i].dy;
		if(y >= 4 || y <= -3)
		{
			fire[i].dx = 100;
			fire[i].dy = 100;
			isFire[i] = 0;
		}
	}
  }

  // blocks
  for(int i=0; i<blockLimit; i++)
  {
  	if(isBlock[i] == 0)
  	{
  		//cout << "block getting created is " << i << endl;
		if(i%3 == 0)
  			createBlackRandomBlock(i);
		if(i%3 == 1)
  			createRedRandomBlock(i);
		if(i%3 == 2)
  			createGreenRandomBlock(i);
		isBlock[i] = 1;
  		//printf("created %d\n",i);
  	}
  }
  for(int i=0; i<blockLimit; i++)
  {
	  if(isBlock[i] == 1)
	  {
	  	//cout << "block number getting positioned changed is "  << "  " << i << endl;
	  	blocks[i].dy -= 0.01;
  		Matrices.model = glm::mat4(1.0f);
  		glm::mat4 translateBlock = glm::translate(glm::vec3((float)blocks[i].tlx, (float)blocks[i].tly+(float)blocks[i].dy, 0));
  		Matrices.model *= translateBlock;
  		MVP = VP * Matrices.model;
  		
  		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  		draw3DObject(blockarr[i]);
	  }
  
  }
  
  for(int i=0; i<blockLimit; i++)
  {
  	if(isBlock[i] == 1)
	{
		if(blocks[i].dy + blocks[i].bry <= -3.5)
		{
			//cout << "block " << i  << "gone at " << blocks[i].dy + blocks[i].bry << endl;
			isBlock[i] = 0;
			blocks[i].dy = -100;
		}
	}
  }
  // recharge bar
  //Matrices.model = glm::mat4(1.0f);
  //glm::mat4 translateBarF = glm::translate(glm::vec3(3.5f, -3.75f, 0));
  //glm::mat4  scaleBar = glm::scale(glm::vec3((float(myClock%26)/25.0),1,1));
  //glm::mat4 translateBarS = glm::translate(glm::vec3(-3.5f, 3.75f, 0));
  //Matrices.model *= translateBarF;
  //Matrices.model *= scaleBar;
  //Matrices.model *= translateBarS;
  //MVP = VP * Matrices.model;
  
  //glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  //draw3DObject(bar);

  // game over constraint
  if(penalty > 20)
  {
  	printf("More than 20 non-black blocks hit by laser\n\t\t\tGame Over\n");
	printf("Total Points --> %d\n",totalPoints);
	exit(0);
  }
  // Swap the frame buffers
  glutSwapBuffers ();


  // Increment angles
  float increments = 1;

  camera_rotation_angle++; // Simulating camera rotation
  //triangle_rotation = triangle_rotation + increments*triangle_rot_dir*triangle_rot_status;
  //rectangle_rotation = rectangle_rotation + increments*rectangle_rot_dir*rectangle_rot_status;

  //Matrices.model = glm::mat4(1.0f);
  //glm::mat4 translateRectangle = glm::translate (glm::vec3(0, 0, 0));        // glTranslatef
  //glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)

}

/* Executed when the program is idle (no I/O activity) */
void idle () {
    // OpenGL should never stop drawing
    // can draw the same scene or a modified scene
    draw (); // drawing same scene
}


/* Initialise glut window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
void initGLUT (int& argc, char** argv, int width, int height)
{
    // Init glut
    glutInit (&argc, argv);

    // Init glut window
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitContextVersion (3, 3); // Init GL 3.3
    glutInitContextFlags (GLUT_CORE_PROFILE); // Use Core profile - older functions are deprecated
    glutInitWindowSize (width, height);
    glutCreateWindow ("Sample OpenGL3.3 Application");

    // Initialize GLEW, Needed in Core profile
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        cout << "Error: Failed to initialise GLEW : "<< glewGetErrorString(err) << endl;
        exit (1);
    }

    // register glut callbacks
    glutKeyboardFunc (keyboardDown);
    glutKeyboardUpFunc (keyboardUp);

    glutSpecialFunc (keyboardSpecialDown);
    glutSpecialUpFunc (keyboardSpecialUp);

    glutMouseFunc (mouseClick);
    glutMotionFunc (mouseMotion);

    glutReshapeFunc (reshapeWindow);

    glutDisplayFunc (draw); // function to draw when active
    glutIdleFunc (idle); // function to draw when idle (no I/O activity)
    
    glutIgnoreKeyRepeat (false); // Ignore keys held down
}

/* Process menu option 'op' */
void menu(int op)
{
    switch(op)
    {
        case 'Q':
        case 'q':
            exit(0);
    }
}

void addGLUTMenus ()
{
    // create sub menus
    int subMenu = glutCreateMenu (menu);
    glutAddMenuEntry ("Do Nothing", 0);
    glutAddMenuEntry ("Really Quit", 'q');

    // create main "middle click" menu
    glutCreateMenu (menu);
    glutAddSubMenu ("Sub Menu", subMenu);
    glutAddMenuEntry ("Quit", 'q');
    glutAttachMenu (GLUT_MIDDLE_BUTTON);
}


/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (int width, int height)
{
	// Create the models

	// Create and compile our GLSL program from the shaders
	programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");


	reshapeWindow (width, height);

	// Background color of the scene
	glClearColor (1, 1, 1, 0.0f); // R, G, B, A
	glClearDepth (1.0f);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);

	createBar();
	createBucket1 ();
	createBucket2 ();
	createLaser ();
	createLaserBase ();
	createMirror1 ();
	createMirror2 ();
	createMirror3 ();

	cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
	cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
	cout << "VERSION: " << glGetString(GL_VERSION) << endl;
	cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

int main (int argc, char** argv)
{
	int width = 600;
	int height = 600;

    for(int i=0; i< 50; i++)isBlock[i] = 0;

    initGLUT (argc, argv, width, height);

    addGLUTMenus ();

	initGL (width, height);

    glutMainLoop ();

    return 0;
}
