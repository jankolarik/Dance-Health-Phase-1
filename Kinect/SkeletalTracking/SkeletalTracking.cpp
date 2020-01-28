#include <Windows.h>
#include "Kinect.h"
#include <iostream>
#include <cmath>
#include <time.h>       /* clock_t, clock, CLOCKS_PER_SEC */
#include <iostream>
#include <fstream>
#include <chrono>

#include <Ole2.h>
#include<gl/GL.h>
#include<gl/GLU.h>
#include<gl/glut.h>

#include<opencv2/opencv.hpp>
#include<opencv2/videoio/videoio_c.h>
#include<opencv2/highgui.hpp>

/* references:
cs Washington tutorial by Ed Zhang:
https://homes.cs.washington.edu/~edzhang/tutorials/kinect2/kinect1.html
https://github.com/kyzyx/Tutorials/tree/master/Kinect2SDK/1_Basics


https://github.com/yzhong52/OpenGLFramebufferAsVideo
*/

static cv::VideoWriter outputVideo;

#define CLOCKS_PER_SEC  ((clock_t)1000)
#define width 1920
#define height 1080
#define GL_BGRA 0x80E1

// OpenGL Variables
GLuint textureId;              // ID of the texture to contain Kinect RGB Data
GLubyte dataGlu[width * height * 4];  // BGRA array containing the texture data

// Kinect variables
IKinectSensor* sensor;         // Kinect sensor
IColorFrameReader* reader;     // Kinect color data source

// Body Tracking Variables
Joint jointsAll[BODY_COUNT][JointType_Count];
BOOLEAN trackList[BODY_COUNT];
ColorSpacePoint colorPoints[BODY_COUNT][JointType_Count];

// No need to include the std keyword before cout
using namespace std;
using namespace cv;

// Safe release for interfaces
template<class Interface>
inline void SafeRelease(Interface*& pInterfaceToRelease)
{
	if (pInterfaceToRelease != NULL)
	{
		pInterfaceToRelease->Release();
		pInterfaceToRelease = NULL;
	}
}


class SkeletalBasics
{
public:
	// Constructor
	SkeletalBasics();

	// Destructor
	~SkeletalBasics();

	void				 Update();

	HRESULT				 InitializeDefaultSensor();

private:
	// Current Kinect
	IKinectSensor* m_pKinectSensor;
	ICoordinateMapper* m_pCoordinateMapper;

	// Body reader
	IBodyFrameReader* m_pBodyFrameReader;

	// Body from last frame
	IBody* m_pBodyFromPreviousFrame[BODY_COUNT] = { 0 };

	// Calibration
	bool				 m_bCalibrationStatus;
	clock_t				 m_nCalibrationStartTime;
	float				 m_fCalibrationDuration;
	float				 m_fCalibrationValue;

	// Session
	bool				 m_bSessionStatus;
	clock_t				 m_nSessionStartTime;
	float                m_fSessionDuration;
	float				 m_fSessionAvgJointDisplacement;
	float				 m_fSessionJointsMaxheight[JointType_Count];

	// Start & Stop posture
	clock_t              m_nSpecialPostureStartTime;
	float				 m_fSpecialPostureDuration;


	void				 ProcessBody(int nBodyCount, IBody** ppBodies);

	float				 ActivityAnalysis(IBody* pBodyFromCurrentFrame, IBody* pBodyFromPreviousFrame);

	float				 JointDisplacementCalculator(Joint firstJoint, Joint secondJoint);

	void				 Calibration(IBody** ppBodies);

	bool				 SpecialPostureIndicator(Joint joints[]);

	void				 MaxJointsData(Joint joints[]);

	void				 Summary();

	ColorSpacePoint	         	 CameraToColor(const CameraSpacePoint& bodyPoint);
};

SkeletalBasics application;
void drawSkeletals();

