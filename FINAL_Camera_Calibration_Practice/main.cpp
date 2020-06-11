#pragma comment(lib, "opencv_aruco349.lib")
#pragma comment(lib, "freeglut.lib")
#pragma comment(lib, "glew32.lib")

// OpenCV
#include "opencv2/core.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/aruco.hpp"
#include "opencv2/calib3d.hpp"

// OpenGL
#include <GL/freeglut.h>

// Basic template class
#include <sstream>
#include <iostream>
#include <fstream>

#include <cmath>

// name space
using namespace cv;
using namespace std;

//기본 정보
const float calibrationSquareDimension = 0.01905f; //meters
// Square Size of Checkerboard
const float arucoSquareDimension = 0.1016f; //meters
//Aruco Marker Size - 프린팅된 아루코 마커 사이즈
const Size chessboardDimensions = Size(6, 9);


// Camera Variables
VideoCapture* webCam;
Mat cam_Frame;
int screenW;					// screen width
int screenH;					// screen height


// GLUT Variables
GLuint texture_background, texture_cube;
float cubeAngle = 0;

// Marker Coordinates
vector<vector<Point2f>> markerCorners;
vector<int> markerIds;

// Mat
Mat distortionCoefficients;
Mat cameraMatrix = Mat::eye(3, 3, CV_64F);

static int gDrawRotate = FALSE;
static float gDrawRotateAngle = 0;			// For use in drawing.

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

	return textureID;
}


// Draw Background
void draw_background()
{
	int x = screenW / 100.0;
	int y = screenH / 100.0;

	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 1.0); 
	glVertex3f(-x, -y, 0.0);
	glTexCoord2f(1.0, 1.0); 
	glVertex3f(x, -y, 0.0);
	glTexCoord2f(1.0, 0.0); 
	glVertex3f(x, y, 0.0);
	glTexCoord2f(0.0, 0.0); 
	glVertex3f(-x, y, 0.0);
	glEnd();
}


static void cubebase(float _ratio)
{
		// 원점 왼쪽 아래인지, 오른쪽 아래인지 확인할 것.
		glBegin(GL_QUADS);

		glVertex3d(-0.5 * _ratio, -0.5 * _ratio, -0.5 * _ratio);
		glVertex3d(-0.5 * _ratio, 0.5 * _ratio, -0.5 * _ratio);
		glVertex3d(0.5 * _ratio, 0.5 * _ratio, -0.5 * _ratio);
		glVertex3d(0.5 * _ratio, -0.5 * _ratio, -0.5 * _ratio);

		/*
		glVertex3d(centerP.x - 0.5, centerP.y - 0.5, 0.0);
		glVertex3d(centerP.x + 0.5, centerP.y - 0.5, 0.0);
		glVertex3d(centerP.x + 0.5, centerP.y + 0.5, 0.0);
		glVertex3d(centerP.x - 0.5, centerP.y + 0.5, 0.0);

		*/
		glEnd();
}

Point3f centerP;

float distanceRatio;

