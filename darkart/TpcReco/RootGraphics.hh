#ifndef ROOTGRAPHICS_h
#define ROOTGRAPHICS_h

//This class allows interaction with root graphical objects in a compiled binary
//Credit: Ben Loer

#include "TThread.h"
#include "TMutex.h"
#include <memory>
#include <thread>
#include <mutex>
#include "TCanvas.h"

namespace darkart{
  class RootGraphics;
  typedef std::lock_guard<std::mutex> LockGuard;
  void divide_canvas(TCanvas& can, int npads);
}


class darkart::RootGraphics{
 private:
  std::thread thread;
  bool pause;
  bool quit;
  
  //private constructor ensures singleton
 
 
 public:

  RootGraphics();
  RootGraphics(bool TApp);
  ~RootGraphics(){
    Stop();
    //mutex.CleanUp();
  }
    
  void Start();  
  void Pause();  
  void Stop();  
  
  
  
  //This should only be used by the thread
  static void* ThreadEntryPoint(void* ptr=NULL);  

  std::mutex mutex; 
};

#endif
