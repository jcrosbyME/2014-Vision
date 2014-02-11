SET(COMPILE_DEFINITIONS -Werror)
add_definitions(-std=c++11 -O2)

find_package( OpenCV REQUIRED )

include_directories(/opt/vc/include)
include_directories(/opt/vc/include/interface/vcos/pthreads)
include_directories(/opt/vc/include/interface/vmcs_host)
include_directories(/opt/vc/include/interface/vmcs_host/linux)

include_directories(/home/pi/src/raspberrypi/userland)
include_directories(/opt/vc/src/hello_pi/libs/vgfont)

link_directories(/opt/vc/lib)
link_directories(/opt/vc/src/hello_pi/libs/vgfont)

add_executable(picam Main.cpp PiCam.cpp)




target_link_libraries(picam bcm2835 mmal_core mmal_util mmal_vc_client vcos bcm_host ${OpenCV_LIBS} openmaxil EGL)
