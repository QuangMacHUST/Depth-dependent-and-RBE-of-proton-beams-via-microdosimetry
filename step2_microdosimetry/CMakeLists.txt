cmake_minimum_required(VERSION 3.16...3.27)
project(ProtonMicrodosimetry CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_TESTING "Build the Geant4-independent unit tests" ON)
option(BUILD_SIMULATION "Build the Geant4 application" ON)

# Site sampling does not depend on Geant4 and is tested on its own
add_library(sitesampler STATIC src/SiteSampler.cc)
target_include_directories(sitesampler PUBLIC ${PROJECT_SOURCE_DIR}/include)

if(BUILD_TESTING)
  enable_testing()
  add_executable(test_site_sampler tests/test_site_sampler.cc)
  target_link_libraries(test_site_sampler PRIVATE sitesampler)
  add_test(NAME site_sampler COMMAND test_site_sampler)
endif()

if(BUILD_SIMULATION)
  find_package(Geant4 REQUIRED)
  include(${Geant4_USE_FILE})

  file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc)
  list(REMOVE_ITEM sources ${PROJECT_SOURCE_DIR}/src/SiteSampler.cc)
  file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.hh)

  add_executable(protonMicro protonMicro.cc ${sources} ${headers})
  target_include_directories(protonMicro PRIVATE ${PROJECT_SOURCE_DIR}/include)
  target_link_libraries(protonMicro PRIVATE sitesampler ${Geant4_LIBRARIES})

  file(GLOB macros RELATIVE ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/macros/*.mac)
  foreach(_m ${macros})
    get_filename_component(_name ${_m} NAME)
    configure_file(${PROJECT_SOURCE_DIR}/${_m} ${PROJECT_BINARY_DIR}/${_name} COPYONLY)
  endforeach()
endif()
