/*

  Some functions that can be used frequently hopefully without
  worrying about the details.

  A lot of this code borrowed from NOvA.

  AFan 2013-07-06
  
*/

#ifndef darkart_TpcReco_utilities_hh
#define darkart_TpcReco_utilities_hh



#include "art/Persistency/Common/Ptr.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Persistency/Common/Assns.h"
#include "art/Framework/Core/EDProducer.h"
#include "art/Persistency/Provenance/ProductID.h"
#include "art/Utilities/Exception.h"
#include "art/Framework/Core/FindOne.h"
#include "art/Framework/Core/FindMany.h"
#include "art/Utilities/InputTag.h"
#include "cetlib/maybe_ref.h"
#include "cetlib/exception.h"
#include "messagefacility/MessageLogger/MessageLogger.h"


#include "darkart/Products/Channel.hh"
#include "darkart/Products/Pmt.hh"


#include <string>
#include <iostream>
#include <algorithm>

namespace util
{
  
  // Add an association between two objects that each live inside
  // separate vector data products.
  // a is a new vector about to be added to the event
  // b is a new vector about to be added to the event
  // a_idx is the index of the object in a
  // b_idx is the index of the object in b
  template<class T, class U>
  bool
  createAssn(art::EDProducer const& prod,
             art::Event const& evt,
             std::vector<T> const& a,
             size_t a_idx,
             std::vector<U> const& b,
             size_t b_idx,
             art::Assns<T,U> & assn,
             std::string const& a_instance = std::string(),
             std::string const& b_instance = std::string());


  
  // Add an association between two objects that each live inside
  // separate vector data products.
  // a is a handle to a vector already in the event
  // b is a new vector about to be added to the event
  // a_idx is the index of the object in *a
  // b_idx is the index of the object in b
  template<class T, class U>
  bool
  createAssn(art::EDProducer const& prod,
             art::Event const& evt,
             art::Handle<std::vector<T> > const& a,
             size_t a_idx,
             std::vector<U> const& b,
             size_t b_idx,
             art::Assns<T,U> & assn,
             std::string const& b_instance = std::string());
  

  // Add an association between two objects that each live inside
  // separate vector data products.
  // a is a handle to a vector already in the event
  // b is a handle to a vector already in the event
  // a_idx is the index of the object in *a
  // b_idx is the index of the object in *b
  template<class T, class U> 
  bool
  createAssn(art::EDProducer const& prod,
             art::Event const& evt,
             art::Handle<std::vector<T> > const& a,
             size_t a_idx,
             art::Handle<std::vector<U> > const& b,
             size_t b_idx,
             art::Assns<T,U> & assn);
  
  
  // Find the index of an object within a vector
  // NOTE: == operator of T must be overloaded
  template<class T>
  size_t
  findIndex(std::vector<T> const& vec,
            T const& obj);
  

  // Add an association between two objects that each live inside
  // separate vector data products. Pass in the objects themselves.
  // a_vec is a new vector about to be added to the event
  // b_vec is a new vector about to be added to the event
  template<class T, class U>
  bool
  createAssn(art::EDProducer const& prod,
             art::Event const& evt,
             std::vector<T> const& a_vec,
             T const& a,
             std::vector<U> const& b_vec,
             U const& b,
             art::Assns<T,U> & assn,
             std::string const& a_instance = std::string(),
             std::string const& b_instance = std::string());

  
  // Add an association between two objects that each live inside
  // separate vector data products. Pass in the objects themselves.
  // a_vec is a Handle to a vector already in the event
  // b_vec is a new vector about to be added to the event
  template<class T, class U>
  bool
  createAssn(art::EDProducer const& prod,
             art::Event const& evt,
             art::Handle<std::vector<T> > const& a_vec,
             T const& a,
             std::vector<U> const& b_vec,
             U const& b,
             art::Assns<T,U> & assn,
             std::string const& b_instance = std::string());
  
  
  
