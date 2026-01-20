cmake_minimum_required(VERSION 3.30)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "") 
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum macOS deployment version")
set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "Build architectures for Mac OS X" FORCE)
set(JUCE_FOLDER "" CACHE PATH "Where is JUCE installed on this system")
set(AAX_FOLDER "" CACHE PATH "Where is AAX SDK installed on this system")
set(SE_LOCAL_BUILD FALSE CACHE BOOL "Execute extra build steps for developers machine")
set(SE2JUCE_FOLDER "" CACHE PATH "Where is SE2JUCE installed on this system")
# CPM bootstrap (shared cache across builds)
set(CPM_DOWNLOAD_VERSION 0.38.6)
set(CPM_USE_NAMED_CACHE_DIRECTORIES ON)
set(CPM_GIT_SHALLOW ON) # shallow clone for speed
if(NOT DEFINED CPM_SOURCE_CACHE)
  if(WIN32)
    set(CPM_SOURCE_CACHE "$ENV{USERPROFILE}/.cpm" CACHE STRING "CPM cache dir")
  else()
    set(CPM_SOURCE_CACHE "$ENV{HOME}/.cache/CPM" CACHE STRING "CPM cache dir")
  endif()
endif()

set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
if(NOT EXISTS "${CPM_DOWNLOAD_LOCATION}")
  file(DOWNLOAD
    "https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake"
    "${CPM_DOWNLOAD_LOCATION}"
    TLS_VERIFY ON
  )
endif()
include("${CPM_DOWNLOAD_LOCATION}")

include(FetchContent)

# JUCE SDK
message(STATUS "Fetching JUCE via CPM (cached)")
# Pin a release or commit to maximize cache hits and ensure reproducibility.
CPMAddPackage(
NAME juce
GITHUB_REPOSITORY juce-framework/JUCE
GIT_TAG 8.0.8       # or a specific commit SHA. KEEP IN SYNC W p_02.yml pipeline.
DOWNLOAD_ONLY YES   # we only need sources; actual add_subdirectory happens later
)
set(JUCE_FOLDER ${juce_SOURCE_DIR})


# SYNTHEDITLIB SDK
if("${SYNTHEDITLIB_FOLDER_OVERRIDE}" STREQUAL "")
message(STATUS "Fetching SE2JUCE from github")
FetchContent_Declare(
  syntheditlib
  GIT_REPOSITORY https://github.com/JeffMcClintock/SE2JUCE
  GIT_TAG origin/main
  SOURCE_SUBDIR Docs
)
FetchContent_MakeAvailable(syntheditlib)
set(SE2JUCE_FOLDER ${syntheditlib_SOURCE_DIR})
else()
message(STATUS "Using local SE2JUCE folder")
set(SE2JUCE_FOLDER ${SYNTHEDITLIB_FOLDER_OVERRIDE})
endif()

set(VST3_SDK ${JUCE_FOLDER}/modules/juce_audio_processors/format_types/VST3_SDK)
set(se_sdk_folder ${SE2JUCE_FOLDER}/SE_DSP_CORE/modules/se_sdk3)
set(SE2JUCE 1)

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

enable_testing()

set(CMAKE_CXX_STANDARD 20)

project(MyPlugins)

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