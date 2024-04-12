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

#ifndef INCLUDE_HCL_COMMUNICATION_RPC_LIB_H_
#define INCLUDE_HCL_COMMUNICATION_RPC_LIB_H_
#if defined(HCL_HAS_CONFIG)
#include <hcl/hcl_config.hpp>
#else
#error "no config"
#endif
#include <hcl/common/constants.h>
#include <hcl/common/data_structures.h>
#include <hcl/common/debug.h>
#include <hcl/common/macros.h>
#include <hcl/common/singleton.h>
#include <hcl/common/typedefs.h>
#include <mpi.h>

/** Thallium Headers **/
#if defined(HCL_COMMUNICATION_ENABLE_THALLIUM)
#include <thallium.hpp>
#include <thallium/serialization/proc_input_archive.hpp>
#include <thallium/serialization/proc_output_archive.hpp>
#include <thallium/serialization/serialize.hpp>
#include <thallium/serialization/stl/array.hpp>
#include <thallium/serialization/stl/complex.hpp>
#include <thallium/serialization/stl/deque.hpp>
#include <thallium/serialization/stl/forward_list.hpp>
#include <thallium/serialization/stl/list.hpp>
#include <thallium/serialization/stl/map.hpp>
#include <thallium/serialization/stl/multimap.hpp>
#include <thallium/serialization/stl/multiset.hpp>
#include <thallium/serialization/stl/pair.hpp>
#include <thallium/serialization/stl/set.hpp>
#include <thallium/serialization/stl/string.hpp>
#include <thallium/serialization/stl/tuple.hpp>
#include <thallium/serialization/stl/unordered_map.hpp>
#include <thallium/serialization/stl/unordered_multimap.hpp>
#include <thallium/serialization/stl/unordered_multiset.hpp>
#include <thallium/serialization/stl/unordered_set.hpp>
#include <thallium/serialization/stl/vector.hpp>
#endif

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/managed_mapped_file.hpp>
#include <cstdint>
#include <fstream>
#include <future>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace bip = boost::interprocess;
#if defined(HCL_COMMUNICATION_ENABLE_THALLIUM)
namespace tl = thallium;
#endif

class RPC {
 private:
  uint16_t server_port;
  std::string name;
#if defined(HCL_COMMUNICATION_ENABLE_THALLIUM)
  std::shared_ptr<tl::engine> thallium_server;
  std::shared_ptr<tl::engine> thallium_client;
  CharStruct engine_init_str;
  std::vector<tl::endpoint> thallium_endpoints;
  tl::endpoint get_endpoint(CharStruct protocol, CharStruct server_name,
                            uint16_t server_port) {
    // We use addr lookup because mercury addresses must be exactly 15 char
    char ip[16];
    struct hostent *he = gethostbyname(server_name.c_str());
    in_addr **addr_list = (struct in_addr **)he->h_addr_list;
    strcpy(ip, inet_ntoa(*addr_list[0]));
    CharStruct lookup_str = protocol + "://" + HCL_CONF->DEVICE +
                            std::string(ip) + ":" + std::to_string(server_port);
    printf("Lookup String is %s\n", lookup_str.c_str());
    return thallium_client->lookup(lookup_str.c_str());
  }
  void init_engine_and_endpoints(CharStruct protocol) {
    thallium_client = hcl::Singleton<tl::engine>::GetInstance(
        protocol.c_str(), MARGO_CLIENT_MODE);
    thallium_endpoints.reserve(server_list.size());
    for (std::vector<CharStruct>::size_type i = 0; i < server_list.size();
         ++i) {
      thallium_endpoints.push_back(
          get_endpoint(protocol, server_list[i], server_port + i));
    }
  }

  /*std::promise<void> thallium_exit_signal;

    void runThalliumServer(std::future<void> futureObj){

    while(futureObj.wait_for(std::chrono::milliseconds(1)) ==
    std::future_status::timeout){} thallium_engine->wait_for_finalize();
    }*/

#endif
  std::vector<CharStruct> server_list;

 public:
  void Stop() {
    if (HCL_CONF->IS_SERVER) {
      switch (HCL_CONF->RPC_IMPLEMENTATION) {
#ifdef HCL_COMMUNICATION_ENABLE_THALLIUM
        case THALLIUM_TCP:
#endif
#if defined(HCL_COMMUNICATION_ENABLE_THALLIUM)
        {
          // Mercury addresses in endpoints must be freed before
          // finalizing Thallium
          thallium_endpoints.clear();
          thallium_server->finalize();
          break;
        }
#endif
      }
    }
  }
  ~RPC() { Stop(); }

  RPC() : server_port(HCL_CONF->RPC_PORT), server_list(HCL_CONF->SERVER_LIST) {
    AutoTrace trace = AutoTrace("RPC");
    if (server_list.empty() && HCL_CONF->SERVER_LIST_PATH.size() > 0) {
      server_list = HCL_CONF->LoadServers();
    }
    /* if current rank is a server */
    if (HCL_CONF->IS_SERVER) {
      switch (HCL_CONF->RPC_IMPLEMENTATION) {
#ifdef HCL_COMMUNICATION_ENABLE_THALLIUM
        case THALLIUM_TCP: {
          engine_init_str = HCL_CONF->URI + "://" + HCL_CONF->DEVICE +
                            HCL_CONF->SERVER_LIST[HCL_CONF->MY_SERVER] + ":" +
                            std::to_string(server_port + HCL_CONF->MY_SERVER);
          printf("Engine String is %s\n", engine_init_str.c_str());
          break;
        }
#endif
      }
    }
    run(HCL_CONF->RPC_THREADS);
  }

  template <typename F>
  void bind(CharStruct str, F func);

  void run(size_t workers = RPC_THREADS) {
    AutoTrace trace = AutoTrace("RPC::run", workers);
    if (HCL_CONF->IS_SERVER) {
      switch (HCL_CONF->RPC_IMPLEMENTATION) {
#ifdef HCL_COMMUNICATION_ENABLE_THALLIUM
        case THALLIUM_TCP:
#endif
#if defined(HCL_COMMUNICATION_ENABLE_THALLIUM)
        {
          thallium_server = hcl::Singleton<tl::engine>::GetInstance(
              engine_init_str.c_str(), THALLIUM_SERVER_MODE, true,
              HCL_CONF->RPC_THREADS);
          break;
        }
#endif
      }
    }
    switch (HCL_CONF->RPC_IMPLEMENTATION) {
#ifdef HCL_COMMUNICATION_ENABLE_THALLIUM
      case THALLIUM_TCP: {
        init_engine_and_endpoints(HCL_CONF->URI);
        break;
      }
#endif
    }
  }

  template <typename Response, typename... Args>
  Response call(uint16_t server_index, CharStruct const &func_name,
                Args... args);

  template <typename Response, typename... Args>
  Response call(CharStruct &server, uint16_t &port, CharStruct const &func_name,
                Args... args);
  template <typename Response, typename... Args>
  Response callWithTimeout(uint16_t server_index, int timeout_ms,
                           CharStruct const &func_name, Args... args);
  template <typename Response, typename... Args>
  std::future<Response> async_call(uint16_t server_index,
                                   CharStruct const &func_name, Args... args);
  template <typename Response, typename... Args>
  std::future<Response> async_call(CharStruct &server, uint16_t &port,
                                   CharStruct const &func_name, Args... args);
};

#include "rpc_lib.cpp"

#endif  // INCLUDE_HCL_COMMUNICATION_RPC_LIB_H_
