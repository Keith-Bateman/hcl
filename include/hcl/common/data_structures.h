/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Distributed under BSD 3-Clause license.                                   *
 * Copyright by The HDF Group.                                               *
 * Copyright by the Illinois Institute of Technology.                        *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of Hermes. The full Hermes copyright notice, including  *
 * terms governing use, modification, and redistribution, is contained in    *
 * the COPYING file, which can be found at the top directory. If you do not  *
 * have access to the file, you may request a copy from help@hdfgroup.org.   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*-------------------------------------------------------------------------
 *
 * Created: data_structures.h
 * May 28 2018
 * Hariharan Devarajan <hdevarajan@hdfgroup.org>
 *
 * Purpose: Defines all data structures required in HCL.
 *
 *-------------------------------------------------------------------------
 */

#ifndef INCLUDE_HCL_COMMON_DATA_STRUCTURES_H_
#define INCLUDE_HCL_COMMON_DATA_STRUCTURES_H_

#include <hcl/hcl_config.hpp>
/* Internal Header */
#include <hcl/common/logging.h>
#include <hcl/common/profiler.h>

/* External Headers*/
#include <hcl/common/typedefs.h>
#include <sys/types.h>

#include <boost/concept_check.hpp>
#include <boost/interprocess/containers/string.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <chrono>
#include <cstdint>
#include <string>
#include <vector>

namespace bip = boost::interprocess;
struct CharStruct {
 private:
  char value[256];
  /*Private Functions */
  void Set(char *data_, size_t size);
  void Set(std::string data_);

 public:
  CharStruct();
  CharStruct(const CharStruct &other); /* copy constructor*/
  CharStruct(CharStruct &&other);      /* move constructor*/

  CharStruct(const char *data_);
  CharStruct(std::string data_);

  CharStruct(char *data_, size_t size);
  const char *c_str() const;
  std::string string() const;

  char *data();
  size_t size() const;
  /**
   * Operators
   */
  CharStruct &operator=(const CharStruct &other);
  /* equal operator for comparing two Chars. */
  bool operator==(const CharStruct &o) const;
  CharStruct operator+(const CharStruct &o);
  CharStruct operator+(std::string &o);
  CharStruct &operator+=(const CharStruct &rhs);
  bool operator>(const CharStruct &o) const;
  bool operator>=(const CharStruct &o) const;
  bool operator<(const CharStruct &o) const;
  bool operator<=(const CharStruct &o) const;
};

struct URI {
  uint16_t server_idx;
  CharStruct protocol;
  CharStruct user_uri;
  CharStruct device;
  CharStruct interface;
  CharStruct ip;
  CharStruct client_uri;
  CharStruct server_uri;
  CharStruct endpoint_uri;
  uint16_t port;

  URI(uint16_t _server_idx, CharStruct _uri, CharStruct _ip, uint16_t _port);
  URI();
  URI(const URI &other); /* copy constructor*/
  URI(URI &&other);      /* move constructor*/
};


template <typename T>
class CalculateSize {
 public:
  really_long GetSize(T value) {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    return sizeof(value);
  }
};
template <>
class CalculateSize<std::string> {
 public:
  really_long GetSize(std::string value) {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    return strlen(value.c_str()) + 1;
  }
};
template <>
class CalculateSize<bip::string> {
 public:
  really_long GetSize(bip::string value) {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    return strlen(value.c_str()) + 1;
  }
};

#endif  // INCLUDE_HCL_COMMON_DATA_STRUCTURES_H_
