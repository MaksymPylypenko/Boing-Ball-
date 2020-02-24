#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "common.h"

#include <iostream>
#include <vector>


const char* WINDOW_TITLE = "Sphere";
const double FRAME_RATE_MS = 1000.0 / 60.0;
const float PI = 3.1415926f;
const float TWO_PI = PI * 2;

// Lists
std::vector<GLfloat> vertices;
std::vector<GLfloat> normals;
std::vector<GLuint> indices;


// Array of rotation angles (in degrees) for each coordinate axis
enum { Xaxis = 0, Yaxis = 1, Zaxis = 2, NumAxes = 3 };
int      Axis = Xaxis;
GLfloat  Theta[NumAxes] = { 0.0, 0.0, 0.0 };

// Uniforms
GLuint ViewCamera;
GLuint ViewSphere, ViewSphereInvTra, sphereIndex;
GLuint ViewGround, ViewGroundInvTra, groundIndex, wallIndex;
GLuint ViewGroundShadow, groundShadowIndex;
GLuint ViewWallShadow, wallShadowIndex;
GLuint Projection;
GLboolean UseLighting;

// Physics
float g = 9.8f;
float mass = 0.0002f;
float velocity = 150;
float gravity = -mass * g;
float impulse = mass * velocity;

// Default settings
float radius = 0.5f;
glm::vec3 currPosition(0, 1, -1);
glm::vec3 vCurrent(impulse, impulse, impulse);
bool rest = false;
bool pause = false;
int view = 0;

glm::vec3 lightPositionTop(0.0f, 20.0f, 0.0f);
glm::vec3 lightPositionNear(0.0f, +1.5f, 20.0f);

// Borders
float ground = -2;
float walls[4] = { -2.0f, 2.0f , -2.0f, 1.9f };
enum { leftWall = 0, rightWall = 1, farWall = 2, nearWall = 3 };

int
makeGround(int lastIndex)
{
	float xLength = 4.0;
	float yLength = 4.0;

	float xPoints = 31; // must be odd for checkerboard pattern
	float yPoints = yLength / xLength * xPoints; // normalize squares (each side is equal)


	float dx = xLength / xPoints;
	float dy = yLength / yPoints;
	float offsetX = -xLength / 2 + dx / 2; // center at 0,0,0
	float offsetY = -2.0f + dy;

	glm::vec3 n(0, -1, 0);

	for (int i = 0; i < yPoints; i++) {
		GLfloat y = i * dx;

		for (float j = 0; j < xPoints; j++) {
			GLfloat z = 0;
			GLfloat x = j * dy;

			vertices.push_back(x + offsetX);
			vertices.push_back(y + offsetY);
			vertices.push_back(z);
			vertices.push_back(1);

			normals.push_back(n.x);
			normals.push_back(n.y);
			normals.push_back(n.z);
		}
	}

	for (int i = 0; i < (round(yPoints) - 1) * round(xPoints); i++) {

		if (i % (int)xPoints != xPoints - 1) // edge
		{
			// right triangle
			indices.push_back(GLuint(lastIndex + i));
			indices.push_back(GLuint(lastIndex + i + xPoints + 1));
			indices.push_back(GLuint(lastIndex + i + 1));

			// left triangle
			indices.push_back(GLuint(lastIndex + i));
			indices.push_back(GLuint(lastIndex + i + xPoints));
			indices.push_back(GLuint(lastIndex + i + xPoints + 1));
		}
	}

	return vertices.size() / 4; // will be used in the shader
}