//cubebase함수에서 그린 사각형을 회전 및 이동시켜
//큐브를 완성시킨다.
void draw_cube()
{
	/*
	glRotated(angle, x, y, z)
	angle(회전시킬 각도), x,y,z는 회전의 기준이 되는 벡터

	ex. glRotated(45.0, 1.0, 0.0, 0.0) -> x축을 기준으로 45도

	glTranslated(x, y, z)
	각 축으로 이동할 거리 지정, 물체를 구성하는 모든 vertex에 더해짐
	*/

	for (int i = 0; i < markerIds.size(); i++)
	{
		// 좌표계 normalize 
		centerP = Point3f(
			(markerCorners[i][0].x + markerCorners[i][1].x) / 640 / 2 * 6.4 - 3.2,
			(-(markerCorners[i][0].y + markerCorners[i][1].y) / 480 / 2 * 4.8 - 2.4),
			0);

		//cout << centerP.x << " " << centerP.y << endl;
		float x_pow = pow((markerCorners[i][1].x - markerCorners[i][0].x), 2);
		float y_pow = pow((markerCorners[i][1].y - markerCorners[i][0].y), 2);
		distanceRatio = sqrt(x_pow + y_pow) * 0.1f / 5;
		cout << distanceRatio << endl;
	}

	glPushMatrix();
	//glTranslated(1.0, 4.0, 0.0);
	glTranslated(centerP.x, centerP.y + 5.0, 0);
	//glTranslated(0, 0, distanceRatio);
	glColor3f(0.0f, 1.0f, 0.0f);     // Green, -Z축 방향
	cubebase(distanceRatio);
	//glPopMatrix();

	glPushMatrix();
	//glTranslated(centerP.x, centerP.y, 0);
	glTranslated(distanceRatio, 0.0, 0.0);
	glRotated(90.0, 0.0, 1.0, 0.0);
	glColor3f(0.0f, 0.0f, 1.0f);     // Blue, +X축 방향
	cubebase(distanceRatio);
	glPopMatrix();

	glPushMatrix();
	//glTranslated(centerP.x, centerP.y, 0);
	glTranslated(-distanceRatio, 0.0, 0.0);
	glRotated(-90.0, 0.0, 1.0, 0.0);
	glColor3f(1.0f, 0.5f, 0.0f);     // Orange, -X축 방향
	cubebase(distanceRatio);

	glPopMatrix();

	glPushMatrix();
	//glTranslated(centerP.x, centerP.y, 0);
	glTranslated(0.0, distanceRatio, 0.0);
	glRotated(-90.0, 1.0, 0.0, 0.0);
	glColor3f(1.0f, 0.0f, 0.0f);     // Red, +Y축 방향
	cubebase(distanceRatio);
	glPopMatrix();

	//	glBegin(GL_QUADS);

	glPushMatrix();
	glTranslated(0.0, -distanceRatio, 0.0);
	//	glTranslated(centerP.x, centerP.y, 0);
	glRotated(90.0, 1.0, 0.0, 0.0);
	glColor3f(1.0f, 1.0f, 0.0f);     // Yellow, -Y축 방향
	cubebase(distanceRatio);
	glPopMatrix();

	//	glEnd();

	glBegin(GL_QUADS);

	glPushMatrix();
	//glTranslated(centerP.x, centerP.y, 0);
	//glTranslated(0.0, 4.0, 0.0);
	glColor3f(1.0f, 0.0f, 1.0f);     // Magenta, +Z축 방향

	/*
	glVertex3d(-0.5, -0.5, 0.5);
	glVertex3d(0.5, -0.5, 0.5);
	glVertex3d(0.5, 0.5, 0.5);
	glVertex3d(-0.5, 0.5, 0.5);
	*/

	glVertex3d(-0.5 * distanceRatio, -0.5 * distanceRatio, 0.5 * distanceRatio);
	glVertex3d(0.5 * distanceRatio, -0.5 * distanceRatio, 0.5 * distanceRatio);
	glVertex3d(0.5 * distanceRatio, 0.5 * distanceRatio, 0.5 * distanceRatio);
	glVertex3d(-0.5 * distanceRatio, 0.5 * distanceRatio, 0.5 * distanceRatio);


	glEnd();
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

	vector<vector <Point2f>> rejectedCandidates;
	aruco::DetectorParameters parameters;

	Ptr<aruco::Dictionary> markerDictionary =
		aruco::getPredefinedDictionary(aruco::PREDEFINED_DICTIONARY_NAME::DICT_4X4_50);

	vector<Vec3d> rotationVecs, translationVecs;


	aruco::detectMarkers(cam_Frame, markerDictionary, markerCorners, markerIds);
	aruco::estimatePoseSingleMarkers(markerCorners, arucoSquareDimension, cameraMatrix,
		distortionCoefficients, rotationVecs, translationVecs);

	for (int i = 0; i < markerIds.size(); i++)
	{
		//cv::line(cam_Frame, Point(markerCorners[i][0].x, markerCorners[i][0].y), Point(markerCorners[i][1].x, markerCorners[i][1].y), (0, 255, 255), 5);
		//cv::line(cam_Frame, Point(markerCorners[i][1].x, markerCorners[i][1].y), Point(markerCorners[i][2].x, markerCorners[i][2].y), (0, 255, 255), 5);
		cv::aruco::drawDetectedMarkers(cam_Frame, markerCorners, markerIds);
			aruco::drawAxis(cam_Frame, cameraMatrix, distortionCoefficients,
			rotationVecs[i], translationVecs[i], 0.1f);
	}

	texture_background = MatToTexture(cam_Frame);
	if (texture_background < 0) return;

	glEnable(GL_TEXTURE_2D);
	glColor3f(1.0f, 1.0f, 1.0f); //큐브나 좌표축 그릴 때 사용한 색의 영향을 안받을려면 필요
	glBindTexture(GL_TEXTURE_2D, texture_background);
	glPushMatrix();
	// -9 ~ -15(z축 통일) Translatef
	glTranslatef(0.0, 0.0, -15.0);
	draw_background(); //배경그림
	glPopMatrix();


	glDisable(GL_TEXTURE_2D);
	glPushMatrix();
	//
	glTranslatef(0.0, 0.0, -15.0);
	//glRotatef(cubeAngle, 1.0, 0.0, 0.0);

	
	glRotatef(45, 1.0, 1.0, 1.5);
	centerP = Point3f(0, 100, 0);
	draw_cube(); //큐브
	glPopMatrix();

	glutSwapBuffers();
}

