project "OpenCV"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "off"

	targetdir ("bin/" .. outputdir)
	objdir ("bin-obj/" .. outputdir)
	
	files
	{
		"include/**.h",
		"include/**.hpp"
	}
	
	includedirs
	{
		"include"
	}
	
	filter "system:linux"
		systemversion "latest"

	filter "system:windows"
		systemversion "latest"
		
	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"