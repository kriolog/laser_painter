set(${PROJECT_NAME}_SOURCES
    main_window.cpp
    video_frame_grabber.cpp
    camera_settings.cpp
    image_modifier.cpp
    laser_detector.cpp
    point_modifier.cpp
    laser_detector_settings.cpp
    tracker_settings.cpp
    laser_detector_calibration_dialog.cpp
    image_widget.cpp
    roi_image_widget.cpp
    track_widget.cpp
    main.cpp
)

include_directories(${OpenCV_INCLUDE_DIRS})

if(CMAKE_BUILD_TYPE MATCHES Release)
    add_definitions(-DQT_NO_DEBUG_OUTPUT)
    if(WIN32)
        set(GUI_TYPE WIN32)
    elseif(APPLE)
        set(GUI_TYPE MACOSX_BUNDLE)
    endif()
endif()

add_executable(${PROJECT_NAME} ${GUI_TYPE} ${${PROJECT_NAME}_SOURCES} ${QRC_SOURCES})

qt5_use_modules(${PROJECT_NAME} LINK_PUBLIC Widgets Multimedia)
target_link_libraries(${PROJECT_NAME} LINK_PUBLIC ${OpenCV_LIBRARIES})
