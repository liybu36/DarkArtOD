#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <math.h>
#include "TTree.h"
#include "TChain.h"
#include "TFile.h"
#include "TString.h"
#include "TSystem.h"
#include "TApplication.h"
#include "TRint.h"
#include "TProof.h"
#include "TDSet.h"
#include "TROOT.h"
#include "SladDSTClass.h"

using namespace std;
TRint* theApp;

void ReadRunList(vector<int>& tpc_run, vector<int>& od_run)
{
#define AAr 
#ifdef AAr
    string runlist = "50days_runlist.txt";
#else
    string runlist = "uar_od_runlist.txt";
#endif
  ifstream indata (runlist.c_str());
  if(indata.is_open())
    {
      while(!indata.eof())
	{
#ifdef AAr
	  string s1,s2;
	  indata>>s1>>s2;
	  tpc_run.push_back(atoi(s1.c_str()));
	  od_run.push_back(atoi(s2.c_str()));
#else
	  string s1;
	  indata>>s1;
	  tpc_run.push_back(atoi(s1.c_str()));
	  od_run.push_back(atoi(s1.c_str()));
#endif
	}
    }
  indata.close();
}

int SladDSTMain(vector<int>& tpc_run, vector<int>& od_run)
{
  //The tpc_path, od_path and outdir path may need changes
  TString tpc_path =  "/scratch/darkside/slad/20150308/";
  TString od_path =  "/ds50/data/test_processing/lsv/darkartod/v1/sladod/";
  TString out_path = "/ds50/data/test_processing/lsv/darkartod/v1/sladdst/";

  if(tpc_run.size() != od_run.size())
    return -1;
  cout<<"Run List size: "<<tpc_run.size()<<endl;

  //  for(size_t j=0; j<tpc_run.size(); ++j)
  for(size_t j=0; j<1; ++j)
    {      
      SladDSTClass *r = new SladDSTClass;
      r->SetTPCPath(tpc_path);
      r->SetODPath(od_path);
      r->SetOutPath(out_path);  

      r->create_tpcchain();
      r->create_odchain();
      if(!(r->add_tpctree(tpc_run.at(j))) || !(r->add_odtree(od_run.at(j))))
	{
	  delete r;	  
	  continue;
	}
      r->add_tpcfriend();
          
      TString outfile;
      outfile.Form("%sSLADDST_Run%06d.root",out_path.Data(),tpc_run.at(j));
      r->SetOutFile(outfile);
 
      r->LoopOverChain();      
      
      delete r;
    }

  std::cout << "==> Saved Plots." << std::endl;  
  return 1;
}

#ifndef __CINT__
int main(int argc, char** argv){
  theApp = new TRint("App",&argc,argv,NULL,0);
  //  gROOT->ProcessLine("#pragma link C++ struct Event+;");
  vector<int> tpc_run;
  vector<int> od_run;
  if(theApp->Argc() == 0)
    {
      cout<<"Usage: ./SladDSTMain"<<endl;
      cout<<"Usage: ./SladDSTMain runnumber ..."<<endl;   
      return 0;
    }  
  else if(theApp->Argc() == 1)
    {
      ReadRunList(tpc_run,od_run);
    }
  else {
    for(int i=1; i<theApp->Argc(); ++i)
      {
	tpc_run.push_back(atoi(theApp->Argv(i)));
	od_run.push_back(atoi(theApp->Argv(i)));      
      }
  }
  SladDSTMain(tpc_run,od_run);
  std::cout << "==> Application finished." << std::endl;
  return 1; 
}
#endif /*__CINT__*/