SkeletalBasics::SkeletalBasics() :
	m_pKinectSensor(NULL),
	m_pCoordinateMapper(NULL),
	m_pBodyFrameReader(NULL),
	m_bCalibrationStatus(false),
	m_fCalibrationValue(0),
	m_nCalibrationStartTime(0),
	m_fCalibrationDuration(3),
	m_bSessionStatus(false),
	m_nSessionStartTime(0),
	m_fSessionDuration(0),
	m_fSessionAvgJointDisplacement(0),
	m_fSessionJointsMaxheight(),
	m_nSpecialPostureStartTime(0),
	m_fSpecialPostureDuration(5)
{
	for (int i = 0; i < JointType_Count; i++)
	{
		// Set the initial value of max height to be low enough that any new height can replace it
		m_fSessionJointsMaxheight[i] = -10;
	}
}


SkeletalBasics::~SkeletalBasics()
{
	// close the Kinect Sensor
	if (m_pKinectSensor)
	{
		m_pKinectSensor->Close();
	}
}

HRESULT SkeletalBasics::InitializeDefaultSensor()
{
	HRESULT hr;

	hr = GetDefaultKinectSensor(&m_pKinectSensor);

	if (FAILED(hr))
	{
		return hr;
	}

	if (m_pKinectSensor)
	{
		// Initialize the Kinect and get coordinate mapper and the body reader
		IBodyFrameSource* pBodyFrameSource = NULL;

		hr = m_pKinectSensor->Open();

		if (SUCCEEDED(hr))
		{
			hr = m_pKinectSensor->get_CoordinateMapper(&m_pCoordinateMapper);
		}

		if (SUCCEEDED(hr))
		{
			hr = m_pKinectSensor->get_BodyFrameSource(&pBodyFrameSource);
		}

		if (SUCCEEDED(hr))
		{
			hr = pBodyFrameSource->OpenReader(&m_pBodyFrameReader);
		}


		SafeRelease(pBodyFrameSource);
	}

	if (!m_pKinectSensor || FAILED(hr))
	{
		std::cout << "No Kinect Found!";
		return E_FAIL;
	}

	return hr;
}


void SkeletalBasics::Update()
{
	if (!m_pBodyFrameReader)
	{
		return;
	}

	IBodyFrame* pBodyFrame = NULL;

	HRESULT hr = m_pBodyFrameReader->AcquireLatestFrame(&pBodyFrame);

	// If the latest frame is acquired successfully
	if (SUCCEEDED(hr))
	{
		IBody* ppBodies[BODY_COUNT] = { 0 };

		hr = pBodyFrame->GetAndRefreshBodyData(_countof(ppBodies), ppBodies);

		// If the Body data is refreshed successfully
		if (SUCCEEDED(hr))
		{

			// If the calibration hasn't done, continue calibration
			if (!m_bCalibrationStatus)
			{
				Calibration(ppBodies);
				cout << "Calibration value : " << m_fCalibrationValue << endl;
			}
			else if (!m_bSessionStatus)
			{
				// The main function to process body data for each frame
				ProcessBody(BODY_COUNT, ppBodies);

				// Load the used frame data into the "previousframe" pointer to be used in the next frmae for comparison
				hr = pBodyFrame->GetAndRefreshBodyData(_countof(m_pBodyFromPreviousFrame), m_pBodyFromPreviousFrame);
			}
			else
			{
				// After the session finished, run summary
				Summary();
				outputVideo.release();
				exit(0);
			}
		}

		for (int i = 0; i < _countof(ppBodies); ++i)
		{
			SafeRelease(ppBodies[i]);
		}
	}
	SafeRelease(pBodyFrame);
}

