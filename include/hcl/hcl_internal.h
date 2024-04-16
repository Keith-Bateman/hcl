#ifndef INCLUDE_HCL_INTERNAL_H_
#define INCLUDE_HCL_INTERNAL_H_

#include <cstdint>
#include <hcl/hcl_config.hpp>
/*Internal*/
#include <hcl/communication/rpc_lib.h>
/*Standard*/
#include <stdint.h>

#include <memory>
#include <stdexcept>
#include <unordered_map>

#include "common/logging.h"

namespace hcl {

class HCL {
 private:
  boost::mutex file_load;
  std::shared_ptr<hcl ::ConfigurationManager> conf;

  // port. rpc
  std::unordered_map<uint16_t, std::shared_ptr<RPC>> rpcs;

  static std::shared_ptr<HCL> instance;

  std::vector<URI> LoadURI(uint16_t _port, CharStruct _server_list_path,
                           CharStruct _uri, int16_t _my_server_idx,
                           bool _is_server) {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    file_load.lock();
    std::fstream file;
    auto uris = std::vector<URI>();
    file.open(_server_list_path.c_str(), std::ios::in);
    uint16_t server_index = 0;
    if (file.is_open()) {
      std::string file_line;

      int count;
      while (getline(file, file_line)) {
        CharStruct server_node_name;
        if (!file_line.empty()) {
          auto split_loc = file_line.find(":");
          auto split_loc2 = file_line.find(':');  // split to node and net
          // int split_loc = file_line.find(" slots=");
          // int split_loc2 = file_line.find('='); // split to node and net
          if (split_loc != std::string::npos) {
            server_node_name = file_line.substr(0, split_loc);
            count = atoi(
                file_line.substr(split_loc2 + 1, std::string::npos).c_str());
          } else {
            // no special network
            server_node_name = file_line;
            count = 1;
          }
          // server list is list of network interfaces
          for (int i = 0; i < count; ++i) {
            auto server_uri =
                URI(server_index, _uri, server_node_name, _port + server_index);
            uris.push_back(server_uri);
            if (_is_server && server_index == _my_server_idx) {
              HCL_LOG_DEBUG("HCL client_uri:%s server_uri:%s",
                            server_uri.client_uri.c_str(),
                            server_uri.server_uri.c_str());
            }
            server_index++;
          }
        }
      }
    } else {
      printf("Error: Can't open server list file %s\n",
             _server_list_path.c_str());
    }
    file.close();
    file_load.unlock();
    return uris;
  }
  int ConfigureInternalEnv() {
    char* uri_str = getenv(HCL_THALLIUM_URI_ENV.c_str());
    if (uri_str != nullptr) {
      conf->URI = uri_str;
    }
    return 0;
  }
  int ConfigureInternal(bool initialize, uint16_t _port, uint16_t _num_servers,
                        int16_t _my_server_idx, really_long _memory_allocated,
                        int16_t _is_server, int16_t _is_server_on_node,
                        CharStruct _backed_file_dir,
                        CharStruct _server_list_path, CharStruct _uri) {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    if (_port != 0) conf->RPC_PORT = _port;
    if (_num_servers != 0) conf->NUM_SERVERS = _num_servers;
    if (_my_server_idx != -1) conf->MY_SERVER = _my_server_idx;
    if (_memory_allocated != 0) conf->MEMORY_ALLOCATED = _memory_allocated;
    if (_is_server != -1) conf->IS_SERVER = _is_server == 1;
    if (_is_server_on_node != -1)
      conf->SERVER_ON_NODE = _is_server_on_node == 1;
    if (_backed_file_dir.size() != 0) conf->BACKED_FILE_DIR = _backed_file_dir;
    if (_server_list_path.size() != 0)
      conf->SERVER_LIST_PATH = _server_list_path;
    if (_uri.size() != 0) conf->URI = _uri;
    if (_port == 0 && (_server_list_path.size() != 0 || _uri.size() != 0)) {
      throw new std::logic_error(
          "To change the server list or URI use a new port");
    }
    if (initialize || _server_list_path.size() != 0 || _uri.size() != 0) {
      auto uris = LoadURI(conf->RPC_PORT, conf->SERVER_LIST_PATH, conf->URI,
                          conf->MY_SERVER, conf->IS_SERVER);
      if (uris.size() != conf->NUM_SERVERS) {
        conf->NUM_SERVERS = uris.size();
        if (conf->IS_SERVER == 1)
          HCL_LOG_INFO("Making number of servers match the server list to %d",
                       conf->NUM_SERVERS);
      }
      CreateRPC(conf->RPC_PORT, uris);
    }
    return 0;
  }
  int CreateRPC(uint16_t server_port, std::vector<URI>& uris) {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    auto rpc = std::make_shared<RPC>(conf->IS_SERVER, conf->MY_SERVER,
                                     conf->RPC_THREADS, uris);
    rpcs.insert_or_assign(server_port, rpc);
    return 0;
  }

