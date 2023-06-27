miniz_include_path = path.getabsolute('vendor/miniz/include')

IncludeDir = {}
IncludeDir["opencv"]             = "%{wks.location}/CamClient/vendor/opencv/include/"
IncludeDir["cam_core"]           = "%{wks.location}/Cam-Core/src"
IncludeDir["miniz"]        		 = miniz_include_path

LibDir = {}
LibDir["opencv_lib_path"]        = "%{wks.location}/CamClient/vendor/opencv/lib"
LibDir["opencv_lib_path_linux"]  = "%{wks.location}/CamClient/vendor/opencv/lib-linux"


LibDir["opencv_world_debug"]     = "%{wks.location}/CamClient/vendor/opencv/lib/Debug/opencv_world455d.lib"
LibDir["opencv_text_debug"]      = "%{wks.location}/CamClient/vendor/opencv/lib/Debug/opencv_text455d.lib"
LibDir["opencv_tracking_debug"]  = "%{wks.location}/CamClient/vendor/opencv/lib/Debug/opencv_tracking455d.lib"
LibDir["opencv_video_debug"]     = "%{wks.location}/CamClient/vendor/opencv/lib/Debug/opencv_video455d.lib"
LibDir["opencv_videoio_debug"]   = "%{wks.location}/CamClient/vendor/opencv/lib/Debug/opencv_videoio455d.lib"
LibDir["opencv_face_debug"]      = "%{wks.location}/CamClient/vendor/opencv/lib/Debug/opencv_face455d.lib"
LibDir["opencv_stitching_debug"] = "%{wks.location}/CamClient/vendor/opencv/lib/Debug/opencv_stitching455d.lib"
LibDir["opencv_highgui_debug"]   = "%{wks.location}/CamClient/vendor/opencv/lib/Debug/opencv_highgui455d.lib"

LibDir["opencv_world"]           = "%{wks.location}/CamClient/vendor/opencv/lib/Release/opencv_world455.lib"
LibDir["opencv_text"]            = "%{wks.location}/CamClient/vendor/opencv/lib/Release/opencv_text455.lib"
LibDir["opencv_tracking"]        = "%{wks.location}/CamClient/vendor/opencv/lib/Release/opencv_tracking455.lib"
LibDir["opencv_video"]           = "%{wks.location}/CamClient/vendor/opencv/lib/Release/opencv_video455.lib"
LibDir["opencv_videoio"]         = "%{wks.location}/CamClient/vendor/opencv/lib/Release/opencv_videoio455.lib"
LibDir["opencv_face"]            = "%{wks.location}/CamClient/vendor/opencv/lib/Release/opencv_face455.lib"
LibDir["opencv_stitching"]       = "%{wks.location}/CamClient/vendor/opencv/lib/Release/opencv_stitching455.lib"
LibDir["opencv_highgui"]         = "%{wks.location}/CamClient/vendor/opencv/lib/Release/opencv_highgui455.lib"

LibDir["opencv_linux_core"]      = "opencv_core"
LibDir["opencv_linux_imgcodecs"] = "opencv_imgcodecs"
LibDir["opencv_linux_imgproc"]   = "opencv_imgproc"
LibDir["opencv_linux_text"]      = "opencv_text"
LibDir["opencv_linux_tracking"]  = "opencv_tracking"
LibDir["opencv_linux_video"]     = "opencv_video"
LibDir["opencv_linux_videoio"]   = "opencv_videoio"
LibDir["opencv_linux_face"]      = "opencv_face"
LibDir["opencv_linux_stitching"] = "opencv_stitching"
LibDir["opencv_linux_highgui"]   = "opencv_highgui"
