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

    debugdir "%{wks.location}/GreatCity"
    targetdir "%{wks.location}/Binaries/%{cfg.architecture}/%{cfg.buildcfg}"
    objdir "%{wks.location}/Intermediates/%{cfg.architecture}/%{cfg.buildcfg}"

    files
    {
        "%{wks.location}/GreatCity/Source/**.c",
        "%{wks.location}/GreatCity/Source/**.cpp",
        "%{wks.location}/GreatCity/Source/**.h",
    }

    removefiles
    {
        "%{wks.location}/GreatCity/Source/ThirdParty/**.c",
        "%{wks.location}/GreatCity/Source/ThirdParty/**.cpp",
        "%{wks.location}/GreatCity/Source/ThirdParty/**.h"
    }

    includedirs
    {
        "%{wks.location}/GreatCity/Source",

        "$(VULKAN_SDK)/Include",
        "%{wks.location}/GreatCity/Source/ThirdParty/stb/Include",
        "%{wks.location}/GreatCity/Source/ThirdParty/TinyObjLoader/Include",
        "%{wks.location}/GreatCity/Source/ThirdParty/Flecs/Include",
        "%{wks.location}/GreatCity/Source/ThirdParty/ImGui/Include"
    }

    libdirs
    {
        "$(VULKAN_SDK)/Lib"
    }

    links
    {
        "vulkan-1",

        "Flecs",
        "ImGui"
    }

    flags "FatalWarnings"

    filter "system:windows"
        systemversion "latest"

        defines "GC_PLATFORM_WINDOWS"

        links
        {
            "ShLwApi"
        }

    filter "configurations:Debug"
        defines
        {
            "GC_BUILD_TYPE_DEBUG",
            "GC_ASSERT_ENABLED"
        }

        links
        {
            "shaderc_sharedd"
        }

        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        defines
        {
            "GC_BUILD_TYPE_RELEASE",
            "GC_ASSERT_ENABLED"
        }

        links
        {
            "shaderc_sharedd"
        }

        runtime "Release"
        symbols "On"

    filter "configurations:Distribution"
        defines "GC_BUILD_TYPE_DISTRIBUTION"

        links
        {
            "shaderc_shared"
        }

        runtime "Release"
        optimize "On"

group "Dependencies"
    include "GreatCity/Source/ThirdParty/Flecs/Flecs.build.lua"
    include "GreatCity/Source/ThirdParty/ImGui/ImGui.build.lua"