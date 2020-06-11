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

//�⺻ ����
const float calibrationSquareDimension = 0.01905f; //meters
// Square Size of Checkerboard
const float arucoSquareDimension = 0.1016f; //meters
//Aruco Marker Size - �����õ� �Ʒ��� ��Ŀ ������
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
		// ���� ���� �Ʒ�����, ������ �Ʒ����� Ȯ���� ��.
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

//cubebase�Լ����� �׸� �簢���� ȸ�� �� �̵�����
//ť�긦 �ϼ���Ų��.
void draw_cube()
{
	/*
	glRotated(angle, x, y, z)
	angle(ȸ����ų ����), x,y,z�� ȸ���� ������ �Ǵ� ����

	ex. glRotated(45.0, 1.0, 0.0, 0.0) -> x���� �������� 45��

	glTranslated(x, y, z)
	�� ������ �̵��� �Ÿ� ����, ��ü�� �����ϴ� ��� vertex�� ������
	*/

	for (int i = 0; i < markerIds.size(); i++)
	{
		// ��ǥ�� normalize 
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
	glColor3f(0.0f, 1.0f, 0.0f);     // Green, -Z�� ����
	cubebase(distanceRatio);
	//glPopMatrix();

	glPushMatrix();
	//glTranslated(centerP.x, centerP.y, 0);
	glTranslated(distanceRatio, 0.0, 0.0);
	glRotated(90.0, 0.0, 1.0, 0.0);
	glColor3f(0.0f, 0.0f, 1.0f);     // Blue, +X�� ����
	cubebase(distanceRatio);
	glPopMatrix();

	glPushMatrix();
	//glTranslated(centerP.x, centerP.y, 0);
	glTranslated(-distanceRatio, 0.0, 0.0);
	glRotated(-90.0, 0.0, 1.0, 0.0);
	glColor3f(1.0f, 0.5f, 0.0f);     // Orange, -X�� ����
	cubebase(distanceRatio);

	glPopMatrix();

	glPushMatrix();
	//glTranslated(centerP.x, centerP.y, 0);
	glTranslated(0.0, distanceRatio, 0.0);
	glRotated(-90.0, 1.0, 0.0, 0.0);
	glColor3f(1.0f, 0.0f, 0.0f);     // Red, +Y�� ����
	cubebase(distanceRatio);
	glPopMatrix();

	//	glBegin(GL_QUADS);

	glPushMatrix();
	glTranslated(0.0, -distanceRatio, 0.0);
	//	glTranslated(centerP.x, centerP.y, 0);
	glRotated(90.0, 1.0, 0.0, 0.0);
	glColor3f(1.0f, 1.0f, 0.0f);     // Yellow, -Y�� ����
	cubebase(distanceRatio);
	glPopMatrix();

	//	glEnd();

	glBegin(GL_QUADS);

	glPushMatrix();
	//glTranslated(centerP.x, centerP.y, 0);
	//glTranslated(0.0, 4.0, 0.0);
	glColor3f(1.0f, 0.0f, 1.0f);     // Magenta, +Z�� ����

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
	//ȭ���� �����. (�÷����ۿ� ���̹���)
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//���� ������ ModelView Matirx�� ������ �ش�.
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
	glColor3f(1.0f, 1.0f, 1.0f); //ť�곪 ��ǥ�� �׸� �� ����� ���� ������ �ȹ������� �ʿ�
	glBindTexture(GL_TEXTURE_2D, texture_background);
	glPushMatrix();
	// -9 ~ -15(z�� ����) Translatef
	glTranslatef(0.0, 0.0, -15.0);
	draw_background(); //���׸�
	glPopMatrix();


	glDisable(GL_TEXTURE_2D);
	glPushMatrix();
	//
	glTranslatef(0.0, 0.0, -15.0);
	//glRotatef(cubeAngle, 1.0, 0.0, 0.0);

	
	glRotatef(45, 1.0, 1.0, 1.5);
	centerP = Point3f(0, 100, 0);
	draw_cube(); //ť��
	glPopMatrix();

	glutSwapBuffers();
}

// ī�޶� �ʱ�ȭ
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
	glViewport(0, 0, (GLsizei)width, (GLsizei)height); //������ ũ��� ������Ʈ ���� 

	glMatrixMode(GL_PROJECTION); //���� ������ Projection Matrix�� ������ �ش�.
	glLoadIdentity();

	//Field of view angle(���� degrees), �������� aspect ratio, Near�� Far Plane����
	gluPerspective(45, (GLfloat)width / (GLfloat)height, 1.0, 100.0);

	glMatrixMode(GL_MODELVIEW); //���� ������ ModelView Matirx�� ������ �ش�. 
}


void timer(int value) {
	//��ķ���κ��� �̹��� ĸó
	webCam->read(cam_Frame);
	cvtColor(cam_Frame, cam_Frame, COLOR_BGR2RGB);

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
		webCam->release();
		exit(0);
	}
}

// *** Aruco Marker ����� ����
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


// Display ���� �־��ֱ� - while �� ���� �ݺ� �����
// *** Aruco Detection
int startWebcamMonitoring(const Mat& cameraMatrix, const Mat& distanceCoefficients, float arucoSquareDimensions)
{

	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH); //���� ���ۿ� ���� ���۸� ����ϵ��� ����, GLUT_RGB=0x00��

	Mat frameCV;

	vector<vector<Point2f>> rejectedCandidates;
	aruco::DetectorParameters parameters;

	Ptr<aruco::Dictionary> markerDictionary =
		aruco::getPredefinedDictionary(aruco::PREDEFINED_DICTIONARY_NAME::DICT_4X4_50);
	//DICT_4X4_50 : the smallest size and the number of markers
	// ��Ŀ�� ���Ҽ��� ��ǻ�Ͱ� �� ���ϰ� ��, �츮�� �� ����, �� ū ��Ŀ �ʿ�����Ƿ� �ּ� ���� ���

	webCam = new VideoCapture(0);
	// �ι�°�� ����� ����

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
		glutInitWindowPosition(100, 100); //�������� ��ġ (x,y)
		glutCreateWindow("Rendering Cube On Marker"); //������ ����


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

	glutInit(&argc, argv);  //GLUT �ʱ�ȭ


	// 1. Camera Calibration - üũ����� �׽�Ʈ�غ� ��
	// => Results_of_CameraCalibration ���� ����
	//cameraCalibrationProcess(cameraMatrix, distortionCoefficients);
	//startWebcamMonitoring(cameraMatrix, distanceCoefficients, arucoSquareDimension);

	// 2. Aruco Marker Detection
	loadCameraCalibration("Results_of_CameraCalibration", cameraMatrix, distortionCoefficients);
	//startWebcamMonitoring(cameraMatrix, distortionCoefficients, arucoSquareDimension);
	
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH); //���� ���ۿ� ���� ���۸� ����ϵ��� ����, GLUT_RGB=0x00��

	cameraInit();
	glutInitWindowSize(screenW, screenH);
	glutInitWindowPosition(100, 100); //�������� ��ġ (x,y)
	glutCreateWindow("Rendering Cube On Marker"); //������ ����


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
