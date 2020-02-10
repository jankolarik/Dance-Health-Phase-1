#include "stdafx.h"
#include "SkeletalTracking.h"


SkeletalBasics::SkeletalBasics() :
	m_pKinectSensor(NULL),
	m_pCoordinateMapper(NULL),
	m_pBodyFrameReader(NULL),
	m_pColorFrameReader(NULL),
	m_fFloorHeight(0),
	m_bSessionFinished(false),
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
		m_fSessionJointsMaxheight[i] = -FLT_MIN;
	}
}


SkeletalBasics::~SkeletalBasics()
{
	//close the video to prevent it from corrupting
	outputVideo.release();
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

		// If the body data is refreshed successfully
		if (SUCCEEDED(hr))
		{
			if (!m_bSessionFinished)
			{
				// The main function to process body data e.g. joints
				ProcessBody(BODY_COUNT, ppBodies);

				// Load the used frame data into the "previousframe" pointer to be used in the next frmae for comparison
				hr = pBodyFrame->GetAndRefreshBodyData(_countof(m_pBodyFromPreviousFrame), m_pBodyFromPreviousFrame);
			}
			else
			{
				UpdateFloorHeight(pBodyFrame);
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


void SkeletalBasics::UpdateFloorHeight(IBodyFrame* ppBodyframe)
{
	HRESULT hr;
	Vector4 floorClipPlane;
	hr = ppBodyframe->get_FloorClipPlane(&floorClipPlane);
	float floorHeight = 0;
	if (SUCCEEDED(hr))
	{
		floorHeight = floorClipPlane.y;
	}
	m_fFloorHeight = floorHeight;
}

// Adding the floor height to the joint coordinate
float SkeletalBasics::Calibrate(float num)
{
	return num + m_fFloorHeight;
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
			m_bTrackList[i] = bTracked;

			// If this body is being tracked
			if (SUCCEEDED(hr) && bTracked)
			{
				Joint joints[JointType_Count];

				hr = pBody->GetJoints(_countof(joints), joints);

				// If joints are obtained successfully, start data process
				if (SUCCEEDED(hr))
				{

					if (SessionStart(joints)) 
					{
						// Initialise Session start time
						if (!m_nSessionStartTime)
						{
							m_nSessionStartTime = clock();
						}
						m_fSessionDuration = float((clock() - m_nSessionStartTime)) / CLOCKS_PER_SEC;

						// Map joints points from Camera point to Color point for displaying purpose
						for (int j = 0; j < _countof(joints); ++j)
						{
							m_cColorPoints[i][j] = CameraToColor(joints[j].Position);
						}

						// Update max height of joints
						MaxJointsData(joints);

						// Joint angle calculation
						cout << "Angle: " << GetJointAngle(joints, 8) << endl;

						// If the data from last frame is loaded, start comparison2
						if (previousBodyLoad)
						{
							cout << "Activity Analysis value : " << ActivityAnalysis(pBody, pBodyPrevious) << endl;
						}
					}
					if (SessionEnd(joints))
					{
						m_bSessionFinished = TRUE;
					}
					m_bSessionFinished = SessionEnd(joints);
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


float SkeletalBasics::GetJointAngle(Joint joints[],int jointNumber)
{
	float jointAngle = 0;
	if (joints[jointNumber].JointType == JointType_ElbowLeft) {
		jointAngle = SingleJointAngleCalculator(joints[5], joints[4], joints[5], joints[6]);
	}
	if (joints[jointNumber].JointType == JointType_ElbowRight) {
		jointAngle = SingleJointAngleCalculator(joints[9], joints[8], joints[9], joints[10]);
	}
	if (joints[jointNumber].JointType == JointType_KneeLeft) {
		jointAngle = SingleJointAngleCalculator(joints[13], joints[12], joints[13], joints[14]);
	}
	if (joints[jointNumber].JointType == JointType_KneeRight) {
		jointAngle = SingleJointAngleCalculator(joints[17], joints[16], joints[17], joints[18]);
	}
	// Joints below are calculated relative to the spine
	if (joints[jointNumber].JointType == JointType_ShoulderLeft) {
		jointAngle = SingleJointAngleCalculator(joints[4], joints[5], joints[20], joints[0]);
	}
	if (joints[jointNumber].JointType == JointType_ShoulderRight) {
		jointAngle = SingleJointAngleCalculator(joints[8], joints[9], joints[20], joints[0]);
	}
	if (joints[jointNumber].JointType == JointType_HipLeft) {
		jointAngle = SingleJointAngleCalculator(joints[12], joints[13], joints[0], joints[20]);
	}
	if (joints[jointNumber].JointType == JointType_HipRight) {
		jointAngle = SingleJointAngleCalculator(joints[16], joints[17], joints[0], joints[20]);
	}
	return jointAngle;
}

// Get joint angle relative to the vertical given another joint connected to it
float SkeletalBasics::SingleJointAngleCalculator(Joint centerJoint, Joint endJoint)
{
	float vector1[] = { endJoint.Position.X - centerJoint.Position.X, endJoint.Position.Y - centerJoint.Position.Y, endJoint.Position.Z - centerJoint.Position.Z };
	float vector2[] = { -1, 0, 0 };

	float magnitude1 = sqrt(pow(vector1[0], 2) + pow(vector1[1], 2) + pow(vector1[2], 2));
	float magnitude2 = 1;

	float dotProduct = vector1[0] * vector2[0] + vector1[1] * vector2[1] + vector1[2] * vector2[2];

	float angleRadian = acos(dotProduct / (magnitude1 * magnitude2));
	float angleDegree = angleRadian * 180 / 3.1415;
	return angleDegree;
}

// Get joint angle given two bones (each bone is defined by connecting two joints together)
float SkeletalBasics::SingleJointAngleCalculator(Joint centerJoint1, Joint endJoint1, Joint centerJoint2, Joint endJoint2)
{
	float vector1[] = { endJoint1.Position.X - centerJoint1.Position.X, endJoint1.Position.Y - centerJoint1.Position.Y, endJoint1.Position.Z - centerJoint1.Position.Z };
	float vector2[] = { endJoint2.Position.X - centerJoint2.Position.X, endJoint2.Position.Y - centerJoint2.Position.Y, endJoint2.Position.Z - centerJoint2.Position.Z };

	float magnitude1 = sqrt(pow(vector1[0], 2) + pow(vector1[1], 2) + pow(vector1[2], 2));
	float magnitude2 = sqrt(pow(vector2[0], 2) + pow(vector2[1], 2) + pow(vector2[2], 2));

	float dotProduct = vector1[0] * vector2[0] + vector1[1] * vector2[1] + vector1[2] * vector2[2];

	float angleRadian = acos(dotProduct / (magnitude1 * magnitude2));
	float angleDegree = angleRadian * 180 / 3.1415;
	return angleDegree;
}


bool SkeletalBasics::SessionStart(Joint joints[])
{
	return true;
}

bool SkeletalBasics::SessionEnd(Joint joints[])
{
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
		<< "Max left  hand height: " << Calibrate(m_fSessionJointsMaxheight[7])  << endl
		<< "Max right hand height: " << Calibrate(m_fSessionJointsMaxheight[23]) << endl
		<< "Max left  knee height: " << Calibrate(m_fSessionJointsMaxheight[13]) << endl
		<< "Max right knee height: " << Calibrate(m_fSessionJointsMaxheight[17]) << endl;

	summaryFile.close();
}

ColorSpacePoint SkeletalBasics::CameraToColor(const CameraSpacePoint& bodyPoint)
{
	ColorSpacePoint colorPoint;
	m_pCoordinateMapper->MapCameraPointsToColorSpace(1, &bodyPoint, 1, &colorPoint);
	return colorPoint;
}