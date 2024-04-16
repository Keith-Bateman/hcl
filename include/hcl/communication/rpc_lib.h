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

#include <hcl/common/constants.h>
#include <hcl/common/data_structures.h>
#include <hcl/common/debug.h>
#include <hcl/common/macros.h>
#include <hcl/common/singleton.h>
#include <hcl/common/typedefs.h>

#include <hcl/hcl_config.hpp>

/** Thallium Headers **/
#if defined(HCL_COMMUNICATION_ENABLE_THALLIUM)
// save diagnostic state
#pragma GCC diagnostic push

// turn off the specific warning. Can also use "-Wall"
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#pragma GCC diagnostic ignored "-Wpedantic"
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
#pragma GCC diagnostic pop
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
  bool is_server;
  uint16_t my_server_index;
  size_t threads;
  std::vector<URI> uris;

#if defined(HCL_COMMUNICATION_ENABLE_THALLIUM)
  std::shared_ptr<tl::engine> thallium_server;
  std::shared_ptr<tl::engine> thallium_client;
  CharStruct engine_init_str;
  std::vector<tl::endpoint> thallium_endpoints;
  tl::endpoint get_endpoint(URI server_uri) {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    // // We use addr lookup because mercury addresses must be exactly 15 char
    // char ip[16];
    // struct hostent *he = gethostbyname(_server_name.c_str());
    // in_addr **addr_list = (struct in_addr **)he->h_addr_list;
    // strcpy(ip, inet_ntoa(*addr_list[0]));
    return thallium_client->lookup(server_uri.client_uri.c_str());
  }
  void init_engine_and_endpoints() {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    thallium_client = hcl::Singleton<tl::engine>::GetInstance(
        uris[my_server_index].client_uri.c_str(), MARGO_CLIENT_MODE);
    auto total_servers = uris.size();
    thallium_endpoints.reserve(total_servers);
    for (std::vector<CharStruct>::size_type i = 0; i < total_servers; ++i) {
      thallium_endpoints.push_back(get_endpoint(uris[i]));
    }
  }

  /*std::promise<void> thallium_exit_signal;

    void runThalliumServer(std::future<void> futureObj){

    while(futureObj.wait_for(std::chrono::milliseconds(1)) ==
    std::future_status::timeout){} thallium_engine->wait_for_finalize();
    }*/

#endif

 public:
  void Stop() {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
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
  ~RPC() {
    HCL_LOG_TRACE();
    Stop();
  }

  RPC(bool _is_server, uint16_t _my_server_index, size_t _threads,
      std::vector<URI> _uris)
      : is_server(_is_server),
        my_server_index(_my_server_index),
        threads(_threads),
        uris(_uris) {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    /* if current rank is a server */
    if (is_server) {
      switch (HCL_CONF->RPC_IMPLEMENTATION) {
#ifdef HCL_COMMUNICATION_ENABLE_THALLIUM
        case THALLIUM_TCP: {
          engine_init_str = uris[my_server_index].server_uri;
          break;
        }
#endif
      }
    }
    run();
  }

  template <typename F>
  void bind(CharStruct str, F func);

  void run() {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    if (is_server) {
      switch (HCL_CONF->RPC_IMPLEMENTATION) {
#ifdef HCL_COMMUNICATION_ENABLE_THALLIUM
        case THALLIUM_TCP:
#endif
#if defined(HCL_COMMUNICATION_ENABLE_THALLIUM)
        {
          thallium_server = hcl::Singleton<tl::engine>::GetInstance(
              engine_init_str.c_str(), THALLIUM_SERVER_MODE, true, threads);
          HCL_LOG_INFO("Running server on URI %s\n", engine_init_str.c_str());
          break;
        }
#endif
      }
    }
    switch (HCL_CONF->RPC_IMPLEMENTATION) {
#ifdef HCL_COMMUNICATION_ENABLE_THALLIUM
      case THALLIUM_TCP: {
        init_engine_and_endpoints();
        HCL_LOG_INFO("Running client on URI %s\n",
                     uris[my_server_index].client_uri.c_str());
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
