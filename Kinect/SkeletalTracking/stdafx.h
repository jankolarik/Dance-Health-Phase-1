//------------------------------------------------------------------------------
// Dance Health Phase 1 - Kinect Skeletal Tracking Project
//------------------------------------------------------------------------------

// include file for system and project includes

#pragma once

// Windows Header Files
#include <Windows.h>

// Kinect Header Files
#include <Kinect.h>

// OpenGL Header Files
#include <Ole2.h>
#include<gl/GL.h>
#include<gl/GLU.h>
#include<gl/glut.h>

// OpenCV Header Files
#include<opencv2/opencv.hpp>
#include<opencv2/videoio/videoio_c.h>
#include<opencv2/highgui.hpp>

// Curl header Files
#include <curl.h>

//GUI Header Files
#include "imgui.h"
#include"imgui_impl_glut.h"
#include <imgui_impl_opengl2.h>

// Other libraries
#include <iostream>
#include <cmath>
#include <time.h>
#include <fstream>
#include <chrono>

#define CLOCKS_PER_SEC  ((clock_t)1000)
#define width 1920
#define height 1080
#define GL_BGRA 0x80E1
#define GL_RGB 0x1907

using namespace std;
using namespace cv;

static cv::VideoWriter outputVideo;
extern cv::VideoWriter outputVideo;

// Dear ImGUI Variables
static bool badUser = false;
static bool show_session_end = false;
static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

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