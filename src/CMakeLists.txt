set(${PROJECT_NAME}_SOURCES
    camera_frame_grabber.cpp
    main.cpp
)

include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_SOURCES} ${QRC_SOURCES})

qt5_use_modules(${PROJECT_NAME} LINK_PUBLIC Widgets Multimedia)
target_link_libraries(${PROJECT_NAME} LINK_PUBLIC ${OpenCV_LIBRARIES})
