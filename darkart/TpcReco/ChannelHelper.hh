#ifndef darkart_TpcReco_ChannelHelper_hh
#define darkart_TpcReco_ChannelHelper_hh
////////////////////////////////////////////////////////////////////////
// ChannelHelper.
//
// Easy and efficient access to channel by ID and ChannelID.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Principal/Handle.h"
#include "darkart/Products/Channel.hh"

#include <unordered_map>

namespace darkart {
  class ChannelHelper;
}

class darkart::ChannelHelper {
public:
  explicit ChannelHelper(art::Handle<ChannelVec> const & h);

  Channel const & operator () (size_t chID) const;
  Channel const & operator () (Channel::ChannelID id) const;

  size_t indexForID(size_t chID) const;
  art::Handle<ChannelVec> const & handle() const;

private:
  void cacheIndices_() const;
  art::Handle<ChannelVec> h_;
  mutable std::unordered_map<size_t, size_t> cachedIndices_;
};

#ifndef __GCCXML__
inline
darkart::ChannelHelper::
ChannelHelper(art::Handle<ChannelVec> const & h)
:
  h_(h),
  cachedIndices_()
{
}

// Note: in the implementations below, the single assumption that the
// ChannelID of each channel in the product is correct allows us to use
// operator[] safely in several cases.

inline
auto
darkart::ChannelHelper::
operator () (size_t chID) const
-> Channel const &
{
  return (*h_)[indexForID(chID)];
}

inline
auto
darkart::ChannelHelper::
operator () (Channel::ChannelID id) const
-> Channel const &
{
  return (*h_)[id.channel_index];
}

inline
size_t
darkart::ChannelHelper::
indexForID(size_t chID) const
{
  if (cachedIndices_.empty()) {
    cacheIndices_();
  }
  return cachedIndices_.at(chID);
}

inline
auto
darkart::ChannelHelper::
handle() const
-> art::Handle<ChannelVec> const &
{
  return h_;
}

inline
void
darkart::ChannelHelper::
cacheIndices_() const
{
  std::for_each(h_->cbegin(),
                h_->cend(),
                [this](Channel const & ch)
                {
                  cachedIndices_[ch.channelID.channel_id] = ch.channelID.channel_index;
                });
}

#endif

#endif /* darkart_TpcReco_ChannelHelper_hh */