int
makeWall(int lastIndex)
{
	float xLength = 4.0;
	float zLength = 4.0;

	float xPoints = 31; // must be odd for checkerboard pattern
	float zPoints = zLength / xLength * xPoints; // normalize squares (each side is equal)

	float dx = xLength / xPoints;
	float dz = zLength / zPoints;
	float offsetX = -xLength / 2 + dx / 2; // center at 0,0,0
	float offsetY = 2.0;
	float offsetZ = 0;

	glm::vec3 n(0, 0, 1);

	for (int i = 0; i < zPoints; i++) {
		GLfloat z = i * dz + offsetZ;

		for (float j = 0; j < xPoints; j++) {
			GLfloat x = j * dx;
			GLfloat y = offsetY;

			vertices.push_back(x + offsetX);
			vertices.push_back(y);
			vertices.push_back(z);
			vertices.push_back(1);

			normals.push_back(n.x);
			normals.push_back(n.y);
			normals.push_back(n.z);
		}
	}

	for (int i = 0; i < (round(zPoints) - 1) * round(xPoints); i++) {

		if (i % (int)xPoints != xPoints - 1) // edge
		{
			// right triangle
			indices.push_back(GLuint(lastIndex + i));
			indices.push_back(GLuint(lastIndex + i + xPoints + 1));
			indices.push_back(GLuint(lastIndex + i + 1));

			// left triangle
			indices.push_back(GLuint(lastIndex + i));
			indices.push_back(GLuint(lastIndex + i + xPoints));
			indices.push_back(GLuint(lastIndex + i + xPoints + 1));
		}
	}

	return vertices.size() / 4; // will be used in the shader
}

int
makeSphere(int lastIndex, float radius)
{
	// UI Sphere variables	
	int circlePoints = 22; // EVEN (for checkerboard pattern)        
	int arcPoints = 15; // ODD (for checkerboard pattern), remember that north and south polls have duplicate points for each point on a circle	 	  

	for (float i = 0; i <= circlePoints; i++) {

		float t = i / (circlePoints);
		float phi = t * TWO_PI;

		// iterate points on an arc from (0,0,-z) to (0,0,+z)
		for (float j = 0; j < arcPoints; j++) {

			float t = j / (arcPoints - 1);
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

			glm::vec3 n = normalize(glm::vec3(x, y, z));

			normals.push_back(n.x);
			normals.push_back(n.y);
			normals.push_back(n.z);
		}
	}

	int squares = arcPoints * (circlePoints)-1;

	for (int i = 0; i <= squares; i++) {

		if (i % arcPoints != arcPoints - 1) // avoid north poll points
		{
			// right triangle
			indices.push_back(GLuint(lastIndex + i));
			indices.push_back(GLuint(lastIndex + i + arcPoints + 1));
			indices.push_back(GLuint(lastIndex + i + 1));

			// left triangle
			indices.push_back(GLuint(lastIndex + i));
			indices.push_back(GLuint(lastIndex + i + arcPoints));
			indices.push_back(GLuint(lastIndex + i + arcPoints + 1));
		}
	}
	return vertices.size() / 4; // will be used in the shader
}

int makeWallShadow(int lastIndex, float radius) {
	GLfloat centerX = 0;
	GLfloat centerY = 0;

	int circlePoints = 22;

	vertices.push_back(centerX);
	vertices.push_back(centerY);
	vertices.push_back(0);
	vertices.push_back(1.0f);

	// glm::vec3 n(0, -1, 0);
	//normals.push_back(n.x);
	//normals.push_back(n.y);
	//normals.push_back(n.z);


	for (float i = 0; i <= circlePoints; i++) {
		float t = i / (circlePoints);
		float phi = t * TWO_PI;

		GLfloat x = centerX + radius * cos(phi);
		GLfloat y = centerY + radius * sin(phi);
		GLfloat z = 0;

		vertices.push_back(x);
		vertices.push_back(y);
		vertices.push_back(z);
		vertices.push_back(1.0f);

		// The code will probably break if i dont have normals for every object					
		/*normals.push_back(n.x);
		normals.push_back(n.y);
		normals.push_back(n.z);*/
	}

	for (int i = 0; i < circlePoints; i++) {

		// triangle
		indices.push_back(GLuint(lastIndex + 1)); // center
		indices.push_back(GLuint(lastIndex + 1 + i)); // on circle
		indices.push_back(GLuint(lastIndex + 1 + i + 1)); // next on circle			
	}
	return vertices.size() / 4; // will be used in the shader
}

