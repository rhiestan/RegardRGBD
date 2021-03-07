cmake_minimum_required(VERSION 3.12.0)
set (CMAKE_CXX_STANDARD 14)

if(POLICY CMP0091)
    # https://stackoverflow.com/a/56490614
    cmake_policy(SET CMP0091 NEW)
endif()

project(RegardRGBD LANGUAGES C CXX)

#SET(CMAKE_VERBOSE_MAKEFILE TRUE)

# Enable Unicode
ADD_DEFINITIONS(-D_UNICODE -DUNICODE)

# Find CMake modules also in source directory
SET(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_SOURCE_DIR};${CMAKE_SOURCE_DIR}/cmake")

# The options need to be the same as Open3D's default
# If Open3D is configured and built with custom options, you'll also need to
# specify the same custom options.
option(STATIC_WINDOWS_RUNTIME "Use static (MT/MTd) Windows runtime" OFF)
# This needs cmake_policy(SET CMP0091 NEW)
if (STATIC_WINDOWS_RUNTIME)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
else()
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

# Find installed Open3D, which exports Open3D::Open3D
if(WIN32)
    find_package(Open3D HINTS ${CMAKE_INSTALL_PREFIX}/CMake)
else()
    find_package(Open3D HINTS ${CMAKE_INSTALL_PREFIX}/lib/cmake)
endif()
if(NOT Open3D_FOUND)
    message(FATAL_ERROR "Open3D not found, please use -DCMAKE_INSTALL_PREFIX=open3d_install_dir")
endif()

# OpenGL
FIND_PACKAGE(OpenGL REQUIRED)
INCLUDE_DIRECTORIES(${OPENGL_INCLUDE_DIR})

# OpenNI
FIND_PACKAGE(OpenNI REQUIRED)
message("OpenNI found: ${OPENNI_INCLUDE_DIRS} ${OPENNI_LIBRARIES}")
INCLUDE_DIRECTORIES(${OPENNI_INCLUDE_DIRS})


# OpenCV
SET(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${OpenCV_DIR}")
SET(OpenCV_STATIC ON)
FIND_PACKAGE( OpenCV REQUIRED core flann imgproc features2d calib3d highgui imgcodecs videoio ml rgbd)	# ml video legacy objdetect photo gpu videostab ts ocl superres nonfree stitching contrib
# Workaround for OpenCV: Use Release libraries for RelWithDebInfo target
# (found here: http://stackoverflow.com/questions/24262081/cmake-relwithdebinfo-links-to-debug-libs)
SET_TARGET_PROPERTIES(${OpenCV_LIBS} PROPERTIES MAP_IMPORTED_CONFIG_RELWITHDEBINFO RELEASE)
IF(TARGET libjasper)
	SET_TARGET_PROPERTIES(libjasper PROPERTIES MAP_IMPORTED_CONFIG_RELWITHDEBINFO RELEASE)
ENDIF(TARGET libjasper)
IF(TARGET IlmImf)
	SET_TARGET_PROPERTIES(IlmImf PROPERTIES MAP_IMPORTED_CONFIG_RELWITHDEBINFO RELEASE)
ENDIF(TARGET IlmImf)
IF(TARGET libwebp)
	SET_TARGET_PROPERTIES(libwebp PROPERTIES MAP_IMPORTED_CONFIG_RELWITHDEBINFO RELEASE)
ENDIF(TARGET libwebp)
INCLUDE_DIRECTORIES(${OpenCV_INCLUDE_DIRS})

# For debugging the CMake targets
INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/cmake/EchoTargetProperties.cmake)
#echo_targets(${OpenCV_LIBRARIES})
#echo_targets(IlmImf)
#echo_targets("Open3D::Open3D")

FIND_PACKAGE(Eigen3 REQUIRED)
#echo_targets("Eigen3::Eigen")

set(Boost_USE_STATIC_LIBS        ON)
set(Boost_USE_MULTITHREADED      ON)
IF(MSVC)
	SET(Boost_USE_STATIC_RUNTIME     OFF)