// 카메라 초기화
void cameraInit()
{

	webCam = new VideoCapture(0);

	if (!webCam) {
		printf("Could not capture a camera\n\7");
		return;
	}

	Mat img_frame;

	webCam->read(img_frame);

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
	webCam->read(cam_Frame);
	cvtColor(cam_Frame, cam_Frame, COLOR_BGR2RGB);

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
		webCam->release();
		exit(0);
	}
}

// *** Aruco Marker 만들고 저장
void createArucoMarkers()
{
	Mat outputMarker;
	Ptr<aruco::Dictionary> markerDictionary
		= aruco::getPredefinedDictionary(aruco::PREDEFINED_DICTIONARY_NAME::DICT_4X4_50);

	for (int i = 0; i < 50; i++)
	{
		aruco::drawMarker(markerDictionary, i, 500, outputMarker, 1);
		ostringstream convert;
		string imageName = "4x4Marker_";
		convert << imageName << i << ".jpg";
		imwrite(convert.str(), outputMarker);
	}
}


// *** Basics of Camera Calibration
void createKnownBoardPosition(Size boardSize, float squareEdgeLength, vector<Point3f>& corners)
{
	for (int i = 0; i < boardSize.height; i++)
	{
		for (int j = 0; j < boardSize.width; j++)
		{
			corners.push_back(Point3f(j * squareEdgeLength, i * squareEdgeLength, 0.0f));
		}
	}
}
void getChessboardCorners(vector<Mat> images, vector<vector<Point2f>>& allFoundCorners, bool showResults = false)
{
	for (vector<Mat>::iterator iter = images.begin(); iter != images.end(); iter++)
	{
		vector<Point2f> pointBuf;
		bool found = findChessboardCorners(*iter, Size(9, 6), pointBuf,
			CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_NORMALIZE_IMAGE);

		if (found)
		{
			allFoundCorners.push_back(pointBuf);
		}

		if (showResults)
		{
			drawChessboardCorners(*iter, Size(9, 6), pointBuf, found);
			imshow("Looking for Corners", *iter);
			waitKey(0);
		}
	}
}


