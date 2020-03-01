//reference: "an introduction to Dear ImGui Library"  https://blog.conan.io/2019/06/26/An-introduction-to-the-Dear-ImGui-library.html
//OpenGL Setup using GLUT : https://github.com/ocornut/imgui/issues/69
//Example code with opengl2: https://github.com/ocornut/imgui/blob/master/examples/example_glut_opengl2/main.cpp

// OpenGL Header Files
#include <Ole2.h>
#include<gl/GL.h>
#include<gl/GLU.h>
#include<gl/glut.h>
#include<iostream>

//GUI Header Files
#include "imgui.h"
#include"imgui_impl_glut.h"
#include <imgui_impl_opengl2.h>


#define width 1920
#define height 1080
#define GL_BGRA 0x80E1
#define GL_RGB 0x1907

using namespace std;

// Our state
static bool show_gui_start = true;
static bool badUser = false;
static bool show_session_start = false;
static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
char session_id[10] = "";

GLuint textureId;              // ID of the texture to contain Kinect RGB Data
GLubyte dataGlu[width * height * 4];  // BGRA array containing the texture data

void my_display_code()
{
	if (show_gui_start){
		ImGui::Begin("Input Session ID", &show_gui_start, 0);
		if (badUser) {
			ImGui::Text("You must input the session ID");
		}
		ImGui::Text("Please input a session ID. It should match the one entered in the Apple Watch, \nand differ from all session IDs used within the same day. It must be under 10 characters.\nIt can contain any characters, but it should not be blank (in which case you wil be re-prompted)."); 
		ImGui::InputText("", session_id, 10);
		if (ImGui::Button("Submit")) {
			//cout << strcmp(session_id,"") << endl;
			if (strlen(session_id)==0) {
				badUser = true;
			}
			else {
				show_gui_start = false;
				show_session_start = true;
			}
		}
		ImGui::End();
	}
	if (show_session_start) {
		ImGui::Begin("Start Menu", &show_session_start, 0);
		ImGui::Text("Welcome to Dance Health!\nPlease try to stay within the camera frame.\nThe session will automatically end when you exit the frame for over 5 seconds.");
		if (ImGui::Button("Start")) {
			show_session_start = false;
		}
		ImGui::End();
	}
}



void draw() {//we use a flashing red cross to see if the update would work
	// Start the Dear ImGui frame
	ImGui_ImplOpenGL2_NewFrame();
	ImGui_ImplGLUT_NewFrame();

	my_display_code();

	// Rendering
	ImGui::Render();
	ImGuiIO& io = ImGui::GetIO();
	glViewport(0, 0, (GLsizei)io.DisplaySize.x, (GLsizei)io.DisplaySize.y);
	//glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
	glClear(GL_COLOR_BUFFER_BIT);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textureId);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_BGRA, GL_UNSIGNED_BYTE, (GLvoid*)dataGlu);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_TEXTURE_2D);//this should allow the line to appear as not a large rectangle
	glLineWidth(4);
	glBegin(GL_LINES);
	glColor3f(1.f, 0.f, 0.f);//this makes every vertex after red
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(width, height, 0);
	glVertex3f(width, 0.0, 0.0);
	glVertex3f(0.0, height, 0);
	glEnd();
	ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
	glutSwapBuffers();
	glutPostRedisplay();
}

bool init(int argc, char* argv[]) {

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
	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_2D, textureId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, (GLvoid*)dataGlu);
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
	//gui();
	glutMainLoop();
	ImGui_ImplOpenGL2_Shutdown();
	ImGui_ImplGLUT_Shutdown();
	ImGui::DestroyContext();

	return 0;
}