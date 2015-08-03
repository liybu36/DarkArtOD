////////////////////////////////////////////////////////////////////////
// Class:       VetoTDCCluster
// Module Type: producer
// File:        VetoTDCCluster_module.cc
//
// Ported at Tue Oct 28 2014 by Masayuki Wada from OnlineTDCCluster_module.cc
// written by Alessandro Razeto.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "artdaq-core/Data/Fragments.hh"
#include "darksidecore/Data/V1495Fragment.hh"
#include "darksidecore/Data/V1190Fragment.hh"
#include "darksidecore/Data/Config.hh"

#include "darkart/Products/TDCCluster.hh"

#include "TH1.h"
#include <iostream>

namespace darkart {
  class VetoTDCCluster : public art::EDProducer {
    public:
      explicit VetoTDCCluster(fhicl::ParameterSet const & p);
      virtual ~VetoTDCCluster();

      void produce(art::Event & e) override;
      void beginRun(art::Run &) override;

    private:
      std::unique_ptr<TH1I> h_;
      int begin_threshold_, end_threshold_; // clustering threshold, number of hits in 50 ns
  };
}

darkart::VetoTDCCluster::VetoTDCCluster(fhicl::ParameterSet const & ps): begin_threshold_(ps.get<int> ("begin_threshold")), end_threshold_(ps.get<int> ("end_threshold")) {
   produces<darkart::TDCClusters> ();
}

darkart::VetoTDCCluster::~VetoTDCCluster() {}

