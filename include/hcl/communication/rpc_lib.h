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
#include <hcl/common/data_structures.h>
#include <hcl/common/debug.h>
#include <hcl/common/macros.h>
#include <hcl/common/typedefs.h>

/** RPC Lib Headers**/
#ifdef HCL_COMMUNICATION_ENABLE_RPCLIB
#include <rpc/client.h>
#include <rpc/rpc_error.h>
#include <rpc/server.h>
#endif
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

namespace hcl {
class RPC {
 private:
  uint16_t server_port;
  std::string name;
#ifdef HCL_COMMUNICATION_ENABLE_RPCLIB
  std::shared_ptr<rpc::server> rpclib_server;
  // We can't use a std::vector<rpc::client> for these since rpc::client is
  // neither copy nor move constructible. See
  // https://github.com/rpclib/rpclib/issues/128
  std::vector<std::shared_ptr<rpc::client>> rpclib_clients;
#endif
#if defined(HCL_COMMUNICATION_ENABLE_THALLIUM)
  std::shared_ptr<tl::engine> thallium_server;
  std::shared_ptr<tl::engine> thallium_client;
  CharStruct engine_init_str;
  std::vector<tl::endpoint> thallium_endpoints;
#endif
  std::vector<CharStruct> server_list;

 public:
  void Stop();
  ~RPC();

  RPC();

  void run(size_t workers = 0);

  template <typename F>
  void bind(CharStruct str, F func) {
    switch (HCL_CONF->RPC_IMPLEMENTATION) {
#ifdef HCL_COMMUNICATION_ENABLE_RPCLIB
      case RPCLIB: {
        rpclib_server->bind(str.c_str(), func);
        break;
      }
#endif
#ifdef HCL_COMMUNICATION_ENABLE_THALLIUM
      case THALLIUM_TCP:
#endif
#ifdef HCL_ENABLE_THALLIUM_ROCE
      case THALLIUM_ROCE:
#endif
#if defined(HCL_COMMUNICATION_ENABLE_THALLIUM)
      {
        thallium_server->define(str.string(), func);
        break;
      }
#endif
    }
  }
  /**
   * Response should be RPCLIB_MSGPACK::object_handle for rpclib and
   * tl::packed_response for thallium/mercury
   */
  template <typename Response, typename... Args>
  Response callWithTimeout(uint16_t server_index, int timeout_ms,
                           CharStruct const &func_name, Args... args) {
    AutoTrace trace = AutoTrace("RPC::call", server_index, func_name);
    int16_t port = server_port + server_index;

    switch (HCL_CONF->RPC_IMPLEMENTATION) {
#ifdef HCL_COMMUNICATION_ENABLE_RPCLIB
      case RPCLIB: {
        auto *client = rpclib_clients[server_index].get();
        if (client->get_connection_state() !=
            rpc::client::connection_state::connected) {
          rpclib_clients[server_index] = std::make_unique<rpc::client>(
              server_list[server_index].c_str(), server_port + server_index);
          client = rpclib_clients[server_index].get();
        }
        client->set_timeout(timeout_ms);
        Response response =
            client->call(func_name.c_str(), std::forward<Args>(args)...);
        client->clear_timeout();
        return response;
        break;
      }
#endif
#ifdef HCL_COMMUNICATION_ENABLE_THALLIUM
      case THALLIUM_TCP: {
        tl::remote_procedure remote_procedure =
            thallium_client->define(func_name.c_str());
        // Setup args for RDMA bulk transfer
        // std::vector<std::pair<void*,std::size_t>> segments(num_args);

        return remote_procedure.on(thallium_endpoints[server_index])(
            std::forward<Args>(args)...);
        break;
      }
#endif
#ifdef HCL_ENABLE_THALLIUM_ROCE
      case THALLIUM_ROCE: {
        tl::remote_procedure remote_procedure =
            thallium_client->define(func_name.c_str());
        return remote_procedure.on(thallium_endpoints[server_index])(
            std::forward<Args>(args)...);
        break;
      }
#endif
    }
  }
  template <typename Response, typename... Args>
  Response call(uint16_t server_index, CharStruct const &func_name,
                Args... args) {
    AutoTrace trace = AutoTrace("RPC::call", server_index, func_name);
    int16_t port = server_port + server_index;

    switch (HCL_CONF->RPC_IMPLEMENTATION) {
#ifdef HCL_COMMUNICATION_ENABLE_RPCLIB
      case RPCLIB: {
        auto *client = rpclib_clients[server_index].get();
        if (client->get_connection_state() !=
            rpc::client::connection_state::connected) {
          rpclib_clients[server_index] = std::make_unique<rpc::client>(
              server_list[server_index].c_str(), server_port + server_index);
          client = rpclib_clients[server_index].get();
        }
        /*client.set_timeout(5000);*/
        return client->call(func_name.c_str(), std::forward<Args>(args)...);
        break;
      }
#endif
#ifdef HCL_COMMUNICATION_ENABLE_THALLIUM
      case THALLIUM_TCP: {
        tl::remote_procedure remote_procedure =
            thallium_client->define(func_name.c_str());
        return remote_procedure.on(thallium_endpoints[server_index])(
            std::forward<Args>(args)...);
        break;
      }
#endif
#ifdef HCL_ENABLE_THALLIUM_ROCE
      case THALLIUM_ROCE: {
        tl::remote_procedure remote_procedure =
            thallium_client->define(func_name.c_str());
        return remote_procedure.on(thallium_endpoints[server_index])(
            std::forward<Args>(args)...);
        break;
      }
#endif
    }
  }

