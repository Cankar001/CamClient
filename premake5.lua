include "./vendor/bin/premake/solution_items.lua"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

include "Dependencies.lua"

workspace "CamVision"
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
		include "vendor/miniz"
		include "vendor/spdlog"
	group ""

	group "Core"
		include "Cam-Core"
	group ""

	group "CamVision"
		include "CamClient"
		include "CamServer"
		include "CamDisplay"
	group ""

	group "Updater"
		include "UpdateClient"
		include "UpdateServer"
	group ""