void SkeletalBasics::Calibration(IBody** ppBodies)
{
	HRESULT hr;

	//cout << "Please stand straight facing the camera, feet firmly planted, legs straight, at maximum 4 meters away." << endl;
	//cout << "When you are ready to start calibration session, please raise both of your hands above your head." << endl;

	for (int i = 0; i < BODY_COUNT; ++i)
	{
		IBody* pBody = ppBodies[i];

		if (pBody)
		{
			BOOLEAN bTracked = false;
			hr = pBody->get_IsTracked(&bTracked);
			trackList[i] = bTracked;

			// If this body is being tracked
			if (SUCCEEDED(hr) && bTracked)
			{
				Joint joints[JointType_Count];

				hr = pBody->GetJoints(_countof(joints), joints);
				pBody->GetJoints(_countof(joints), jointsAll[i]);
				if (SUCCEEDED(hr))
				{
					float leftHandHeight = joints[7].Position.Y;
					float rightHandHeight = joints[23].Position.Y;
					float headHeight = joints[3].Position.Y;

					// Calibration starting position: both hands above head
					if (SpecialPostureIndicator(joints))
					{
						// Initial calibration start time
						if (!m_nCalibrationStartTime)
						{
							m_nCalibrationStartTime = clock();
						}

						float timeSpent = float((clock() - m_nCalibrationStartTime)) / CLOCKS_PER_SEC;

						if (timeSpent > m_fCalibrationDuration)
						{
							m_bCalibrationStatus = true;
						}

						Joint leftFeet = joints[15];
						Joint rightFeet = joints[19];

						if (SUCCEEDED(leftFeet.TrackingState) && SUCCEEDED(rightFeet.TrackingState))
						{
							m_fCalibrationValue = (m_fCalibrationValue + (leftFeet.Position.Y + rightFeet.Position.Y)) / 2;
						}
					}
					// If the posture not detected, reset the start time
					else
					{
						m_nCalibrationStartTime = 0;
					}
				}
			}
		}
	}
}

void SkeletalBasics::ProcessBody(int nBodyCount, IBody** ppBodies)
{
	HRESULT hr;

	// For each single body in bodies list
	for (int i = 0; i < nBodyCount; ++i)
	{
		IBody* pBody = ppBodies[i];
		IBody* pBodyPrevious = NULL;

		boolean previousBodyLoad = false;

		// If the previous frame data is ready
		if (m_pBodyFromPreviousFrame[i] != NULL)
		{
			previousBodyLoad = true;
			pBodyPrevious = m_pBodyFromPreviousFrame[i];
		}

		if (pBody)
		{
			BOOLEAN bTracked = false;
			hr = pBody->get_IsTracked(&bTracked);
			trackList[i] = bTracked;

			// If this body is being tracked
			if (SUCCEEDED(hr) && bTracked)
			{
				Joint joints[JointType_Count];

				hr = pBody->GetJoints(_countof(joints), joints);
				pBody->GetJoints(_countof(joints), jointsAll[i]);

				// If joints are obtained successfully, start data process
				if (SUCCEEDED(hr))
				{
					// Initialise Session start time
					if (!m_nSessionStartTime)
					{
						m_nSessionStartTime = clock();
					}

					m_fSessionDuration = float((clock() - m_nSessionStartTime)) / CLOCKS_PER_SEC;

					// Special posture recognition
					if (SpecialPostureIndicator(joints))
					{
						if (!m_nSpecialPostureStartTime)
						{
							m_nSpecialPostureStartTime = clock();
						}

						float timeSpent = float((clock() - m_nSessionStartTime)) / CLOCKS_PER_SEC;

						// If the special posture is performed for long enough time, finish session
						if (timeSpent > m_fSpecialPostureDuration)
						{
							m_bSessionStatus = true;
						}
					}
					else
					{
						// If the person stop performing the posture, reset the start time
						m_nSpecialPostureStartTime = 0;
					}

					for (int j = 0; j < _countof(joints); ++j)
					{
						colorPoints[i][j] = CameraToColor(joints[j].Position);
					}

					// Update max height of joints
					MaxJointsData(joints);

					// If the data from last frame is loaded, start comparison
					if (previousBodyLoad)
					{
						cout << "Activity Analysis value : " << ActivityAnalysis(pBody, pBodyPrevious) << endl;
					}
				}
			}
		}
	}
}

