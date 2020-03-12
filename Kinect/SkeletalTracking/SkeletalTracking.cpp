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
	m_fSpecialPostureDuration(5),
	m_SessionDate(""),
	m_session_id(""),
	m_sessionID(""),
	m_show_gui_start(true),
	m_show_session_start(false),
	m_endSessionButton(false)
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
	/*
	if (m_session_id == "") {
		cout << "Please input a session ID. It should match the one entered in the Apple Watch, \nand differ from all session IDs used within the same day." << endl;
		cout << "It can contain any characters, but it should not be blank (in which case you wil be re-prompted)." << endl;
		cout << "Example: \"Name 1\"." << endl;
		cin >> m_session_id;
	}
	*/
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
			//before the session starts
			if (m_show_gui_start || m_show_session_start) {
				cout << "Session hasn't started yet" <<endl;
			}
			//ends after user left screen for 7 seconds or more
			else if ((time(0) - m_fLatestBodyDetectedTime < 7) || (m_nSessionStartTime == 0)  && !m_endSessionButton) //the session can't end if the start button hasn't been pressed
			{
				//cout << m_show_session_start << endl;
				// The main function to process body data e.g. joints
				ProcessBody(BODY_COUNT, ppBodies);

				// Load the used frame data into the "previousframe" pointer to be used in the next frame for comparison
				hr = pBodyFrame->GetAndRefreshBodyData(_countof(m_pBodyFromPreviousFrame), m_pBodyFromPreviousFrame);
			}
			else
			{
				cout << "close" << endl;
				UpdateFloorHeight(pBodyFrame);
				//this creates all the files properly
				CloseClean();
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

// Get the floor height and saved the height into m_fFloorHeight
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

					// Update the latest time that a body is detected on the screen
					m_fLatestBodyDetectedTime = time(0);
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
	// Build the summary data in JSON
	string json;
	json += "{\"id\" : \"" + m_sessionID + "\",";
	json += "\"watchDuration\" : \"-1\",";
	json += "\"minHeartRate\" : \"-1\",";
	json += "\"maxHeartRate\" : \"-1\",";
	json += "\"averageHeartRate\" : \"-1\",";
	json += "\"caloriesBurned\" : \"-1\",";
	json += "\"distanceTravelled\" : \"-1\",";
	json += "\"twists\" : \"-1\", ";
	json += "\"timeStamp\" : \"" + m_SessionDate + "\",";
	json += "\"kinectDurationInSec\" : \"" + to_string(m_fSessionDuration) + "\",";
	json += "\"avgJointDistanceMoved\" : \"" + to_string(m_fSessionAvgJointDisplacement) + "\",";
	json += "\"maxLeftHandHeight\" : \"" + to_string(Calibrate(m_fSessionJointsMaxheight[7])) + "\",";
	json += "\"maxRightHandHeight\" : \"" + to_string(Calibrate(m_fSessionJointsMaxheight[23])) + "\",";
	json += "\"maxLeftKneeHeight\" : \"" + to_string(Calibrate(m_fSessionJointsMaxheight[13])) + "\",";
	json += "\"maxRightKneeHeight\" : \"" + to_string(Calibrate(m_fSessionJointsMaxheight[17])) + "\",";
	json += "\"linkToVideo\" : \"-1\"}";

	// Initialise the curl handle to send the summary json to server
	CURL* curl = curl_easy_init();
	
	// Define http header file
	struct curl_slist *headers = NULL;
	headers = curl_slist_append(headers, "Accept: application/json");
	headers = curl_slist_append(headers, "Content-Type: application/json");
	headers = curl_slist_append(headers, "charsets: utf-8");

	if (curl)
	{
		curl_easy_setopt(curl, CURLOPT_URL, "http://51.11.52.98:3300/dance");
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json.c_str());

		curl_easy_perform(curl);
	}

	// Get the current date and time
	auto currentTime = chrono::system_clock::now();
	time_t time = std::chrono::system_clock::to_time_t(currentTime);

	// Each summary file will be named with the SessionDate and saved as a json file
	ofstream summaryFile;
	//String filename = "SessionSummary" + m_SessionDate + ".json";//clinical resource json in fhir?

	summaryFile.open("kinect"+ m_SessionDate + m_SessionTime +".json");
	summaryFile << json;
	summaryFile.close();
}

ColorSpacePoint SkeletalBasics::CameraToColor(const CameraSpacePoint& bodyPoint)
{
	ColorSpacePoint colorPoint;
	m_pCoordinateMapper->MapCameraPointsToColorSpace(1, &bodyPoint, 1, &colorPoint);
	return colorPoint;
}

void SkeletalBasics::CloseClean() {
	//this gets date and time (at the end of the session) for the naming of the video file and summary
	const time_t now = time(0);
	tm *ltm = localtime(&now);
	if (ltm->tm_mday < 10) {
		m_SessionDate += "0";
	}
	m_SessionDate += to_string(ltm->tm_mday) + "-";
	if (ltm->tm_mon + 1 < 10) {
		m_SessionDate += "0";
	}
	m_SessionDate += to_string(ltm->tm_mon + 1) + "-";
	m_SessionDate += to_string(ltm->tm_year + 1900);

	m_SessionTime += "_" + to_string(ltm->tm_hour) + "-";
	m_SessionTime += to_string(ltm->tm_min);

	string vidName = "vid" + m_SessionDate + m_SessionTime + ".avi";
	// After the session finished, run summary
	Summary();
	//end timer here and calculate time difference
	m_videoTimer = time(0) - m_videoTimer;//gets the length of the video in seconds
	//close the video writer
	outputVideo.release();
	//calculates the fps we want to set the playback speed to, and sets the commands we want to run in the command line
	int newFPS = m_frameCounter / m_videoTimer;
	string commandFfmpeg = "ffmpeg -r " + to_string(newFPS) + " -i vide0.mjpeg -c copy " + vidName;
	//runs ffmpeg video manipulation in command line
	//using "system" is technically not secure, but cross-platform
	system("ffmpeg.exe -i vide0.avi -c copy -f mjpeg vide0.mjpeg");
	system(commandFfmpeg.c_str());

	//deletes the old file with the wrong fps (and the intermediate mjpeg)
	remove("vide0.mjpeg");
	int deleteVid = remove("vide0.avi");
	if (deleteVid != 0) {
		perror("failed to delete obsolete file");
	}
}