int makeGroundShadow(int lastIndex, float radius) {
	GLfloat centerX = 0;
	GLfloat centerY = 0;

	int circlePoints = 22;

	vertices.push_back(centerX);
	vertices.push_back(centerY);
	vertices.push_back(0);
	vertices.push_back(1.0f);

	//glm::vec3 n(0, -1, 0);
	//normals.push_back(n.x);
	//normals.push_back(n.y);
	//normals.push_back(n.z);


	for (float i = 0; i <= circlePoints; i++) {
		float t = i / (circlePoints);
		float phi = t * TWO_PI;

		GLfloat x = centerX + radius * cos(phi);
		GLfloat y = centerY + radius * sin(phi);
		GLfloat z = 0;

		vertices.push_back(x);
		vertices.push_back(z);
		vertices.push_back(y);
		vertices.push_back(1.0f);

		// The code will probably break if i dont have normals for every object	
	/*	normals.push_back(n.x);
		normals.push_back(n.y);
		normals.push_back(n.z);*/
	}

	for (int i = 0; i < circlePoints; i++) {

		// triangle
		indices.push_back(GLuint(lastIndex + 1)); // center
		indices.push_back(GLuint(lastIndex + 1 + i)); // on circle
		indices.push_back(GLuint(lastIndex + 1 + i + 1)); // next on circle			
	}
	return vertices.size() / 4; // will be used in the shader
}


void initLight(GLuint shader) {
	// Initialize shader lighting parameters
	glm::vec4 light_position_top(lightPositionTop.x, lightPositionTop.y, lightPositionTop.z, 0.0);
	glm::vec4 light_position_near(lightPositionNear.x, lightPositionNear.y, lightPositionNear.z, 0.0);
	glm::vec4 light_ambient(0.9, 0.9, 0.9, 1.0); // 0.9 looks fine
	glm::vec4 light_diffuse(0.07, 0.07, 0.07, 1.0);
	glm::vec4 light_specular(0.05, 0.05, 0.05, 1.0);

	UseLighting = glGetUniformLocation(shader, "UseLighting");
	glUniform1f(UseLighting, true);

	glUniform4fv(glGetUniformLocation(shader, "AmbientLight"), 1, glm::value_ptr(light_ambient));
	glUniform4fv(glGetUniformLocation(shader, "DiffuseLight"), 1, glm::value_ptr(light_diffuse));
	glUniform4fv(glGetUniformLocation(shader, "SpecularLight"), 1, glm::value_ptr(light_specular));
	glUniform4fv(glGetUniformLocation(shader, "lightPositionTop"), 1, glm::value_ptr(light_position_top));
	glUniform4fv(glGetUniformLocation(shader, "lightPositionNear"), 1, glm::value_ptr(light_position_near));
}
//----------------------------------------------------------------------------

// OpenGL initialization
void
init()
{
	int endOfGround = makeGround(0);
	int endOfWall = makeWall(endOfGround); // I really should just rotate ground instead... 
	int endOfSpehre = makeSphere(endOfWall, radius);
	int enfOfGroundShadow = makeGroundShadow(endOfSpehre, radius);
	int enfOfWallShadow = makeWallShadow(enfOfGroundShadow, radius);

	// Bind and create Vertex Array Objects
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Create Vertex Buffer Object
	GLuint buffer;

	// Load geomerty to GPU
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);

	//glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * (vertices.size() + normals.size()), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * vertices.size(), vertices.data());
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vertices.size(), sizeof(GLfloat) * normals.size(), normals.data());

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

	GLuint vNormal = glGetAttribLocation(shader, "vNormal");
	glEnableVertexAttribArray(vNormal);
	glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(GLuint) * vertices.size()));

	// Retrieve transformation uniform variable locations
	ViewCamera = glGetUniformLocation(shader, "ViewCamera");
	ViewSphere = glGetUniformLocation(shader, "ViewSphere");
	ViewGround = glGetUniformLocation(shader, "ViewGround");
	ViewGroundShadow = glGetUniformLocation(shader, "ViewGroundShadow");
	ViewWallShadow = glGetUniformLocation(shader, "ViewWallShadow");
	ViewSphereInvTra = glGetUniformLocation(shader, "ViewSphereInvTra");
	ViewGroundInvTra = glGetUniformLocation(shader, "ViewGroundInvTra");
	Projection = glGetUniformLocation(shader, "Projection");

	groundIndex = glGetUniformLocation(shader, "groundIndex");
	wallIndex = glGetUniformLocation(shader, "wallIndex");
	sphereIndex = glGetUniformLocation(shader, "sphereIndex");
	groundShadowIndex = glGetUniformLocation(shader, "groundShadowIndex");
	glUniform1i(groundIndex, endOfGround);
	glUniform1i(wallIndex, endOfWall);
	glUniform1i(sphereIndex, endOfSpehre);
	glUniform1i(groundShadowIndex, enfOfGroundShadow);

	initLight(shader);

	glProvokingVertex(GL_FIRST_VERTEX_CONVENTION);

	glEnable(GL_DEPTH_TEST);
	glClearColor(1.0, 1.0, 1.0, 1.0);

}