float SkeletalBasics::ActivityAnalysis(IBody* pBodyFromCurrentFrame, IBody* pBodyFromPreviousFrame)
{
	HRESULT hr;

	Joint currentjoints[JointType_Count];
	Joint previousjoints[JointType_Count];

	float totalDistanceTravelPerJoint = 0;
	float averageDistanceTravelPerJoint = 0;

	int   trackedJointNumber = 0;


	hr = pBodyFromCurrentFrame->GetJoints(_countof(currentjoints), currentjoints);

	// Current frame data loaded
	if (SUCCEEDED(hr))
	{
		hr = pBodyFromPreviousFrame->GetJoints(_countof(previousjoints), previousjoints);

		// Previous frame loaded
		if (SUCCEEDED(hr))
		{
			for (int j = 0; j < _countof(currentjoints); ++j)
			{
				// Only take into account of joints that are being tracked in the both frames
				if (SUCCEEDED(currentjoints[j].TrackingState) && SUCCEEDED(previousjoints[j].TrackingState))
				{
					totalDistanceTravelPerJoint += JointDisplacementCalculator(currentjoints[j], previousjoints[j]);
					trackedJointNumber++;
				}
			}
		}
	}

	averageDistanceTravelPerJoint = totalDistanceTravelPerJoint / trackedJointNumber;

	// Add the average joint displacement into the total displacement to get the displacement across whole seesion
	m_fSessionAvgJointDisplacement += averageDistanceTravelPerJoint;

	return averageDistanceTravelPerJoint;
}


float SkeletalBasics::JointDisplacementCalculator(Joint firstJoint, Joint secondJoint)
{
	// sqrt( x^2 + y^2 + z^2 )
	float distanceTraveled = sqrt(pow((firstJoint.Position.X - secondJoint.Position.X), 2) + pow((firstJoint.Position.Y - secondJoint.Position.Y), 2) + pow((firstJoint.Position.Z - secondJoint.Position.Z), 2));
	return distanceTraveled;
}


bool SkeletalBasics::SpecialPostureIndicator(Joint joints[])
{
	float leftHandHeight = joints[7].Position.Y;
	float rightHandHeight = joints[23].Position.Y;
	float headHeight = joints[3].Position.Y;

	if ((leftHandHeight > headHeight) && (rightHandHeight > headHeight))
	{
		return true;
	}

	return false;
}


void SkeletalBasics::MaxJointsData(Joint joints[])
{
	for (int i = 0; i < JointType_Count; i++)
	{
		if (joints[i].Position.Y > m_fSessionJointsMaxheight[i])
		{
			m_fSessionJointsMaxheight[i] = joints[i].Position.Y;
		}
	}
}


void SkeletalBasics::Summary()
{
	auto currentTime = chrono::system_clock::now();
	time_t time = std::chrono::system_clock::to_time_t(currentTime);

	ofstream summaryFile;
	summaryFile.open("Session_Summary.txt");
	summaryFile << ctime(&time) << endl
		<< "Session Duration: " << m_fSessionDuration << " seconds" << endl
		<< "Average distance moved per joint in this session: " << m_fSessionAvgJointDisplacement << " meters" << endl << endl
		<< "Max left  hand height: " << m_fSessionJointsMaxheight[7] << endl
		<< "Max right hand height: " << m_fSessionJointsMaxheight[23] << endl
		<< "Max left  knee height: " << m_fSessionJointsMaxheight[13] << endl
		<< "Max right knee height: " << m_fSessionJointsMaxheight[17] << endl;

	summaryFile.close();
}

ColorSpacePoint SkeletalBasics::CameraToColor(const CameraSpacePoint& bodyPoint)
{
	ColorSpacePoint colorPoint;
	m_pCoordinateMapper->MapCameraPointsToColorSpace(1, &bodyPoint, 1, &colorPoint);
	return colorPoint;
}



bool initKinect() {
	if (FAILED(GetDefaultKinectSensor(&sensor))) {
		return false;
	}
	if (sensor) {
		sensor->Open();

		IColorFrameSource* framesource = NULL;
		sensor->get_ColorFrameSource(&framesource);
		framesource->OpenReader(&reader);
		if (framesource) {
			framesource->Release();
			framesource = NULL;
		}
		return true;
	}
	else {
		return false;
	}
}

void getKinectData(GLubyte* dest) {
	IColorFrame* frame = NULL;
	if (SUCCEEDED(reader->AcquireLatestFrame(&frame))) {
		frame->CopyConvertedFrameDataToArray(width * height * 4, dataGlu, ColorImageFormat_Bgra);
	}
	if (frame) frame->Release();
}

void drawKinectData() {
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textureId);
	getKinectData(dataGlu);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_BGRA, GL_UNSIGNED_BYTE, (GLvoid*)dataGlu);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(0, 0, 0);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(width, 0, 0);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(width, height, 0.0f);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(0, height, 0.0f);
	glEnd();
	drawSkeletals();//want to move this line so it finds multiple bodies
}

