project "Miniz"
    kind "StaticLib"
    language "C"
	staticruntime "off"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-obj/" .. outputdir .. "/%{prj.name}")

    files
    {
        "src/**.c",
    }

	includedirs
	{
		"include/miniz"
	}	

    filter "system:windows"
        systemversion "latest"
		
	filter "system:macosx"
        systemversion "latest"

	filter "system:linux"
		systemversion "latest"

    filter "configurations:Debug"
        symbols "On"

    filter "configurations:Release"
        optimize "On"
		defines "NDEBUG"

