#ifndef darkart_TpcReco_PulseHelper_hh
#define darkart_TpcReco_PulseHelper_hh
////////////////////////////////////////////////////////////////////////
// PulseHelper.
//
// Easy and efficient access to objects associated to a pulse by ID and
// PulseID
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Principal/Handle.h"
#include "darkart/Products/Pulse.hh"

#include <unordered_map>

namespace darkart {
  class PulseHelper;
}

class darkart::PulseHelper {
public:
  explicit PulseHelper(art::Handle<PulseVec> const & h);

  Pulse const & operator () (size_t pulseID) const;
  Pulse const & operator () (Pulse::PulseID id) const;

  size_t indexForID(size_t pulseID) const;
  art::Handle<PulseVec> const & handle() const;

private:
  void cacheIndices_() const;
  art::Handle<PulseVec> h_;
  mutable std::unordered_map<size_t, size_t> cachedIndices_;
};

#ifndef __GCCXML__
inline
darkart::PulseHelper::
PulseHelper(art::Handle<PulseVec> const & h)
:
  h_(h),
  cachedIndices_()
{
}

// Note: in the implementations below, the single assumption that the
// PulseID of each pulse in the product is correct allows us to use
// operator[] safely in several cases.

inline
auto
darkart::PulseHelper::
operator () (size_t pulseID) const
-> Pulse const &
{
  return (*h_)[indexForID(pulseID)];
}

inline
auto
darkart::PulseHelper::
operator () (Pulse::PulseID id) const
-> Pulse const &
{
  return (*h_)[id.pulse_index];
}

inline
size_t
darkart::PulseHelper::
indexForID(size_t pulseID) const
{
  if (cachedIndices_.empty()) {
    cacheIndices_();
  }
  return cachedIndices_.at(pulseID);
}

inline
auto
darkart::PulseHelper::
handle() const
-> art::Handle<PulseVec> const &
{
  return h_;
}

inline
void
darkart::PulseHelper::
cacheIndices_() const
{
  std::for_each(h_->cbegin(),
                h_->cend(),
                [this](Pulse const & pulse)
                {
                  cachedIndices_[pulse.pulseID.pulse_id] = pulse.pulseID.pulse_index;
                });
}

#endif

#endif /* darkart_TpcReco_PulseHelper_hh */

