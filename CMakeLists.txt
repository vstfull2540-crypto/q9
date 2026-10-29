cmake_minimum_required(VERSION 3.22)
project(ArmNongMuangLA2A VERSION 1.1.1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(JUCE_BUILD_EXTRAS OFF CACHE BOOL "" FORCE)
set(JUCE_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

include(FetchContent)

# JUCE is a normal CMake project, so it is safe to add as a subdirectory.
FetchContent_Declare(JUCE
  GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
  GIT_TAG 8.0.6
  GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(JUCE)

# Fetch NAM Core as source only. Do NOT recursively initialize its submodules:
# the upstream Eigen submodule is hosted on GitLab and can fail on Windows CI.
# Fetch the dependencies explicitly so the build has deterministic source paths
# and does not invoke NAM's standalone tools/tests CMake targets.
FetchContent_Declare(NeuralAmpModelerCore
  GIT_REPOSITORY https://github.com/sdatkinson/NeuralAmpModelerCore.git
  GIT_TAG v0.5.4
  GIT_SHALLOW TRUE
  GIT_SUBMODULES "")
FetchContent_GetProperties(NeuralAmpModelerCore)
if(NOT neuralampmodelercore_POPULATED)
  FetchContent_Populate(NeuralAmpModelerCore)
endif()
set(NAM_CORE_DIR "${neuralampmodelercore_SOURCE_DIR}")

# NAM Core v0.5.4 updates its Eigen submodule to Eigen 5.0.1.
# Pin the matching Eigen release explicitly: older Eigen 3.x headers do not
# provide all APIs used by NAM's LSTM implementation (including lastN/seq).
FetchContent_Declare(NAMEigen
  GIT_REPOSITORY https://eigen.googlesource.com/mirror
  GIT_TAG 5.0.1
  GIT_SHALLOW TRUE
  GIT_SUBMODULES "")
FetchContent_GetProperties(NAMEigen)
if(NOT nameigen_POPULATED)
  FetchContent_Populate(NAMEigen)
endif()
set(NAM_EIGEN_DIR "${nameigen_SOURCE_DIR}")
if(EXISTS "${NAM_EIGEN_DIR}/Eigen/src/Core/util/Macros.h")
  file(STRINGS "${NAM_EIGEN_DIR}/Eigen/src/Core/util/Macros.h" NAM_EIGEN_WORLD_LINE REGEX "^#define EIGEN_WORLD_VERSION ")
  file(STRINGS "${NAM_EIGEN_DIR}/Eigen/src/Core/util/Macros.h" NAM_EIGEN_MAJOR_LINE REGEX "^#define EIGEN_MAJOR_VERSION ")
  message(STATUS "Using pinned Eigen headers from ${NAM_EIGEN_DIR}")
endif()

FetchContent_Declare(AudioDSPTools
  GIT_REPOSITORY https://github.com/sdatkinson/AudioDSPTools.git
  GIT_TAG main
  GIT_SHALLOW TRUE
  GIT_SUBMODULES "")
FetchContent_GetProperties(AudioDSPTools)
if(NOT audiodsptools_POPULATED)
  FetchContent_Populate(AudioDSPTools)
endif()
set(NAM_AUDIO_DSP_DIR "${audiodsptools_SOURCE_DIR}")

file(GLOB_RECURSE NAM_CORE_SOURCES CONFIGURE_DEPENDS
  "${NAM_CORE_DIR}/NAM/*.cpp"
  "${NAM_CORE_DIR}/NAM/*.c"
  "${NAM_CORE_DIR}/NAM/*/*.cpp"
  "${NAM_CORE_DIR}/NAM/*/*.c")
if(NOT NAM_CORE_SOURCES)
  message(FATAL_ERROR "NAM Core sources were not downloaded. Check the NeuralAmpModelerCore fetch step.")
endif()
if(NOT EXISTS "${NAM_EIGEN_DIR}/Eigen/Core")
  message(FATAL_ERROR "Eigen headers are missing: ${NAM_EIGEN_DIR}/Eigen/Core")
endif()
if(NOT EXISTS "${NAM_AUDIO_DSP_DIR}/dsp")
  message(FATAL_ERROR "AudioDSPTools headers are missing: ${NAM_AUDIO_DSP_DIR}/dsp")
endif()
if(NOT EXISTS "${NAM_CORE_DIR}/Dependencies/nlohmann/json.hpp")
  message(FATAL_ERROR "NAM Core's bundled nlohmann/json.hpp was not found.")
endif()

add_library(nam_core STATIC ${NAM_CORE_SOURCES})
target_compile_features(nam_core PUBLIC cxx_std_20)
target_include_directories(nam_core SYSTEM PUBLIC
  "${NAM_CORE_DIR}"
  "${NAM_CORE_DIR}/NAM"
  "${NAM_CORE_DIR}/Dependencies"
  "${NAM_CORE_DIR}/Dependencies/nlohmann"
  "${NAM_EIGEN_DIR}"
  "${NAM_AUDIO_DSP_DIR}/dsp")
target_compile_definitions(nam_core PUBLIC NOMINMAX WIN32_LEAN_AND_MEAN)
if(MSVC)
  target_compile_options(nam_core PRIVATE /EHsc /bigobj)
endif()

juce_add_binary_data(LA2AAssets SOURCES Assets/LA2A_Interface.png)
juce_add_plugin(ArmNongMuangLA2A
  COMPANY_NAME "Arm Nong Muang - Muepim Studio"
  BUNDLE_ID "com.armnongmuang.la2a"
  PLUGIN_MANUFACTURER_CODE Anmg
  PLUGIN_CODE La2a
  FORMATS VST3 Standalone
  PRODUCT_NAME "LA-2A Arm Nong Muang"
  IS_SYNTH FALSE NEEDS_MIDI_INPUT FALSE NEEDS_MIDI_OUTPUT FALSE IS_MIDI_EFFECT FALSE
  EDITOR_WANTS_KEYBOARD_FOCUS TRUE COPY_PLUGIN_AFTER_BUILD FALSE)
juce_generate_juce_header(ArmNongMuangLA2A)
target_sources(ArmNongMuangLA2A PRIVATE
  Source/PluginProcessor.cpp Source/PluginProcessor.h
  Source/PluginEditor.cpp Source/PluginEditor.h)
target_compile_definitions(ArmNongMuangLA2A PUBLIC
  JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(ArmNongMuangLA2A PRIVATE
  LA2AAssets nam_core juce::juce_audio_utils juce::juce_audio_processors juce::juce_dsp
  PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)
