workspace "simple-webhook"
    architecture "x64"
    targetdir "build"
    configurations {
        "Debug",
        "Release"
    }
    flags {
        "MultiProcessorCompile"
    }
    defines {
        "CURL_STATICLIB"
    }
    startproject "webhook"
    filter "system:windows"
        defines {
            "_CRT_SECURE_NO_WARNINGS"
        }
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
group "dependencies"
project "libcurl"
    kind "StaticLib"
    language "C"
    cdialect "C11"
    staticruntime "on"
    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
    includedirs {
        "vendor/curl/include",
        "vendor/curl/lib"
    }
    defines {
        "USE_OPENSSL",
        "BUILDING_LIBCURL"
    }
    files {
        "vendor/curl/lib/**.c",
        "vendor/curl/lib/**.h",
        "vendor/curl/include/**.h",
    }
    filter "system:windows"
        links {
            "ws2_32.lib",
            "libssl.lib",
            "libcrypto.lib",
            "wldap32.lib",
            "crypt32.lib",
        }
        libdirs {
            "C:/Program Files/OpenSSL/lib"
        }
        sysincludedirs {
            "C:/Program Files/OpenSSL/include"
        }
    filter "configurations:Debug"
        symbols "on"
    filter "configurations:Release"
        optimize "on"
group ""
group "my code"
project "webhook"
kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"
    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
    files {
        "src/**.cpp",
        "src/**.h",
        "stillalive.txt"
    }
    sysincludedirs {
        "vendor/json/include",
        "vendor/curl/include"
    }
    links {
        "libcurl"
    }
    filter "system:windows"
        postbuildcommands {
            '{COPY} "C:/Program Files/OpenSSL/bin/*.dll" "%{cfg.targetdir}"',
        }
    filter "action:xcode4"
        postbuildcommands {
            '{COPY} "stillalive.txt" "%{cfg.targetdir}"'
        }
    filter "configurations:Debug"
        symbols "on"
    filter "configurations:Release"
        optimize "on"
group ""