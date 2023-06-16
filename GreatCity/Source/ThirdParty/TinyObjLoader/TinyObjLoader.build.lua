project "TinyObjLoader"
    kind "StaticLib"
    language "C++"
    cppdialect "C++11"
    warnings "Off"

    targetdir "%{wks.location}/Binaries/%{cfg.architecture}/%{cfg.buildcfg}"
    objdir "%{wks.location}/Intermediates/%{cfg.architecture}/%{cfg.buildcfg}"

    files
    {
        "%{wks.location}/GreatCity/Source/ThirdParty/TinyObjLoader/Source/tiny_obj_loader.cpp",
        "%{wks.location}/GreatCity/Source/ThirdParty/TinyObjLoader/Include/tiny_obj_loader.h"
    }

    includedirs
    {
        "%{wks.location}/GreatCity/Source/ThirdParty/TinyObjLoader/Include"
    }

    filter "system:windows"
        systemversion "latest"

    filter "configurations:Debug"
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        runtime "Release"
        symbols "On"

    filter "configurations:Distribution"
        runtime "Release"
        optimize "On"