void setupVideo() {
	outputVideo.open("video0.avi", 0, 30, cv::Size(width, height), true);//eventually we want this to change each session
}
void writeToVideo() {
	cv::Mat pixels(height, width, CV_8UC3);
	glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels.data);
	cv::Mat cv_pixels(height, width, CV_8UC3);
	for (int y = 0; y < height; y++) for (int x = 0; x < width; x++)
	{
		cv_pixels.at<cv::Vec3b>(y, x)[2] = pixels.at<cv::Vec3b>(height - y - 1, x)[0];
		cv_pixels.at<cv::Vec3b>(y, x)[1] = pixels.at<cv::Vec3b>(height - y - 1, x)[1];
		cv_pixels.at<cv::Vec3b>(y, x)[0] = pixels.at<cv::Vec3b>(height - y - 1, x)[2];
		//cout << "\ncv pixels length: " << cv_pixels.size() << endl;
	}
	outputVideo << cv_pixels;
}

void draw() {
	drawKinectData();
	application.Update();
	glutSwapBuffers();
	writeToVideo();
}

void drawSkeletals() {
	glDisable(GL_TEXTURE_2D);//this should allow the line to appear without transparancy, as it makes it hard to see
	glLineWidth(4);
	glBegin(GL_LINES);
	glColor3f(1.f, 0.f, 0.f);// makes every line thereafter red
	glLineWidth(4);
	for (int i = 0; i < BODY_COUNT; i++) {
		if (trackList[i]) {
			/*
			since we get the joint coordinates in cameraSpacePoints between -1 and 1
			here, we need to move the joint positions by 1 so they're between 0 and 2 (they're currently between -1 and 1),
			half them, as we've doubled the range, then multiply them by the height (X) or width (Y) respectively
			so the coordinates are relative to the screen size
			We've taken out the Z axis (depth) as it limits the detectability of the user when the distance from the camera is too large
			*/
			//restructures the cameraSpacePoints constants into an array:

			/* test line: screen diagonal
			glVertex3f(0, 0, 0);
			glVertex3f(width, height, 0);
			test the coords: we want these values approx. between 0 and 1900, and 0 and 1000 respectively
			cout << jointTranslate[6][0] << endl;
			cout << jointTranslate[6][1] << endl;
			*/





			// neck
			glVertex3f(colorPoints[i][3].X, colorPoints[i][3].Y, 0);
			glVertex3f(colorPoints[i][2].X, colorPoints[i][2].Y, 0);
			// left collarbone
			glVertex3f(colorPoints[i][2].X, colorPoints[i][2].Y, 0);
			glVertex3f(colorPoints[i][4].X, colorPoints[i][4].Y, 0);
			// right collarbone
			glVertex3f(colorPoints[i][2].X, colorPoints[i][2].Y, 0);
			glVertex3f(colorPoints[i][8].X, colorPoints[i][8].Y, 0);
			// left hand
			glVertex3f(colorPoints[i][7].X, colorPoints[i][7].Y, 0);
			glVertex3f(colorPoints[i][6].X, colorPoints[i][6].Y, 0);
			// lower left arm
			glVertex3f(colorPoints[i][6].X, colorPoints[i][6].Y, 0);
			glVertex3f(colorPoints[i][5].X, colorPoints[i][5].Y, 0);
			// upper left arm
			glVertex3f(colorPoints[i][5].X, colorPoints[i][5].Y, 0);
			glVertex3f(colorPoints[i][4].X, colorPoints[i][4].Y, 0);
			// right hand
			glVertex3f(colorPoints[i][11].X, colorPoints[i][11].Y, 0);
			glVertex3f(colorPoints[i][10].X, colorPoints[i][10].Y, 0);
			// lower right arm
			glVertex3f(colorPoints[i][10].X, colorPoints[i][10].Y, 0);
			glVertex3f(colorPoints[i][9].X, colorPoints[i][9].Y, 0);
			// upper right arm
			glVertex3f(colorPoints[i][9].X, colorPoints[i][9].Y, 0);
			glVertex3f(colorPoints[i][8].X, colorPoints[i][8].Y, 0);
			// upper spine
			glVertex3f(colorPoints[i][2].X, colorPoints[i][2].Y, 0);
			glVertex3f(colorPoints[i][20].X, colorPoints[i][20].Y, 0);
			// mid spine -> it can't seem to actually track the midpoint. -> problematic for posture tracking?
			glVertex3f(colorPoints[i][20].X, colorPoints[i][20].Y, 0);
			glVertex3f(colorPoints[i][1].X, colorPoints[i][1].Y, 0);
			// lower spine
			glVertex3f(colorPoints[i][1].X, colorPoints[i][1].Y, 0);
			glVertex3f(colorPoints[i][0].X, colorPoints[i][0].Y, 0);
			// left hand tip
			glVertex3f(colorPoints[i][7].X, colorPoints[i][7].Y, 0);
			glVertex3f(colorPoints[i][21].X, colorPoints[i][21].Y, 0);
			// right hand tip
			glVertex3f(colorPoints[i][11].X, colorPoints[i][11].Y, 0);
			glVertex3f(colorPoints[i][23].X, colorPoints[i][23].Y, 0);
			// left thumb
			glVertex3f(colorPoints[i][7].X, colorPoints[i][7].Y, 0);
			glVertex3f(colorPoints[i][22].X, colorPoints[i][22].Y, 0);
			// right thumb
			glVertex3f(colorPoints[i][11].X, colorPoints[i][11].Y, 0);
			glVertex3f(colorPoints[i][24].X, colorPoints[i][24].Y, 0);
			// left hip
			glVertex3f(colorPoints[i][0].X, colorPoints[i][0].Y, 0);
			glVertex3f(colorPoints[i][12].X, colorPoints[i][12].Y, 0);
			// left thigh
			glVertex3f(colorPoints[i][12].X, colorPoints[i][12].Y, 0);
			glVertex3f(colorPoints[i][13].X, colorPoints[i][13].Y, 0);
			// left shin
			glVertex3f(colorPoints[i][13].X, colorPoints[i][13].Y, 0);
			glVertex3f(colorPoints[i][14].X, colorPoints[i][14].Y, 0);
			// left foot
			glVertex3f(colorPoints[i][14].X, colorPoints[i][14].Y, 0);
			glVertex3f(colorPoints[i][15].X, colorPoints[i][15].Y, 0);
			// right hip
			glVertex3f(colorPoints[i][0].X, colorPoints[i][0].Y, 0);
			glVertex3f(colorPoints[i][16].X, colorPoints[i][16].Y, 0);
			// right thigh
			glVertex3f(colorPoints[i][16].X, colorPoints[i][16].Y, 0);
			glVertex3f(colorPoints[i][17].X, colorPoints[i][17].Y, 0);
			// right shin
			glVertex3f(colorPoints[i][17].X, colorPoints[i][17].Y, 0);
			glVertex3f(colorPoints[i][18].X, colorPoints[i][18].Y, 0);
			// right foot
			glVertex3f(colorPoints[i][18].X, colorPoints[i][18].Y, 0);
			glVertex3f(colorPoints[i][19].X, colorPoints[i][19].Y, 0);
		}
	}
	glColor3f(1.0f, 1.0f, 1.0f);//this makes every vertex after white, which clears the color
	glEnd();
}

bool init(int argc, char* argv[]) {
	application.InitializeDefaultSensor();
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(width, height);
	glutCreateWindow("Dance Health Phase 1");
	glutDisplayFunc(draw);
	glutIdleFunc(draw);
	return true;
}

int main(int argc, char* argv[]) {
	setupVideo();
	if (!init(argc, argv)) return 1;
	if (!initKinect()) return 1;
	// Initialize textures
	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_2D, textureId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height,
		0, GL_BGRA, GL_UNSIGNED_BYTE, (GLvoid*)dataGlu);
	glBindTexture(GL_TEXTURE_2D, 0);

	// OpenGL setup
	glClearColor(0, 0, 0, 0);
	glClearDepth(1.0f);
	glEnable(GL_TEXTURE_2D);

	// Camera setup
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, width, height, 0, 1, -1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glutMainLoop();
	return 0;
}
