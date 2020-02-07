#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "common.h"

#include <iostream>
#include <vector>


const char *WINDOW_TITLE = "Sphere";
const double FRAME_RATE_MS = 1000.0 / 60.0;
const float PI = 3.1415926f;
const float TWO_PI = PI * 2;

// Lists
std::vector<GLfloat> vertices;
std::vector<GLuint> indices;


// Array of rotation angles (in degrees) for each coordinate axis
enum { Xaxis = 0, Yaxis = 1, Zaxis = 2, NumAxes = 3 };
int      Axis = Xaxis;
GLfloat  Theta[NumAxes] = { 0.0, 0.0, 0.0 };

GLuint  ModelView, Projection;

// UI Sphere variables
float radius = 0.5f;
int circlePoints = 22; // EVEN (for checkerboard pattern)        
int arcPoints = 21; // ODD (for checkerboard pattern), remember that north and south polls have duplicate points for each point on a circle	 	  

float g = 9.8;
float mass = 0.0001;
float velocity = 150;
float gravity = -mass * g;
float impulse = mass * velocity;

glm::vec3 currPosition(0, 0, 0);
glm::vec3 vCurrent(impulse, impulse, 0);
bool rest = false;

// Borders
float ground = -1.0f+radius;
float walls[2] = { -1.0f + radius, 1.0f - radius };

//----------------------------------------------------------------------------

// OpenGL initialization
void
init()
{
	// iterate points on a 2d circle 
	for (float i = 0; i <= circlePoints; i++) {

		float t = i / (circlePoints);
		float phi = t * TWO_PI;

		// iterate points on an arc from (0,0,-z) to (0,0,+z)
		for (float j = 0; j < arcPoints; j++) {

			float t = j / (arcPoints-1);
			float theta = t * PI;

			// get spherical coordinates
			// formulas: https://en.wikipedia.org/wiki/Spherical_coordinate_system
			GLfloat x = radius * sin(theta) * cos(phi);
			GLfloat y = radius * sin(theta) * sin(phi);
			GLfloat z = radius * cos(theta);

			vertices.push_back(x);
			vertices.push_back(y);
			vertices.push_back(z);
			vertices.push_back(1.0f);
		}
	}

	// find triangles that form a sphere 
	int squares = arcPoints * (circlePoints)-1;

	for (int i = 0; i <= squares; i++) {

		if (i % arcPoints != arcPoints-1) // avoid north poll points
		{
			// right triangle
			indices.push_back(GLuint(i));
			indices.push_back(GLuint(i + arcPoints + 1));
			indices.push_back(GLuint(i + 1));

			// left triangle
			indices.push_back(GLuint(i));
			indices.push_back(GLuint(i + arcPoints));
			indices.push_back(GLuint(i + arcPoints + 1));
		}
	}

	// Bind and create Vertex Array Objects
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Create Vertex Buffer Object
	GLuint buffer;

	// Load geomerty to GPU
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indices.size(), indices.data(), GL_STATIC_DRAW);

	// Load Shader
	GLuint shader = InitShader("vshader.glsl", "fshader.glsl");
	glUseProgram(shader);

	// Set up vertex arrays
	GLuint vPosition = glGetAttribLocation(shader, "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	ModelView = glGetUniformLocation(shader, "ModelView");
	Projection = glGetUniformLocation(shader, "Projection");

	
	glProvokingVertex(GL_FIRST_VERTEX_CONVENTION);

	glEnable(GL_DEPTH_TEST);
	glClearColor(1.0, 1.0, 1.0, 1.0);
}

//----------------------------------------------------------------------------

void
display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//  Generate the model-view matrix

	const glm::vec3 viewer_pos(0.0, 0.0, 2.7);
	glm::mat4 trans, rot, model_view;
	trans = glm::translate(trans, glm::vec3(currPosition.x,currPosition.y,0));
	trans = glm::translate(trans, -viewer_pos);
	rot = glm::rotate(rot, glm::radians(Theta[Xaxis]), glm::vec3(1, 0, 0));
	rot = glm::rotate(rot, glm::radians(Theta[Yaxis]), glm::vec3(0, 1, 0));
	rot = glm::rotate(rot, glm::radians(Theta[Zaxis]), glm::vec3(0, 0, 1));
	model_view = trans * rot;

	glUniformMatrix4fv(ModelView, 1, GL_FALSE, glm::value_ptr(model_view));

	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

	//for (int i = 0; i < indices.size(); i += 3) {
	//    glDrawElements(GL_LINE_LOOP, 3, GL_UNSIGNED_INT, (void*)(i * sizeof(GLuint)));
	//}

	glutSwapBuffers();
}

//----------------------------------------------------------------------------

bool pause = true;

void
keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case 033: // Escape Key
	case 'q': case 'Q':
		exit(EXIT_SUCCESS);
		break;
	case ' ':  // hold
		pause = !pause;	
		break;
	case 'w':  
		rest = false;
		vCurrent.y += impulse;
		break;
	case 'a':  
		rest = false;
		vCurrent.x -= impulse;
		break;
	case 's':  
		rest = false;
		vCurrent.y -= impulse;
		break;
	case 'd':  
		rest = false;
		vCurrent.x += impulse;
		break;
	case 'r': // restart
		rest = false;
		currPosition = glm::vec3(0, 0, 0);
		vCurrent = glm::vec3(impulse, impulse, 0);
		break;

	}
}


//----------------------------------------------------------------------------

void
mouse(int button, int state, int x, int y)
{
	if (state == GLUT_DOWN) {
		switch (button) {
		case GLUT_LEFT_BUTTON:    Axis = Xaxis;  break;
		case GLUT_MIDDLE_BUTTON:  Axis = Yaxis;  break;
		case GLUT_RIGHT_BUTTON:   Axis = Zaxis;  break;
		}
	}
}

//----------------------------------------------------------------------------

void
update(void)
{
	if (!pause && !rest) {
		Theta[Axis] += 0.5;

		if (Theta[Axis] > 360.0) {
			Theta[Axis] -= 360.0;
		}
				
		// apply forces to move 
		currPosition += vCurrent;
		currPosition.y += gravity;			
					   
		// collision detection 
		if (currPosition.y <= ground) {
			currPosition.y = ground; 
			vCurrent.y = -vCurrent.y;
			if (abs(vCurrent.y) <= abs(gravity))
			{
				rest = true;
				vCurrent = glm::vec3(0, 0, 0);
			}
		}
		if (currPosition.x >= walls[1]) {
			currPosition.x = walls[1];
			vCurrent.x = -vCurrent.x;
		}
		else if (currPosition.x <= walls[0]) {
			currPosition.x = walls[0];
			vCurrent.x = -vCurrent.x;
		}
		
		// apply gravity to a new force
		vCurrent.y += gravity;		
	}
	
}

//----------------------------------------------------------------------------

void
reshape(int width, int height)
{
	glViewport(0, 0, width, height);

	GLfloat aspect = GLfloat(width) / height;
	glm::mat4  projection = glm::perspective(glm::radians(45.0f), aspect, 0.5f, 3.0f);

	glUniformMatrix4fv(Projection, 1, GL_FALSE, glm::value_ptr(projection));
}
