project "zlib"
    kind "StaticLib"
    language "C"
    cdialect "C11"
    warnings "Off"

    targetdir "%{wks.location}/Binaries/%{cfg.architecture}/%{cfg.buildcfg}"
    objdir "%{wks.location}/Intermediates/%{cfg.architecture}/%{cfg.buildcfg}"

    files
    {
        "%{wks.location}/GreatCity/Source/ThirdParty/zlib/Source/*.c",
        "%{wks.location}/GreatCity/Source/ThirdParty/zlib/Include/*.h"
    }

    includedirs
    {
        "%{wks.location}/GreatCity/Source/ThirdParty/zlib/Include"
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
