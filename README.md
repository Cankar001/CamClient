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

