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

#ifndef INCLUDE_HCL_COMMUNICATION_RPC_LIB_CPP_
#define INCLUDE_HCL_COMMUNICATION_RPC_LIB_CPP_
#include <hcl/hcl_config.hpp>
#include <stdexcept>
template <typename F>
void RPC::bind(CharStruct str, F func) {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  switch (HCL_CONF->RPC_IMPLEMENTATION) {
#ifdef HCL_COMMUNICATION_ENABLE_THALLIUM
    case THALLIUM_TCP:
#endif
#if defined(HCL_COMMUNICATION_ENABLE_THALLIUM)
    {
      thallium_server->define(str.string(), func);
      break;
    }
#endif
  }
}
template <typename Response, typename... Args>
Response RPC::callWithTimeout(uint16_t server_index, int timeout_ms,
                              CharStruct const &func_name, Args... args) {
  HCL_LOG_TRACE_FORMAT("(%d, %s)", server_index, func_name.c_str());
  switch (HCL_CONF->RPC_IMPLEMENTATION) {
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
  }
}
template <typename Response, typename... Args>
Response RPC::call(uint16_t server_index, CharStruct const &func_name,
                   Args... args) {
  HCL_LOG_TRACE_FORMAT("(%d, %s)", server_index, func_name.c_str());
  switch (HCL_CONF->RPC_IMPLEMENTATION) {
#ifdef HCL_COMMUNICATION_ENABLE_THALLIUM
    case THALLIUM_TCP: {
      tl::remote_procedure remote_procedure =
          thallium_client->define(func_name.c_str());
      return remote_procedure.on(thallium_endpoints[server_index])(
          std::forward<Args>(args)...);
      break;
    }
#endif
  }
  throw std::logic_error("Function not implemented error.");
}

template <typename Response, typename... Args>
Response RPC::call(CharStruct &server, uint16_t &port,
                   CharStruct const &func_name, Args... args) {
  HCL_LOG_TRACE_FORMAT("(%d, %d, %s)", server, port, func_name.c_str());
  switch (HCL_CONF->RPC_IMPLEMENTATION) {
#ifdef HCL_COMMUNICATION_ENABLE_THALLIUM
    case THALLIUM_TCP: {
      tl::remote_procedure remote_procedure =
          thallium_client->define(func_name.c_str());
      auto new_uri = URI(0, uris[0].user_uri, server, port);
      auto end_point = get_endpoint(new_uri);
      return remote_procedure.on(end_point)(std::forward<Args>(args)...);
      break;
    }
#endif
  }
  throw std::logic_error("Function not implemented error.");
}

template <typename Response, typename... Args>
std::future<Response> RPC::async_call(uint16_t server_index,
                                      CharStruct const &func_name,
                                      Args... args) {
  HCL_LOG_TRACE_FORMAT("(%d, %s)", server_index, func_name.c_str());

  switch (HCL_CONF->RPC_IMPLEMENTATION) {
#ifdef HCL_COMMUNICATION_ENABLE_THALLIUM
    case THALLIUM_TCP: {
      // TODO:NotImplemented error
      break;
    }
#endif
  }
}

template <typename Response, typename... Args>
std::future<Response> RPC::async_call(CharStruct &server, uint16_t &port,
                                      CharStruct const &func_name,
                                      Args... args) {
  HCL_LOG_TRACE_FORMAT("(%d, %d, %s)", server, port, func_name.c_str());

  switch (HCL_CONF->RPC_IMPLEMENTATION) {
#ifdef HCL_COMMUNICATION_ENABLE_THALLIUM
    case THALLIUM_TCP: {
      // TODO:NotImplemented error
      break;
    }
#endif
  }
}

#endif  // INCLUDE_HCL_COMMUNICATION_RPC_LIB_CPP_
