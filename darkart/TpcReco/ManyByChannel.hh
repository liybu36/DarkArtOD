#ifndef darkart_TpcReco_ManyByChannel_hh
#define darkart_TpcReco_ManyByChannel_hh
////////////////////////////////////////////////////////////////////////
// ManyByChannel.
//
// Class template to enable easy access to items in the event associated
// with a particular Channel. Specifically for case of variable number
// of many-to-one associations to single channel.
//
// 2014-10-15 AFan
// WARNING: since art v1_12_00, std::vector< art::Ptr<T> > returned by
// find many does not necessarily preserve order! Object may be in
// reverse order from insertion. Since these are returns by reference,
// you must std::sort the result in your module. Be sure there is a
// comparator defined for your objects!
//
////////////////////////////////////////////////////////////////////////
#include "art/Framework/Core/FindManyP.h"
#include "art/Framework/Principal/Event.h"
#include "art/Utilities/InputTag.h"
#include "darkart/TpcReco/ChannelHelper.hh"
#include "darkart/Products/Pmt.hh"
#include <vector>
#include <algorithm>

namespace darkart {
  template <typename T>
  class ManyByChannel;
}

template <typename T>
class darkart::ManyByChannel {
public:
  ManyByChannel(ChannelHelper const & getChannel,
                art::Event const & e,
                art::InputTag const & tag);
  ManyByChannel(art::Handle<ChannelVec> const & h,
                art::Event const & e,
                art::InputTag const & tag);
  
  std::vector< art::Ptr<T> >  const & operator () (size_t chID) const;
  std::vector< art::Ptr<T> > const & operator () (Channel::ChannelID id) const;

private:
  ChannelHelper ch_;
  art::FindManyP<T> finder_;
};

#ifndef __GCCXML__
template <typename T>
inline
darkart::ManyByChannel<T>::
ManyByChannel(ChannelHelper const & ch,
              art::Event const & e,
              art::InputTag const & tag)
:
  ch_(ch),
  finder_(ch_.handle(), e, tag)
{
}

template <typename T>
inline
darkart::ManyByChannel<T>::
ManyByChannel(art::Handle<ChannelVec> const & h,
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
darkart::ManyByChannel<T>::
operator () (size_t chID) const
  -> std::vector< art::Ptr<T> > const &
{
  /*
  //std::vector<T const*> v = finder_.at(ch_.indexForID(chID));
  std::vector< art::Ptr<T> > v = finder_.at(ch_.indexForID(chID));
  std::vector<T> result;
  result.reserve(v.size());
  for (size_t i=0; i<v.size(); i++)
    result.push_back(*v[i]);
  return result;
  */

  return finder_.at(ch_.indexForID(chID));
}

template <typename T>
inline
auto
darkart::ManyByChannel<T>::
operator () (Channel::ChannelID id) const
  -> std::vector< art::Ptr<T> > const &
{
  /*
  //std::vector<T const*> v = finder_.at(id.channel_index);
  std::vector< art::Ptr<T> > v = finder_.at(id.channel_index);
  std::vector<T> result;
  result.reserve(v.size());
  for (size_t i=0; i<v.size(); i++)
    result.push_back(*v[i]);
  return result;
  */

  return finder_.at(id.channel_index);
}

#endif

#endif /* darkart_TpcReco_ManyByChannel_hh */
