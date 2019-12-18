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
	
	void				 Update(float y);

	HRESULT				 InitializeDefaultSensor();

	void				 ProcessBody(int nBodyCount, IBody** ppBodies);

	float	    		 Calibrate();
	float				 leftAndRightFeet(int nBodyCount, IBody** ppBodies);

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
	//Update(false, 0);
	float y = Calibrate();
	while(true){
		Update(y);
	}
	return 0;
}

float SkeletalBasics::Calibrate() {
	/*The calibration period is 5 seconds of the kinect finding the average coordinate between the feet of the subject
	as they stand still in front of the camera. The intention is that this coordinate will be used as the floor position,
	and the origin of the graph for other coordinates.
	*/
	char wait;
	std::cout << "Please stand straight facing the camera, feet firmly planted, legs straight, at maximum 4 meters away.\nPress any key + enter to continue.\n";
	std::cin >> wait; //waits for user to input anything
	clock_t t;
	t = clock();
	float floorHeight = 0;//stores the average height each time
	int i = 0;//keeps track of the number
	while (t < 5) {//keeps checking for 5 seconds
		if (!m_pBodyFrameReader)
		{
			return NULL;
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
				floorHeight += leftAndRightFeet(BODY_COUNT, ppBodies);
			}

			for (int i = 0; i < _countof(ppBodies); ++i)
			{
				SafeRelease(ppBodies[i]);
			}
		}
		SafeRelease(pBodyFrame);
		i++;
		t = clock() - t;
	}
	floorHeight = floorHeight / i;
	return floorHeight;
}

void SkeletalBasics::Update(float FloorY)
{
	if (!m_pBodyFrameReader)
	{
		return;
	}

	IBodyFrame* pBodyFrame = NULL;

	HRESULT hr = m_pBodyFrameReader->AcquireLatestFrame(&pBodyFrame);//Stack Overflow happens here

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

float SkeletalBasics::leftAndRightFeet(int nBodyCount, IBody** ppBodies)
{/*This basically does the same as ProcessBody but it returns the X and Y coordinates of 
   the left and right feet in a pointer to an int array so as to be useful in the calibration
   period
 */
	HRESULT hr;
	char wait;

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
				//this checks if your feet are even in frame before performing the calculations
				hr = pBody->GetJoints(_countof(joints), joints);
				if (SUCCEEDED(joints[15].TrackingState) && SUCCEEDED(joints[19].TrackingState))
				{
					float floorHeight = ((joints[15].Position.Y) + (joints[19].Position.Y)) / 2;
					return floorHeight;
				}
				else {
					std::cout << "Please make sure your foot is in frame!\nPress any key + enter to continue.\n";
					std::cin >> wait; //waits for user to input anything
					return leftAndRightFeet(nBodyCount, ppBodies);
				}
			}
			else {
				std::cout << "Please make sure you are fully in frame!\nPress any key + enter to continue.\n";	
				std::cin >> wait; //waits for user to input anything
				return leftAndRightFeet(nBodyCount, ppBodies);
			}
		}
		else {
			std::cout << "Please make sure there is a person in frame!\nPress any key + enter to continue.\n";
			std::cin >> wait; //waits for user to input anything
			return leftAndRightFeet(nBodyCount, ppBodies);
		}
	}

}

int main()
{
	SkeletalBasics application;
	application.Run();
}
