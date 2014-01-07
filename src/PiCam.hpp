#include <functional>

#include "bcm_host.h"

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "interface/mmal/mmal.h"
#include "interface/mmal/mmal_logging.h"
#include "interface/mmal/mmal_buffer.h"
#include "interface/mmal/util/mmal_util.h"
#include "interface/mmal/util/mmal_util_params.h"
#include "interface/mmal/util/mmal_default_components.h"
#include "interface/mmal/util/mmal_connection.h"

#include <semaphore.h>

/**
 * A class to interact with the raspberry pi camera module.
 */
class PiCam {
public:
    static const int VIDEO_OUTPUT_BUFFERS_NUM = 3;
    static const int VIDEO_FRAME_RATE_NUM = 30;
    static const int VIDEO_FRAME_RATE_DEN = 1;

    /**
     * Construct a camera object. This initializes the connection
     * to the raspberry pi camera module. There should probably only
     * ever be one of these at a time. Trying to construct a second
     * instance will proabbly fail because only one camera can be 
     * connected anyway.
     */
    PiCam(unsigned int width, unsigned int height, std::function<void(cv::Mat)> callback);

    /**
     * Destructor. This should eventually clean up all the underlying
     * MMAL objects. 
     */
    ~PiCam();
    int port;
    //! Requested width for the video stream.
    unsigned int width;
    //! Requested height for the video stream.
    unsigned int height;
    //! Current frame
    cv::Mat frame;
    //! Callback function that processes each frame.
    std::function<void(cv::Mat)> callback;
    MMAL_POOL_T*        videoPool;
private:
    //! MMAL Component for the camera module.
    MMAL_COMPONENT_T*   cameraComponent;
    //! Port for preview video.
    MMAL_PORT_T*        previewPort;
    //! Port for video.
    MMAL_PORT_T*        videoPort;
    //! Port for still images.
    MMAL_PORT_T*        stillPort;

    //! Current framerate. Currently unused.
    float framerate;
};
