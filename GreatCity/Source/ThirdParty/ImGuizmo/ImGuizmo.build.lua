project "ImGuizmo"
    kind "StaticLib"
    language "C++"
    cppdialect "C++11"

    targetdir "%{wks.location}/Binaries/%{cfg.architecture}/%{cfg.buildcfg}"
    objdir "%{wks.location}/Intermediates/%{cfg.architecture}/%{cfg.buildcfg}"

    files
    {
        "%{wks.location}/GreatCity/Source/ThirdParty/ImGuizmo/Source/**.cpp",
        "%{wks.location}/GreatCity/Source/ThirdParty/ImGuizmo/Include/**.h"
    }

    includedirs
    {
        "%{wks.location}/GreatCity/Source/ThirdParty/ImGuizmo/Include",
        "%{wks.location}/GreatCity/Source/ThirdParty/ImGui/Include",

        "$(VULKAN_SDK)/Include"
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