void cameraCalibration(vector<Mat> calibrationImages, Size boardSize, float squareEdgeLength, Mat& cameraMatrix, Mat& distanceCoefficients)
{
	vector<vector<Point2f>> checkerboardImageSpacePoints;
	getChessboardCorners(calibrationImages, checkerboardImageSpacePoints, false);

	vector<vector<Point3f>> worldSpaceCornerPoints(1);

	createKnownBoardPosition(boardSize, squareEdgeLength, worldSpaceCornerPoints[0]);
	worldSpaceCornerPoints.resize(checkerboardImageSpacePoints.size(), worldSpaceCornerPoints[0]);

	vector<Mat> rVectors, tVectors;
	distanceCoefficients = Mat::zeros(8, 1, CV_64F);

	calibrateCamera(worldSpaceCornerPoints, checkerboardImageSpacePoints,
		boardSize, cameraMatrix, distanceCoefficients, rVectors, tVectors);
}



bool saveCameraCalibration(string name, Mat cameraMatrix, Mat distanceCoefficients)
{
	ofstream outStream(name);
	if (outStream)
	{
		uint16_t rows = cameraMatrix.rows;
		uint16_t columns = cameraMatrix.cols;


		outStream << rows << endl;
		outStream << columns << endl;


		for (int r = 0; r < rows; r++)
		{
			for (int c = 0; c < columns; c++)
			{
				double value = cameraMatrix.at<double>(r, c);
				outStream << value << endl;
			}
		}

		rows = distanceCoefficients.rows;
		columns = distanceCoefficients.cols;


		outStream << rows << endl;
		outStream << columns << endl;


		for (int r = 0; r < rows; r++)
		{
			for (int c = 0; c < columns; c++)
			{
				double value = distanceCoefficients.at<double>(r, c);
				outStream << value << endl;
			}
		}

		outStream.close();
		return true;
	}

	return false;
}


// *** Load Camera Calibration Method
bool loadCameraCalibration(string name, Mat& cameraMatrix, Mat& distanceCoefficients)
{
	ifstream inStream(name);
	if (inStream)
	{
		uint16_t rows;
		uint16_t columns;

		inStream >> rows;
		inStream >> columns;

		cameraMatrix = Mat(Size(columns, rows), CV_64F);

		for (int r = 0; r < rows; r++)
		{
			for (int c = 0; c < columns; c++)
			{
				double read = 0.0f;
				inStream >> read;
				cameraMatrix.at<double>(r, c) = read;
				cout << cameraMatrix.at<double>(r, c) << "\n";
			}
		}
		// Distance Coefficients
		inStream >> rows;
		inStream >> columns;

		distanceCoefficients = Mat::zeros(rows, columns, CV_64F);


		for (int r = 0; r < rows; r++)
		{
			for (int c = 0; c < columns; c++)
			{
				double read = 0.0f;
				inStream >> read;
				distanceCoefficients.at<double>(r, c) = read;
				cout << distanceCoefficients.at<double>(r, c) << "\n";
			}
		}
		inStream.close();
		return true;
	}

	return false;
}


// Display 내에 넣어주기 - while 문 내의 반복 내용들
// *** Aruco Detection
int startWebcamMonitoring(const Mat& cameraMatrix, const Mat& distanceCoefficients, float arucoSquareDimensions)
{

	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH); //더블 버퍼와 깊이 버퍼를 사용하도록 설정, GLUT_RGB=0x00임

	Mat frameCV;

	vector<vector<Point2f>> rejectedCandidates;
	aruco::DetectorParameters parameters;

	Ptr<aruco::Dictionary> markerDictionary =
		aruco::getPredefinedDictionary(aruco::PREDEFINED_DICTIONARY_NAME::DICT_4X4_50);
	//DICT_4X4_50 : the smallest size and the number of markers
	// 마커를 더할수록 컴퓨터가 더 일하게 됨, 우리는 더 많은, 더 큰 마커 필요없으므로 최소 단위 사용

	webCam = new VideoCapture(0);
	// 두번째로 연결된 비디오

	if (!(webCam->isOpened()))
	{
		cout << "[ERROR] Could not capture a camera" << endl;
		return -1;
	}

	namedWindow("Webcam", WINDOW_OPENGL);

	vector<Vec3d> rotationVectors, translationVectors;

	webCam->read(frameCV);

