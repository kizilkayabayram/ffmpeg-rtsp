//
// Created by bay on 14.12.2023.
//

#ifndef STREAM_TOOLS__RTSP_RECEIVER_H_
#define STREAM_TOOLS__RTSP_RECEIVER_H_

#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <map>
#include <atomic>
#include <chrono>
#include <utility>

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



struct MeasureExecTime{
  explicit MeasureExecTime(std::string name) : name_(std::move(name)){
    tp_start_ = std::chrono::steady_clock::now();
  }
  ~MeasureExecTime() {
    auto tp_now = std::chrono::steady_clock::now();
    auto tp_diff = std::chrono::duration_cast<std::chrono::milliseconds>(tp_now - tp_start_);
    std::cout << "[" << name_ << "]" << " Took: " << tp_diff.count() << " ms"<<std::endl;
  }
 private:
  std::string name_;
  std::chrono::time_point<std::chrono::steady_clock> tp_start_;
};

class RtspReceiver {
 public:
  explicit RtspReceiver(std::string stream_url);
  ~RtspReceiver();
  bool Setup();
  void DecodeLoop();
  void Stop(){
    stop_.store(true);
  }
 private:
  std::string stream_url_;

  AVCodecContext *codec_context_;
  AVFormatContext *format_context_;
  int found_stream_id_{-1};
  const static inline std::map<std::string, std::string> format_opts_map_ = {
      {"flags", "low_delay"},
      {"strict", "experimental"},
  };
  std::atomic<bool> stop_{false};

  bool SetFormatContextParameters();
  bool FindStreamInfo();
  bool FindStream();
  bool SetupCodec();
  bool ConvertFrameFromYuvToBgr(AVFrame *frame,AVFrame *frame_rgb);
  static void ShowImageOpenCv(AVFrame *frame, AVFrame *frame_rgb);
  bool DecodeFrame(const AVPacket* packet, AVFrame* frame);
  static std::string MaketErrorString(int err);

};

#endif //STREAM_TOOLS__RTSP_RECEIVER_H_
