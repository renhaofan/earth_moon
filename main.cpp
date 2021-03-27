#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cassert>
#include <sstream>
#include <stdexcept>
#include <ctime>

#include <Eigen/Core>
#include <Eigen/Dense>
#include <Eigen/Geometry>
#include <Eigen/LU> 

#include <Eigen/SparseCore>
#include <Eigen/SparseLU>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <GL/glut.h>
#include <GL/glu.h>
#include <GL/gl.h>

#include "GLCamera.hpp"

#define DEG2RAD (3.1415926535f / 180.0f)
#define RAD2DEG (180.0f / 3.1415926535f)


// reference:https://github.com/gostepbystep/EarthDemo/blob/master/main.c

GLCamera camera;

// earth concerned
float earth_rotation = 0; // day
float earth_revolution = 0; // year
unsigned int earth_texture;

// moon concerned
float moon_rotation = 0; // day
float moon_revolution = 0; // year
unsigned int moon_texture;

int frame = 0;
size_t current_time, time_stamp;
char fps_chars[50];

// keyboard concerned
GLboolean normal_keys_status[256] = { false };
GLboolean special_keys_status[256] = { false };
float normal_key_speed = 1.f;
float special_key_speed = 0.3f;

bool show_grid = true;
bool show_axes = true;

// width and height of the window
int window_height = 800;
int window_width = 1024;
int screen_height = glutGet(GLUT_SCREEN_HEIGHT);
int screen_width = glutGet(GLUT_SCREEN_WIDTH);

int render_way = 2;

// camera concerned
float camera_position[3] = { 0.f, 0.f, -10.f };
float camera_angle[3] = { 0.f, 0.f, 0.f };
float camera_axes[9] = { 1, 0, 0, 0, 1, 0, 0, 0, 1 };
float mouse_speed = 0.001f;
float mouse_delta_move[3] = { 0.f };
int x_origin = -1;