//----------------------------------------------------------------------------

void
display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::vec3 viewer_pos(0.0, 0.0, 6.9);


	//  Generate model-view matrices
	glm::mat4 view_sphere;
	{
		glm::mat4 trans, rot;
		trans = glm::translate(trans, currPosition);
		trans = glm::translate(trans, -viewer_pos);

		rot = glm::rotate(rot, glm::radians(-20.0f), glm::vec3(0, 0, 1)); // 3
		rot = glm::rotate(rot, glm::radians(90.0f), glm::vec3(1, 0, 0)); // 2
		rot = glm::rotate(rot, glm::radians(Theta[Yaxis]), glm::vec3(0, 0, 1)); // 1	
		view_sphere = trans * rot;
	}

	glm::mat4 view_wall_shadow;
	{
		glm::vec3 u = glm::normalize(lightPositionNear + currPosition);
		float steps = lightPositionNear.z / u.z; // how much longer than normalized
		float magicFactor = 0.3; // pretend that the light is further than it actually is 
		float dy = u.y * steps * magicFactor; // undo normalization 

		glm::mat4 trans;
		glm::vec3 change(currPosition.x, currPosition.y + dy, walls[farWall] + 0.01);
		trans = glm::translate(trans, change);
		trans = glm::translate(trans, -viewer_pos);
		view_wall_shadow = trans;
	}

	glm::mat4 view_ground_shadow;
	{
		glm::vec3 u = glm::normalize(lightPositionTop + currPosition);
		float steps = lightPositionTop.y / u.y; // how much longer than normalized
		float magicFactor = 0.3; // pretend that the light is further than it actually is 
		float dx = u.x * steps * magicFactor; // undo normalization 

		glm::mat4 trans;
		glm::vec3 change(currPosition.x + dx, ground + 0.01, currPosition.z);
		trans = glm::translate(trans, change);
		trans = glm::translate(trans, -viewer_pos);
		view_ground_shadow = trans;
	}

	glm::mat4 view_ground;
	{
		glm::mat4 trans, rot;
		trans = glm::translate(trans, glm::vec3(0, -2, 0));
		trans = glm::translate(trans, -viewer_pos);
		rot = glm::rotate(rot, glm::radians(-90.0f), glm::vec3(1, 0, 0));
		view_ground = trans * rot;
	}

	glm::mat4 view_camera;
	{
		glm::mat4 rot, trans;
		trans = glm::mat4();
		rot = glm::mat4();

		if (view == 1)
		{
			trans = glm::translate(trans, glm::vec3(0, -7.5, -5.3));
			rot = glm::rotate(rot, glm::radians(80.0f), glm::vec3(1, 0, 0)); // 3		
		}
		else if (view == 2)
		{
			trans = glm::translate(trans, glm::vec3(7, 0, -7));
			rot = glm::rotate(rot, glm::radians(90.0f), glm::vec3(0, 1, 0)); // 3	
		}

		view_camera = trans * rot;
	}

	// Can make more efficient by sending computed matricies
	// However this would require additional variables...
	//view_sphere = view_camera * view_sphere;
	//view_ground = view_camera * view_ground;
	//view_wall_shadow = view_camera * view_wall_shadow;
	//view_ground_shadow = view_camera * view_ground_shadow;

	glUniformMatrix4fv(ViewCamera, 1, GL_FALSE, glm::value_ptr(view_camera));
	glUniformMatrix4fv(ViewSphere, 1, GL_FALSE, glm::value_ptr(view_sphere));
	glUniformMatrix4fv(ViewGround, 1, GL_FALSE, glm::value_ptr(view_ground));
	glUniformMatrix4fv(ViewGroundShadow, 1, GL_FALSE, glm::value_ptr(view_ground_shadow));
	glUniformMatrix4fv(ViewWallShadow, 1, GL_FALSE, glm::value_ptr(view_wall_shadow));
	glUniformMatrix4fv(ViewSphereInvTra, 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(view_sphere))));
	glUniformMatrix4fv(ViewGroundInvTra, 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(view_ground))));

	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

	//for (int i = 0; i < indices.size(); i += 3) {
	//    glDrawElements(GL_LINE_LOOP, 3, GL_UNSIGNED_INT, (void*)(i * sizeof(GLuint)));
	//}

	glutSwapBuffers();
}