  // Add an association between two objects that each live inside
  // separate vector data products. Pass in the objects themselves.
  // a_vec is a Handle to a darkart::ChannelVec already in the event
  // b_vec is a new vector about to be added to the event
  template<class U>
  bool
  createAssn(art::EDProducer const& prod,
             art::Event const& evt,
             art::Handle<darkart::ChannelVec > const& a_vec,
             darkart::Channel const& a,
             std::vector<U> const& b_vec,
             U const& b,
             art::Assns<darkart::Channel,U> & assn,
             std::string const& b_instance = std::string());
  
  
  // Add an association between two objects that each live inside
  // separate vector data products. Pass in the objects themselves.
  // a_vec is a Handle to a vector already in the event
  // b_vec is a Handle to a vector already in the event
  template<class T, class U> static
  bool
  createAssn(art::EDProducer const& prod,
             art::Event const& evt,
             art::Handle<std::vector<T> > const& a_vec,
             T const& a,
             art::Handle<std::vector<U> > const& b_vec,
             U const& b,
             art::Assns<T,U> & assn);


  
  // Find an object associated with a specific channel
  template<class T>
  T
  getByChannelID(art::Event const& evt,
                 art::Handle<darkart::ChannelVec> const& chVecHandle,
                 int const ch,
                 art::InputTag const& input_tag);

  
  // Find the darkart::Pmt object associated with a specific channel.
  // darkart::Pmt objects are stored in the art::Run.
  template<>
  darkart::Pmt
  getByChannelID<darkart::Pmt>(art::Event const& evt,
                               art::Handle<darkart::ChannelVec> const& chVecHandle,
                               int const ch,
                               art::InputTag const& input_tag);
  
                               
  // Find an object associated with a specific channel
  template<class T>
  T
  getByChannelID(art::Event const& evt,
                 art::Handle<darkart::ChannelVec> const& chVecHandle,
                 darkart::Channel::ChannelID const& channelID,
                 art::InputTag const& input_tag);

  // Find the darkart::Pmt object associated with a specific channel.
  // darkart::Pmt objects are stored in the art::Run.
  template<>
  darkart::Pmt
  getByChannelID<darkart::Pmt>(art::Event const& evt,
                               art::Handle<darkart::ChannelVec> const& chVecHandle,
                               darkart::Channel::ChannelID const& channelID,
                               art::InputTag const& input_tag);
  
  // Retrieve the channel object by channel ID
  darkart::Channel
  getByChannelID(art::Handle<darkart::ChannelVec> const& chVecHandle,
                 int const ch);

  // Retrieve the channel object by darkart::Channel::ChannelID
  darkart::Channel
  getByChannelID(art::Handle<darkart::ChannelVec> const& chVecHandle,
                 darkart::Channel::ChannelID const& channelID);

  // Find an arbitrary number of objects associated with a specific channel
  template<class T>
  std::vector<T>
  getManyByChannelID(art::Event const& evt,
                     art::Handle<darkart::ChannelVec> const& chVecHandle,
                     darkart::Channel::ChannelID const& channelID,
                     art::InputTag const& input_tag);
  
  // Find an arbitrary number of objects associated with a specific channel
  template<class T>
  std::vector<T>
  getManyByChannelID(art::Event const& evt,
                     art::Handle<darkart::ChannelVec> const& chVecHandle,
                     int const ch,
                     art::InputTag const& input_tag);
  
  

  
  
  // Find an object associated with an entry in vector
  template<class T, class U> 
  T
  getByAssn(art::Event const& evt,
            art::Handle<std::vector<U> > const& handle,
            U const& obj,
            art::InputTag const& input_tag);
  

  
  // Decide if this channel should be skipped
  bool
  skipChannel(int ch, std::vector<int> skip_ch)
  {
    return std::find(skip_ch.begin(),skip_ch.end(),ch) != skip_ch.end();
  }
  
