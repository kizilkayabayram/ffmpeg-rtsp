//
// Created by bay on 14.12.2023.
//


#include <csignal>
#include <thread>
#include "rtsp_receiver.h"

namespace
{
volatile std::sig_atomic_t gSignalStatus;
}

void signal_handler(int signal)
{
  gSignalStatus = signal;
}

int main(){
  std::signal(SIGINT, signal_handler);

  RtspReceiver rr("rtsp://admin:Massive6378@192.168.1.199:554/live");

  std::thread sig_thread([&rr](){
    while (gSignalStatus == 0){
      std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    std::cout<<"Stopping receiver"<<std::endl;
    rr.Stop();
  });

  if(rr.Setup()){
    rr.DecodeLoop();
  }
  sig_thread.join();
}