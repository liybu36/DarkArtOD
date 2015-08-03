
#include "darkart/ArtModules/VetoPMT.hh"

#include <cerrno>
#include <cstdlib>
#include <iostream>
#include <math.h>

using namespace art;
using namespace std;

#include "darkart/ArtModules/VetoMapping.hh"


namespace darkart 
{
  VetoPMT::VetoPMT(fhicl::ParameterSet const &,
		   ActivityRegistry &)
  {
    fill(locs_);
  }

  float VetoPMT::theta(size_t i) const
  {
    return (i<111)?locs_[i][0]:-1;
  }

  float VetoPMT::phi(size_t i) const
  {
    return (i<111)?locs_[i][1]:-1;
  }
}

DEFINE_ART_SERVICE(darkart::VetoPMT)
