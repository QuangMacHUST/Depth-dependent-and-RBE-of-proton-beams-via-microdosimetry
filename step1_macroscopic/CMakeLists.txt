cmake_minimum_required(VERSION 3.16...3.27)
project(ProtonDepthRBE CXX)

option(WITH_GEANT4_UIVIS "Build with Geant4 UI and Vis drivers" ON)
if(WITH_GEANT4_UIVIS)
  find_package(Geant4 REQUIRED ui_all vis_all)
else()
  find_package(Geant4 REQUIRED)
endif()

include(${Geant4_USE_FILE})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.hh)

add_executable(protonDepth protonDepth.cc ${sources} ${headers})
target_include_directories(protonDepth PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(protonDepth PRIVATE ${Geant4_LIBRARIES})

# Copy macros next to the executable so they can be run from the build dir
file(GLOB macros RELATIVE ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/macros/*.mac)
foreach(_m ${macros})
  get_filename_component(_name ${_m} NAME)
  configure_file(${PROJECT_SOURCE_DIR}/${_m} ${PROJECT_BINARY_DIR}/${_name} COPYONLY)
endforeach()
