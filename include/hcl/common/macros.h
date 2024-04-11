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

#ifndef INCLUDE_HCL_COMMON_MACROS_H_
#define INCLUDE_HCL_COMMON_MACROS_H_

#include <hcl/common/configuration_manager.h>
#include <hcl/common/singleton.h>

#define EXPAND_ARGS(...) __VA_ARGS__
#define HCL_CONF hcl::Singleton<hcl::ConfigurationManager>::GetInstance()

#define THALLIUM_DEFINE(name, args, args_t...)                         \
  void Thallium##name(const thallium::request &thallium_req, args_t) { \
    auto result = name args;                                           \
    thallium_req.respond(result);                                      \
  }

#define THALLIUM_DEFINE1(name)                                 \
  void Thallium##name(const thallium::request &thallium_req) { \
    thallium_req.respond(name());                              \
  }

#ifdef HCL_COMMUNICATION_ENABLE_THALLIUM
#define RPC_CALL_WRAPPER_THALLIUM_TCP() case THALLIUM_TCP:
#else
#define RPC_CALL_WRAPPER_THALLIUM_TCP()
#endif
#if defined(HCL_COMMUNICATION_ENABLE_THALLIUM)
#define RPC_CALL_WRAPPER_THALLIUM1(funcname, serverVar, ret)  \
  {                                                           \
    return rpc->call<ret>(serverVar, func_prefix + funcname); \
    break;                                                    \
  }
#define RPC_CALL_WRAPPER_THALLIUM(funcname, serverVar, ret, args...) \
  {                                                                  \
    return rpc->call<ret>(serverVar, func_prefix + funcname, args);  \
    break;                                                           \
  }
#else
#define RPC_CALL_WRAPPER_THALLIUM1(funcname, serverVar, ret)
#define RPC_CALL_WRAPPER_THALLIUM(funcname, serverVar, ret, args...)
#endif

#define RPC_CALL_WRAPPER1(funcname, serverVar, ret)        \
  [&]() -> ret {                                           \
    switch (HCL_CONF->RPC_IMPLEMENTATION) {                \
      RPC_CALL_WRAPPER_THALLIUM_TCP()                      \
      RPC_CALL_WRAPPER_THALLIUM1(funcname, serverVar, ret) \
    }                                                      \
  }();
#define RPC_CALL_WRAPPER(funcname, serverVar, ret, args...)     \
  [&]() -> ret {                                                \
    switch (HCL_CONF->RPC_IMPLEMENTATION) {                     \
      RPC_CALL_WRAPPER_THALLIUM_TCP()                           \
      RPC_CALL_WRAPPER_THALLIUM(funcname, serverVar, ret, args) \
    }                                                           \
  }();
#define RPC_CALL_WRAPPER1_CB(funcname, serverVar, ret)     \
  [&]() -> ret {                                           \
    switch (HCL_CONF->RPC_IMPLEMENTATION) {                \
      RPC_CALL_WRAPPER_THALLIUM_TCP()                      \
      RPC_CALL_WRAPPER_THALLIUM1(funcname, serverVar, ret) \
    }                                                      \
  }();

#define RPC_CALL_WRAPPER_CB(funcname, serverVar, ret, ...)              \
  [&]() -> ret {                                                        \
    switch (HCL_CONF->RPC_IMPLEMENTATION) {                             \
      RPC_CALL_WRAPPER_THALLIUM_TCP()                                   \
      RPC_CALL_WRAPPER_THALLIUM(funcname, serverVar, ret, __VA_ARGS__)  \
    }                                                                   \
  }();

#endif  // INCLUDE_HCL_COMMON_MACROS_H_