void renderBitmapString(float x, float y, float z, void *font, char *string) {
	// example: renderBitmapString(1.0f, 0.0f, 0.0f, GLUT_BITMAP_TIMES_ROMAN_24, (char*)"X");
	char *c;
	glRasterPos3f(x, y, z);
	for (c = string; *c != '\0'; c++) {
		glutBitmapCharacter(font, *c);
	}
}
void plotWorldAxes() {
	float half_length = 20.f;
	// draw axess
	glPushMatrix();
	glLineWidth(2.0f);
	glBegin(GL_LINES);

	glColor3f(1.f, 0.f, 0.f);
	glVertex3f(-half_length, 0.0f, 0.0f);
	glVertex3f(half_length, 0.0f, 0.0f);

	glColor3f(0.f, 1.f, 0.f);
	glVertex3f(0.f, -half_length, 0.0f);
	glVertex3f(0.f, half_length, 0.0f);

	glColor3f(0.f, 0.f, 1.f);
	glVertex3f(0.f, 0.f, -half_length);
	glVertex3f(0.f, 0.f, half_length);
	glEnd();
	glLineWidth(1.0f);
	glPopMatrix();
	// draw cone
	glColor3f(1.f, 1.f, 1.f);
	glPushMatrix();
	//glColor3f(1.f, 0.f, 0.f);
	glTranslatef(half_length, 0, 0);
	glRotatef(90, 0, 1, 0);
	glutSolidCone(0.1, 0.3, 16, 16);
	glPopMatrix();

	glPushMatrix();
	//glColor3f(0.f, 1.f, 0.f);
	glTranslatef(0, half_length, 0);
	glRotatef(-90, 1, 0, 0);
	glutSolidCone(0.1, 0.3, 16, 16);
	glPopMatrix();

	glPushMatrix();
	//glColor3f(0.f, 0.f, 1.f);
	glTranslatef(0, 0, half_length);
	glutSolidCone(0.1, 0.3, 16, 16);
	glPopMatrix();

	// draw sephere
	glPushMatrix();
	glTranslatef(-half_length, 0, 0);
	glutSolidSphere(0.03, 6, 6);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0, -half_length, 0);
	glutSolidSphere(0.03, 6, 6);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0, 0, -half_length);
	glutSolidSphere(0.03, 6, 6);
	glPopMatrix();

	glPushMatrix();
	renderBitmapString(half_length + 0.3f, 0.f, 0.f, GLUT_BITMAP_TIMES_ROMAN_24, (char*)"X");
	renderBitmapString(0.f, half_length + 0.3f, 0.f, GLUT_BITMAP_TIMES_ROMAN_24, (char*)"Y");
	renderBitmapString(0.f, 0.f, half_length + 0.3f, GLUT_BITMAP_TIMES_ROMAN_24, (char*)"Z");
	glPopMatrix();
}
void plotReferenceGrid(float start = 20.0f, float gridSize = 1.0f) {
	if (gridSize < 1.f || start < 0) {
		std::cerr << "less than one | start < 0" << "\n";
		return;
	}
	glPushMatrix();
	glColor3f(0.5f, 0.5, 0.5); // gray color
	for (float i = -start; i <= start; i += gridSize) {
		glBegin(GL_LINES);
		glVertex3f(-start, -1.f, i);
		glVertex3f(start, -1.f, i);
		glEnd();
		glBegin(GL_LINES);
		glVertex3f(i, -1.f, -start);
		glVertex3f(i, -1.f, start);
		glEnd();
	}
	glColor3f(1.0f, 1.0f, 1.0f);
	glPopMatrix();
}
void plotEarth() {
	glPushMatrix();
	earth_rotation += 0.01f;
	
	if (earth_rotation >= 360) earth_rotation = 0.f;
	glRotatef(earth_rotation, 0, 1, 0);
	
	glRotatef(90, 1, 0, 0);
	glBindTexture(GL_TEXTURE_2D, earth_texture);
	glBegin(GL_QUADS);//绘制四边形
	GLUquadric* quadricObj = gluNewQuadric(); //gluNewQuadric 创建一个新的二次曲面对象
	gluQuadricTexture(quadricObj, GL_TRUE);
	gluSphere(quadricObj, 10.f, 18, 36);  //参数1：二次曲面对象指针，参数2：球半径，参数3：Z轴方向片数，经度方向，参数4：Y轴方向片数，维度方向
	gluDeleteQuadric(quadricObj); //gluDeleteQuadric 删除一个二次曲面对象
	//glutSolidSphere(10.f, 18, 36);
	glPopMatrix();
}

void plotMoon() {
	glPushMatrix();
	moon_rotation += 0.1f;
	moon_revolution += 0.01f;

	if (moon_rotation >= 360) moon_rotation = 0.f;
	if (moon_revolution >= 360) moon_revolution = 0.f;


	glRotatef(moon_revolution, 0, 1, 0);
	glTranslatef(50.f, 0.f, 0);
	glRotatef(moon_rotation, 0, 1, 0);




	glRotatef(90, 1, 0, 0);
	glBindTexture(GL_TEXTURE_2D, moon_texture);
	glBegin(GL_QUADS);//绘制四边形
	GLUquadric* quadricObj = gluNewQuadric(); //gluNewQuadric 创建一个新的二次曲面对象
	gluQuadricTexture(quadricObj, GL_TRUE);
	gluSphere(quadricObj, 4.f, 18, 36);  //参数1：二次曲面对象指针，参数2：球半径，参数3：Z轴方向片数，经度方向，参数4：Y轴方向片数，维度方向
	gluDeleteQuadric(quadricObj); //gluDeleteQuadric 删除一个二次曲面对象
	glPopMatrix();
}


