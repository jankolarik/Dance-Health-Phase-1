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

	//video and file saving
	string				 SessionDate;

	long				 videoTimer;

	int					 frameCounter;

private:

	// Body from previous frame
	IBody*               m_pBodyFromPreviousFrame[BODY_COUNT] = { 0 };

	// Floor Height for calibration
	float				 m_fFloorHeight;

	// Session
	bool				 m_bSessionFinished;
	clock_t				 m_nSessionStartTime;
	float                m_fSessionDuration;
	float				 m_fSessionAvgJointDisplacement;
	float				 m_fSessionJointsMaxheight[JointType_Count];
	long				 LastBodyDetectTime;
	string				 session_id;

	// Start & Stop posture
	clock_t              m_nSpecialPostureStartTime;
	float				 m_fSpecialPostureDuration;

	void				 ProcessBody(int nBodyCount, IBody** ppBodies);

	float				 ActivityAnalysis(IBody* pBodyFromCurrentFrame, IBody* pBodyFromPreviousFrame);

	float				 JointDisplacementCalculator(Joint firstJoint, Joint secondJoint);

	float				 GetJointAngle(Joint joints[],int jointNumber);

	float                SingleJointAngleCalculator(Joint centerJoint, Joint endJoint);

	float                SingleJointAngleCalculator(Joint centerJoint1, Joint endJoint1, Joint centerJoint2, Joint endJoint2);

	void				 MaxJointsData(Joint joints[]);

	bool				 SessionStart(Joint joints[]);

	bool                 SessionEnd(Joint joints[]);

	void                 UpdateFloorHeight(IBodyFrame* ppBodyframe);

	float                Calibrate(float num);

	void				 Summary();

	ColorSpacePoint	     CameraToColor(const CameraSpacePoint& bodyPoint);

	void				 CloseClean();

};