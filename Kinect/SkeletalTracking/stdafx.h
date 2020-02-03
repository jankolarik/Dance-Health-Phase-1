//------------------------------------------------------------------------------
// Dance Health Phase 1 - Kinect Skeletal Tracking Project
//------------------------------------------------------------------------------

// include file for system and project includes

#pragma once

// Windows Header Files
#include <Windows.h>

// Kinect Header Files
#include "Kinect.h"

// OpenGL Header Files
#include <Ole2.h>
#include<gl/GL.h>
#include<gl/GLU.h>
#include<gl/glut.h>

// OpenCV Header Files
#include<opencv2/opencv.hpp>
#include<opencv2/videoio/videoio_c.h>
#include<opencv2/highgui.hpp>

// Other libraries
#include <iostream>
#include <cmath>
#include <time.h>
#include <iostream>
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