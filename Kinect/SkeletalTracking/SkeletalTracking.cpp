#include <Windows.h>
#include "Kinect.h"
#include <iostream>
#include <time.h>       /* clock_t, clock, CLOCKS_PER_SEC */

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

	int Run();

private:
	// Current Kinect
	IKinectSensor*		 m_pKinectSensor;
	ICoordinateMapper*	 m_pCoordinateMapper;

	// Body reader
	IBodyFrameReader*	 m_pBodyFrameReader;
	
	void				 Update(bool calibrated, int x, int y);

	HRESULT				 InitializeDefaultSensor();

	void				 ProcessBody(int nBodyCount, IBody** ppBodies);

	int *	    		 Calibrate();
	int *				 leftAndRightFeet((int nBodyCount, IBody** ppBodies));

};

SkeletalBasics::SkeletalBasics() :
	m_pKinectSensor(NULL),
	m_pCoordinateMapper(NULL),
	m_pBodyFrameReader(NULL)
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

int SkeletalBasics::Run()
{
	InitializeDefaultSensor();
	Update(False, 0, 0);
	return 0;
}

int* SkeletalBasics::Calibrate() {
	cout << "Please stand straight facing the camera, feet firmly planted, legs straight, at maximum 4 meters away.";
	clock_t t;
	t = clock();
	int * temp;
	int xPos; //stores the x axis in the middle/average of the left and right feet each time
	int height;//stores the average height each time
	int* finalCoords;
	int i = 0;//keeps track of the number
	while (t < 300) {//keeps checking for 5 minutes
		if (!m_pBodyFrameReader)
		{
			return;
		}

		IBodyFrame* pBodyFrame = NULL;

		HRESULT hr = m_pBodyFrameReader->AcquireLatestFrame(&pBodyFrame);

		if (SUCCEEDED(hr))
		{

			IBody* ppBodies[BODY_COUNT] = { 0 };

			if (SUCCEEDED(hr))
			{
				hr = pBodyFrame->GetAndRefreshBodyData(_countof(ppBodies), ppBodies);
			}

			if (SUCCEEDED(hr))
			{
				temp = leftAndRightFeet(BODY_COUNT, ppBodies);
			}

			for (int i = 0; i < _countof(ppBodies); ++i)
			{
				SafeRelease(ppBodies[i]);
			}
		}
		SafeRelease(pBodyFrame);
		xPos += (temp[0] + temp[2]) / 2;
		height += (temp[1] + temp[3]) / 2;
		i++;
		t = clock() - t;
	}
	xPos = xPos / i;
	height = height / i;
	finalCoords[0] = xPos;
	finalCoords[1] = height;
	return finalCoords;
}

void SkeletalBasics::Update(bool calibrated, int FloorX, int FloorY)
{ 
	if (!calibrated) {
		int * temp = Calibrate();
		FloorX = temp[0];
		FloorY = temp[1];
		calibrated = !calibrated;
	}

	if (!m_pBodyFrameReader)
	{
		return;
	}

	IBodyFrame* pBodyFrame = NULL;

	HRESULT hr = m_pBodyFrameReader->AcquireLatestFrame(&pBodyFrame);

	if (SUCCEEDED(hr))
	{

		IBody* ppBodies[BODY_COUNT] = { 0 };

		if (SUCCEEDED(hr))
		{
			hr = pBodyFrame->GetAndRefreshBodyData(_countof(ppBodies), ppBodies);
		}

		if (SUCCEEDED(hr))
		{
			ProcessBody(BODY_COUNT, ppBodies);
		}

		for (int i = 0; i < _countof(ppBodies); ++i)
		{
			SafeRelease(ppBodies[i]);
		}
	}
	SafeRelease(pBodyFrame);
	Update(calibrated, FloorX, FloorY);
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

void SkeletalBasics::ProcessBody(int nBodyCount, IBody** ppBodies)
{
	HRESULT hr;

	for (int i = 0; i < nBodyCount; ++i)
	{
		IBody* pBody = ppBodies[i];

		if (pBody)
		{
			BOOLEAN bTracked = false;
			hr = pBody->get_IsTracked(&bTracked);

			if (SUCCEEDED(hr) && bTracked)
			{
				Joint joints[JointType_Count];

				HandState leftHandState = HandState_Unknown;
				HandState rightHandState = HandState_Unknown;

				pBody->get_HandLeftState(&leftHandState);
				pBody->get_HandRightState(&rightHandState);

				hr = pBody->GetJoints(_countof(joints), joints);
				if (SUCCEEDED(hr))
				{
					std::cout << joints[7].Position.Y << std::endl;
					for (int j = 0; j < _countof(joints); ++j)
					{
						//std::cout << joints[j].JointType;
						//std::cout << joints[j].Position.X << std::endl;
					}
				}

			}
		}
	}

}

int * SkeletalBasics::leftAndRightFeet(int nBodyCount, IBody** ppBodies)
{
	HRESULT hr;

	for (int i = 0; i < nBodyCount; ++i)
	{
		IBody* pBody = ppBodies[i];

		if (pBody)
		{
			BOOLEAN bTracked = false;
			hr = pBody->get_IsTracked(&bTracked);

			if (SUCCEEDED(hr) && bTracked)
			{
				Joint joints[JointType_Count];

				FootState leftFootState = FootState_Unknown;
				FootState rightFootState = FootState_Unknown;

				pBody->get_FootLeftState(&leftFootState);
				pBody->get_FootRightState(&rightFootState);

				hr = pBody->GetJoints(_countof(joints), joints);
				if (SUCCEEDED(hr))
				{
					int* positions;
					positions[0] = joints[15].Position.X;
					positions[1] = joints[15].Position.Y;
					positions[2] = joints[19].Position.X;
					positions[3] = joints[19].Position.Y;
					return positions;
				}

			}
		}
	}

}

int main()
{
	SkeletalBasics application;
	application.Run();
}
