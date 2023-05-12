workspace "GreatCity"
    architecture "x86_64"
    startproject "GreatCity"

    configurations
    {
        "Debug",
        "Release",
        "Distribution"
    }

project "GreatCity"
    kind "ConsoleApp"
    language "C"
    cdialect "C11"
    warnings "Extra"

    targetdir "%{wks.location}/Binaries/%{cfg.architecture}/%{cfg.buildcfg}"
    objdir "%{wks.location}/Intermediates/%{cfg.architecture}/%{cfg.buildcfg}"

    files
    {
        "%{wks.location}/GreatCity/Source/**.c",
        "%{wks.location}/GreatCity/Source/**.h"
    }

    includedirs
    {
        "%{wks.location}/GreatCity/Source"
    }

    flags "FatalWarnings"

    filter "system:windows"
        systemversion "latest"

        defines "GC_PLATFORM_WINDOWS"

    filter "configurations:Debug"
        defines "GC_BUILD_TYPE_DEBUG"
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        defines "GC_BUILD_TYPE_RELEASE"
        runtime "Release"
        symbols "On"

    filter "configurations:Distribution"
        defines "GC_BUILD_TYPE_DISTRIBUTION"
        runtime "Release"
        optimize "On"