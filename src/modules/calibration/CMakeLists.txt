# Main Module Library
add_library(calibrationmodule
  core.cpp
  definition.cpp
  functions.cpp
  init.cpp
  process.cpp

  calibration.h
)

# 'nogui' library
add_library(calibrationmodulenogui
  nogui.cpp
)

# gui library
if(GUI)
  add_subdirectory(gui)
endif(GUI)

include_directories(
../../
${CMAKE_SOURCE_DIR}
${CMAKE_BINARY_DIR}
${CMAKE_BINARY_DIR}/src
)
