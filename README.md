# CamVision

CamVision is an open source security camera project. It uses OpenCV for image analysis and has a server/client architecture. Each client can either be a camera device or a display. Camera clients analyze the footage locally with opencv, and send the frames to the server, if movement is detected. The server stores the video and also sends the frames to all registered display clients, which show the video on connected monitors.

# Getting started

## Clone the repository
```shell
git clone https://github.com/Cankar001/CamVision && cd CamVision
```

## Generate the visual studio solution or makefiles
```shell
python Setup.py
```

# Features

The project currently supports these features:

- self-updater: The self updater enables you, to very easily ship new versions to all in-use cameras or displays. You only have to drag-and-drop the update package into a pre-defined folder on the server, the running server listens to this pre-defined folder and recognizes a file system change, re-assembles the update into a transferrable package and ships it to all registered clients fully automated.
- Server/Client system for the camera: The Server/client system has the advantage, that each camera device doesn't have to have a large drive for the videos. It sends the camera feed over the native socket implementation to the server. The server stores the video feed of each camera in a separate ring queue, which has a configurable size. This enables the user to store the last N minutes on demand.
- Different client types: This system currently supports two different client types. The first type is a camera client, which records each frame from a connected camera and sends the frames to the server. The second type is a display client, which gets a live feed from the server from each camera and can display the camera feed on a connected display. The system has these two different types, because not every camera might have a display connected directly to it. In this way, you can setup multiple raspberrys, which are located at different locations and server different roles.

# Planned features

- face detection: planned to support face detection with the Mediapipe library from Google soon.
- face recognition: planned to support face recognition with OpenCV soon.
- web interface: planned to create a web interface (with Laravel and React/Vue.js?)
- Record X last minutes: The ring queue, in which the frames from each client are stored on the server already make this feature possible, but it is planned to create a command interface or a web UI, in which this can be enabled/disabled/triggered.
- Record at specific time: planned to support recording at specific times (for example if the home owner is on vacation)

# Hardware

Hardware list is coming soon

# Screenshots

Currently I have built a very basic first prototype, including a very basic closure for the camera and for the display.

In these pictures you can see the CamClient application running on a raspberry pi 3b, with the standard touch screen display from raspberry.

![day-time](/Images/client_screenshot_day.jpeg?raw=true "picture from the client at day-time")
![night-time](/Images/client_screenshot_night.jpeg?raw=true "picture from the client at night-time")

