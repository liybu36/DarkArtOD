#include "darkart/TpcReco/RootGraphics.hh"
#include "TApplication.h"
#include "TSystem.h"
#include <iostream>
#include <cmath>

//#include "TEnv.h"

darkart::RootGraphics::RootGraphics() : thread(RootGraphics::ThreadEntryPoint,this),
                               pause(true), quit(false), mutex()
{
  if(!gApplication){
    TApplication* app = new TApplication("app",0,0);
    app->SetReturnFromRun(kTRUE);
    
  }
}

darkart::RootGraphics::RootGraphics(bool TApp) : 
    thread(RootGraphics::ThreadEntryPoint,this),
    pause(true), quit(false), mutex()
{
    if(TApp && !gApplication)
    {
	TApplication* app = new TApplication("app",0,0);
	app->SetReturnFromRun(kTRUE);
    }
}

void  darkart::RootGraphics::Start()
{
  pause = false;
}

void  darkart::RootGraphics::Pause()
{
  pause = true;
}

void  darkart::RootGraphics::Stop()
{
  quit = true;
  if(thread.joinable()){
    thread.join();
  }
    
}

void*  darkart::RootGraphics::ThreadEntryPoint(void* graphics)
{
  RootGraphics* gr = (RootGraphics*)graphics;
  while(!gr->quit){
    gSystem->Sleep(100);
    if(gr->pause) continue;
    gr->mutex.lock();
    if(gSystem->ProcessEvents())
      break;
    gr->mutex.unlock();
  }
  return 0;
}

void darkart::divide_canvas(TCanvas& can, int npads){
  int width = floor(sqrt(npads*1.77));
  int height = ceil(npads/floor(sqrt(npads*1.77)));
  can.Divide(width,height);

}