void plotObject() {
	// plot reference grid
	if (show_grid) plotReferenceGrid(100.f, 10.f);
	// plot world coordinate axis
	if (show_axes) plotWorldAxes();

	GLfloat white[] = { 1.0, 1.0, 1.0, 1.0 }; // 定义颜色
	GLfloat light_pos[] = { 0,0,70,1 };  //定义光源位置
	glLightfv(GL_LIGHT0, GL_POSITION, light_pos); //设置第0号光源的光照位置
	glLightfv(GL_LIGHT0, GL_AMBIENT, white); //设置第0号光源多次反射后的光照颜色（环境光颜色）


	glEnable(GL_LIGHTING); //开启光照模式
	glEnable(GL_LIGHT0); //开启第0号光源
	//glShadeModel(GL_FLAT);
	plotMoon();
	plotEarth();
	glDisable(GL_LIGHTING); 
	glDisable(GL_LIGHT0); 
}

void pressNormalKeys(unsigned char key, int x, int y) {
	switch (key) {
	case 27://key ESC
		exit(0);
		break;
	case 'w': normal_keys_status['w'] = true; break;
	case 'W': normal_keys_status['W'] = true; break;

	case 's': normal_keys_status['s'] = true; break;
	case 'S': normal_keys_status['S'] = true; break;

	case 'a': normal_keys_status['a'] = true; break;
	case 'A': normal_keys_status['A'] = true; break;

	case 'd': normal_keys_status['d'] = true; break;
	case 'D': normal_keys_status['D'] = true; break;

	case 'q': normal_keys_status['q'] = true; break;
	case 'Q': normal_keys_status['Q'] = true; break;

	case 'e': normal_keys_status['e'] = true; break;
	case 'E': normal_keys_status['E'] = true; break;

	case 'm':
	case 'M':
		render_way++;
		if (render_way >= 3) render_way = 0;
		break;
	case 'n':
	case 'N':
		show_grid = !show_grid;
		break;
	case '/':
		show_axes = !show_axes;
		break;
	case 'x':
	case 'X':
		break;
	case 'y':
	case 'Y':
		break;
	case 'z':
	case 'Z':
		break;
	}

}
void pressSpecialKeys(int key, int x, int y) {
	switch (key) {
	case GLUT_KEY_LEFT:
		special_keys_status[GLUT_KEY_LEFT] = true;
		break;
	case GLUT_KEY_RIGHT:
		special_keys_status[GLUT_KEY_RIGHT] = true;
		break;
	case GLUT_KEY_UP:
		special_keys_status[GLUT_KEY_UP] = true;
		break;
	case GLUT_KEY_DOWN:
		special_keys_status[GLUT_KEY_DOWN] = true;
		break;
	case GLUT_KEY_PAGE_UP:
		break;
	case GLUT_KEY_PAGE_DOWN:
		break;
	case GLUT_KEY_HOME:
		break;
	case GLUT_KEY_END:
		break;
	}
}
void releaseNormalKeys(unsigned char key, int x, int y) {
	switch (key) {
	case 27:
		exit(0);
		break; //key ESC
	case 'w': normal_keys_status['w'] = false; break;
	case 'W': normal_keys_status['W'] = false; break;
	case 's': normal_keys_status['s'] = false; break;
	case 'S': normal_keys_status['S'] = false; break;
	case 'a': normal_keys_status['a'] = false; break;
	case 'A': normal_keys_status['A'] = false; break;
	case 'd': normal_keys_status['d'] = false; break;
	case 'D': normal_keys_status['D'] = false; break;
	case 'q': normal_keys_status['q'] = false; break;
	case 'Q': normal_keys_status['Q'] = false; break;
	case 'e': normal_keys_status['e'] = false; break;
	case 'E': normal_keys_status['E'] = false; break;
	}
}
void releaseSpacialKeys(int key, int x, int y) {
	switch (key) {
	case GLUT_KEY_LEFT:
		special_keys_status[GLUT_KEY_LEFT] = false;
		break;
	case GLUT_KEY_RIGHT:
		special_keys_status[GLUT_KEY_RIGHT] = false;
		break;
	case GLUT_KEY_UP:
		special_keys_status[GLUT_KEY_UP] = false;
		break;
	case GLUT_KEY_DOWN:
		special_keys_status[GLUT_KEY_DOWN] = false;
		break;
	case GLUT_KEY_F1:
		glutFullScreen();
		break;
	case GLUT_KEY_F2:
		window_width = 1024;
		window_height = 800;
		glutReshapeWindow(window_width, window_height);
		break;
	case GLUT_KEY_PAGE_UP:
		break;
	case GLUT_KEY_PAGE_DOWN:
		break;
	case GLUT_KEY_HOME:
		break;
	case GLUT_KEY_END:
		break;
	}
}