//----------------------------------------------------------------------------

void
keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case 033: // Escape Key
	case 'q': case 'Q':
		exit(EXIT_SUCCESS);
		break;
	case ' ':  // hold
		view++;
		if (view > 2) {
			view = 0;
		}
		break;
	case 'e':  // hold
		pause = !pause;
		break;
	case 'w':
		rest = false;
		vCurrent.z -= impulse;
		break;
	case 'a':
		rest = false;
		vCurrent.x -= impulse;
		break;
	case 's':
		rest = false;
		vCurrent.z += impulse;
		break;
	case 'd':
		rest = false;
		vCurrent.x += impulse;
		break;
	case 'j':
		rest = false;
		vCurrent.y += impulse;
		break;
	case 'k':
		rest = false;
		vCurrent.y = 0;
		vCurrent.x = 0;
		vCurrent.z = 0;
		break;
	case 'r': // restart
		rest = false;
		currPosition = glm::vec3(0, 1, -1);
		vCurrent = glm::vec3(impulse, impulse, impulse);
		break;

	}
}


//----------------------------------------------------------------------------

void
mouse(int button, int state, int x, int y)
{
	if (state == GLUT_DOWN) {
		//switch (button) {
		//	case GLUT_LEFT_BUTTON:    Axis = Xaxis;  break;
		//	case GLUT_MIDDLE_BUTTON:  Axis = Yaxis;  break;
		//	case GLUT_RIGHT_BUTTON:   Axis = Zaxis;  break;
		//}
	}
}

//----------------------------------------------------------------------------

bool clockwiseRotation = true;

void
update(void)
{


	if (!pause && !rest) {
		if (clockwiseRotation) {
			Theta[Yaxis] -= 2;
		}
		else {
			Theta[Yaxis] += 2;
		}

		if (Theta[Yaxis] > 360.0) {
			Theta[Yaxis] -= 360.0;
		}
		else if (Theta[Yaxis] < 0) {
			Theta[Yaxis] += 360.0;
		}


		// apply forces to move 
		currPosition += vCurrent;
		currPosition.y += gravity;

		// collision detection 
		if (currPosition.y <= ground + radius) {
			currPosition.y = ground + radius;
			vCurrent.y = -vCurrent.y;
			if (abs(vCurrent.y) <= abs(gravity))
			{
				rest = true;
				vCurrent = glm::vec3(0, 0, 0);
			}
		}
		if (currPosition.x >= walls[rightWall] - radius) {
			currPosition.x = walls[rightWall] - radius;
			vCurrent.x = -vCurrent.x;
		}
		else if (currPosition.x <= walls[leftWall] + radius) {
			currPosition.x = walls[leftWall] + radius;
			vCurrent.x = -vCurrent.x;
		}

		if (currPosition.z >= walls[nearWall] - radius) {
			currPosition.z = walls[nearWall] - radius;
			vCurrent.z = -vCurrent.z;
		}
		else if (currPosition.z <= walls[farWall] + radius) {
			currPosition.z = walls[farWall] + radius;
			vCurrent.z = -vCurrent.z;
			//clockwiseRotation = !clockwiseRotation;

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
	glm::mat4  projection = glm::perspective(glm::radians(45.0f), aspect, 0.5f, 20.0f);

	projection = projection;
	glUniformMatrix4fv(Projection, 1, GL_FALSE, glm::value_ptr(projection));
}
