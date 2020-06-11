#include "GL/freeglut.h"
#include "opencv2/opencv.hpp"
#include <iostream>
#include <string>

using namespace cv;
using namespace std;


// Video Varables
VideoCapture* vid;		// video capture obj
Mat cam_frame;			// web cam frame obj
int screenW;					// screen width
int screenH;					// screen height


// GLUT variables
GLuint texture_background, texture_cube;
float cubeAngle = 0;


// Convert Function : OpenCV Mat -> OpenGL Texture
GLuint MatToTexture(Mat frame)
{
	// ������ ��� �ִٸ� ����
	if (frame.empty())
	{
		cout << "[ERROR] Image is empty." << endl;
		exit(100);
	}

	// OpenGL Texture ����
	GLuint textureID;
	glGenTextures(1, &textureID);

	// Texture ID�� ���ε� : ����� �ؽ�ó ������ ����
	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frame.cols, frame.rows,
		0, GL_RGB, GL_UNSIGNED_BYTE, frame.ptr());

	// Texture�� ��������.
	return textureID;
}


// Draw Background
void draw_background()
{
	int x = screenH / 100.0;
	int y = screenW / 100.0;

	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 1.0); glVertex3f(-x, -y, 0.0);
	glTexCoord2f(1.0, 1.0); glVertex3f(x, -y, 0.0);
	glTexCoord2f(1.0, 0.0); glVertex3f(x, y, 0.0);
	glTexCoord2f(0.0, 0.0); glVertex3f(-x, y, 0.0);
	glEnd();
}


void drawBitmapText(char* str, float x, float y, float z)
{
	glRasterPos3f(x, y, z); //���ڿ��� �׷��� ��ġ ����

	while (*str)
	{
		//GLUT_BITMAP_TIMES_ROMAN_24 ��Ʈ�� ����Ͽ� ���ڿ��� �׸���.
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *str);

		str++;
	}
}


//ť���� �� ��, ȭ�� ���� ������ -Z��������� 0.5�̵��Ͽ� ���簢���� �׸���.
static void cubebase(void)
{
	glBegin(GL_QUADS);
	glVertex3d(-0.5, -0.5, -0.5);
	glVertex3d(-0.5, 0.5, -0.5);
	glVertex3d(0.5, 0.5, -0.5);
	glVertex3d(0.5, -0.5, -0.5);
	glEnd();
}

//cubebase�Լ����� �׸� �簢���� ȸ�� �� �̵�����
//ť�긦 �ϼ���Ų��.
void draw_cube()
{
	glPushMatrix();

	glColor3f(0.0f, 1.0f, 0.0f);     // Green, -Z�� ����
	cubebase();

	glPushMatrix();

	glTranslated(1.0, 0.0, 0.0);
	glRotated(90.0, 0.0, 1.0, 0.0);
	glColor3f(0.0f, 0.0f, 1.0f);     // Blue, +X�� ����
	cubebase();

	glPopMatrix();

	glPushMatrix();
	glTranslated(-1.0, 0.0, 0.0);
	glRotated(-90.0, 0.0, 1.0, 0.0);
	glColor3f(1.0f, 0.5f, 0.0f);     // Orange, -X�� ����
	cubebase();
	glPopMatrix();

	glPushMatrix();
	glTranslated(0.0, 1.0, 0.0);
	glRotated(-90.0, 1.0, 0.0, 0.0);
	glColor3f(1.0f, 0.0f, 0.0f);     // Red, +Y�� ����
	cubebase();
	glPopMatrix();

	glPushMatrix();
	glTranslated(0.0, -1.0, 0.0);
	glRotated(90.0, 1.0, 0.0, 0.0);
	glColor3f(1.0f, 1.0f, 0.0f);     // Yellow, -Y�� ����
	cubebase();
	glPopMatrix();

	glBegin(GL_QUADS);
	glColor3f(1.0f, 0.0f, 1.0f);     // Magenta, +Z�� ����
	glVertex3d(-0.5, -0.5, 0.5);
	glVertex3d(0.5, -0.5, 0.5);
	glVertex3d(0.5, 0.5, 0.5);
	glVertex3d(-0.5, 0.5, 0.5);
	glEnd();

	glPopMatrix();

	glFlush();
}

void draw_line()
{

	glPushMatrix(); //X�� ������
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_LINES);
	glVertex3f(1.0, 0.0, 0.0);
	glVertex3f(-1.0, 0.0, 0.0);
	glEnd();
	//	drawBitmapText('+X', 0.8, 0.0, 0.0);
	//	drawBitmapText('-X', -0.8, 0.0, 0.0);
	glPopMatrix();

	glPushMatrix(); //Y�� ���
	glColor3f(0.0, 1.0, 0.0);
	glBegin(GL_LINES);
	glVertex3f(0.0, 1.0, 0.0);
	glVertex3f(0.0, -1.0, 0.0);
	glEnd();
	//	drawBitmapText('+Y', 0.0, 0.8, 0.0);
	//	drawBitmapText('-Y', 0.0, -0.8, 0.0);
	glPopMatrix();

	glPushMatrix(); //Z�� �Ķ���
	glColor3f(0.0, 0.0, 1.0);
	glBegin(GL_LINES);
	glVertex3f(0.0, 0.0, 1.0);
	glVertex3f(0.0, 0.0, -1.0);
	glEnd();
	//	drawBitmapText('+Z', 0.0, 0.0, 0.8);
	//	drawBitmapText('-Z', 0.0, 0.0, -0.8);
	glPopMatrix();


	glFlush();
}

