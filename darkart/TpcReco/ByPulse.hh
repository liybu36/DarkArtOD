#ifndef darkart_TpcReco_ByPulse_hh
#define darkart_TpcReco_ByPulse_hh
////////////////////////////////////////////////////////////////////////
// ByPulse.
//
// Class template to enable easy access to items in the event associated
// with a particular Pulse. Can access only by darkart::Pulse::PulseID
// because a single (int) pulse_id is not sufficient to uniquely
// identify a Pulse object; each channel has a set of Pulse objects.
////////////////////////////////////////////////////////////////////////
#include "art/Framework/Core/FindOneP.h"
#include "art/Framework/Principal/Event.h"
#include "art/Utilities/InputTag.h"
#include "darkart/TpcReco/PulseHelper.hh"

namespace darkart {
  template <typename T>
  class ByPulse;
}

template <typename T>
class darkart::ByPulse {
public:
  ByPulse(PulseHelper const & getPulse,
            art::Event const & e,
            art::InputTag const & tag);
  ByPulse(art::Handle<PulseVec> const & h,
            art::Event const & e,
            art::InputTag const & tag);

  art::Ptr<T> const & operator () (Pulse::PulseID id) const;

private:
  PulseHelper pulse_;
  art::FindOneP<T> finder_;
};

#ifndef __GCCXML__
template <typename T>
inline
darkart::ByPulse<T>::
ByPulse(PulseHelper const & pulse,
          art::Event const & e,
          art::InputTag const & tag)
:
  pulse_(pulse),
  finder_(pulse_.handle(), e, tag)
{
}

template <typename T>
inline
darkart::ByPulse<T>::
ByPulse(art::Handle<PulseVec> const & h,
          art::Event const & e,
          art::InputTag const & tag)
:
  pulse_(h),
  finder_(h, e, tag)
{
}

template <typename T>
inline
auto
darkart::ByPulse<T>::
operator () (Pulse::PulseID id) const
-> art::Ptr<T> const &
{
  return finder_.at(id.pulse_index);
}

#endif

#endif /* darkart_TpcReco_ByPulse_hh */