 public:
  HCL(uint16_t _port, uint16_t _num_servers, int16_t _my_server_idx,
      really_long _memory_allocated, int16_t _is_server,
      int16_t _is_server_on_node, CharStruct _backed_file_dir,
      CharStruct _server_list_path, CharStruct _uri)
      : rpcs() {
    conf = HCL_CONF;
    ConfigureInternalEnv();
    HCL_LOGGER_INIT();
    HCL_PROFILER_CPP_INIT(NULL);
    ConfigureInternal(true, _port, _num_servers, _my_server_idx,
                      _memory_allocated, _is_server, _is_server_on_node,
                      _backed_file_dir, _server_list_path, _uri);
  }                         /* hidden default constructor. */
  HCL(const HCL&) = delete; /* deleting copy constructor. */

  int Finalize() {
    for (auto iter : rpcs) {
      iter.second.reset();
    }
    rpcs.clear();
    return 0;
  }

  static std::shared_ptr<HCL> GetInstance(
      bool initialize = true, uint16_t _port = 0, uint16_t _num_servers = 0,
      int32_t _my_server_idx = -1, really_long _memory_allocated = 0,
      int16_t _is_server = -1, int16_t _is_server_on_node = -1,
      CharStruct _backed_file_dir = "", CharStruct _server_list_path = "",
      CharStruct _uri = "") {
    if (instance == nullptr) {
      HCL_LOG_TRACE();
      HCL_CPP_FUNCTION()
      if (initialize) {
        instance = std::make_shared<HCL>(
            _port, _num_servers, _my_server_idx, _memory_allocated, _is_server,
            _is_server_on_node, _backed_file_dir, _server_list_path, _uri);
      } else {
        throw new std::logic_error("HCL was not initialized");
      }
    }
    return instance;
  }
  int ReConfigure(uint16_t _port = 0, uint16_t _num_servers = 0,
                  int32_t _my_server_idx = -1,
                  really_long _memory_allocated = 0, int16_t _is_server = -1,
                  int16_t _is_server_on_node = -1,
                  CharStruct _backed_file_dir = "",
                  CharStruct _server_list_path = "", CharStruct _uri = "") {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    return ConfigureInternal(false, _port, _num_servers, _my_server_idx,
                             _memory_allocated, _is_server, _is_server_on_node,
                             _backed_file_dir, _server_list_path, _uri);
  }
  std::shared_ptr<RPC> GetRPC(uint16_t server_port) {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    auto iter = rpcs.find(server_port);
    if (iter != rpcs.end()) {
      return iter->second;
    } else {
      throw new std::logic_error(
          "RPC is not initialized, ReConfigure HCL and then create the "
          "datastructure on new port.");
    }
  }
};

std::shared_ptr<HCL> HCL::instance = nullptr;
}  // namespace hcl

#endif  // INCLUDE_HCL_INTERNAL_H_