  template <typename Response, typename... Args>
  Response call(CharStruct &server, uint16_t &port, CharStruct const &func_name,
                Args... args) {
    AutoTrace trace = AutoTrace("RPC::call", server, port, func_name);
    switch (HCL_CONF->RPC_IMPLEMENTATION) {
#ifdef HCL_COMMUNICATION_ENABLE_RPCLIB
      case RPCLIB: {
        auto client = rpc::client(server.c_str(), port);
        /*client.set_timeout(5000);*/
        return client.call(func_name.c_str(), std::forward<Args>(args)...);
        break;
      }
#endif
#ifdef HCL_COMMUNICATION_ENABLE_THALLIUM
      case THALLIUM_TCP: {
        tl::remote_procedure remote_procedure =
            thallium_client->define(func_name.c_str());
        auto end_point = get_endpoint(HCL_CONF->TCP_CONF, server, port);
        return remote_procedure.on(end_point)(std::forward<Args>(args)...);
        break;
      }
#endif
#ifdef HCL_ENABLE_THALLIUM_ROCE
      case THALLIUM_ROCE: {
        tl::remote_procedure remote_procedure =
            thallium_client->define(func_name.c_str());
        auto end_point = get_endpoint(HCL_CONF->TCP_CONF, server, port);
        return remote_procedure.on(end_point)(std::forward<Args>(args)...);
        break;
      }
#endif
    }
  }

  template <typename Response, typename... Args>
  std::future<Response> async_call(uint16_t server_index,
                                   CharStruct const &func_name, Args... args) {
    AutoTrace trace = AutoTrace("RPC::call", server_index, func_name);
    int16_t port = server_port + server_index;

    switch (HCL_CONF->RPC_IMPLEMENTATION) {
#ifdef HCL_COMMUNICATION_ENABLE_RPCLIB
      case RPCLIB: {
        auto *client = rpclib_clients[server_index].get();
        if (client->get_connection_state() !=
            rpc::client::connection_state::connected) {
          rpclib_clients[server_index] = std::make_unique<rpc::client>(
              server_list[server_index].c_str(), server_port + server_index);
          client = rpclib_clients[server_index].get();
        }
        // client.set_timeout(5000);
        return client->async_call(func_name.c_str(),
                                  std::forward<Args>(args)...);
        break;
      }
#endif
#ifdef HCL_COMMUNICATION_ENABLE_THALLIUM
      case THALLIUM_TCP: {
        // TODO:NotImplemented error
        break;
      }
#endif
#ifdef HCL_ENABLE_THALLIUM_ROCE
      case THALLIUM_ROCE: {
        // TODO:NotImplemented error
        break;
      }
#endif
    }
  }

  template <typename Response, typename... Args>
  std::future<Response> async_call(CharStruct &server, uint16_t &port,
                                   CharStruct const &func_name, Args... args) {
    AutoTrace trace = AutoTrace("RPC::async_call", server, port, func_name);

    switch (HCL_CONF->RPC_IMPLEMENTATION) {
#ifdef HCL_COMMUNICATION_ENABLE_RPCLIB
      case RPCLIB: {
        auto client = rpc::client(server.c_str(), port);
        // client.set_timeout(5000);
        return client.async_call(func_name.c_str(),
                                 std::forward<Args>(args)...);
        break;
      }
#endif
#ifdef HCL_COMMUNICATION_ENABLE_THALLIUM
      case THALLIUM_TCP: {
        // TODO:NotImplemented error
        break;
      }
#endif
#ifdef HCL_ENABLE_THALLIUM_ROCE
      case THALLIUM_ROCE: {
        // TODO:NotImplemented error
        break;
      }
#endif
    }
  }

#if defined(HCL_COMMUNICATION_ENABLE_THALLIUM)
  tl::endpoint get_endpoint(CharStruct protocol, CharStruct server_name,
                            uint16_t server_port);
  void init_engine_and_endpoints(CharStruct protocol);

  /*std::promise<void> thallium_exit_signal;

    void RPC::runThalliumServer(std::future<void> futureObj);*/

#endif
#if defined(HCL_COMMUNICATION_ENABLE_THALLIUM) && \
    defined(HCL_COMMUNICATION_PROTOCOL_ENABLE_VERBS)
  template <typename MappedType>
  MappedType prep_rdma_server(tl::endpoint endpoint, tl::bulk &bulk_handle);

  template <typename MappedType>
  tl::bulk prep_rdma_client(MappedType &data);
#endif
};
}  // namespace hcl
#endif  // INCLUDE_HCL_COMMUNICATION_RPC_LIB_H_