ELSE(MSVC)
	IF(MSYS)
		SET(Boost_USE_STATIC_RUNTIME     OFF)
	ELSE(MSYS)
		SET(Boost_USE_STATIC_RUNTIME     OFF)
	ENDIF(MSYS)
ENDIF(MSVC)
#SET(Boost_DEBUG TRUE)
find_package( Boost 1.53.0 COMPONENTS date_time thread
	system chrono serialization filesystem locale random)


# Qt
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC OFF)
set(CMAKE_AUTOUIC OFF)
find_package(Qt5 COMPONENTS Widgets Core REQUIRED)

# OpenSceneGraph
FIND_PACKAGE(OpenSceneGraph 3.0.0 REQUIRED osgFX osgPresentation osgVolume
	osgWidget osgViewer osgAnimation osgText osgDB osgGA osgManipulator
	osgTerrain osgParticle osgShadow osgSim osgUtil osg OpenThreads)

SET(ALL_SRC ${ALL_SRC}
	RegardRGBDMain.cpp
	RegardRGBDMainWindow.cpp
	ONI3DConverter.cpp
	ONIDevice.cpp
	ONIListener.cpp
	Stitcher.cpp
	RegardRGBDModelViewHelper.cpp
	ONIToQtConverter.cpp
	PixmapLabel.cpp
	ScanImageTo3D.cpp
	third_party/QtOSG/OSGWidget.cpp
	third_party/QtOSG/PickHandler.cpp
)

SET(ALL_HEADERS ${ALL_HEADERS}
	RegardRGBDMainWindow.h
	ONI3DConverter.h
	ONIDevice.h
	ONIListener.h
	Stitcher.h
	StitcherI.h
	Vector.h
	RegardRGBDModelViewHelper.h
	ONIToQtConverter.h
	PixmapLabel.h
	ConverterInterface.h
	ScanImageTo3D.h
	version.h
	third_party/QtOSG/OSGWidget.h
	third_party/QtOSG/PickHandler.h
)

# Utils
SET(UTILS_SRC
	utilities/R3DFontHandler.cpp
	utilities/R3DFontHandler.h
	utilities/Conversions.cpp
	utilities/Conversions.h
)
SOURCE_GROUP("Utils" FILES ${UTILS_SRC})

IF(WIN32)
	SET(ALL_SRC ${ALL_SRC} res/regardrgbd.rc)
ENDIF(WIN32)

SET(UI_FILES res/regardrgbd.ui)
SET(RES_FILES res/regardrgbd.qrc)

qt5_wrap_ui(UISrcs ${UI_FILES})
qt5_add_resources(QRC_src ${RES_FILES})
# Move all generated files into a separate group
SOURCE_GROUP("qt_generated" FILES ${UISrcs} ${QRC_src})

ADD_EXECUTABLE(RegardRGBD WIN32 ${ALL_SRC} ${ALL_HEADERS} ${UTILS_SRC} ${UISrcs} ${QRC_src})

TARGET_LINK_LIBRARIES(RegardRGBD
	Open3D::Open3D ${OPENNI_LIBRARIES} ${OpenCV_LIBRARIES}
	Boost::boost Boost::filesystem
	Qt5::Widgets Qt5::Core ${OSG_PLUGINS} ${OPENSCENEGRAPH_LIBRARIES})

target_include_directories(RegardRGBD
	PUBLIC ${CMAKE_BINARY_DIR} ${OPENSCENEGRAPH_INCLUDE_DIRS}
	PRIVATE third_party/QtOSG
)
target_link_directories(RegardRGBD
	PUBLIC ${OSG_PLUGINS_DIR})
target_compile_definitions(RegardRGBD PUBLIC OSG_LIBRARY_STATIC)

# On Windows, when BUILD_SHARED_LIBS, copy .dll to the executable directory
if(WIN32)
    get_target_property(open3d_type Open3D::Open3D TYPE)
    if(open3d_type STREQUAL "SHARED_LIBRARY")
        message(STATUS "Will copy Open3D.dll to ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>")
        add_custom_command(TARGET RegardRGBD POST_BUILD
                        COMMAND ${CMAKE_COMMAND} -E copy
                                ${CMAKE_INSTALL_PREFIX}/bin/Open3D.dll
                                ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>)
    endif()
endif()
