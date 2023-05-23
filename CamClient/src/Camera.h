#pragma once

#include "Core/Core.h"

#include <opencv2/opencv.hpp>
#include <thread>

class Camera
{
public:

	Camera(bool flip, uint32 width = 640, uint32 height = 480);
	~Camera();

	void Release();
	void ReleaseStream();

	bool IsStreaming() const { return m_CameraIsStreaming; }
	void Stream();
	
	void Show();


	cv::Mat Zoom(cv::Mat frame, std::pair<float, float> center);
	void ZoomIn();
	void ZoomOut();

private:

	void StreamImage();

private:

	bool m_Flip = false;
	bool m_TouchedZoom = false;
	bool m_Recording = false;
	bool m_CameraIsStreaming = false;

	cv::VideoCapture m_CameraStream;
	std::thread m_CameraStreamThread;

	uint32 m_Width = 0, m_Height = 0;
	float m_CenterX = 0, m_CenterY = 0;
	float m_RadiusX = 0, m_RadiusY = 0;
	float m_Scale = 1;

};