//	while (true)
//	{
		
	//	if (!webCam->read(frameCV))
	//		break;

		aruco::detectMarkers(frameCV, markerDictionary, markerCorners, markerIds);
		aruco::estimatePoseSingleMarkers(markerCorners, arucoSquareDimension, cameraMatrix,
			distanceCoefficients, rotationVectors, translationVectors);

		for (int i = 0; i < markerIds.size(); i++)
		{
			aruco::drawAxis(frameCV, cameraMatrix, distanceCoefficients,
				rotationVectors[i], translationVectors[i], 0.1f);
		}


	//	if (waitKey(30) >= 0) break;


		cam_Frame = frameCV;
		cameraInit();

		

		glutInitWindowSize(screenW, screenH);
		glutInitWindowPosition(100, 100); //윈도우의 위치 (x,y)
		glutCreateWindow("Rendering Cube On Marker"); //윈도우 생성


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
//	}

	return 1;
}


// *** Previous Work - Calibration
void cameraCalibrationProcess(Mat& cameraMatrix, Mat& distanceCoefficients)
{
	Mat frame;
	Mat drawToFrame;

	vector<Mat> savedImages;

	vector<vector<Point2f>> markerCorners, rejectedCandidates;

	VideoCapture vid(0);

	if (!vid.isOpened())
		return;

	int framesPerSecond = 20;
	namedWindow("Webcam", CV_WINDOW_AUTOSIZE);

	while (true)
	{
		if (!vid.read(frame))
			break;

		vector<Vec2f> foundPoints;
		bool found = false;

		found = findChessboardCorners(frame, chessboardDimensions, foundPoints,
			CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_NORMALIZE_IMAGE);
		frame.copyTo(drawToFrame);
		drawChessboardCorners(drawToFrame, chessboardDimensions, foundPoints, found);

		if (found)
			imshow("Webcam", drawToFrame);
		else
			imshow("Webcam", frame);

		char character = waitKey(1000 / framesPerSecond);

		switch (character)
		{
		case -1: //Nothing
			// saving image
			if (found)
			{
				Mat temp;
				frame.copyTo(temp);
				savedImages.push_back(temp);
				cout << "found" << endl;
			}
			break;
		case '\r': //Enter
			// start calibration
			if (savedImages.size() > 15)
			{
				cameraCalibration(savedImages, chessboardDimensions, calibrationSquareDimension,
					cameraMatrix, distanceCoefficients);
				saveCameraCalibration("Results_of_CameraCalibration", cameraMatrix, distanceCoefficients);
				cout << "Complete" << endl;
			}
			break;
		case 27: //Esc
			// exit program
			return;
			break;
		}
	}
}



int main(int argc, char** argv)
{

	glutInit(&argc, argv);  //GLUT 초기화


	// 1. Camera Calibration - 체크보드로 테스트해볼 것
	// => Results_of_CameraCalibration 파일 생성
	//cameraCalibrationProcess(cameraMatrix, distortionCoefficients);
	//startWebcamMonitoring(cameraMatrix, distanceCoefficients, arucoSquareDimension);

	// 2. Aruco Marker Detection
	loadCameraCalibration("Results_of_CameraCalibration", cameraMatrix, distortionCoefficients);
	//startWebcamMonitoring(cameraMatrix, distortionCoefficients, arucoSquareDimension);
	
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH); //더블 버퍼와 깊이 버퍼를 사용하도록 설정, GLUT_RGB=0x00임

	cameraInit();
	glutInitWindowSize(screenW, screenH);
	glutInitWindowPosition(100, 100); //윈도우의 위치 (x,y)
	glutCreateWindow("Rendering Cube On Marker"); //윈도우 생성


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
