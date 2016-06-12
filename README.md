A laser painter.

#### Depoly on Windows ####

1. build:
- cmake-gui
- visual studio -> compile
2. deploy:
- windeployqt -> copies all necessary qt dlls
- dependency walker to find dlls for other libraries and frameworks
- 0xc000007b error raises if 32 and 64 libs were mixed; note: system32 = x64; syswow64 = x32
3 installer:
- advanced installer: the easiest solution

#### Depoly on Mac ####

#note: adding the following to cmake creates the native mac project tree
#if(CMAKE_BUILD_TYPE MATCHES Release)
#    add_definitions(-DQT_NO_DEBUG_OUTPUT)
#    if(WIN32)
#        set(GUI_TYPE WIN32)
#    elseif(APPLE)
#        set(GUI_TYPE MACOSX_BUNDLE)
#    endif()
#endif()
#
#add_executable(${PROJECT_NAME} ${GUI_TYPE} ${${PROJECT_NAME}_SOURCES} ${QRC_SOURCES})
#ldd:
#use macdeployqt
#http://stackoverflow.com/questions/9263256/can-you-please-help-me-understand-how-mach-o-libraries-work-in-mac-os-x

ccmake ..
make

~/programs/Qt/5.6/clang_64/bin/macdeployqt ./bin/laser_painter.app/  -dmg

mkdir -p ./bin/laser_painter.app/Contents/Libraries

cp /Users/username/programs/opencv/build/lib/libopencv_imgproc.2.4.dylib ./bin/laser_painter.app/Contents/Libraries
cp /Users/username/programs/opencv/build/lib/libopencv_core.2.4.dylib ./bin/laser_painter.app/Contents/Libraries

/usr/bin/install_name_tool -change /Users/username/programs/opencv/build/lib/libopencv_core.2.4.dylib @rpath/Libraries/libopencv_core.2.4.dylib ./bin/laser_painter.app/Contents/Libraries/libopencv_imgproc.2.4.dylib
/usr/bin/install_name_tool -change /Users/username/programs/opencv/build/lib/libopencv_imgproc.2.4.dylib @rpath/Libraries/libopencv_imgproc.2.4.dylib ./bin/laser_painter.app/Contents/MacOS/laser_painter
/usr/bin/install_name_tool -change /Users/username/programs/opencv/build/lib/libopencv_core.2.4.dylib @rpath/Libraries/libopencv_core.2.4.dylib ./bin/laser_painter.app/Contents/MacOS/laser_painter

~/programs/Qt/5.6/clang_64/bin/macdeployqt ./bin/laser_painter.app/  -dmg
