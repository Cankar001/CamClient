#pragma once

#include <Cam-Core.h>

#include <opencv2/opencv.hpp>

class Camera
{
public:

	Camera(bool flipImage, uint32 width = 0, uint32 height = 0);
	~Camera();

	/// <summary>
	/// Frees all camera resources.
	/// </summary>
	void Release();

	/// <summary>
	/// Generates the most current frame. Must be called in a loop, to generate continuesly new images.
	/// </summary>
	void GenerateFrames();

	/// <summary>
	/// Shows a specific frame, which was captured before by GenerateFrames. Must be called in a loop, to display a specific frame continuesly.
	/// </summary>
	/// <param name="frameIndex">The frameIndex, from which the image should be displayed.</param>
	/// <returns>Returns true, if the frame index was found and the current image is displayed.</returns>
	bool Show(uint32 frameIndex);

	/// <summary>
	/// Shows every current frame, must be called continuesly, to display each current frame (aka. live video feed)
	/// </summary>
	void ShowLive();

	/// <summary>
	/// Retrieves the image data from a specific frame. The data can be used to e.g. send the frame via the network layer to the server.
	/// </summary>
	/// <param name="frameIndex">The frameIndex, from which the frame should be taken.</param>
	/// <param name="outFrameSize">The size of the frame in bytes.</param>
	/// <param name="outFrameWidth">The width of the resulting frame.</param>
	/// <param name="outFrameHeight">The height of the resulting frame.</param>
	/// <returns>Returns a byte buffer, containing the pixel data of the frame, or nullptr on failure.</returns>
	Byte *GetFrame(uint32 frameIndex, uint32 *outFrameSize, uint32 *outFrameWidth, uint32 *outFrameHeight);

	bool IsRunning() const { return m_CameraRunning; }

private:

	cv::Mat Zoom(cv::Mat frame, std::pair<float, float> center);
	void ZoomIn();
	void ZoomOut();

private:

	uint32 m_Width = 0;
	uint32 m_Height = 0;
	bool m_FlipImage = false;
	float m_Scale = 1.0f;

	bool m_CameraRunning = true;

	bool m_TouchedZoom = false;
	float m_CenterX = 0, m_CenterY = 0;
	float m_RadiusX = 0, m_RadiusY = 0;

	cv::VideoCapture m_CameraStream;
	Core::ThreadSafeQueue<cv::Mat> m_ImageQueue;
};

