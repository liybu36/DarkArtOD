#ifndef darkart_TpcReco_PmtHelper_hh
#define darkart_TpcReco_PmtHelper_hh
////////////////////////////////////////////////////////////////////////
// PmtHelper.
//
// Easy and efficient access to PMT information by channel.
////////////////////////////////////////////////////////////////////////

#include "art/Utilities/InputTag.h"
#include "darkart/Products/Pmt.hh"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Run.h"

#include <unordered_map>

namespace darkart {
  class PmtHelper;
}

class darkart::PmtHelper {
public:
  PmtHelper(art::Event const & e,
            art::InputTag const & tag);

  Pmt const & operator () (size_t chID) const;
  Pmt const & operator () (Channel::ChannelID id) const;

private:
  art::Handle<PmtVec> getPmts_(art::Event const & e, art::InputTag const & tag);

  art::Handle<PmtVec> pmts_;
  std::unordered_map<size_t, size_t> cachedIndices_;
};

#ifndef __GCCXML__
inline
darkart::PmtHelper::
PmtHelper(art::Event const & e,
          art::InputTag const & tag)
:
  pmts_(getPmts_(e, tag)),
  cachedIndices_()
{
  size_t idx { 0ul };
  std::for_each(pmts_->cbegin(),
                pmts_->cend(),
                [this, &idx](Pmt const & pmt)
                {
                  cachedIndices_[pmt.channel_id] = idx++;
                });
}

inline
auto
darkart::PmtHelper::
operator () (size_t chID) const
-> Pmt const &
{
  return (*pmts_)[cachedIndices_.at(chID)];
}

inline
auto
darkart::PmtHelper::
operator () (Channel::ChannelID id) const
-> Pmt const &
{
  return (*pmts_)[cachedIndices_.at(id.channel_id)];
}

inline
auto
darkart::PmtHelper::
getPmts_(art::Event const & e,
         art::InputTag const & tag)
-> art::Handle<PmtVec>
{
  art::Handle<PmtVec> h;
  e.getRun().getByLabel(tag, h);
  return h;
}


#endif

#endif /* darkart_TpcReco_PmtHelper_hh */
