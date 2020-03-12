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

void setupVideo() {
	outputVideo.open("vide0.avi", CV_FOURCC('M', 'J', 'P', 'G'), 5, cv::Size(width, height), true);
	//start timer for first frame here, end it before the exit: this will adjust the playback speed to fit "real life"
	application.m_videoTimer = time(0);
	//initialise frame counter here: we'll increment it each time we write to the video
	application.m_frameCounter = 0;
	if (!outputVideo.isOpened()) {
		cout << "video writer failed to open" << endl;
	}
}

void writeToVideo() {
	//this gets the x and y window coordinates of the viewport, followed by its width and height
	double ViewPortParams[4];
	glGetDoublev(GL_VIEWPORT, ViewPortParams);
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
	//writes to the video
	outputVideo << cv_pixels;
	//increments the video counter
	application.m_frameCounter++;
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

void gui_start()
{
	if (application.m_show_gui_start) {
		application.m_show_session_start = false;
		ImGui::SetNextWindowSize(ImVec2(800, 150));
		ImGui::Begin("Input Session ID", NULL);//setting p_open to NULL removes the close button
		if (badUser) {
			ImGui::Text("You mustn't input a blank session ID.");
		}
		else {
			ImGui::Text("Please input a session ID.");
		}
		ImGui::Text("It should match the one entered in the Apple Watch, \nand differ from all session IDs used within the same day. It must be under 10 characters.\nIt can contain any characters, but it should not be blank (in which case you will be re-prompted).");
		ImGui::InputText("", application.m_session_id, 10);
		if (ImGui::Button("Submit")) {
			if (strlen(application.m_session_id) == 0) {
				badUser = true;
			}
			else {//saves the sessionID, closes ID input window, opens start menu
				ImGui::SetNextWindowSize(ImVec2(800, 220));
				application.m_sessionID = application.m_session_id;
				application.m_show_session_start = true;
				application.m_show_gui_start = false;
			}
		}
		ImGui::End();
	}
	if (application.m_show_session_start) {
		ImGui::Begin("Start Window", NULL);
		ImGui::Text("Welcome to Dance Health!\nPlease try to stay within the camera frame.\nThe session will automatically end when you exit the frame for over 5 seconds.");
		ImGui::Text("You can also end the session by pressing the \"End Session\" Button, \nbut if you don't want it to show up in the final video, you can close that pop-up. \nPress start to begin recording.");
		ImGui::Text("Note: When using the Dance Health Application Package with a Kinect \nfor Windows v2 sensor, Microsoft will collect telemetry data(e.g., operating system, \nnumber of processors, graphic chipset, memory, device type, locale, time) \nin order to improve Microsoft products and services.\nThe data will not be used to identify specific individuals.");
		if (ImGui::Button("Start")) {
			application.m_show_session_start = false;
			ImGui::SetNextWindowSize(ImVec2(100, 80));
			show_session_end = true;
			setupVideo();
		}
		ImGui::End();
	}
	if (show_session_end) {
		ImGui::Begin("", &show_session_end);
		application.m_endSessionButton = ImGui::Button("End Session");
		ImGui::End();
	}
}


void drawKinectData() {
	// Start the Dear ImGui frame
	ImGui_ImplOpenGL2_NewFrame();
	ImGui_ImplGLUT_NewFrame();

	gui_start();

	// Rendering
	ImGui::Render();
	ImGuiIO& io = ImGui::GetIO();
	glViewport(0, 0, (GLsizei)io.DisplaySize.x, (GLsizei)io.DisplaySize.y);
	glClear(GL_COLOR_BUFFER_BIT);
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
	ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
	//glutSwapBuffers();
	//glutPostRedisplay();
}

void draw() {
	drawKinectData();
	application.Update();
	if (!application.m_show_gui_start && !application.m_show_session_start){ writeToVideo(); }
	glutSwapBuffers();
	glutPostRedisplay();
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

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	// Setup Platform/Renderer bindings
	ImGui_ImplGLUT_Init();
	ImGui_ImplGLUT_InstallFuncs();
	ImGui_ImplOpenGL2_Init();

	// Load Fonts
	// - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
	// - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
	// - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
	// - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
	// - Read 'docs/FONTS.txt' for more instructions and details.
	// - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
	//io.Fonts->AddFontDefault();
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
	//ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
	//IM_ASSERT(font != NULL);

	// Camera setup
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, width, height, 0, 1, -1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glutMainLoop();

	ImGui_ImplOpenGL2_Shutdown();
	ImGui_ImplGLUT_Shutdown();
	ImGui::DestroyContext();

	return 0;
}