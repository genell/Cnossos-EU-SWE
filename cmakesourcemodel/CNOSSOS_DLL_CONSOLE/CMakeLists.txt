cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)

################### Variables. ####################
# Change if you want modify path or other values. #
###################################################

set(PROJECT_NAME CNOSSOS_DLL_CONSOLE)
# Output Variables
# Folders files
set(CPP_DIR_1 .)
set(HEADER_DIR_1 .)

############## Define Project. ###############
# ---- This the main options of project ---- #
##############################################

project(${PROJECT_NAME} CXX)

# Define Release by default.
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
  message(STATUS "Build type not specified: defaulting to release.")
endif(NOT CMAKE_BUILD_TYPE)

# Definition of Macros
add_definitions(
   -D_DEBUG 
   -D_CONSOLE 
   -DUNICODE
   -D_UNICODE
)

################### Dependencies ##################
# Add Dependencies to project.                    #
###################################################

option(BUILD_DEPENDS 
   "Build other CMake project." 
   ON 
)

# Dependencies : disable BUILD_DEPENDS to link with lib already build.
if(BUILD_DEPENDS)
   add_subdirectory(../CNOSSOS_INDUSTRIAL_NOISE_DLL ${CMAKE_BINARY_DIR}/CNOSSOS_INDUSTRIAL_NOISE_DLL)
   add_subdirectory(../CNOSSOS_RAILNOISE_DLL ${CMAKE_BINARY_DIR}/CRONNOS_RAILNOISE_DLL)
   add_subdirectory(../CNOSSOS_ROADNOISE_DLL ${CMAKE_BINARY_DIR}/CRONNOS_ROADNOISE_DLL)
else()
   link_directories(dependencies/CNOSSOS_INDUSTRIAL_NOISE_DLL/build/)
   link_directories(dependencies/CRONNOS_RAILNOISE_DLL/build/)
   link_directories(dependencies/CRONNOS_ROADNOISE_DLL/build/)
endif()

################# Flags ################
# Defines Flags for Windows and Linux. #
########################################

if(MSVC)
   set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /W3 /EHsc")
   set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /W3 /EHsc")
endif(MSVC)
if(NOT MSVC)
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
   if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
       set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
   endif()
endif(NOT MSVC)

################ Files ################
#   --   Add files to project.   --   #
#######################################

file(GLOB SRC_FILES
    ${CPP_DIR_1}/*.cpp
    ${HEADER_DIR_1}/*.h
)

# Add executable to build.
#add_executable(${PROJECT_NAME} 
add_library(${PROJECT_NAME} ${SRC_FILES})

# Link with other dependencies.
target_link_libraries(${PROJECT_NAME} CNOSSOS_INDUSTRIAL_NOISE_DLL CRONNOS_RAILNOISE_DLL CRONNOS_ROADNOISE_DLL )
