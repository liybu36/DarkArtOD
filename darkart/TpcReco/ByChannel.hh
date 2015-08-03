#ifndef darkart_TpcReco_ByChannel_hh
#define darkart_TpcReco_ByChannel_hh
////////////////////////////////////////////////////////////////////////
// ByChannel.
//
// Class template to enable easy access to items in the event associated
// with a particular Channel.
////////////////////////////////////////////////////////////////////////
#include "art/Framework/Core/FindOneP.h"
#include "art/Framework/Principal/Event.h"
#include "art/Utilities/InputTag.h"
#include "darkart/TpcReco/ChannelHelper.hh"
#include "darkart/Products/Pmt.hh"

namespace darkart {
  template <typename T>
  class ByChannel;
}

template <typename T>
class darkart::ByChannel {
public:
  ByChannel(ChannelHelper const & getChannel,
            art::Event const & e,
            art::InputTag const & tag);
  ByChannel(art::Handle<ChannelVec> const & h,
            art::Event const & e,
            art::InputTag const & tag);

  art::Ptr<T> const & operator () (size_t chID) const;
  art::Ptr<T> const & operator () (Channel::ChannelID id) const;

private:
  ChannelHelper ch_;
  art::FindOneP<T> finder_;
};

#ifndef __GCCXML__
template <typename T>
inline
darkart::ByChannel<T>::
ByChannel(ChannelHelper const & ch,
          art::Event const & e,
          art::InputTag const & tag)
:
  ch_(ch),
  finder_(ch_.handle(), e, tag)
{
}

template <typename T>
inline
darkart::ByChannel<T>::
ByChannel(art::Handle<ChannelVec> const & h,
          art::Event const & e,
          art::InputTag const & tag)
:
  ch_(h),
  finder_(h, e, tag)
{
}

template <typename T>
inline
auto
darkart::ByChannel<T>::
operator () (size_t chID) const
-> art::Ptr<T> const &
{
  return finder_.at(ch_.indexForID(chID));
}

template <typename T>
inline
auto
darkart::ByChannel<T>::
operator () (Channel::ChannelID id) const
-> art::Ptr<T> const &
{
  return finder_.at(id.channel_index);
}

#endif

#endif /* darkart_TpcReco_ByChannel_hh */
