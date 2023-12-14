#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <map>

extern "C" {
#include <libavutil/opt.h>
#include <libavdevice/avdevice.h>
#include <libswscale/swscale.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavfilter/avfilter.h>
}

#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avdevice.lib")
#pragma comment(lib, "avfilter.lib")
#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "swscale.lib")


static const int fps = 20;

const static std::map<std::string, std::string> format_opts_map = {
    {"flags", "low_delay"},
    {"strict", "experimental"},
};

bool SetFormatContextParameters(AVFormatContext *fctx) {

  std::string error_str;
  auto is_opt_set_ok = [&error_str](int res) {
    if (res >= 0) {
      return true;
    }
    switch (res) {
      case AVERROR_OPTION_NOT_FOUND: error_str = "Option not found";
        break;
      case AVERROR(ERANGE): error_str = "Out of range";
        break;
      case AVERROR(EINVAL): error_str = "Invalid Value";
        break;
      default: error_str = "Unknown error: " + std::to_string(res);
    }
    return false;
  };

  if (!is_opt_set_ok(av_opt_set(fctx, "fflags", "nobuffer", 0))) {
    std::cerr << "Failed to set fflags: " << error_str << std::endl;
    return false;
  }

  if (!is_opt_set_ok(av_opt_set_int(fctx, "analyzeduration", 1, 0))) {
    std::cerr << "Failed to set analyzeduration: " << error_str << std::endl;
    return false;
  }

  if (!is_opt_set_ok(av_opt_set_int(fctx, "probesize", 64, 0))) {
    std::cerr << "Failed to set probesize: " << error_str << std::endl;
    return false;
  }

  return true;
}

