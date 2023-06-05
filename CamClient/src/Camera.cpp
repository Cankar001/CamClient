#include "Camera.h"

#define MAX_RETRIES 5

Camera::Camera(bool flipImage, uint32 width, uint32 height)
	: m_FlipImage(flipImage), m_Width(width), m_Height(height)
{
	m_CameraStream = cv::VideoCapture(0);
	m_CenterX = (float)m_Width / 2;
	m_CenterY = (float)m_Height / 2;

	m_CameraStream.set(cv::CAP_PROP_FRAME_WIDTH, (double)m_Width);
	m_CameraStream.set(cv::CAP_PROP_FRAME_HEIGHT, (double)m_Height);
	std::this_thread::sleep_for(std::chrono::seconds(2));
}

Camera::~Camera()
{
	Release();
}


void Camera::GenerateFrames()
{
	cv::Mat frame;
	uint32 failed_retries = 0;

	bool success = m_CameraStream.read(frame);
	if (!success)
	{
		++failed_retries;
		if (failed_retries >= MAX_RETRIES)
		{
			Release();
		}
		
		return;
	}

	if (m_FlipImage)
	{
		cv::flip(frame, frame, 0);
		cv::flip(frame, frame, 1);
	}

	if (m_TouchedZoom)
	{
		frame = Zoom(frame, { m_CenterX, m_CenterY });
	}
	else
	{
		if (m_Scale != 1)
			frame = Zoom(frame, { 0.0f, 0.0f });
	}

	m_ImageQueue.Enqueue(frame);
	++m_FrameCount;
}

void Camera::Release()
{
	m_CameraRunning = false;
	m_CameraStream.release();
	cv::destroyAllWindows();
}

Byte *Camera::Show(uint32 frameIndex, uint32 *out_frame_size, uint32 *out_frame_width, uint32 *out_frame_height)
{
	if (frameIndex >= m_ImageQueue.Size())
	{
		*out_frame_size = 0;
		*out_frame_width = 0;
		*out_frame_height = 0;
		return nullptr;
	}

	cv::Mat frame = m_ImageQueue.Get(frameIndex);
	cv::namedWindow("Frame", cv::WND_PROP_FULLSCREEN);
	cv::setWindowProperty("Frame", cv::WND_PROP_FULLSCREEN, cv::WND_PROP_FULLSCREEN);
	cv::imshow("Frame", frame);

	*out_frame_size = sizeof(frame.data);
	*out_frame_width = frame.cols;
	*out_frame_height = frame.rows;

	char key = cv::waitKey(1);
	if (key == 'q')
	{
		Release();
	}
	else if (key == 'z')
	{
		ZoomIn();
	}
	else if (key == 'x')
	{
		ZoomOut();
	}

	return frame.data;
}

Byte *Camera::ShowLive(uint32 *out_frame_size, uint32 *out_frame_width, uint32 *out_frame_height)
{
	cv::Mat frame = m_ImageQueue.Dequeue();
	cv::namedWindow("Frame", cv::WND_PROP_FULLSCREEN);
	cv::setWindowProperty("Frame", cv::WND_PROP_FULLSCREEN, cv::WND_PROP_FULLSCREEN);
	cv::imshow("Frame", frame);

	*out_frame_size = sizeof(frame.data);
	*out_frame_width = frame.cols;
	*out_frame_height = frame.rows;

	char key = cv::waitKey(1);
	if (key == 'q')
	{
		Release();
	}
	else if (key == 'z')
	{
		ZoomIn();
	}
	else if (key == 'x')
	{
		ZoomOut();
	}

	return frame.data;
}

cv::Mat Camera::Zoom(cv::Mat frame, std::pair<float, float> center)
{
	int32 width = frame.cols;
	int32 height = frame.rows;

	if (center.first == 0 && center.second == 0)
	{
		m_CenterX = (float)width / 2.0f;
		m_CenterY = (float)height / 2.0f;
		m_RadiusX = (float)width / 2.0f;
		m_RadiusY = (float)height / 2.0f;
	}
	else
	{
		uint32 rate = width / height;
		m_CenterX = center.first;
		m_CenterY = center.second;

		if (m_CenterX < width * (1 - rate))
		{
			m_CenterX = (float)(width * (1 - rate));
		}
		else if (m_CenterX > width * rate)
		{
			m_CenterX = (float)(width * rate);
		}

		if (m_CenterY < height * (1 - rate))
		{
			m_CenterY = (float)(height * (1 - rate));
		}
		else if (m_CenterY > height * rate)
		{
			m_CenterY = (float)(height * rate);
		}

		float left_x = m_CenterX;
		float right_x = width - m_CenterX;
		float up_y = height - m_CenterY;
		float down_y = m_CenterY;

		m_RadiusX = MIN(left_x, right_x);
		m_RadiusY = MIN(up_y, down_y);
	}

	m_RadiusX = m_Scale * m_RadiusX;
	m_RadiusY = m_Scale * m_RadiusY;

	float min_x = m_CenterX - m_RadiusX;
	float max_x = m_CenterX + m_RadiusX;
	float min_y = m_CenterY - m_RadiusY;
	float max_y = m_CenterY + m_RadiusY;

	cv::Rect rect((int32)min_x, (int32)min_y, (int32)max_x, (int32)max_y);
	return frame(rect);
}

void Camera::ZoomIn()
{
	if (m_Scale > 0.2f)
		m_Scale -= 0.1f;
}

void Camera::ZoomOut()
{
	if (m_Scale < 1.0f)
		m_Scale += 0.1f;

	if (m_Scale == 1.0f)
	{
		m_CenterX = (float)m_Width;
		m_CenterY = (float)m_Height;
		m_TouchedZoom = false;
	}
}

