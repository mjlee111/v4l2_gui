cmake_minimum_required(VERSION 3.5)

# Project name
project(v4l2_gui VERSION 1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Try to find Qt6 first, if not found, try Qt5
find_package(Qt6 COMPONENTS Widgets QUIET)
if(Qt6_FOUND)
    set(QT_VERSION_MAJOR 6)
    message(STATUS "Found Qt6: ${Qt6_VERSION}")
else()
    find_package(Qt5 COMPONENTS Widgets REQUIRED)
    set(QT_VERSION_MAJOR 5)
    message(STATUS "Found Qt5: ${Qt5_VERSION}")
endif()

if(NOT Qt5_FOUND AND NOT Qt6_FOUND)
    message(FATAL_ERROR "Qt5 or Qt6 not found. Please install Qt.")
endif()

# Find libuvc
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBUVC REQUIRED libuvc)

# Find OpenCV
find_package(OpenCV REQUIRED)

# Add threading support
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include 
    ${CMAKE_BINARY_DIR}
    ${LIBUVC_INCLUDE_DIRS}
    ${OpenCV_INCLUDE_DIRS}  # Add OpenCV include directory
)

# Source files
set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/usb_camera.cpp
    src/joystick.cpp
)

# Header files
set(HEADERS
    include/mainwindow.h
    include/usb_camera.h
    include/joystick.h
    include/debug.h
)

# UI files
set(UI_FILES
    ui/mainwindow.ui
)

# Resource files
set(RESOURCE_FILES
    resources/resources.qrc
)

# Generate MOC files
if(QT_VERSION_MAJOR EQUAL 6)
    qt6_wrap_cpp(MOC_SOURCES ${HEADERS})
    qt6_wrap_ui(UIC_SOURCES ${UI_FILES})
    qt_add_resources(RESOURCE_SOURCES ${RESOURCE_FILES})
else()
    qt5_wrap_cpp(MOC_SOURCES ${HEADERS})
    qt5_wrap_ui(UIC_SOURCES ${UI_FILES})
    qt5_add_resources(RESOURCE_SOURCES ${RESOURCE_FILES})
endif()

# Add the executable
add_executable(${PROJECT_NAME} ${SOURCES} ${MOC_SOURCES} ${UIC_SOURCES} ${RESOURCE_SOURCES})

# Link the appropriate Qt Widgets library, libuvc, OpenCV, and pthread
if(QT_VERSION_MAJOR EQUAL 6)
    target_link_libraries(${PROJECT_NAME} Qt6::Widgets ${LIBUVC_LIBRARIES} ${OpenCV_LIBS} Threads::Threads)
else()
    target_link_libraries(${PROJECT_NAME} Qt5::Widgets ${LIBUVC_LIBRARIES} ${OpenCV_LIBS} Threads::Threads)
endif()

# Platform-specific settings
if(WIN32)
    # Windows-specific settings (if any)
elseif(UNIX)
    # Linux-specific settings (if any)
endif()
