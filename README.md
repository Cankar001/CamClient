# CamClient

This client opens a camera feed and does multiple 
calculations with each current frame. 

First, it does a pedestrian detection, 
to ensure that moving humans get detected, 
if they are not very close to the camera. 

If a human gets near the camera, a face detection is 
run as well, where the client compares each face 
in the frame with pictures of humans, 
uploaded to the `profiles` folder.

# Getting started

## Clone the repository
```shell
git clone https://github.com/Cankar001/CamClient.git
```

## Generate the visual studio solution or makefiles
```shell
cd Scripts && python Setup.py
```

