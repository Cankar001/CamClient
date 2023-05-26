include "./vendor/bin/premake/solution_items.lua"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

include "Dependencies.lua"

workspace "Cam"
    architecture "x64"
    configurations { "Debug", "Release" }

	solution_items
	{
		".editorconfig"
	}

	flags
	{
		"MultiProcessorCompile"
	}

	group "Dependencies"
		include "CamClient/vendor/opencv"
	group ""

	group "Core"
		include "Client-Core"
	group ""

	group "Cam"
		include "CamClient"
		include "CamServer"
		include "CamDisplay"
	group ""

	group "Updater"
		include "UpdateClient"
		include "UpdateServer"
	group ""

