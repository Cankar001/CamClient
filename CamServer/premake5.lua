project "CamServer"
    kind "ConsoleApp"
    language "C++"
	cppdialect "C++17"
	staticruntime "off"
	entrypoint "mainCRTStartup"
	
	dependson
	{
		"Cam-Core"
	}

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    debugdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-obj/" .. outputdir .. "/%{prj.name}")

    files
    { 
        "src/**.h",
        "src/**.cpp"
    }

    includedirs
    {
		"src",
		"%{IncludeDir.cam_core}",
		"%{IncludeDir.opencv}",
    }

    links
    {
        "Cam-Core"
    }
	
	filter { "system:windows", "configurations:Debug" }
        systemversion "latest"
		symbols "On"

        defines
        {
            "CAM_PLATFORM_WINDOWS",
			"CAM_DEBUG"
        }

		links
		{
			"%{LibDir.opencv_world_debug}",
			"%{LibDir.opencv_text_debug}",
			"%{LibDir.opencv_tracking_debug}",
			"%{LibDir.opencv_video_debug}",
			"%{LibDir.opencv_videoio_debug}",
			"%{LibDir.opencv_face_debug}",
			"%{LibDir.opencv_stitching_debug}",
			"%{LibDir.opencv_highgui_debug}",
		}

		postbuildcommands
		{	
			("{COPY} %{wks.location}CamClient/vendor/opencv/lib/opencv_world455d.dll %{cfg.targetdir}"),
			("{COPY} %{wks.location}CamClient/vendor/opencv/lib/opencv_text455d.dll %{cfg.targetdir}"),
			("{COPY} %{wks.location}CamClient/vendor/opencv/lib/opencv_tracking455d.dll %{cfg.targetdir}"),
			("{COPY} %{wks.location}CamClient/vendor/opencv/lib/opencv_video455d.dll %{cfg.targetdir}"),
			("{COPY} %{wks.location}CamClient/vendor/opencv/lib/opencv_videoio455d.dll %{cfg.targetdir}"),
			("{COPY} %{wks.location}CamClient/vendor/opencv/lib/opencv_face455d.dll %{cfg.targetdir}"),
			("{COPY} %{wks.location}CamClient/vendor/opencv/lib/opencv_stitching455d.dll %{cfg.targetdir}"),
			("{COPY} %{wks.location}CamClient/vendor/opencv/lib/opencv_highgui455d.dll %{cfg.targetdir}"),
			("{COPY} %{wks.location}CamClient/vendor/opencv/lib/opencv_imgproc455d.dll %{cfg.targetdir}"),
			("{COPY} %{wks.location}CamClient/vendor/opencv/lib/opencv_core455d.dll %{cfg.targetdir}"),
			("{COPY} %{wks.location}CamClient/vendor/opencv/lib/opencv_dnn455d.dll %{cfg.targetdir}"),
			("{COPY} %{wks.location}CamClient/vendor/opencv/lib/opencv_calib3d455d.dll %{cfg.targetdir}"),
			("{COPY} %{wks.location}CamClient/vendor/opencv/lib/opencv_features2d455d.dll %{cfg.targetdir}"),
			("{COPY} %{wks.location}CamClient/vendor/opencv/lib/opencv_flann455d.dll %{cfg.targetdir}"),
	
			("{COPY} %{wks.location}CamClient/vendor/opencv/lib/opencv_videoio_msmf455_64d.dll %{cfg.targetdir}"),
				
			("{COPY} %{wks.location}CamClient/vendor/opencv/lib/opencv_world455.dll %{cfg.targetdir}"),
			("{COPY} %{wks.location}CamClient/vendor/opencv/lib/opencv_text455.dll %{cfg.targetdir}"),
			("{COPY} %{wks.location}CamClient/vendor/opencv/lib/opencv_tracking455.dll %{cfg.targetdir}"),
			("{COPY} %{wks.location}CamClient/vendor/opencv/lib/opencv_video455.dll %{cfg.targetdir}"),
			("{COPY} %{wks.location}CamClient/vendor/opencv/lib/opencv_videoio455.dll %{cfg.targetdir}"),
			("{COPY} %{wks.location}CamClient/vendor/opencv/lib/opencv_face455.dll %{cfg.targetdir}"),
			("{COPY} %{wks.location}CamClient/vendor/opencv/lib/opencv_highgui455.dll %{cfg.targetdir}"),
			("{COPY} %{wks.location}CamClient/vendor/opencv/lib/opencv_imgproc455.dll %{cfg.targetdir}"),
			("{COPY} %{wks.location}CamClient/vendor/opencv/lib/opencv_core455.dll %{cfg.targetdir}"),
			("{COPY} %{wks.location}CamClient/vendor/opencv/lib/opencv_dnn455.dll %{cfg.targetdir}"),
			("{COPY} %{wks.location}CamClient/vendor/opencv/lib/opencv_calib3d455.dll %{cfg.targetdir}"),
			("{COPY} %{wks.location}CamClient/vendor/opencv/lib/opencv_features2d455.dll %{cfg.targetdir}"),
			("{COPY} %{wks.location}CamClient/vendor/opencv/lib/opencv_flann455.dll %{cfg.targetdir}"),
	
			("{COPY} %{wks.location}CamClient/vendor/opencv/lib/opencv_videoio_msmf455_64.dll %{cfg.targetdir}"),
			("{COPY} %{wks.location}CamClient/vendor/opencv/lib/opencv_videoio_ffmpeg455_64.dll %{cfg.targetdir}"),
		}

	filter { "system:linux", "configurations:Debug" }
		systemversion "latest"
		symbols "On"
		
		defines
		{
			"CAM_PLATFORM_LINUX",
			"CAM_DEBUG"
		}

		libdirs { "%{LibDir.opencv_lib_path_linux}/Debug/" }

		links
		{
			"pthread",
			"anl",
			"%{LibDir.opencv_linux_core}",
			"%{LibDir.opencv_linux_imgcodecs}",
			"%{LibDir.opencv_linux_imgproc}",
			"%{LibDir.opencv_linux_text}",
			"%{LibDir.opencv_linux_tracking}",
			"%{LibDir.opencv_linux_video}",
			"%{LibDir.opencv_linux_videoio}",
			"%{LibDir.opencv_linux_face}",
			"%{LibDir.opencv_linux_stitching}",
			"%{LibDir.opencv_linux_highgui}"
		}

	filter { "system:macos", "configurations:Debug" }
		systemversion "latest"
		symbols "On"

		defines
		{
			"CAM_PLATFORM_MACOS",
			"CAM_DEBUG"
		}

		links
		{

		}

	filter { "system:windows", "configurations:Release" }
		systemversion "latest"
        optimize "On"

		defines
		{
			"CAM_PLATFORM_WINDOWS",
			"CAM_RELEASE"
		}

		links
		{
			"%{LibDir.opencv_world}",
			"%{LibDir.opencv_text}",
			"%{LibDir.opencv_tracking}",
			"%{LibDir.opencv_video}",
			"%{LibDir.opencv_videoio}",
			"%{LibDir.opencv_face}",
			"%{LibDir.opencv_stitching}",
			"%{LibDir.opencv_highgui}",
		}

		postbuildcommands
		{	
			("{COPY} %{wks.location}CamClient/vendor/opencv/lib/opencv_world455d.dll %{cfg.targetdir}"),
			("{COPY} %{wks.location}CamClient/vendor/opencv/lib/opencv_text455d.dll %{cfg.targetdir}"),
			("{COPY} %{wks.location}CamClient/vendor/opencv/lib/opencv_tracking455d.dll %{cfg.targetdir}"),
			("{COPY} %{wks.location}CamClient/vendor/opencv/lib/opencv_video455d.dll %{cfg.targetdir}"),
			("{COPY} %{wks.location}CamClient/vendor/opencv/lib/opencv_videoio455d.dll %{cfg.targetdir}"),
			("{COPY} %{wks.location}CamClient/vendor/opencv/lib/opencv_face455d.dll %{cfg.targetdir}"),
			("{COPY} %{wks.location}CamClient/vendor/opencv/lib/opencv_stitching455d.dll %{cfg.targetdir}"),
			("{COPY} %{wks.location}CamClient/vendor/opencv/lib/opencv_highgui455d.dll %{cfg.targetdir}"),
			("{COPY} %{wks.location}CamClient/vendor/opencv/lib/opencv_imgproc455d.dll %{cfg.targetdir}"),
			("{COPY} %{wks.location}CamClient/vendor/opencv/lib/opencv_core455d.dll %{cfg.targetdir}"),
			("{COPY} %{wks.location}CamClient/vendor/opencv/lib/opencv_dnn455d.dll %{cfg.targetdir}"),
			("{COPY} %{wks.location}CamClient/vendor/opencv/lib/opencv_calib3d455d.dll %{cfg.targetdir}"),
			("{COPY} %{wks.location}CamClient/vendor/opencv/lib/opencv_features2d455d.dll %{cfg.targetdir}"),
			("{COPY} %{wks.location}CamClient/vendor/opencv/lib/opencv_flann455d.dll %{cfg.targetdir}"),
	
			("{COPY} %{wks.location}CamClient/vendor/opencv/lib/opencv_videoio_msmf455_64d.dll %{cfg.targetdir}"),
				
			("{COPY} %{wks.location}CamClient/vendor/opencv/lib/opencv_world455.dll %{cfg.targetdir}"),
			("{COPY} %{wks.location}CamClient/vendor/opencv/lib/opencv_text455.dll %{cfg.targetdir}"),
			("{COPY} %{wks.location}CamClient/vendor/opencv/lib/opencv_tracking455.dll %{cfg.targetdir}"),
			("{COPY} %{wks.location}CamClient/vendor/opencv/lib/opencv_video455.dll %{cfg.targetdir}"),
			("{COPY} %{wks.location}CamClient/vendor/opencv/lib/opencv_videoio455.dll %{cfg.targetdir}"),
			("{COPY} %{wks.location}CamClient/vendor/opencv/lib/opencv_face455.dll %{cfg.targetdir}"),
			("{COPY} %{wks.location}CamClient/vendor/opencv/lib/opencv_highgui455.dll %{cfg.targetdir}"),
			("{COPY} %{wks.location}CamClient/vendor/opencv/lib/opencv_imgproc455.dll %{cfg.targetdir}"),
			("{COPY} %{wks.location}CamClient/vendor/opencv/lib/opencv_core455.dll %{cfg.targetdir}"),
			("{COPY} %{wks.location}CamClient/vendor/opencv/lib/opencv_dnn455.dll %{cfg.targetdir}"),
			("{COPY} %{wks.location}CamClient/vendor/opencv/lib/opencv_calib3d455.dll %{cfg.targetdir}"),
			("{COPY} %{wks.location}CamClient/vendor/opencv/lib/opencv_features2d455.dll %{cfg.targetdir}"),
			("{COPY} %{wks.location}CamClient/vendor/opencv/lib/opencv_flann455.dll %{cfg.targetdir}"),
	
			("{COPY} %{wks.location}CamClient/vendor/opencv/lib/opencv_videoio_msmf455_64.dll %{cfg.targetdir}"),
			("{COPY} %{wks.location}CamClient/vendor/opencv/lib/opencv_videoio_ffmpeg455_64.dll %{cfg.targetdir}"),
		}

	filter { "system:linux", "configurations:Release" }
		systemversion "latest"
		optimize "On"

		defines
		{
			"CAM_PLATFORM_LINUX",
			"CAM_RELEASE"
		}

		libdirs { "%{LibDir.opencv_lib_path_linux}/Release/" }

		links
		{
			"pthread",
			"anl",
			"%{LibDir.opencv_linux_core}",
			"%{LibDir.opencv_linux_imgcodecs}",
			"%{LibDir.opencv_linux_imgproc}",
			"%{LibDir.opencv_linux_text}",
			"%{LibDir.opencv_linux_tracking}",
			"%{LibDir.opencv_linux_video}",
			"%{LibDir.opencv_linux_videoio}",
			"%{LibDir.opencv_linux_face}",
			"%{LibDir.opencv_linux_stitching}",
			"%{LibDir.opencv_linux_highgui}"
		}

	filter { "system:macos", "configurations:Release" }
		systemversion "latest"
		optimize "On"

		defines
		{
			"CAM_PLATFORM_MACOS",
			"CAM_RELEASE"
		}

		links
		{

		}

