include "./vendor/bin/premake/solution_items.lua"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

include "Dependencies.lua"

workspace "CamClient"
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

	include "CamClient"
	include "UpdateClient"

