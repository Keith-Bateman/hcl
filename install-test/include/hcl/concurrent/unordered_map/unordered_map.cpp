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

#ifndef INCLUDE_HCL_CONCURRENT_UNORDERED_MAP_CPP_
#define INCLUDE_HCL_CONCURRENT_UNORDERED_MAP_CPP_

template <typename KeyT, typename ValueT, typename HashFcn, typename EqualFcn>
bool concurrent_unordered_map<KeyT, ValueT, HashFcn, EqualFcn>::Insert(
    KeyT &key, ValueT &data) {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  uint16_t key_int = static_cast<uint16_t>(serverLocation(key));
  HCL_CPP_FUNCTION_UPDATE("access", "remote");
  HCL_CPP_FUNCTION_UPDATE("server", key_int);
  return RPC_CALL_WRAPPER("_Insert", key_int, bool, key, data);
}

template <typename KeyT, typename ValueT, typename HashFcn, typename EqualFcn>
bool concurrent_unordered_map<KeyT, ValueT, HashFcn, EqualFcn>::Find(
    KeyT &key) {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  uint16_t key_int = static_cast<uint16_t>(serverLocation(key));
  HCL_CPP_FUNCTION_UPDATE("access", "remote");
  HCL_CPP_FUNCTION_UPDATE("server", key_int);
  return RPC_CALL_WRAPPER("_Find", key_int, bool, key);
}

template <typename KeyT, typename ValueT, typename HashFcn, typename EqualFcn>
bool concurrent_unordered_map<KeyT, ValueT, HashFcn, EqualFcn>::Erase(
    KeyT &key) {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  uint16_t key_int = static_cast<uint16_t>(serverLocation(key));
  HCL_CPP_FUNCTION_UPDATE("access", "remote");
  HCL_CPP_FUNCTION_UPDATE("server", key_int);
  return RPC_CALL_WRAPPER("_Erase", key_int, bool, key);
}

template <typename KeyT, typename ValueT, typename HashFcn, typename EqualFcn>
ValueT concurrent_unordered_map<KeyT, ValueT, HashFcn, EqualFcn>::Get(
    KeyT &key) {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  uint16_t key_int = static_cast<uint16_t>(serverLocation(key));
  HCL_CPP_FUNCTION_UPDATE("access", "remote");
  HCL_CPP_FUNCTION_UPDATE("server", key_int);
  return RPC_CALL_WRAPPER("_Get", key_int, ValueT, key);
}

template <typename KeyT, typename ValueT, typename HashFcn, typename EqualFcn>
bool concurrent_unordered_map<KeyT, ValueT, HashFcn, EqualFcn>::Update(
    KeyT &key, ValueT &data) {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  uint16_t key_int = static_cast<uint16_t>(serverLocation(key));
  HCL_CPP_FUNCTION_UPDATE("access", "remote");
  HCL_CPP_FUNCTION_UPDATE("server", key_int);
  return RPC_CALL_WRAPPER("_Update", key_int, bool, key, data);
}

#endif