int main(int argc, char *argv[]) {
  avdevice_register_all();
  avformat_network_init();

  int ret = 0;

  const char *filenameSrc = "rtsp://admin:Massive6378@192.168.1.199:554/live"; //Axis

  AVCodecContext *codec_context;
  AVFormatContext *format_context;
  AVFrame *frame;
  AVFrame *frame_rgb;

  format_context = avformat_alloc_context();
  if (format_context == NULL)
    return -8;

  if (!SetFormatContextParameters(format_context)) {
    std::cerr << "Can't set args for format_context" << std::endl;
    return -1;
  }

  if (avformat_open_input(&format_context, filenameSrc, NULL, NULL) != 0) {
    std::cout << "Open File Error 12" << std::endl;
    return -12;
  }

  AVDictionary *dict_format_opts = NULL;
  for (const auto &[k, v] : format_opts_map) {
    av_dict_set(&dict_format_opts, k.c_str(), v.c_str(), 0);
  }

  if (avformat_find_stream_info(format_context, &dict_format_opts) < 0) {
    std::cout << "Get Stream Information Error 13" << std::endl;
    avformat_close_input(&format_context);
    format_context = NULL;
    return -13;
  }

  for (const auto &[k, v] : format_opts_map) {
    AVDictionaryEntry *ent = av_dict_get(dict_format_opts, k.c_str(), NULL, 0);
    if (ent) {
      std::cout << k.c_str() << " -> Failed" << std::endl;
    } else {
      std::cout << k.c_str() << " -> OK" << std::endl;
    }
  }

  av_dump_format(format_context, 0, filenameSrc, 0);
  int video_stream_index = -1;

  for (int i = 0; i < format_context->nb_streams; i++) {
    if (format_context->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
      video_stream_index = i;
      break;
    }
  }
  if (video_stream_index < 0) {
    std::cout << "Video stream was not found Error 14" << std::endl;
    avformat_close_input(&format_context);
    format_context = NULL;
    return -14;
  }

  //codec_context = format_context->streams[video_stream_index]->codec;

  auto codec = avcodec_find_decoder(format_context->streams[video_stream_index]->codecpar->codec_id);
  if (codec == NULL) {
    std::cout << "codec not found Error 15" << std::endl;
    return -15;
  }


  codec_context = avcodec_alloc_context3(codec);

  if (codec_context == NULL) {
    std::cerr << "Failed to alloc context" << std::endl;
    return -1;
  }

  codec_context->time_base.num = 1;
  codec_context->time_base.den = fps;
  codec_context->framerate.num = fps;
  codec_context->framerate.den = 1;


  codec_context->gop_size = fps * 2;
  codec_context->max_b_frames = 3;
  codec_context->refs = 3;


//  auto hwtype = AV_HWDEVICE_TYPE_CUDA;// = av_hwdevice_find_type_by_name("cuda");
//
//  AVBufferRef *hwdevice_ctx;
//  int ret = av_hwdevice_ctx_create(&hwdevice_ctx, hwtype, NULL, NULL, 0);
//  if(ret<0){
//    char err_str[AV_ERROR_MAX_STRING_SIZE];
//    av_make_error_string(err_str,AV_ERROR_MAX_STRING_SIZE,ret);
//    std::cerr<<"Failed to create device: "<<err_str<<std::endl;
//    return -1;
//  }else{
//    std::cout<<"Device created"<<std::endl;
//  }
//
//  if(codec_context->hw_device_ctx == NULL){
//    codec_context->hw_device_ctx = hwdevice_ctx;
//  }else{
//    std::cout<<"Device context already set"<<std::endl;
//    return -1;
//  }


//  if (format_context->oformat->flags & AVFMT_GLOBALHEADER)
//    codec_context->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

//  codec_context->thread_count = 12;
//  codec_context->thread_type = FF_THREAD_SLICE;

  if (avcodec_open2(codec_context, codec, NULL) < 0) {
    std::cout << "Open Codec Error 16" << std::endl;
    return -16;
  }

  avcodec_parameters_to_context(codec_context, format_context->streams[video_stream_index]->codecpar);

  frame = av_frame_alloc();
  frame_rgb = av_frame_alloc();

  AVPixelFormat pFormat = AV_PIX_FMT_RGB24;
  int numBytes = av_image_get_buffer_size(pFormat, codec_context->width, codec_context->height, 1);
  auto *buffer = (uint8_t *) av_malloc(numBytes * sizeof(uint8_t));
  av_image_fill_arrays(frame_rgb->data,
                       frame_rgb->linesize,
                       buffer,
                       pFormat,
                       codec_context->width,
                       codec_context->height,
                       1);

  int y_size = codec_context->width * codec_context->height;
  auto *packet = (AVPacket *) malloc(sizeof(AVPacket));
  av_new_packet(packet, y_size);

  while (av_read_frame(format_context, packet) >= 0) {
    if (packet->stream_index != video_stream_index) {
      continue;
    }

    ret = avcodec_send_packet(codec_context, packet);
    if (ret < 0) {
      continue;
    }

    ret = avcodec_receive_frame(codec_context, frame);
    if (ret < 0) {
      // Sometimes we cannot get a new frame, continue in this case
      if (ret == AVERROR(EAGAIN)) continue;

      fprintf(stderr, "avcodec_receive_frame ret : %d\n", ret);
      break;
    }

    struct SwsContext *img_convert_ctx;
    img_convert_ctx = sws_getCachedContext(NULL,
                                           codec_context->width,
                                           codec_context->height,
                                           codec_context->pix_fmt,
                                           codec_context->width,
                                           codec_context->height,
                                           AV_PIX_FMT_BGR24,
                                           SWS_BICUBIC,
                                           NULL,
                                           NULL,
                                           NULL);
    sws_scale(img_convert_ctx,
              frame->data,
              frame->linesize,
              0,
              codec_context->height,
              frame_rgb->data,
              frame_rgb->linesize
    );

    cv::Mat img(frame->height, frame->width, CV_8UC3, frame_rgb->data[0]);
    cv::imshow("Display", img);
    cv::waitKey(30);

    av_packet_unref(packet);
    sws_freeContext(img_convert_ctx);
  }

  av_packet_unref(packet);
  avcodec_close(codec_context);
  av_free(frame);
  av_free(frame_rgb);
  avformat_close_input(&format_context);

  return (EXIT_SUCCESS);
}