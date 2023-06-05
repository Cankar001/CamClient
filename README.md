# CamVision

CamVision is an open source security camera project. It uses OpenCV for image analysis and has a server/client architecture. Each client can either be a camera device or a display. Camera clients analyze the footage locally with opencv, and send the frames to the server, if movement is detected. The server stores the video and also sends the frames to all registered display clients, which show the video on connected monitors.

# Getting started

## Clone the repository
```shell
git clone https://github.com/Cankar001/CamClient.git
```

## Generate the visual studio solution or makefiles
```shell
cd Scripts && python Setup.py
```

# Hardware

Hardware list is coming soon

# Screenshots

Currently I have built a very basic first prototype, including a very basic closure for the camera and for the display.

In these pictures you can see the CamClient application running on a raspberry pi 3b, with the standard touch screen display from raspberry.

![day-time](/Images/client_screenshot_day.jpeg?raw=true "picture from the client at day-time")
![night-time](/Images/client_screenshot_night.jpeg?raw=true "picture from the client at night-time")

