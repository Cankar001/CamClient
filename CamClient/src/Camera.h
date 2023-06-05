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
	/// <param name="frameIndex">The frameIndex, from which the image should be extracted.</param>
	/// <param name="out_frame_size">The total size of the frame data in bytes.</param>
	/// <param name="out_frame_width">The width of the resulting frame data.</param>
	/// <param name="out_frame_height">The height of the resulting frame data.</param>
	/// <returns>Returns the pixel data of the requested frame number, if successful, otherwise nullptr.</returns>
	Byte *Show(uint32 frameIndex, uint32 *out_frame_size, uint32 *out_frame_width, uint32 *out_frame_height);

	/// <summary>
	/// Shows every current frame, must be called continuesly, to display each current frame (aka. live video feed)
	/// </summary>
	/// <param name="out_frame_size">The total size of the frame data in bytes.</param>
	/// <param name="out_frame_width">The width of the resulting frame data.</param>
	/// <param name="out_frame_height">The height of the resulting frame data.</param>
	/// <returns>Returns the pixel data of the most current frame, if successful, otherwise nullptr.</returns>
	Byte *ShowLive(uint32 *out_frame_size, uint32 *out_frame_width, uint32 *out_frame_height);

	/// <summary>
	/// Determines, if the camera was closed by the user or is currently running.
	/// </summary>
	/// <returns>Returns true, if the camera is currently running. If it has been closed, it returns false.</returns>
	bool IsRunning() const { return m_CameraRunning; }

	/// <summary>
	/// Returns the current number of frames, which are read from the camera feed.
	/// </summary>
	/// <returns>Returns the current frame count.</returns>
	uint32 GetFrameCount() const { return m_FrameCount; }

	/// <summary>
	/// Returns the current frames per second, which are configured in the camera device feed. It is set once at startup.
	/// </summary>
	/// <returns>Returns the current FPS.</returns>
	uint32 GetFPS() const { return m_FPS; }

	/// <summary>
	/// Returns the current color format, gets overwritten everytime GenerateFrames() is called.
	/// </summary>
	/// <returns>Returns the current color format</returns>
	uint32 GetFormat() const { return m_Format; }

private:

	cv::Mat Zoom(cv::Mat frame, std::pair<float, float> center);
	void ZoomIn();
	void ZoomOut();

private:

	uint32 m_Width = 0;
	uint32 m_Height = 0;
	uint32 m_FrameCount = 0;
	uint32 m_FPS = 0;
	int32 m_Format = 0;
	bool m_FlipImage = false;
	float m_Scale = 1.0f;

	bool m_CameraRunning = true;

	bool m_TouchedZoom = false;
	float m_CenterX = 0, m_CenterY = 0;
	float m_RadiusX = 0, m_RadiusY = 0;

	cv::VideoCapture m_CameraStream;
	Core::ThreadSafeQueue<cv::Mat> m_ImageQueue;
};