void normalKeyStatus() {
	if (normal_keys_status['w'] || normal_keys_status['W']) {
		camera.ShiftForward(0.1 * normal_key_speed);
	}
	if (normal_keys_status['s'] || normal_keys_status['S']) {
		camera.ShiftBackward(0.1 * normal_key_speed);
	}
	if (normal_keys_status['a'] || normal_keys_status['A']) {
		camera.ShiftLeft(0.1 * normal_key_speed);
	}
	if (normal_keys_status['d'] || normal_keys_status['D']) {
		camera.ShiftLeft(-0.1 * normal_key_speed);
	}
	if (normal_keys_status['q'] || normal_keys_status['Q']) {
		camera.ShiftUp(0.1 * normal_key_speed);
	}
	if (normal_keys_status['e'] || normal_keys_status['E']) {
		camera.ShiftUp(-0.1 * normal_key_speed);
	}
}
void specialKeyStatus() {
	if (special_keys_status[GLUT_KEY_LEFT]) {
		camera_angle[1] += 1.f * special_key_speed;
		camera.YawV(-1.f * DEG2RAD * special_key_speed);
	}
	if (special_keys_status[GLUT_KEY_RIGHT]) {
		camera_angle[1] += -1.f * special_key_speed;
		camera.YawV(1.f * DEG2RAD * special_key_speed);
	}
	if (special_keys_status[GLUT_KEY_UP]) {
		camera_angle[0] += 1.f * special_key_speed;
		camera.PitchU(-1.f * DEG2RAD * special_key_speed);
	}
	if (special_keys_status[GLUT_KEY_DOWN]) {
		camera_angle[0] += -1.f * special_key_speed;
		camera.PitchU(1.f * DEG2RAD * special_key_speed);
	}
}
void mouseMoveStatus() {
	if (x_origin != -1) {
		camera.YawV(mouse_delta_move[0]);
	}
}

void mouseButton(int button, int state, int x, int y) {

	// only start motion if the left button is pressed
	if (button == GLUT_LEFT_BUTTON) {
		// when the buttion is released
		if (state == GLUT_UP) {
			camera_angle[0] = 0.f;
			x_origin = -1;
		}
		else { // state == GLUT_DOWN
			x_origin = x;
		}
	}
}
void mouseMove(int x, int y) {
	// this will only be true when the left button is down
	if (x_origin >= 0) {
		mouse_delta_move[0] = (x - x_origin) * mouse_speed;
	}
}

void setOrthoProjection() {
	// switch to projection mode
	glMatrixMode(GL_PROJECTION);

	// save previous matrix which contains the
	//settings for the perspective projection
	glPushMatrix();

	// reset matrix
	glLoadIdentity();

	// set a 2D orthographic projection
	gluOrtho2D(0, window_width, window_height, 0);

	// switch back to modelview mode
	glMatrixMode(GL_MODELVIEW);
}
void restorePerspProjection() {
	glMatrixMode(GL_PROJECTION);
	// restore previous projection matrix
	glPopMatrix();

	// get back to modelview mode
	glMatrixMode(GL_MODELVIEW);
}


