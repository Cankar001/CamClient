project "UpdateServer"
    kind "ConsoleApp"
    language "C++"
	cppdialect "C++17"
	staticruntime "off"
	entrypoint "mainCRTStartup"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    debugdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-obj/" .. outputdir .. "/%{prj.name}")

	dependson
	{
		"Cam-Core",
		"Miniz"
	}

    files
    { 
        "src/**.h",
        "src/**.cpp"
    }

    includedirs
    {
		"src",
		"%{IncludeDir.cam_core}",
		"%{IncludeDir.miniz}"
    }
	
	postbuildcommands
	{
	}
	
	links
	{
		"Cam-Core",
		"Miniz"
	}

    filter "system:windows"
        systemversion "latest"

    filter "system:linux"
        systemversion "latest"

        links
        {
            "pthread",
			"anl",
        }

    filter "configurations:Debug"
        defines "CAM_DEBUG"
        symbols "On"

    filter "configurations:Release"
        defines "CAM_RELEASE"
        optimize "On"
		
		