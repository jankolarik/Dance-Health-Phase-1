#include "stdafx.h"
#include "SkeletalTracking.h"

// OpenGL Variables
GLuint textureId;              // ID of the texture to contain Kinect RGB Data
GLubyte dataGlu[width * height * 4];  // BGRA array containing the texture data

SkeletalBasics application;
IKinectSensor* kinectSensor = application.m_pKinectSensor;
IColorFrameReader* colorFrameReader = application.m_pColorFrameReader;


bool initKinect() {
	if (FAILED(GetDefaultKinectSensor(&kinectSensor))) {
		return false;
	}
	if (kinectSensor) {
		kinectSensor->Open();

		IColorFrameSource* framesource = NULL;
		kinectSensor->get_ColorFrameSource(&framesource);
		framesource->OpenReader(&colorFrameReader);
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

void drawSkeletals() {
	glDisable(GL_TEXTURE_2D);//this should allow the line to appear without transparancy, as it makes it hard to see
	glLineWidth(4);
	glBegin(GL_LINES);
	glColor3f(1.f, 0.f, 0.f);// makes every line thereafter red
	glLineWidth(4);
	for (int i = 0; i < BODY_COUNT; i++) {
		if (application.m_bTrackList[i]) {
			// neck
			glVertex3f(application.m_cColorPoints[i][3].X, application.m_cColorPoints[i][3].Y, 0);
			glVertex3f(application.m_cColorPoints[i][2].X, application.m_cColorPoints[i][2].Y, 0);
			// left collarbone
			glVertex3f(application.m_cColorPoints[i][20].X, application.m_cColorPoints[i][20].Y, 0);
			glVertex3f(application.m_cColorPoints[i][4].X, application.m_cColorPoints[i][4].Y, 0);
			// right collarbone
			glVertex3f(application.m_cColorPoints[i][20].X, application.m_cColorPoints[i][20].Y, 0);
			glVertex3f(application.m_cColorPoints[i][8].X, application.m_cColorPoints[i][8].Y, 0);
			// left hand
			glVertex3f(application.m_cColorPoints[i][7].X, application.m_cColorPoints[i][7].Y, 0);
			glVertex3f(application.m_cColorPoints[i][6].X, application.m_cColorPoints[i][6].Y, 0);
			// lower left arm
			glVertex3f(application.m_cColorPoints[i][6].X, application.m_cColorPoints[i][6].Y, 0);
			glVertex3f(application.m_cColorPoints[i][5].X, application.m_cColorPoints[i][5].Y, 0);
			// upper left arm
			glVertex3f(application.m_cColorPoints[i][5].X, application.m_cColorPoints[i][5].Y, 0);
			glVertex3f(application.m_cColorPoints[i][4].X, application.m_cColorPoints[i][4].Y, 0);
			// right hand
			glVertex3f(application.m_cColorPoints[i][11].X, application.m_cColorPoints[i][11].Y, 0);
			glVertex3f(application.m_cColorPoints[i][10].X, application.m_cColorPoints[i][10].Y, 0);
			// lower right arm
			glVertex3f(application.m_cColorPoints[i][10].X, application.m_cColorPoints[i][10].Y, 0);
			glVertex3f(application.m_cColorPoints[i][9].X, application.m_cColorPoints[i][9].Y, 0);
			// upper right arm
			glVertex3f(application.m_cColorPoints[i][9].X, application.m_cColorPoints[i][9].Y, 0);
			glVertex3f(application.m_cColorPoints[i][8].X, application.m_cColorPoints[i][8].Y, 0);
			// upper spine
			glVertex3f(application.m_cColorPoints[i][2].X, application.m_cColorPoints[i][2].Y, 0);
			glVertex3f(application.m_cColorPoints[i][20].X, application.m_cColorPoints[i][20].Y, 0);
			// mid spine -> it can't seem to actually track the midpoint. -> problematic for posture tracking?
			glVertex3f(application.m_cColorPoints[i][20].X, application.m_cColorPoints[i][20].Y, 0);
			glVertex3f(application.m_cColorPoints[i][1].X, application.m_cColorPoints[i][1].Y, 0);
			// lower spine
			glVertex3f(application.m_cColorPoints[i][1].X, application.m_cColorPoints[i][1].Y, 0);
			glVertex3f(application.m_cColorPoints[i][0].X, application.m_cColorPoints[i][0].Y, 0);
			// left hand tip
			glVertex3f(application.m_cColorPoints[i][7].X, application.m_cColorPoints[i][7].Y, 0);
			glVertex3f(application.m_cColorPoints[i][21].X, application.m_cColorPoints[i][21].Y, 0);
			// right hand tip
			glVertex3f(application.m_cColorPoints[i][11].X, application.m_cColorPoints[i][11].Y, 0);
			glVertex3f(application.m_cColorPoints[i][23].X, application.m_cColorPoints[i][23].Y, 0);
			// left thumb
			glVertex3f(application.m_cColorPoints[i][7].X, application.m_cColorPoints[i][7].Y, 0);
			glVertex3f(application.m_cColorPoints[i][22].X, application.m_cColorPoints[i][22].Y, 0);
			// right thumb
			glVertex3f(application.m_cColorPoints[i][11].X, application.m_cColorPoints[i][11].Y, 0);
			glVertex3f(application.m_cColorPoints[i][24].X, application.m_cColorPoints[i][24].Y, 0);
			// left hip
			glVertex3f(application.m_cColorPoints[i][0].X, application.m_cColorPoints[i][0].Y, 0);
			glVertex3f(application.m_cColorPoints[i][12].X, application.m_cColorPoints[i][12].Y, 0);
			// left thigh
			glVertex3f(application.m_cColorPoints[i][12].X, application.m_cColorPoints[i][12].Y, 0);
			glVertex3f(application.m_cColorPoints[i][13].X, application.m_cColorPoints[i][13].Y, 0);
			// left shin
			glVertex3f(application.m_cColorPoints[i][13].X, application.m_cColorPoints[i][13].Y, 0);
			glVertex3f(application.m_cColorPoints[i][14].X, application.m_cColorPoints[i][14].Y, 0);
			// left foot
			glVertex3f(application.m_cColorPoints[i][14].X, application.m_cColorPoints[i][14].Y, 0);
			glVertex3f(application.m_cColorPoints[i][15].X, application.m_cColorPoints[i][15].Y, 0);
			// right hip
			glVertex3f(application.m_cColorPoints[i][0].X, application.m_cColorPoints[i][0].Y, 0);
			glVertex3f(application.m_cColorPoints[i][16].X, application.m_cColorPoints[i][16].Y, 0);
			// right thigh
			glVertex3f(application.m_cColorPoints[i][16].X, application.m_cColorPoints[i][16].Y, 0);
			glVertex3f(application.m_cColorPoints[i][17].X, application.m_cColorPoints[i][17].Y, 0);
			// right shin
			glVertex3f(application.m_cColorPoints[i][17].X, application.m_cColorPoints[i][17].Y, 0);
			glVertex3f(application.m_cColorPoints[i][18].X, application.m_cColorPoints[i][18].Y, 0);
			// right foot
			glVertex3f(application.m_cColorPoints[i][18].X, application.m_cColorPoints[i][18].Y, 0);
			glVertex3f(application.m_cColorPoints[i][19].X, application.m_cColorPoints[i][19].Y, 0);
		}
	}
	glColor3f(1.0f, 1.0f, 1.0f);//this makes every vertex after white, which clears the color
	glEnd();
}

void getKinectData(GLubyte* dest) {
	IColorFrame* frame = NULL;
	if (SUCCEEDED(colorFrameReader->AcquireLatestFrame(&frame))) {
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
	drawSkeletals();
}

void setupVideo() {
	//eventually we want the video name to change each session: the writer will create a new video file.
	//maximum speed (fps here) it'll accept is 14.5; we calibrated it frame by frame to be true to real time
	//10.0 was 28% too fast so we found 7.81 was correct, we might have to use the time library to perfect this
	outputVideo.open("video0.avi", CV_FOURCC('M', 'J', 'P', 'G'), 7.81, cv::Size(width, height), true);
	if (!outputVideo.isOpened()) {
		cout << "video writer failed to open" << endl;
	}
}

void writeToVideo() {
	//this gets the x and y window coordinates of the viewport, followed by its width and height
	double ViewPortParams[4];
	glGetDoublev(GL_VIEWPORT, ViewPortParams);
	//cout << ViewPortParams[2] << " " << ViewPortParams[3] << endl;
	cv::Mat gl_pixels(ViewPortParams[3], ViewPortParams[2], CV_8UC3);
	glReadPixels(0, 0, ViewPortParams[2], ViewPortParams[3], GL_RGB, GL_UNSIGNED_BYTE, gl_pixels.data);
	cv::Mat pixels(height, width, CV_8UC3);
	resize(gl_pixels, pixels, cv::Size(width, height));
	cv::Mat cv_pixels(height, width, CV_8UC3);
	for (int y = 0; y < height; y++) for (int x = 0; x < width; x++)
	{
		cv_pixels.at<cv::Vec3b>(y, x)[2] = pixels.at<cv::Vec3b>(height - y - 1, x)[0];
		cv_pixels.at<cv::Vec3b>(y, x)[1] = pixels.at<cv::Vec3b>(height - y - 1, x)[1];
		cv_pixels.at<cv::Vec3b>(y, x)[0] = pixels.at<cv::Vec3b>(height - y - 1, x)[2];
	}
	outputVideo << cv_pixels;
}

void draw() {
	drawKinectData();
	application.Update();
	writeToVideo();
	glutSwapBuffers();
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