void init() {
	glutKeyboardFunc(pressNormalKeys);
	glutSpecialFunc(pressSpecialKeys);

	glutIgnoreKeyRepeat(1);
	glutKeyboardUpFunc(releaseNormalKeys);
	glutSpecialUpFunc(releaseSpacialKeys);

	// mouse
	glutMouseFunc(mouseButton);
	glutMotionFunc(mouseMove);

	// OpenGL init
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_2D);
	glShadeModel(GL_SMOOTH);

	//glEnable(GL_TEXTURE_2D); 				// 启用纹理映射
	//glShadeModel(GL_SMOOTH); 				// 启用阴影平滑
	//glClearColor(0.0f, 0.0f, 0.0f, 0.5f); 	// 黑色背景
	//glClearDepth(1.0f); 					// 设置深度缓存
	//glEnable(GL_DEPTH_TEST); 				// 启用深度测试
	//glDepthFunc(GL_LEQUAL); 				// 所作深度测试的类型
	//glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST); // 真正精细的透视修正
}

void changeSize(int w, int h) {
	window_height = h;
	window_width = w;
	// Prevent a divide by zero, when window is too short
	// (you cant make a window of zero width).
	if (h == 0)
		h = 1;
	float ratio = w * 1.0 / h;

	// Use the Projection Matrix
	glMatrixMode(GL_PROJECTION);

	// Reset Matrix
	glLoadIdentity();

	// Set the viewport to be the entire window
	glViewport(0, 0, w, h);

	// Set the correct perspective.
	gluPerspective(45.0f, ratio, 0.1, 2000.0);
	//gluPerspective(45.0f, ratio, 1, 10.0);


	// Get Back to the Modelview
	glMatrixMode(GL_MODELVIEW);
}
void idle() {
	glutPostRedisplay();//调用当前绘制函数 
}
void renderScene() {
	// Clear Color and Depth Buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	switch (render_way) {
	case 0:
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
		break;
	case 1:
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		break;
	case 2:
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		break;
	default:
		break;
	}


	normalKeyStatus();
	specialKeyStatus();
	mouseMoveStatus();
	// Reset transformations
	glLoadIdentity();

	float m[16];
	camera.LoadToGLMatrix(m);
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(m);

	// display fps in the window
	frame++;
	current_time = glutGet(GLUT_ELAPSED_TIME); // milisecond
	if (current_time - time_stamp > 1000) {
		sprintf(fps_chars, "FPS:%4.2f", frame * 1000.f / (current_time - time_stamp));
		time_stamp = current_time;
		frame = 0;
	}

	// display fps
	setOrthoProjection();
	void *font = GLUT_BITMAP_8_BY_13;
	glPushMatrix();
	glLoadIdentity();
	glColor3f(1.f, 1.f, 1.f);
	renderBitmapString(5, 40, 0, font, fps_chars);
	renderBitmapString(30, 70, 0, font, (char *)"F1 - Full Screen");
	renderBitmapString(30, 100, 0, font, (char *)"F2 - Window Mode");
	renderBitmapString(30, 130, 0, font, (char *)"Esc - Quit");
	glPopMatrix();
	restorePerspProjection();


	plotObject();

	glutSwapBuffers();
}



int main(int argc, char ** argv) {
	camera.LookAt(70, 0, 70,
		0, 0, 0,
		0, 1, 0);
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(window_width, window_height);
	glutCreateWindow("Tutorial");

	// callbacks for main window
	glutDisplayFunc(renderScene);
	glutReshapeFunc(changeSize);
	glutIdleFunc(idle);
	init();

	// load and create a texture 
// -------------------------

	glGenTextures(1, &earth_texture);
	glBindTexture(GL_TEXTURE_2D, earth_texture); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load image, create texture and generate mipmaps
	int width, height, nrChannels;
	// The FileSystem::getPath(...) is part of the GitHub repository so we can find files on any IDE/platform; replace it with your own image path.
	//unsigned char *data = stbi_load("container.jpg", &width, &height, &nrChannels, 0);
	unsigned char *data = stbi_load("images/earth2048.bmp", &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		//glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);

	glGenTextures(1, &moon_texture);
	glBindTexture(GL_TEXTURE_2D, moon_texture); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load image, create texture
	data = stbi_load("images/moon1024.bmp", &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		//glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);












	// enter GLUT event processing cycle
	glutMainLoop();


	system("pause");
	return 0;
}