  // Decide if this channel should be skipped
  bool
  skipChannel(darkart::Channel ch, std::vector<int> skip_ch)
  {
    return std::find(skip_ch.begin(),skip_ch.end(),ch.channel_id()) != skip_ch.end();
  }
  
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
template<class T, class U> 
bool
util::createAssn(art::EDProducer const& prod,
                 art::Event const& evt,
                 std::vector<T> const& a,
                 size_t a_idx,
                 std::vector<U> const& b,
                 size_t b_idx,
                 art::Assns<T,U> & assn,
                 std::string const& a_instance = std::string(),
                 std::string const& b_instance = std::string())
{
  bool ret = true;
  
  if ( !(a_idx < a.size()) || !(b_idx < b.size()) ) {
    throw cet::exception("ProductNotFound") << "createAssn couldn't find input object within the input vector."<<std::endl;
  }

  art::ProductID a_id = prod.getProductID<std::vector<T> >(evt, a_instance);
  art::ProductID b_id = prod.getProductID<std::vector<U> >(evt, b_instance);
  auto a_getter = evt.productGetter(a_id);
  auto b_getter = evt.productGetter(b_id);
  art::Ptr<T> a_ptr(a_id, a_idx, a_getter);
  art::Ptr<U> b_ptr(b_id, b_idx, b_getter);
  assn.addSingle(a_ptr,b_ptr);
  return ret;

}


//----------------------------------------------------------------------------
template<class T, class U> 
bool
util::createAssn(art::EDProducer const& prod,
                 art::Event const& evt,
                 art::Handle<std::vector<T> > const& a,
                 size_t a_idx,
                 std::vector<U> const& b,
                 size_t b_idx,
                 art::Assns<T,U> & assn,
                 std::string const& b_instance=std::string())
{
  bool ret = true;
  
  if ( !(a_idx < a->size()) || !(b_idx < b.size()) ) {
    throw cet::exception("ProductNotFound") << "createAssn couldn't find input object within the input vector."<<std::endl;
  }
    
  art::ProductID b_id = prod.getProductID<std::vector<U> >(evt, b_instance);
  auto b_getter = evt.productGetter(b_id);
  art::Ptr<T> a_ptr(a, a_idx);
  art::Ptr<U> b_ptr(b_id, b_idx, b_getter);
  assn.addSingle(a_ptr,b_ptr);
  return ret;
}



//----------------------------------------------------------------------------
template<class T, class U> 
bool
util::createAssn(art::EDProducer const& ,
           art::Event const& ,
           art::Handle<std::vector<T> > const& a,
           size_t a_idx,
           art::Handle<std::vector<U> > const& b,
           size_t b_idx,
           art::Assns<T,U> & assn)
{
  bool ret = true;
  
  if ( !(a_idx < a->size()) || !(b_idx < b->size()) ) {
    throw cet::exception("ProductNotFound") << "createAssn couldn't find input object within the input vector."<<std::endl;
  }
  
  art::Ptr<T> a_ptr(a, a_idx);
  art::Ptr<U> b_ptr(b, b_idx);
  assn.addSingle(a_ptr,b_ptr);
  return ret;
}



//----------------------------------------------------------------------------
template<class T> 
size_t
util::findIndex(std::vector<T> const& vec,
                T const& obj)
{
  // do the search backwards because this usually terminates faster
  int obj_idx = vec.size()-1;
  while (!(vec.at(obj_idx)==obj)) {
    obj_idx--;
    if (obj_idx==-1) {
      obj_idx = UINT_MAX;
      break;
    }
  }

  return obj_idx;
}



//----------------------------------------------------------------------------
template<class T, class U> 
bool
util::createAssn(art::EDProducer const& prod,
                 art::Event const& evt,
                 std::vector<T> const& a_vec,
                 T const& a,
                 std::vector<U> const& b_vec,
                 U const& b,
                 art::Assns<T,U> & assn,
                 std::string const& a_instance = std::string(),
                 std::string const& b_instance = std::string())
{
  size_t a_idx = findIndex(a_vec, a);
  size_t b_idx = findIndex(b_vec, b);
  if (a_idx == UINT_MAX || b_idx == UINT_MAX) {
    throw cet::exception("ProductNotFound") << "createAssn couldn't find input object within the input vector."<<std::endl;
  }

  return createAssn(prod, evt, a_vec, a_idx, b_vec, b_idx, assn, a_instance,
                    b_instance);
}


//----------------------------------------------------------------------------
template<class T, class U> 
bool
util::createAssn(art::EDProducer const& prod,
                 art::Event const& evt,
                 art::Handle<std::vector<T> > const& a_vec,
                 T const& a,
                 std::vector<U> const& b_vec,
                 U const& b,
                 art::Assns<T,U> & assn,
                 std::string const& b_instance = std::string())
{
  size_t a_idx = findIndex(*a_vec, a);
  size_t b_idx = findIndex(b_vec, b);
  if (a_idx == UINT_MAX || b_idx == UINT_MAX) {
    throw cet::exception("ProductNotFound") << "createAssn couldn't find input object within the input vector."<<std::endl;
  }

  return createAssn(prod, evt, a_vec, a_idx, b_vec, b_idx, assn, b_instance);
}



//----------------------------------------------------------------------------
template<class U>
bool
util::createAssn(art::EDProducer const& prod,
                 art::Event const& evt,
                 art::Handle<darkart::ChannelVec > const& a_vec,
                 darkart::Channel const& a,
                 std::vector<U> const& b_vec,
                 U const& b,
                 art::Assns<darkart::Channel,U> & assn,
                 std::string const& b_instance = std::string())
{
  size_t a_idx = a.channelID.channel_index;
  size_t b_idx = UINT_MAX;
  if (!b_vec.empty() && b == b_vec.back()) {
    b_idx = b_vec.size() - 1;
  }
  if (b_idx == UINT_MAX) {
    throw cet::exception("ProductNotFound") << "createAssn couldn't find input object within the input vector."<<std::endl;
  }

  return createAssn(prod, evt, a_vec, a_idx, b_vec, b_idx, assn, b_instance);

}


//----------------------------------------------------------------------------
template<class T, class U> 
bool
util::createAssn(art::EDProducer const& prod,
                 art::Event const& evt,
                 art::Handle<std::vector<T> > const& a_vec,
                 T const& a,
                 art::Handle<std::vector<U> > const& b_vec,
                 U const& b,
                 art::Assns<T,U> & assn)
{
  size_t a_idx = findIndex(*a_vec, a);
  size_t b_idx = findIndex(*b_vec, b);
  if (a_idx == UINT_MAX || b_idx == UINT_MAX) {
    throw cet::exception("ProductNotFound") << "createAssn couldn't find input object within the input vector."<<std::endl;
  }
  
  return createAssn(prod, evt, a_vec, a_idx, b_vec, b_idx, assn);
}




//----------------------------------------------------------------------------
template<class T>
T
util::getByChannelID(art::Event const& evt,
                     art::Handle<darkart::ChannelVec> const& chVecHandle,
                     int const ch,
                     art::InputTag const& input_tag)
{
  
  int ch_idx = -1;
  for (size_t i=0; i<chVecHandle->size(); ++i) {
    if (ch==chVecHandle->at(i).channelID.channel_id) {
      ch_idx = chVecHandle->at(i).channelID.channel_index;
      break;
    }
  }
  if (ch_idx == -1)
    LOG_ERROR("util") << "Index not found.";


  art::FindOne<T> fa(chVecHandle, evt, input_tag); 
  cet::maybe_ref<T const> result(fa.at(ch_idx));
  if (!result) {
    throw cet::exception("ProductRetrieval") << "Could not find object associated with ch"<<ch;
  }
  return result.ref();
}


//----------------------------------------------------------------------------
template<> 
darkart::Pmt
util::getByChannelID<darkart::Pmt>(art::Event const& evt,
                                   art::Handle<darkart::ChannelVec> const& chVecHandle,
                                   int const ch,
                                   art::InputTag const& input_tag)
{

  if (static_cast<int>(chVecHandle->size()) < ch)
    throw cet::exception("ProductRetrieval") << "Bad ChannelID";

  art::Run const& r = evt.getRun();
  art::Handle<darkart::PmtVec> pmtVecHandle;
  r.getByLabel(input_tag, pmtVecHandle);
  int idx = -1;
  for (auto const& pmt : *pmtVecHandle) {
    ++idx;
    if (ch == pmt.channel_id)
      break;
  }
  return pmtVecHandle->at(idx);
}


//----------------------------------------------------------------------------
template<class T> 
T
util::getByChannelID(art::Event const& evt,
                     art::Handle<darkart::ChannelVec> const& chVecHandle,
                     darkart::Channel::ChannelID const& channelID,
                     art::InputTag const& input_tag)
{
  art::FindOne<T> fa(chVecHandle, evt, input_tag);
  cet::maybe_ref<T const> result(fa.at(channelID.channel_index));
  if (!result) {
    throw cet::exception("ProductRetrieval") << "Could not find object associated with channel "
                                             << channelID.channel_id;
  }
  return result.ref();

}

//----------------------------------------------------------------------------
template<>
darkart::Pmt
util::getByChannelID<darkart::Pmt>(art::Event const& evt,
                                   art::Handle<darkart::ChannelVec> const& chVecHandle,
                                   darkart::Channel::ChannelID const& channelID,
                                   art::InputTag const& input_tag)
{
  if (static_cast<int>(chVecHandle->size()) < channelID.channel_index)
    throw cet::exception("ProductRetrieval") << "Bad ChannelID";
  
  art::Run const& r = evt.getRun();
  art::Handle<darkart::PmtVec> pmtVecHandle;
  r.getByLabel(input_tag, pmtVecHandle);
  int idx = -1;
  for (auto const& pmt : *pmtVecHandle) {
    ++idx;
    if (channelID.channel_id == pmt.channel_id)
      break;
  }
  return pmtVecHandle->at(idx);
}

//----------------------------------------------------------------------------
darkart::Channel
util::getByChannelID(art::Handle<darkart::ChannelVec> const& chVecHandle,
                     int const ch)
{
  int ch_idx = -1;
  for (size_t i=0; i<chVecHandle->size(); ++i) {
    if (ch == (*chVecHandle)[i].channelID.channel_id) {
      ch_idx = (*chVecHandle)[i].channelID.channel_index;
      break;
    }
  }
  if (ch_idx == -1)
    throw cet::exception("ProductRetrieval") << "Invalid input index to retrieve object.";
  
  return chVecHandle->at(ch_idx);
}

//----------------------------------------------------------------------------
darkart::Channel 
util::getByChannelID(art::Handle<darkart::ChannelVec> const& chVecHandle,
                     darkart::Channel::ChannelID const& channelID)
{
  if (channelID.channel_index<0)
    throw cet::exception("ProductRetrieval") << "Invalid input index to retrieve object.";
  return chVecHandle->at(channelID.channel_index);
}

//----------------------------------------------------------------------------
template<class T> 
std::vector<T>
util::getManyByChannelID(art::Event const& evt,
                         art::Handle<darkart::ChannelVec> const& chVecHandle,
                         darkart::Channel::ChannelID const& channelID,
                         art::InputTag const& input_tag)
{
  art::FindMany<T> f(chVecHandle, evt, input_tag);
  std::vector<T const*> v = f.at(channelID.channel_index);
  std::vector<T> result;
  result.reserve(v.size());
  for (size_t i=0; i<v.size(); i++)
    result.push_back(*v[i]);
  return result;
}


//----------------------------------------------------------------------------
template<class T> 
std::vector<T>
util::getManyByChannelID(art::Event const& evt,
                         art::Handle<darkart::ChannelVec> const& chVecHandle,
                         int const ch,
                         art::InputTag const& input_tag)
{
  int ch_idx = -1;
  for (size_t i=0; i<chVecHandle->size(); ++i) {
    if (ch == (*chVecHandle)[i].channelID.channel_id) {
      ch_idx = (*chVecHandle)[i].channelID.channel_index;
      break;
    }
  }
  if (ch_idx == -1)
    LOG_ERROR("util") << "Index not found.";

  art::FindMany<T> f(chVecHandle, evt, input_tag);
  std::vector<T const*> v = f.at(ch_idx);
  std::vector<T> result;
  result.reserve(v.size());
  for (size_t i=0; i<v.size(); i++)
    result.push_back(*v[i]);
  return result;
}






//----------------------------------------------------------------------------
template<class T, class U> 
T
util::getByAssn(art::Event const& evt,
                art::Handle<std::vector<U> > const& handle,
                U const& obj,
                art::InputTag const& input_tag)
{
  size_t idx = findIndex(*handle, obj);
  art::FindOne<T> f(handle, evt, input_tag);
  cet::maybe_ref<T const> result(f.at(idx));
  if (!result) {
    throw cet::exception("ProductRetrieval") << "Could not find associated object";
  }
  return result.ref();
  
}


#endif

