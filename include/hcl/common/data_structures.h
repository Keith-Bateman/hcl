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

#include <hcl/common/logging.h>
#include <hcl/common/profiler.h>
#include <sys/types.h>

#include <boost/concept_check.hpp>
#include <boost/interprocess/containers/string.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <chrono>
#include <cstdint>
#include <hcl/hcl_config.hpp>
#include <string>
#include <vector>

#include "typedefs.h"

namespace bip = boost::interprocess;

typedef struct CharStruct {
 private:
  char value[256];
  void Set(char *data_, size_t size) {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    snprintf(this->value, size + 1, "%s", data_);
  }
  void Set(std::string data_) {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    snprintf(this->value, data_.length() + 1, "%s", data_.c_str());
  }

 public:
  CharStruct() {}
  CharStruct(const CharStruct &other) : CharStruct(other.value) {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
  } /* copy constructor*/
  CharStruct(CharStruct &&other) : CharStruct(other.value) {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
  } /* move constructor*/

  CharStruct(const char *data_) {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    snprintf(this->value, strlen(data_) + 1, "%s", data_);
  }
  CharStruct(std::string data_) : CharStruct(data_.c_str()) {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
  }

  CharStruct(char *data_, size_t size) {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    snprintf(this->value, size, "%s", data_);
  }
  const char *c_str() const {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    return value;
  }
  std::string string() const {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    return std::string(value);
  }

  char *data() {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    return value;
  }
  size_t size() const {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    return strlen(value);
  }
  /**
   * Operators
   */
  CharStruct &operator=(const CharStruct &other) {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    strcpy(value, other.c_str());
    return *this;
  }
  /* equal operator for comparing two Chars. */
  bool operator==(const CharStruct &o) const {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    return strcmp(value, o.value) == 0;
  }
  CharStruct operator+(const CharStruct &o) {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    std::string added = std::string(this->c_str()) + std::string(o.c_str());
    return CharStruct(added);
  }
  CharStruct operator+(std::string &o) {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    std::string added = std::string(this->c_str()) + o;
    return CharStruct(added);
  }
  CharStruct &operator+=(const CharStruct &rhs) {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    std::string added = std::string(this->c_str()) + std::string(rhs.c_str());
    Set(added);
    return *this;
  }
  bool operator>(const CharStruct &o) const {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    return strcmp(this->value, o.c_str()) > 0;
  }
  bool operator>=(const CharStruct &o) const {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    return strcmp(this->value, o.c_str()) >= 0;
  }
  bool operator<(const CharStruct &o) const {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    return strcmp(this->value, o.c_str()) < 0;
  }
  bool operator<=(const CharStruct &o) const {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    return strcmp(this->value, o.c_str()) <= 0;
  }

} CharStruct;

[[maybe_unused]] static CharStruct operator+(const std::string &a1,
                                             const CharStruct &a2) {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  std::string added = a1 + std::string(a2.c_str());
  return CharStruct(added);
}

struct URI {
  uint16_t server_idx;
  CharStruct protocol;
  CharStruct user_uri;
  CharStruct device;
  CharStruct interface;
  CharStruct ip;
  CharStruct client_uri;
  CharStruct server_uri;
  uint16_t port;

  URI(uint16_t _server_idx, CharStruct _uri, CharStruct _ip, uint16_t _port)
      : server_idx(0),
#if defined(HCL_COMMUNICATION_ENABLE_THALLIUM)
#if defined(HCL_COMMUNICATION_PROTOCOL_ENABLE_UCX)
        protocol("ucx+tcp"),
#else  // if defined(HCL_COMMUNICATION_PROTOCOL_ENABLE_OFI)
        protocol("ofi+tcp"),
#endif
#endif
        user_uri(_uri),
        device(""),
        interface(""),
        ip(""),
        client_uri(""),
        server_uri(""),
        port(9000) {
    std::string uri_str = _uri.string();
    auto protocol_end_pos = uri_str.find("://");
    if (protocol_end_pos == std::string::npos) {
      protocol = _uri;
    } else {
      protocol = uri_str.substr(0, protocol_end_pos);
      auto device_start_pos = protocol_end_pos + 3;
      auto rest = uri_str.substr(device_start_pos);
      // printf("rest %s\n", rest.c_str());
      auto device_end_pos = rest.find("/");
      auto interface_start_pos = device_end_pos + 1;
      if (device_end_pos == std::string::npos) {
        device = "";
        interface_start_pos = device_start_pos;
      } else {
        device = rest.substr(0, device_end_pos);
        interface_start_pos = device_end_pos + 1;
      }
      if (interface_start_pos < uri_str.size() - 2)
        interface = rest.substr(interface_start_pos, uri_str.size() - 2);
    }
    ip = _ip;
    port = _port;
    server_uri = protocol + "://";
    if (device.size() > 0) server_uri += (device + "/");
    server_uri += _ip + ":" + std::to_string(port);
    client_uri = protocol + "://" + ip + ":" + std::to_string(port);
  }
  URI()
      : server_idx(0),
#if defined(HCL_COMMUNICATION_ENABLE_THALLIUM)
#if defined(HCL_COMMUNICATION_PROTOCOL_ENABLE_UCX)
        protocol("ucx+tcp"),
        user_uri("ucx+tcp://"),
#else  // if defined(HCL_COMMUNICATION_PROTOCOL_ENABLE_OFI)
        protocol("ofi+tcp"),
        user_uri("ofi+tcp://"),
#endif
#endif
        device(""),
        interface(""),
        ip(""),
        port(9000) {
  }
  URI(const URI &other)
      : server_idx(other.server_idx),
        protocol(other.protocol),
        user_uri(other.user_uri),
        device(other.device),
        interface(other.interface),
        ip(other.ip),
        client_uri(other.client_uri),
        server_uri(other.server_uri),
        port(other.port) {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
  } /* copy constructor*/
  URI(URI &&other)
      : server_idx(other.server_idx),
        protocol(other.protocol),
        user_uri(other.user_uri),
        device(other.device),
        interface(other.interface),
        ip(other.ip),
        client_uri(other.client_uri),
        server_uri(other.server_uri),
        port(other.port) {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
  } /* move constructor*/
};

namespace std {
template <>
struct hash<CharStruct> {
  size_t operator()(const CharStruct &k) const {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    std::string val(k.c_str());
    return std::hash<std::string>()(val);
  }
};
}  // namespace std

/**
 * Outstream conversions
 */

template <typename T, typename O>
inline std::ostream &operator<<(std::ostream &os, std::pair<T, O> const m) {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  return os << "{TYPE:pair,"
            << "first:" << m.first << ","
            << "second:" << m.second << "}";
}

inline std::ostream &operator<<(std::ostream &os, char const *m) {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  return os << std::string(m);
}

inline std::ostream &operator<<(std::ostream &os, uint8_t const &m) {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  return os << std::to_string(m);
}

inline std::ostream &operator<<(std::ostream &os, CharStruct const &m) {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  return os << "{TYPE:CharStruct,"
            << "value:" << m.c_str() << "}";
}

template <typename T>
inline std::ostream &operator<<(std::ostream &os, std::vector<T> const &ms) {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  os << "[";
  for (auto m : ms) {
    os << m << ",";
  }
  os << "]";
  return os;
}

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
