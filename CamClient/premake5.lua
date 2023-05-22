project "CamClient"
    kind "ConsoleApp"
    language "C++"
	cppdialect "C++17"
	staticruntime "off"
	entrypoint "mainCRTStartup"

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
    }

    links
    {
    }

	postbuildcommands
	{
	}

    filter "system:windows"
        systemversion "latest"

    filter "configurations:Debug"
        defines "CAM_DEBUG"
        symbols "On"

    filter "configurations:Release"
        defines "CAM_RELEASE"
        optimize "On"
		
		