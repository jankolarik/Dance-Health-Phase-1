//------------------------------------------------------------------------------
// Dance Health Phase 1 - Kinect Skeletal Tracking Project
//------------------------------------------------------------------------------

#pragma once

class SkeletalBasics
{
public:
	// Constructor
	SkeletalBasics();

	// Destructor
	~SkeletalBasics();

	// Current Kinect
	IKinectSensor*       m_pKinectSensor;
	ICoordinateMapper*   m_pCoordinateMapper;

	// Body & Color reader
	IBodyFrameReader*    m_pBodyFrameReader;
	IColorFrameReader*   m_pColorFrameReader;

	// Drawing 
	bool                 m_bTrackList[BODY_COUNT];
	ColorSpacePoint      m_cColorPoints[BODY_COUNT][JointType_Count];

	void				 Update();

	HRESULT				 InitializeDefaultSensor();

private:

	// Body from previous frame
	IBody*               m_pBodyFromPreviousFrame[BODY_COUNT] = { 0 };

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