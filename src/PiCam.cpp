/*
 * This file was adapted from RaspiVid.c, to serve as a simple c++ 
 * interface to the raspberry pi camera module.
 */

#include "PiCam.hpp"
#include <stdexcept>

#define MMAL_CAMERA_PREVIEW_PORT 0
#define MMAL_CAMERA_VIDEO_PORT 1
#define MMAL_CAMERA_CAPTURE_PORT 2

static void video_buffer_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer) {
    PiCam* cam = (PiCam*) port->userdata;
    if (buffer->length) {
	    mmal_buffer_header_mem_lock(buffer);
        memcpy(cam->frame.data, buffer->data, 3*cam->width*cam->height);
        cam->callback(cam->frame);
        cv::waitKey(1);
        mmal_buffer_header_mem_unlock(buffer);
    }
    
    // release buffer back to the pool
    mmal_buffer_header_release(buffer);


    // and send one back to the port (if still open)
    if (port->is_enabled)
    {
        MMAL_STATUS_T status;

        MMAL_BUFFER_HEADER_T* new_buffer = mmal_queue_get(cam->videoPool->queue);

        if (new_buffer)
        status = mmal_port_send_buffer(port, new_buffer);

        if (!new_buffer || status != MMAL_SUCCESS)
            vcos_log_error("Unable to return a buffer to the encoder port");
    }
}

PiCam::PiCam(unsigned int width, unsigned int height, std::function<void(cv::Mat)> callback) :
    width(width), height(height), callback(callback), videoPool(videoPool),
    cameraComponent(nullptr), previewPort(nullptr), videoPort(nullptr), stillPort(nullptr)
{
    bcm_host_init();

    MMAL_STATUS_T status;
	status = mmal_component_create(MMAL_COMPONENT_DEFAULT_CAMERA, &cameraComponent);

    if(status != MMAL_SUCCESS) throw std::runtime_error("Couldn't create camera component.");
    if(!cameraComponent->output_num) throw std::runtime_error("Camera doesn't have output ports.");

	videoPort = cameraComponent->output[MMAL_CAMERA_VIDEO_PORT];
	stillPort = cameraComponent->output[MMAL_CAMERA_CAPTURE_PORT];

	{
	   MMAL_PARAMETER_CAMERA_CONFIG_T cam_config =
	   {
	      { MMAL_PARAMETER_CAMERA_CONFIG, sizeof(cam_config) },
	      .max_stills_w = width,
	      .max_stills_h = height,
	      .stills_yuv422 = 0,
	      .one_shot_stills = 0,
	      .max_preview_video_w = width,
	      .max_preview_video_h = height,
	      .num_preview_video_frames = 3,
	      .stills_capture_circular_buffer_height = 0,
	      .fast_preview_resume = 0,
	      .use_stc_timestamp = MMAL_PARAM_TIMESTAMP_MODE_RESET_STC
	   };
	   mmal_port_parameter_set(cameraComponent->control, &cam_config.hdr);
	}

    // Setup format for the video port.
    MMAL_ES_FORMAT_T* format = videoPort->format;
    format->encoding = MMAL_ENCODING_BGR24;
    format->encoding_variant = MMAL_ENCODING_BGR24;
	//format->encoding = MMAL_ENCODING_I420;
	//format->encoding_variant = MMAL_ENCODING_I420;
	format->es->video.width = width;
	format->es->video.height = height;
	format->es->video.crop.x = 0;
	format->es->video.crop.y = 0;
	format->es->video.crop.width = width;
	format->es->video.crop.height = height;
	format->es->video.frame_rate.num = framerate;
	format->es->video.frame_rate.den = VIDEO_FRAME_RATE_DEN;

    status = mmal_port_format_commit(videoPort);
    if(status) throw std::runtime_error("Couldn't set video format.");

    status = mmal_port_enable(videoPort, video_buffer_callback);
    if(status) throw std::runtime_error("Couldn't enable video buffer callback");

    if (videoPort->buffer_num < VIDEO_OUTPUT_BUFFERS_NUM)
        videoPort->buffer_num = VIDEO_OUTPUT_BUFFERS_NUM;

    // Setup format for the still port.
    format = stillPort->format;
    format->encoding = MMAL_ENCODING_OPAQUE;
    format->encoding_variant = MMAL_ENCODING_I420;
    format->es->video.width = width;
    format->es->video.height = height;
    format->es->video.crop.x = 0;
    format->es->video.crop.y = 0;
    format->es->video.crop.width = width;
    format->es->video.crop.height = height;
    format->es->video.frame_rate.num = 1;
    format->es->video.frame_rate.den = 1;

    status = mmal_port_format_commit(stillPort);
    if(status) throw std::runtime_error("Couldn't set still format.");

	// Ensure there are enough buffers to avoid dropping frames.
	if (stillPort->buffer_num < VIDEO_OUTPUT_BUFFERS_NUM)
	    stillPort->buffer_num = VIDEO_OUTPUT_BUFFERS_NUM;

    videoPort->buffer_size = videoPort->buffer_size_recommended;
    videoPort->buffer_num  = videoPort->buffer_num_recommended;

    videoPool = mmal_port_pool_create(videoPort, videoPort->buffer_num, videoPort->buffer_size);
    if(!videoPool) throw std::runtime_error("Couldn't create buffer header pool for video output port");

    status = mmal_component_enable(cameraComponent);

    //
    frame = cv::Mat(height, width, CV_8UC3);

    videoPort->userdata = (struct MMAL_PORT_USERDATA_T*)this;

    if (mmal_port_parameter_set_boolean(videoPort, MMAL_PARAMETER_CAPTURE, 1) != MMAL_SUCCESS)
    {
        throw std::runtime_error("Couldn't start video capture");
    }

    int num = mmal_queue_length(videoPool->queue);
    int q;
    for (q=0;q<num;q++)
    {
        MMAL_BUFFER_HEADER_T *buffer = mmal_queue_get(videoPool->queue);
    
        // TODO: Add numbers to these error messages.
        if (!buffer)
            throw std::runtime_error("Unable to get a required buffer from pool queue");
    
        if (mmal_port_send_buffer(videoPort, buffer)!= MMAL_SUCCESS)
            throw std::runtime_error("Unable to send a buffer to an encoder output port");
    }

}

PiCam::~PiCam() {
    if(cameraComponent) mmal_component_destroy(cameraComponent);

    // TODO: More? Cleanup goes here
}
