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
	// 영상이 비어 있다면 종료
	if (frame.empty())
	{
		cout << "[ERROR] Image is empty." << endl;
		exit(100);
	}

	// OpenGL Texture 생성
	GLuint textureID;
	glGenTextures(1, &textureID);

	// Texture ID를 바인딩 : 사용할 텍스처 차원을 설정
	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frame.cols, frame.rows,
		0, GL_RGB, GL_UNSIGNED_BYTE, frame.ptr());

	// Texture를 리턴해줌.
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
	glRasterPos3f(x, y, z); //문자열이 그려질 위치 지정

	while (*str)
	{
		//GLUT_BITMAP_TIMES_ROMAN_24 폰트를 사용하여 문자열을 그린다.
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *str);

		str++;
	}
}


//큐브의 한 면, 화면 안쪽 방향인 -Z축방향으로 0.5이동하여 정사각형을 그린다.
static void cubebase(void)
{
	glBegin(GL_QUADS);
	glVertex3d(-0.5, -0.5, -0.5);
	glVertex3d(-0.5, 0.5, -0.5);
	glVertex3d(0.5, 0.5, -0.5);
	glVertex3d(0.5, -0.5, -0.5);
	glEnd();
}

//cubebase함수에서 그린 사각형을 회전 및 이동시켜
//큐브를 완성시킨다.
void draw_cube()
{
	glPushMatrix();

	glColor3f(0.0f, 1.0f, 0.0f);     // Green, -Z축 방향
	cubebase();

	glPushMatrix();

	glTranslated(1.0, 0.0, 0.0);
	glRotated(90.0, 0.0, 1.0, 0.0);
	glColor3f(0.0f, 0.0f, 1.0f);     // Blue, +X축 방향
	cubebase();

	glPopMatrix();

	glPushMatrix();
	glTranslated(-1.0, 0.0, 0.0);
	glRotated(-90.0, 0.0, 1.0, 0.0);
	glColor3f(1.0f, 0.5f, 0.0f);     // Orange, -X축 방향
	cubebase();
	glPopMatrix();

	glPushMatrix();
	glTranslated(0.0, 1.0, 0.0);
	glRotated(-90.0, 1.0, 0.0, 0.0);
	glColor3f(1.0f, 0.0f, 0.0f);     // Red, +Y축 방향
	cubebase();
	glPopMatrix();

	glPushMatrix();
	glTranslated(0.0, -1.0, 0.0);
	glRotated(90.0, 1.0, 0.0, 0.0);
	glColor3f(1.0f, 1.0f, 0.0f);     // Yellow, -Y축 방향
	cubebase();
	glPopMatrix();

	glBegin(GL_QUADS);
	glColor3f(1.0f, 0.0f, 1.0f);     // Magenta, +Z축 방향
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

	glPushMatrix(); //X축 붉은색
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_LINES);
	glVertex3f(1.0, 0.0, 0.0);
	glVertex3f(-1.0, 0.0, 0.0);
	glEnd();
	//	drawBitmapText('+X', 0.8, 0.0, 0.0);
	//	drawBitmapText('-X', -0.8, 0.0, 0.0);
	glPopMatrix();

	glPushMatrix(); //Y축 녹색
	glColor3f(0.0, 1.0, 0.0);
	glBegin(GL_LINES);
	glVertex3f(0.0, 1.0, 0.0);
	glVertex3f(0.0, -1.0, 0.0);
	glEnd();
	//	drawBitmapText('+Y', 0.0, 0.8, 0.0);
	//	drawBitmapText('-Y', 0.0, -0.8, 0.0);
	glPopMatrix();

	glPushMatrix(); //Z축 파란색
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
	//화면을 지운다. (컬러버퍼와 깊이버퍼)
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//이후 연산은 ModelView Matirx에 영향을 준다.
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	texture_background = MatToTexture(cam_frame);
	if (texture_background < 0) return;


	glEnable(GL_TEXTURE_2D);
	glColor3f(1.0f, 1.0f, 1.0f); //큐브나 좌표축 그릴 때 사용한 색의 영향을 안받을려면 필요
	glBindTexture(GL_TEXTURE_2D, texture_background);
	glPushMatrix();
	glTranslatef(0.0, 0.0, -9.0);
	draw_background(); //배경그림
	glPopMatrix();


	glDisable(GL_TEXTURE_2D);
	glPushMatrix();
	glTranslatef(0.0, 0.0, -4.0);
	glRotatef(cubeAngle, 1.0, 1.0, 1.0);
	draw_cube(); //큐브
	draw_line();  //좌표축
	glPopMatrix();


	glutSwapBuffers();
}

// 카메라 초기화
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
	glViewport(0, 0, (GLsizei)width, (GLsizei)height); //윈도우 크기로 뷰포인트 설정 

	glMatrixMode(GL_PROJECTION); //이후 연산은 Projection Matrix에 영향을 준다.
	glLoadIdentity();

	//Field of view angle(단위 degrees), 윈도우의 aspect ratio, Near와 Far Plane설정
	gluPerspective(45, (GLfloat)width / (GLfloat)height, 1.0, 100.0);

	glMatrixMode(GL_MODELVIEW); //이후 연산은 ModelView Matirx에 영향을 준다. 
}


void timer(int value) {
	//웹캠으로부터 이미지 캡처
	vid->read(cam_frame);
	cvtColor(cam_frame, cam_frame, COLOR_BGR2RGB);

	cubeAngle += 1.0f;
	if (cubeAngle > 360) {
		cubeAngle -= 360;
	}

	glutPostRedisplay();      //윈도우를 다시 그리도록 요청
	glutTimerFunc(1, timer, 0); //다음 타이머 이벤트는 1밀리세컨트 후  호출됨.
}



void init()
{
	glGenTextures(1, &texture_background);

	//화면 지울때 사용할 색 지정
	glClearColor(0.0, 0.0, 0.0, 0.0);

	//깊이 버퍼 지울 때 사용할 값 지정
	glClearDepth(1.0);

	//깊이 버퍼 활성화
	glEnable(GL_DEPTH_TEST);

}

void keyboard(unsigned char key, int x, int y)
{
	//ESC 키가 눌러졌다면 프로그램 종료
	if (key == 27)
	{
		vid->release();
		exit(0);
	}
}


int main(int argc, char** argv)
{
	glutInit(&argc, argv);  //GLUT 초기화

	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH); //더블 버퍼와 깊이 버퍼를 사용하도록 설정, GLUT_RGB=0x00임

	cameraInit();
	glutInitWindowSize(screenW, screenH);
	glutInitWindowPosition(100, 100); //윈도우의 위치 (x,y)
	glutCreateWindow("OpenGL Example"); //윈도우 생성


	init();

	//디스플레이 콜백 함수 등록, display함수는 윈도우 처음 생성할 때와 화면 다시 그릴 필요 있을때 호출된다. 
	glutDisplayFunc(display);

	//reshape 콜백 함수 등록, reshape함수는 윈도우 처음 생성할 때와 윈도우 크기 변경시 호출된다.
	glutReshapeFunc(reshape);
	//타이머 콜백 함수 등록, 처음에는 바로 호출됨.
	glutTimerFunc(0, timer, 0);
	//키보드 콜백 함수 등록, 키보드가 눌러지면 호출된다. 
	glutKeyboardFunc(keyboard);

	//GLUT event processing loop에 진입한다.
	//이 함수는 리턴되지 않기 때문에 다음줄에 있는 코드가 실행되지 않는다. 
	glutMainLoop();


	return 0;
}

