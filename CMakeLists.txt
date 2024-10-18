cmake_minimum_required(VERSION 3.15)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "") 
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum OS X deployment version")
set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "Build architectures for Mac OS X" FORCE)
set(SE2JUCE_FOLDER "SE2JUCE_FOLDER_HERE" CACHE PATH "Where is SE2JUCE installed on this system")
set(JUCE_FOLDER "JUCE_FOLDER_HERE" CACHE PATH "Where is JUCE installed on this system")
set(AAX_FOLDER "AAX_SDK_FOLDER_HERE" CACHE PATH "Where is AAX SDK installed on this system")
set(SE_LOCAL_BUILD FALSE CACHE BOOL "Execute extra build steps for developers machine")

set(VST3_SDK ${JUCE_FOLDER}/modules/juce_audio_processors/format_types/VST3_SDK)
set(se_sdk_folder ${SE2JUCE_FOLDER}/SE_DSP_CORE/modules/se_sdk3)
set(SE2JUCE 1)

include(FetchContent)

# GMPI SDK
if("${GMPI_SDK_FOLDER_OVERRIDE}" STREQUAL "")
message(STATUS "Fetching GMPI from github")
#note: SOURCE_SUBDIR is a subfolder with NO cmake file (so we don't needlessly generate projects for the GMPI examples)
FetchContent_Declare(
  gmpi
  GIT_REPOSITORY https://github.com/JeffMcClintock/GMPI
  GIT_TAG origin/main
  SOURCE_SUBDIR Core
)
FetchContent_MakeAvailable(gmpi)
set(GMPI_SDK ${gmpi_SOURCE_DIR})
else()
message(STATUS "Using local GMPI folder")
set(GMPI_SDK ${GMPI_SDK_FOLDER_OVERRIDE})
endif()


if(${SE_LOCAL_BUILD})
set(CMAKE_CONFIGURATION_TYPES "Debug;Release;RelWithDebInfo") 
endif()

# This is for macOS commandline only, because it uses a single-target generator. for other targets, ref: target_compile_definitions
# this point of this is to ensure NDEBUG macro is set
if(NOT GENERATOR_IS_MULTI_CONFIG)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
endif()

project(MyPlugins)

enable_testing()

set(CMAKE_CXX_STANDARD 20)

add_definitions(-D_UNICODE)
add_definitions(-DGMPI_IS_PLATFORM_JUCE=1)

# enable relaxed floating-point compliance for speed
if (MSVC)
    # Floating Point Model: Fast (/fp:fast)
    # Buffer Security Check: No (/GS-)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /fp:fast /GS-")
endif()

if(APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffast-math")
endif()

# include the SynthEdit library
add_subdirectory(${SE2JUCE_FOLDER}/SE_DSP_CORE SE_DSP_CORE)

# include all the plugins you want to build
add_subdirectory(PD303)