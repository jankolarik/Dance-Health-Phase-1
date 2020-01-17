#include <Windows.h>
#include "Kinect.h"
#include <iostream>
#include <cmath>
#include <time.h>       /* clock_t, clock, CLOCKS_PER_SEC */
#include <d2d1.h>

#include <Ole2.h>
#include<gl/GL.h>
#include<gl/GLU.h>
#include<gl/glut.h>

// reference: cs Washington tutorial by Ed Zhang: https://homes.cs.washington.edu/~edzhang/tutorials/kinect2/kinect1.html

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
BOOLEAN bTracked = false;
Joint joints[JointType_Count];

// No need to include the std keyword before cout
using namespace std;

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

	// Calibration Indicator
	bool				 m_bCalibrationStatus;
	float				 m_fCalibrationValue;
	clock_t				 m_nCalibrationStartTime;

	void				 ProcessBody(int nBodyCount, IBody** ppBodies);

	float				 ActivityAnalysis(IBody* pBodyFromCurrentFrame, IBody* pBodyFromPreviousFrame);

	float				 JointDisplacementCalculator(Joint firstJoint, Joint secondJoint);

	void				 Calibration(IBody** ppBodies);

	IBody*				 getSingleBody(IBody** ppBodies);

};

SkeletalBasics application;
void drawSkeletals();

SkeletalBasics::SkeletalBasics() :
	m_pKinectSensor(NULL),
	m_pCoordinateMapper(NULL),
	m_pBodyFrameReader(NULL),
	m_bCalibrationStatus(false),
	m_fCalibrationValue(0),
	m_nCalibrationStartTime(0)
{
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
			else
			{
				// The main function to process body data for each frame
				ProcessBody(BODY_COUNT, ppBodies);
				//kinectMain(argc, argv);
				// Load the used frame data into the "previousframe" pointer to be used in the next frmae for comparison
				hr = pBodyFrame->GetAndRefreshBodyData(_countof(m_pBodyFromPreviousFrame), m_pBodyFromPreviousFrame);
			}
		}

		for (int i = 0; i < _countof(ppBodies); ++i)
		{
			SafeRelease(ppBodies[i]);
		}
	}
	SafeRelease(pBodyFrame);
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
			bTracked = false;
			hr = pBody->get_IsTracked(&bTracked);

			// If this body is being tracked
			if (SUCCEEDED(hr) && bTracked)
			{
				Joint joints[JointType_Count];

				hr = pBody->GetJoints(_countof(joints), joints);

				// If joints are obtained successfully, start data process
				if (SUCCEEDED(hr))
				{

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

	return averageDistanceTravelPerJoint;
}


float SkeletalBasics::JointDisplacementCalculator(Joint firstJoint, Joint secondJoint)
{
	// sqrt( x^2 + y^2 + z^2 )
	float distanceTraveled = sqrt(pow((firstJoint.Position.X - secondJoint.Position.X), 2) + pow((firstJoint.Position.Y - secondJoint.Position.Y), 2) + pow((firstJoint.Position.Z - secondJoint.Position.Z), 2));
	return distanceTraveled;
}

IBody* SkeletalBasics::getSingleBody(IBody** ppBodies)
{

	return nullptr;
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
			bTracked = false;
			hr = pBody->get_IsTracked(&bTracked);

			// If this body is being tracked
			if (SUCCEEDED(hr) && bTracked)
			{
				Joint joints[JointType_Count];

				hr = pBody->GetJoints(_countof(joints), joints);

				if (SUCCEEDED(hr))
				{
					float leftHandHeight = joints[7].Position.Y;
					float rightHandHeight = joints[23].Position.Y;
					float headHeight = joints[3].Position.Y;

					// Calibration starting position: both hands above head
					if ((leftHandHeight > headHeight) && (rightHandHeight > headHeight))
					{
						// Initial calibration start time
						if (!m_nCalibrationStartTime)
						{
							m_nCalibrationStartTime = clock();
						}

						float timeSpent = float((clock() - m_nCalibrationStartTime)) / CLOCKS_PER_SEC;

						if (timeSpent > 5)
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
				}
			}
		}
	}
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
	glClearColor(0, 0, 0, 0);
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
	drawSkeletals();
}

void draw() {	
	application.Update();
	drawKinectData();
	glutSwapBuffers();
}

void drawSkeletals() {
	if (bTracked || !bTracked) {
		/*
		cout << "I found a body!" << endl;
		
		// Draw some arms
		const CameraSpacePoint& lh = joints[6].Position; //wristleft
		const CameraSpacePoint& le = joints[5].Position;//elbowleft
		const CameraSpacePoint& ls = joints[4].Position;//shoulderleft
		const CameraSpacePoint& rh = joints[10].Position;//wristright
		const CameraSpacePoint& re = joints[9].Position;//elbowright
		const CameraSpacePoint& rs = joints[8].Position;//shoulderright
		glDisable(GL_TEXTURE_2D);//this should allow the line to appear as not a large rectangle
		glLineWidth(4);
		glBegin(GL_LINES);
		glColor3f(1.f, 0.f, 0.f);
		glLineWidth(4);
		// lower left arm
		glVertex3f(lh.X, lh.Y, lh.Z);
		glVertex3f(le.X, le.Y, le.Z);
		// upper left arm
		glVertex3f(le.X, le.Y, le.Z);
		glVertex3f(ls.X, ls.Y, ls.Z);
		// lower right arm
		glVertex3f(rh.X, rh.Y, rh.Z);
		glVertex3f(re.X, re.Y, re.Z);
		// upper right arm
		glVertex3f(re.X, re.Y, re.Z);
		glVertex3f(rs.X, rs.Y, rs.Z);
		glColor3f(1.0f, 1.0f, 1.0f);//this makes every vertex after white
		glEnd();
		*/
		glDisable(GL_TEXTURE_2D);//this should allow the line to appear as not a large rectangle
		glLineWidth(4);
		glBegin(GL_LINES);
		glColor3f(1.f, 0.f, 0.f);//this makes every vertex after red
		glVertex3f(0.0, 0.0, 0.0);
		glVertex3f(150,100, 0);
		glColor3f(1.0f, 1.0f, 1.0f);//this makes every vertex after white
		glEnd();
	}
	else {
		//cout << "oops! no body" << endl;
	}
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