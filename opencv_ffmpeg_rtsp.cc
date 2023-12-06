#include <iostream>
#include <opencv2/highgui/highgui.hpp>

extern "C" {
#include <libavutil/opt.h>
#include <libavdevice/avdevice.h>
#include <libswscale/swscale.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
}

#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avdevice.lib")
#pragma comment(lib, "avfilter.lib")
#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "swscale.lib")

int main(int argc, char * argv[])
{
  avdevice_register_all();
  avformat_network_init();

  const char  *filenameSrc = "rtsp://admin:Massive6378@192.168.1.199:554/live"; //Axis

  AVCodecContext  *pCodecCtx;
  AVFormatContext *pFormatCtx;
  AVCodec *pCodec;
  AVFrame *pFrame;
  AVFrame *pFrameRGB;

  pFormatCtx = avformat_alloc_context();
  if ( pFormatCtx == NULL )
    return -8;

  if ( avformat_open_input( &pFormatCtx, filenameSrc, NULL, NULL ) != 0 ) {
    std::cout << "Open File Error 12" << std::endl;
    return -12;
  }

  if ( avformat_find_stream_info( pFormatCtx,NULL ) < 0 ) {
    std::cout << "Get Stream Information Error 13" << std::endl;
    avformat_close_input( &pFormatCtx );
    pFormatCtx = NULL;
    return -13;
  }
  av_dump_format( pFormatCtx, 0, filenameSrc, 0 );
  int video_stream_index = -1;

  for ( int i = 0; i < pFormatCtx->nb_streams; i++ ) {
    if ( pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO ) {
      video_stream_index = i;
      break;
    }
  }
  if ( video_stream_index < 0 ) {
    std::cout << "Video stream was not found Error 14" << std::endl;
    avformat_close_input( &pFormatCtx );
    pFormatCtx = NULL;
    return -14;
  }

  //pCodecCtx = pFormatCtx->streams[video_stream_index]->codec;
  pCodec  = avcodec_find_decoder( pFormatCtx->streams[video_stream_index]->codecpar->codec_id);
  if ( pCodec == NULL ) {
    std::cout << "codec not found Error 15" << std::endl;
    return -15;
  }

  pCodecCtx = avcodec_alloc_context3(pCodec);

  if(pCodecCtx == NULL){
    std::cerr<<"Failed to alloc context"<<std::endl;
  }

//  if (pFormatCtx->oformat->flags & AVFMT_GLOBALHEADER)
//    pCodecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;


  int res = av_opt_set(pCodecCtx->priv_data, "tune", "zerolatency", 0);
  if(res <0) {
    std::cout << "tune failed: ";
    switch (res) {
      case AVERROR_OPTION_NOT_FOUND:
        std::cout<<"Option not found"<<std::endl;
        break;
      case AVERROR(ERANGE):
        std::cout<<"Out of range"<<std::endl;
        break;
      case AVERROR(EINVAL):
        std::cout<<"Inval"<<std::endl;
        break;
      default:
        std::cout<<"ret: "<<res<<std::endl;
    }
  }
  res = av_opt_set(pCodecCtx->priv_data, "fflags", "nobuffer", 0);
  if(res<0) {
    std::cout << "fflags failed: ";
    switch (res) {
      case AVERROR_OPTION_NOT_FOUND:
        std::cout<<"Option not found"<<std::endl;
        break;
      case AVERROR(ERANGE):
        std::cout<<"Out of range"<<std::endl;
        break;
      case AVERROR(EINVAL):
        std::cout<<"Inval"<<std::endl;
        break;
      default:
        std::cout<<"ret: "<<res<<std::endl;
    }
  }
  res = av_opt_set(pCodecCtx->priv_data, "flags", "low_delay", 0);
  if(res<0) {
    std::cout << "flags failed: ";
    switch (res) {
      case AVERROR_OPTION_NOT_FOUND:
        std::cout<<"Option not found"<<std::endl;
        break;
      case AVERROR(ERANGE):
        std::cout<<"Out of range"<<std::endl;
        break;
      case AVERROR(EINVAL):
        std::cout<<"Inval"<<std::endl;
        break;
      default:
        std::cout<<"ret: "<<res<<std::endl;
    }
  }
  res = av_opt_set(pCodecCtx->priv_data, "probesize", "32", 0);
  if(res<0) {
    std::cout << "probesize failed: ";
    switch (res) {
      case AVERROR_OPTION_NOT_FOUND:
        std::cout<<"Option not found"<<std::endl;
        break;
      case AVERROR(ERANGE):
        std::cout<<"Out of range"<<std::endl;
        break;
      case AVERROR(EINVAL):
        std::cout<<"Inval"<<std::endl;
        break;
      default:
        std::cout<<"ret: "<<res<<std::endl;
    }
  }
  res = av_opt_set(pCodecCtx->priv_data, "analyzeduration", "1", 0);
  if(res<0) {
    std::cout << "analyzeduration failed: ";
    switch (res) {
      case AVERROR_OPTION_NOT_FOUND:
        std::cout<<"Option not found"<<std::endl;
        break;
      case AVERROR(ERANGE):
        std::cout<<"Out of range"<<std::endl;
        break;
      case AVERROR(EINVAL):
        std::cout<<"Inval"<<std::endl;
        break;
      default:
        std::cout<<"ret: "<<res<<std::endl;
    }
  }
  res = av_opt_set(pCodecCtx->priv_data, "strict", "experimental", 0);
  if(res<0) {
    std::cout << "strict failed: ";
    switch (res) {
      case AVERROR_OPTION_NOT_FOUND:
        std::cout<<"Option not found"<<std::endl;
        break;
      case AVERROR(ERANGE):
        std::cout<<"Out of range"<<std::endl;
        break;
      case AVERROR(EINVAL):
        std::cout<<"Inval"<<std::endl;
        break;
      default:
        std::cout<<"ret: "<<res<<std::endl;
    }
  }
  res = av_opt_set(pCodecCtx->priv_data, "framedrop", "1", 0);
  if(res<0) {
    std::cout << "framedrop failed: " ;
    switch (res) {
      case AVERROR_OPTION_NOT_FOUND:
        std::cout<<"Option not found"<<std::endl;
        break;
      case AVERROR(ERANGE):
        std::cout<<"Out of range"<<std::endl;
        break;
      case AVERROR(EINVAL):
        std::cout<<"Inval"<<std::endl;
        break;
      default:
        std::cout<<"ret: "<<res<<std::endl;
    }
  }
  res = av_opt_set(pCodecCtx->priv_data, "vf", "setpts=0", 0);
  if(res<0) {
    std::cout << "vf failed: ";
    switch (res) {
      case AVERROR_OPTION_NOT_FOUND:
        std::cout<<"Option not found"<<std::endl;
        break;
      case AVERROR(ERANGE):
        std::cout<<"Out of range"<<std::endl;
        break;
      case AVERROR(EINVAL):
        std::cout<<"Inval"<<std::endl;
        break;
      default:
        std::cout<<"ret: "<<res<<std::endl;
    }
  }

  if (avcodec_open2( pCodecCtx, pCodec, NULL ) < 0) {
    std::cout << "Open Codec Error 16" << std::endl;
    return -16;
  }

  avcodec_parameters_to_context(pCodecCtx,pFormatCtx->streams[video_stream_index]->codecpar);


  pFrame    = av_frame_alloc();
  pFrameRGB = av_frame_alloc();

  AVPixelFormat pFormat = AV_PIX_FMT_RGB24 ;
  int numBytes  = av_image_get_buffer_size( pFormat, pCodecCtx->width, pCodecCtx->height,1 );
  uint8_t *buffer = (uint8_t *)av_malloc( numBytes * sizeof(uint8_t) );
  av_image_fill_arrays(pFrameRGB->data,pFrameRGB->linesize,buffer, pFormat, pCodecCtx->width, pCodecCtx->height,1);

  int y_size  = pCodecCtx->width * pCodecCtx->height;
  AVPacket *packet = (AVPacket *)malloc(sizeof(AVPacket));
  av_new_packet( packet, y_size );

  int frameFinished;
  while (av_read_frame( pFormatCtx, packet ) >= 0 )
  {
    if ( packet->stream_index == video_stream_index ) {
      //@TODO replace deprecated call here
      avcodec_decode_video2( pCodecCtx, pFrame, &frameFinished, packet );
      //avcodec_send_packet(pCodecCtx,packet);
      //savcodec_receive_frame(pCodecCtx,pFrame);

      if (frameFinished) {
        struct SwsContext *img_convert_ctx;
        img_convert_ctx = sws_getCachedContext( NULL,
                                                pCodecCtx->width,
                                                pCodecCtx->height,
                                                pCodecCtx->pix_fmt,
                                                pCodecCtx->width,
                                                pCodecCtx->height,
                                                AV_PIX_FMT_BGR24,
                                                SWS_BICUBIC,
                                                NULL,
                                                NULL,
                                                NULL );
        sws_scale( img_convert_ctx,
                   ((AVPicture*)pFrame)->data,
                   ((AVPicture*)pFrame)->linesize,
                   0,
                   pCodecCtx->height,
                   ((AVPicture *)pFrameRGB)->data,
                   ((AVPicture *)pFrameRGB)->linesize );

        cv::Mat img( pFrame->height, pFrame->width, CV_8UC3, pFrameRGB->data[0] );
        cv::imshow( "Display" , img );
        cv::waitKey( 30 );

        av_packet_unref( packet );
        sws_freeContext( img_convert_ctx );
      }
    }
  }

  av_packet_unref( packet );
  avcodec_close( pCodecCtx );
  av_free( pFrame );
  av_free( pFrameRGB );
  avformat_close_input( &pFormatCtx );

  return ( EXIT_SUCCESS );
}