//
// Created by bay on 14.12.2023.
//

#include "rtsp_receiver.h"
RtspReceiver::RtspReceiver(const std::string& stream_url): stream_url_(stream_url){
//  avdevice_register_all();
//  avformat_network_init();
}

RtspReceiver::~RtspReceiver() {
  stop_.store(true);
  avcodec_close(codec_context_);
  avformat_close_input(&format_context_);
}

bool RtspReceiver::Setup() {
  format_context_ = avformat_alloc_context();
  if (format_context_ == nullptr)
    return false;

  if(!SetFormatContextParameters())
    return false;

  if(!FindStreamInfo())
    return false;

  if(!FindStream())
    return false;

  if(!SetupCodec())
    return false;

  return true;

}

bool RtspReceiver::SetFormatContextParameters() {

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

  if (!is_opt_set_ok(av_opt_set(format_context_, "fflags", "nobuffer", 0))) {
    std::cerr << "Failed to set fflags: " << error_str << std::endl;
    return false;
  }

  if (!is_opt_set_ok(av_opt_set_int(format_context_, "analyzeduration", 1, 0))) {
    std::cerr << "Failed to set analyzeduration: " << error_str << std::endl;
    return false;
  }

  if (!is_opt_set_ok(av_opt_set_int(format_context_, "probesize", 64, 0))) {
    std::cerr << "Failed to set probesize: " << error_str << std::endl;
    return false;
  }

  return true;
}

bool RtspReceiver::FindStreamInfo() {

  if (int ret = avformat_open_input(&format_context_, stream_url_.c_str(), nullptr, nullptr); ret != 0) {
    std::cerr << "Error occurred at avformat_open_input: " << MakeErrorString(ret) << std::endl;
    return false;
  }

  AVDictionary *dict_format_opts = nullptr;
  for (const auto &[k, v] : format_opts_map_) {
    av_dict_set(&dict_format_opts, k.c_str(), v.c_str(), 0);
  }

  if (avformat_find_stream_info(format_context_, &dict_format_opts) < 0) {
    std::cout << "Get Stream Information Error 13" << std::endl;
    avformat_close_input(&format_context_);
    format_context_ = nullptr;
    return false;
  }

  for (const auto &[k, v] : format_opts_map_) {
    AVDictionaryEntry *ent = av_dict_get(dict_format_opts, k.c_str(), nullptr, 0);
    if (ent) {
      std::cerr<<"Param Set: " << k.c_str() << " -> Failed" << std::endl;
      return false;
    }
  }

  av_dict_free(&dict_format_opts);

  return true;
}


bool RtspReceiver::FindStream() {
  for (int i = 0; i < format_context_->nb_streams; i++) {
    if (format_context_->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
      found_stream_id_ = i;
      break;
    }
  }
  if (found_stream_id_ < 0) {
    std::cout << "Video stream was not found" << std::endl;
    return false;
  }
  return true;
}

bool RtspReceiver::SetupCodec() {
  auto codec = avcodec_find_decoder(format_context_->streams[found_stream_id_]->codecpar->codec_id);
  if (codec == nullptr) {
    std::cerr << "codec not found" << std::endl;
    return false;
  }

  codec_context_ = avcodec_alloc_context3(codec);

  if (codec_context_ == nullptr) {
    std::cerr << "Failed to alloc context" << std::endl;
    return false;
  }

  codec_context_->time_base.num = 1;
  codec_context_->time_base.den = 20;
  codec_context_->framerate.num = 20;
  codec_context_->framerate.den = 1;
  codec_context_->gop_size = 20 * 2;
  codec_context_->max_b_frames = 3;
  codec_context_->refs = 3;

  if (int ret = avcodec_open2(codec_context_, codec, nullptr); ret < 0) {
    std::cerr << "Open Codec Error: " << MakeErrorString(ret) << std::endl;
    return false;
  }

  avcodec_parameters_to_context(codec_context_, format_context_->streams[found_stream_id_]->codecpar);

  return true;
}

std::string RtspReceiver::MakeErrorString(int err) {
  //char err_str[AV_ERROR_MAX_STRING_SIZE];
  std::string  err_str;
  av_make_error_string(err_str.data(),AV_ERROR_MAX_STRING_SIZE,err);
  return err_str;
}
void RtspReceiver::DecodeLoop() {
  int ret = 0;
  auto frame = av_frame_alloc();
  auto frame_rgb = av_frame_alloc();

  AVPixelFormat pixel_format = AV_PIX_FMT_RGB24;
  int num_bytes = av_image_get_buffer_size(pixel_format, codec_context_->width, codec_context_->height, 1);
  auto *buffer = (uint8_t *) av_malloc(num_bytes * sizeof(uint8_t));
  av_image_fill_arrays(frame_rgb->data,
                       frame_rgb->linesize,
                       buffer,
                       pixel_format,
                       codec_context_->width,
                       codec_context_->height,
                       1);

  int y_size = codec_context_->width * codec_context_->height;
  auto packet = (AVPacket *) malloc(sizeof(AVPacket));
  av_new_packet(packet, y_size);

  while (!stop_) {

    {
      //MeasureExecTime me("read_frame");
      ret = av_read_frame(format_context_, packet);
    }
    if(ret<0){
      std::cerr << "Can't read frame" << MakeErrorString(ret) << std::endl;
      continue;
    }

    {
      //MeasureExecTime me("decode_frame");
      if (!DecodeFrame(packet, frame))
        continue;

    }
    {
      //MeasureExecTime me("convert_frame");
      if (!ConvertFrameFromYuvToBgr(frame, frame_rgb)) {
        continue;
      }
    }

    ShowImageOpenCv(frame,frame_rgb);
    av_packet_unref(packet);
  }

  av_packet_unref(packet);
  av_free(frame);
  av_free(frame_rgb);
}

bool RtspReceiver::DecodeFrame(const AVPacket *packet, AVFrame *frame) {
  int ret = 0;
  if (packet->stream_index != found_stream_id_) {
    return false;
  }

  ret = avcodec_send_packet(codec_context_, packet);
  if (ret < 0) {
    return false;
  }

  ret = avcodec_receive_frame(codec_context_, frame);
  if (ret < 0) {
    if (ret == AVERROR(EAGAIN)) return false;
    std::cerr << "Receive frame failed: " << MakeErrorString(ret) << std::endl;
    stop_ = true;
    return false;
  }

  return true;
}


bool RtspReceiver::ConvertFrameFromYuvToBgr(AVFrame *frame,AVFrame *frame_rgb) {
  struct SwsContext *img_convert_ctx;
  img_convert_ctx = sws_getCachedContext(nullptr,
                                         codec_context_->width,
                                         codec_context_->height,
                                         codec_context_->pix_fmt,
                                         codec_context_->width,
                                         codec_context_->height,
                                         AV_PIX_FMT_BGR24,
                                         SWS_BICUBIC,
                                         nullptr,
                                         nullptr,
                                         nullptr);
  int ret =
  sws_scale(img_convert_ctx,
            frame->data,
            frame->linesize,
            0,
            codec_context_->height,
            frame_rgb->data,
            frame_rgb->linesize
  );
  sws_freeContext(img_convert_ctx);
  return ret >= 0;
}
void RtspReceiver::ShowImageOpenCv(AVFrame *frame,AVFrame *frame_rgb) {
  cv::Mat img(frame->height, frame->width, CV_8UC3, frame_rgb->data[0]);
  cv::imshow("Display", img);
  cv::waitKey(30);
}

