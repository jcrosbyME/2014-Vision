SET(COMPILE_DEFINITIONS -Werror)
add_definitions(-std=c++11 -O2)

include(FindPkgConfig)
pkg_search_module(SDL2 Required sdl2)
find_package( OpenCV REQUIRED )

include_directories(/opt/vc/include)
include_directories(/opt/vc/include/interface/vcos/pthreads)
include_directories(/opt/vc/include/interface/vmcs_host)
include_directories(/opt/vc/include/interface/vmcs_host/linux)

include_directories(/home/pi/src/raspberrypi/userland)
include_directories(/opt/vc/src/hello_pi/libs/vgfont)
include_directories(${SDL2_INCLUDE_DIRS})

link_directories(/opt/vc/lib)
link_directories(/opt/vc/src/hello_pi/libs/vgfont)

add_executable(picam Main.cpp PiCam.cpp)




target_link_libraries(picam mmal_core mmal_util mmal_vc_client vcos bcm_host ${SDL2_LIBRARIES} ${OpenCV_LIBS} openmaxil EGL)
