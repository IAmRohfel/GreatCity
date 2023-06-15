project "libpng"
    kind "StaticLib"
    language "C"
    cdialect "C11"
    warnings "Off"

    targetdir "%{wks.location}/Binaries/%{cfg.architecture}/%{cfg.buildcfg}"
    objdir "%{wks.location}/Intermediates/%{cfg.architecture}/%{cfg.buildcfg}"

    files
    {
        "%{wks.location}/GreatCity/Source/ThirdParty/libpng/Source/*.c",
        "%{wks.location}/GreatCity/Source/ThirdParty/libpng/Include/*.h"
    }

    includedirs
    {
        "%{wks.location}/GreatCity/Source/ThirdParty/libpng/Include",
        "%{wks.location}/GreatCity/Source/ThirdParty/zlib/Include"
    }

    links
    {
        "zlib"
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
