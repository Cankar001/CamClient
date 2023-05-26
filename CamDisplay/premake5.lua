project "CamDisplay"
    kind "ConsoleApp"
    language "C++"
	cppdialect "C++17"
	staticruntime "off"
	entrypoint "mainCRTStartup"
	
	dependson
	{
		"Client-Core"
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
        "Client-Core"
    }
	
	postbuildcommands
	{
		--("{COPY} %{wks.location}Client-Core/bin/" .. outputdir .. "/Client-Core/Client-Core.dll %{cfg.targetdir}"),
	
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

    filter "system:windows"
        systemversion "latest"

        defines
        {
            "CAM_PLATFORM_WINDOWS"
        }

	filter "system:linux"
		systemversion "latest"
		
		defines
		{
			"CAM_PLATFORM_LINUX"
		}

	filter "system:macos"
		systemversion "latest"
		
		defines
		{
			"CAM_PLATFORM_MACOS"
		}

    filter "configurations:Debug"
        defines "CAM_DEBUG"
        symbols "On"
		
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

    filter "configurations:Release"
        defines "CAM_RELEASE"
        optimize "On"

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
		
		