void display()
{
	//ȭ���� �����. (�÷����ۿ� ���̹���)
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//���� ������ ModelView Matirx�� ������ �ش�.
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	texture_background = MatToTexture(cam_frame);
	if (texture_background < 0) return;


	glEnable(GL_TEXTURE_2D);
	glColor3f(1.0f, 1.0f, 1.0f); //ť�곪 ��ǥ�� �׸� �� ����� ���� ������ �ȹ������� �ʿ�
	glBindTexture(GL_TEXTURE_2D, texture_background);
	glPushMatrix();
	glTranslatef(0.0, 0.0, -9.0);
	draw_background(); //���׸�
	glPopMatrix();


	glDisable(GL_TEXTURE_2D);
	glPushMatrix();
	glTranslatef(0.0, 0.0, -4.0);
	glRotatef(cubeAngle, 1.0, 1.0, 1.0);
	draw_cube(); //ť��
	draw_line();  //��ǥ��
	glPopMatrix();


	glutSwapBuffers();
}

// ī�޶� �ʱ�ȭ
void cameraInit()
{

	vid = new VideoCapture(0);

	if (!vid) {
		printf("Could not capture a camera\n\7");
		return;
	}

	Mat img_frame;

	vid->read(img_frame);

	screenW = img_frame.cols;
	screenH = img_frame.rows;

	cout << screenW << " " << screenH << endl;
}

void reshape(GLsizei width, GLsizei height)
{
	glViewport(0, 0, (GLsizei)width, (GLsizei)height); //������ ũ��� ������Ʈ ���� 

	glMatrixMode(GL_PROJECTION); //���� ������ Projection Matrix�� ������ �ش�.
	glLoadIdentity();

	//Field of view angle(���� degrees), �������� aspect ratio, Near�� Far Plane����
	gluPerspective(45, (GLfloat)width / (GLfloat)height, 1.0, 100.0);

	glMatrixMode(GL_MODELVIEW); //���� ������ ModelView Matirx�� ������ �ش�. 
}


void timer(int value) {
	//��ķ���κ��� �̹��� ĸó
	vid->read(cam_frame);
	cvtColor(cam_frame, cam_frame, COLOR_BGR2RGB);

	cubeAngle += 1.0f;
	if (cubeAngle > 360) {
		cubeAngle -= 360;
	}

	glutPostRedisplay();      //�����츦 �ٽ� �׸����� ��û
	glutTimerFunc(1, timer, 0); //���� Ÿ�̸� �̺�Ʈ�� 1�и�����Ʈ ��  ȣ���.
}



void init()
{
	glGenTextures(1, &texture_background);

	//ȭ�� ���ﶧ ����� �� ����
	glClearColor(0.0, 0.0, 0.0, 0.0);

	//���� ���� ���� �� ����� �� ����
	glClearDepth(1.0);

	//���� ���� Ȱ��ȭ
	glEnable(GL_DEPTH_TEST);

}

void keyboard(unsigned char key, int x, int y)
{
	//ESC Ű�� �������ٸ� ���α׷� ����
	if (key == 27)
	{
		vid->release();
		exit(0);
	}
}


int main(int argc, char** argv)
{
	glutInit(&argc, argv);  //GLUT �ʱ�ȭ

	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH); //���� ���ۿ� ���� ���۸� ����ϵ��� ����, GLUT_RGB=0x00��

	cameraInit();
	glutInitWindowSize(screenW, screenH);
	glutInitWindowPosition(100, 100); //�������� ��ġ (x,y)
	glutCreateWindow("OpenGL Example"); //������ ����


	init();

	//���÷��� �ݹ� �Լ� ���, display�Լ��� ������ ó�� ������ ���� ȭ�� �ٽ� �׸� �ʿ� ������ ȣ��ȴ�. 
	glutDisplayFunc(display);

	//reshape �ݹ� �Լ� ���, reshape�Լ��� ������ ó�� ������ ���� ������ ũ�� ����� ȣ��ȴ�.
	glutReshapeFunc(reshape);
	//Ÿ�̸� �ݹ� �Լ� ���, ó������ �ٷ� ȣ���.
	glutTimerFunc(0, timer, 0);
	//Ű���� �ݹ� �Լ� ���, Ű���尡 �������� ȣ��ȴ�. 
	glutKeyboardFunc(keyboard);

	//GLUT event processing loop�� �����Ѵ�.
	//�� �Լ��� ���ϵ��� �ʱ� ������ �����ٿ� �ִ� �ڵ尡 ������� �ʴ´�. 
	glutMainLoop();


	return 0;
}

