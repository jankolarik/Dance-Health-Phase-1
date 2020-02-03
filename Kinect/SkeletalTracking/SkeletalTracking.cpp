#include "stdafx.h"
#include "SkeletalTracking.h"


SkeletalBasics::SkeletalBasics() :
	m_pKinectSensor(NULL),
	m_pCoordinateMapper(NULL),
	m_pBodyFrameReader(NULL),
	m_pColorFrameReader(NULL),
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
			m_bTrackList[i] = bTracked;

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
			m_bTrackList[i] = bTracked;

			// If this body is being tracked
			if (SUCCEEDED(hr) && bTracked)
			{
				Joint joints[JointType_Count];

				hr = pBody->GetJoints(_countof(joints), joints);

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
						m_cColorPoints[i][j] = CameraToColor(joints[j].Position);
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

