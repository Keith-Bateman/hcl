
#include <hcl/common/data_structures.h>
void CharStruct::Set(char *data_, size_t size) {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  snprintf(this->value, size + 1, "%s", data_);
}

void CharStruct::Set(std::string data_) {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  snprintf(this->value, data_.length() + 1, "%s", data_.c_str());
}

CharStruct::CharStruct() {}
CharStruct::CharStruct(const CharStruct &other) : CharStruct(other.value) {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
} /* copy constructor*/
CharStruct::CharStruct(CharStruct &&other) : CharStruct(other.value) {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
} /* move constructor*/

CharStruct::CharStruct(const char *data_) {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  snprintf(this->value, strlen(data_) + 1, "%s", data_);
}
CharStruct::CharStruct(std::string data_) : CharStruct(data_.c_str()) {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
}

CharStruct::CharStruct(char *data_, size_t size) {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  snprintf(this->value, size, "%s", data_);
}
const char *CharStruct::c_str() const {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  return value;
}
std::string CharStruct::string() const {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  return std::string(value);
}

char *CharStruct::data() {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  return value;
}
size_t CharStruct::size() const {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  return strlen(value);
}
/**
 * Operators
 */
CharStruct &CharStruct::operator=(const CharStruct &other) {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  strcpy(value, other.c_str());
  return *this;
}
/* equal operator for comparing two Chars. */
bool CharStruct::operator==(const CharStruct &o) const {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  return strcmp(value, o.value) == 0;
}
CharStruct CharStruct::operator+(const CharStruct &o) {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  std::string added = std::string(this->c_str()) + std::string(o.c_str());
  return CharStruct(added);
}
CharStruct CharStruct::operator+(std::string &o) {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  std::string added = std::string(this->c_str()) + o;
  return CharStruct(added);
}
CharStruct &CharStruct::operator+=(const CharStruct &rhs) {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  std::string added = std::string(this->c_str()) + std::string(rhs.c_str());
  Set(added);
  return *this;
}
bool CharStruct::operator>(const CharStruct &o) const {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  return strcmp(this->value, o.c_str()) > 0;
}
bool CharStruct::operator>=(const CharStruct &o) const {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  return strcmp(this->value, o.c_str()) >= 0;
}
bool CharStruct::operator<(const CharStruct &o) const {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  return strcmp(this->value, o.c_str()) < 0;
}
bool CharStruct::operator<=(const CharStruct &o) const {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  return strcmp(this->value, o.c_str()) <= 0;
}

URI::URI(uint16_t _server_idx, CharStruct _uri, CharStruct _ip, uint16_t _port)
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
      endpoint_uri(""),
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
  server_uri += ip + ":" + std::to_string(port);
  client_uri = protocol + "://";
  // if (device.size() > 0) client_uri += (device + "/");
  client_uri += ip + ":" + std::to_string(port);

  endpoint_uri = protocol + "://";
  if (device.size() > 0) endpoint_uri += (device + "/");
  endpoint_uri += interface;
}

URI::URI()
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
URI::URI(const URI &other)
    : server_idx(other.server_idx),
      protocol(other.protocol),
      user_uri(other.user_uri),
      device(other.device),
      interface(other.interface),
      ip(other.ip),
      client_uri(other.client_uri),
      server_uri(other.server_uri),
      endpoint_uri(other.endpoint_uri),
      port(other.port) {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
} /* copy constructor*/
URI::URI(URI &&other)
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

[[maybe_unused]] static CharStruct operator+(const std::string &a1,
                                             const CharStruct &a2) {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  std::string added = a1 + std::string(a2.c_str());
  return CharStruct(added);
}

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