void darkart::VetoTDCCluster::produce(art::Event & e) {
  std::unique_ptr<darkart::TDCClusters> clusters(new darkart::TDCClusters);

  art::Handle<artdaq::Fragments> v1495;
  e.getByLabel ("daq", "V1495", v1495);
  if (!v1495.isValid ()) {
//    LOG_INFO("Invalid v1495") << "\nInvalid v1495\n"<<std::endl;
    e.put (std::move (clusters));
    return;
  }

  if(v1495->size() != 1) {
    mf::LogError ("VetoTDCCluster") << v1495->size() << " fragment for V1495";
    e.put (std::move (clusters));
    return;
  }

  if (ds50::V1495Fragment((*v1495)[0]).trigger_type () != ds50::V1495Fragment::tpc && ds50::V1495Fragment((*v1495)[0]).trigger_type () != ds50::V1495Fragment::tpc_high && ds50::V1495Fragment((*v1495)[0]).trigger_type () != ds50::V1495Fragment::tpc_low) {
//     LOG_INFO("TriggerType: ") << "\nTrigger type: "<<ds50::V1495Fragment((*v1495)[0]).trigger_type ()<<"\n"<<std::endl;
    e.put (std::move (clusters));
    return;
  }

  art::Handle<artdaq::Fragments> v1190s;
  e.getByLabel ("daq", "V1190", v1190s);
  if (!v1190s.isValid ()){
//     LOG_INFO("Invalid v1190") << "\nInvalid v1190\n"<<std::endl;
    e.put (std::move (clusters));
     return;
  }
 
//  std::unique_ptr<darkart::TDCClusters> clusters(new darkart::TDCClusters);
  for (size_t i = 0; i < v1190s->size(); ++i) {
    const auto& frag((*v1190s)[i]);

    float resolution_ns = frag.hasMetadata () ? frag.metadata<ds50::V1190Fragment::metadata> ()->resolution_ps * 1e-3 : 0.8;

    ds50::V1190Fragment b(frag);
    if (b.dataBegin ()->geo () != ds50::Config::V1190_LS_board_id && b.dataBegin ()->geo () != ds50::Config::V1190_WC_board_id) continue;

    h_->Reset ();
    for (const ds50::V1190Fragment::Word *w = b.dataBegin (); w != b.dataEnd (); ++w) 
      if (w->type () == ds50::V1190Fragment::Word::measurement_w && !w->trailing_edge ())
	h_->Fill (w->measurement () * resolution_ns); // binning in 50 ns

    //Start searching clusters
    for (int i = 1; i < h_->GetNbinsX (); i++) {
      if ((h_->GetBinContent(i) + h_->GetBinContent(i + 1)) >= begin_threshold_) { // begining of cluser
	int start_coarse = i;
	for (; i < h_->GetNbinsX (); i++) if ((h_->GetBinContent(i) + h_->GetBinContent(i + 1)) < end_threshold_) break; // end of cluster

	darkart::TDCCluster current_cluster;
	current_cluster.start_time_ns = 10000000;
	current_cluster.stop_time_ns = -100;
	current_cluster.mean_time_ns = 0;
	current_cluster.n_hits = 0;
	current_cluster.n_long_hits = 0;
	current_cluster.long_hits_avg = 0;
        for (size_t c = 0; c < sizeof (current_cluster.multiplicity) / sizeof (current_cluster.multiplicity[0]); c++) current_cluster.multiplicity[c]=0;

	std::array<short int, 256> multiplicity;
        for(size_t j=0; j<multiplicity.size(); j++) multiplicity[j]=0;

	double start_bin = h_->GetBinLowEdge (start_coarse);
	double stop_bin = h_->GetBinLowEdge (i + 1) + h_->GetBinWidth (1);
	for (const ds50::V1190Fragment::Word *w = b.dataBegin (); w != b.dataEnd (); ++w) {
	  if (w->type () != ds50::V1190Fragment::Word::measurement_w || w->trailing_edge ()) continue;// only leading edges
	  double t = w->measurement () * resolution_ns;
	  if (t < start_bin) continue;
	  if (t > stop_bin) continue;
	  current_cluster.start_time_ns = std::min (current_cluster.start_time_ns, int(t));
	  current_cluster.stop_time_ns = std::max (current_cluster.stop_time_ns, int(t));
	  current_cluster.n_hits ++;
//	  current_cluster.mean_time_ns += t - current_cluster.start_time_ns;
	  current_cluster.mean_time_ns += t;
	  multiplicity[w->channel ()] ++;
//          std::cout<< "channel: "<<(Int_t)w->channel()<<std::endl;

	  for (const ds50::V1190Fragment::Word *w2 = b.dataBegin (); w2 != b.dataEnd (); ++w2) {
	    if (w2->type () != ds50::V1190Fragment::Word::measurement_w || !w2->trailing_edge () || w2->channel () != w->channel ()) continue;
	    float t2 = w2->measurement () * resolution_ns;
	    if (t2 <= t || t2 > stop_bin) continue;
	    //float charge = std::max ((t2 - t) / 50.F, 1.F);
	    float dt = t2 - t;
	    if (dt > 20 && dt < 200) {
	      current_cluster.n_long_hits ++;
	      current_cluster.long_hits_avg += dt;
	    }
	    break; // Do we need this line?
	  }
	}
        current_cluster.mean_time_ns -= current_cluster.n_hits*current_cluster.start_time_ns;
	current_cluster.mean_time_ns /= current_cluster.n_hits;
	if (current_cluster.n_long_hits > 0) current_cluster.long_hits_avg /= current_cluster.n_long_hits;
//           std::cout << "size_max: "<<sizeof (current_cluster.multiplicity)/sizeof (current_cluster.multiplicity[0])<<std::endl;
	for (size_t c = 0; c < sizeof (current_cluster.multiplicity) / sizeof (current_cluster.multiplicity[0]); c++){
           current_cluster.multiplicity[c] = std::count (multiplicity.begin (), multiplicity.end (), c);
//           std::cout<<"c: "<<(Int_t)c<<" multiplicity: "<<(Int_t)current_cluster.multiplicity[c]<<" : "<<std::count (multiplicity.begin (), multiplicity.end (), c)<<std::endl;
        }

	if (b.dataBegin ()->geo () == ds50::Config::V1190_LS_board_id) clusters->ls_clusters.push_back (current_cluster);
	else clusters->wc_clusters.push_back (current_cluster);
      }
    }
  } 
  
  //  LOG_INFO("LS_Custer") << "\nNumver of clusters in LS: "<<clusters->ls_clusters.size()<<"\n"<<std::endl;
  e.put (std::move (clusters));
}

void darkart::VetoTDCCluster::beginRun(art::Run &) {
  h_ = std::unique_ptr<TH1I> (new TH1I ("otdc__", "", 2400, 0, 120e3)); //50 ns bin width
}

DEFINE_ART_MODULE(darkart::VetoTDCCluster)
