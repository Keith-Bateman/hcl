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

#ifndef INCLUDE_HCL_CONCURRENT_SKIPLIST_CPP_
#define INCLUDE_HCL_CONCURRENT_SKIPLIST_CPP_
template <typename T, typename HashFcn, typename Comp, typename NodeAlloc,
          int MAX_HEIGHT>
bool concurrent_skiplist<T, HashFcn, Comp, NodeAlloc, MAX_HEIGHT>::Insert(
    T &key) {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  uint16_t key_int = static_cast<uint16_t>(serverLocation(key));
  HCL_CPP_FUNCTION_UPDATE("access", "remote");
  HCL_CPP_FUNCTION_UPDATE("server", key_int);

  return RPC_CALL_WRAPPER("_Insert", key_int, bool, key);
}

template <typename T, typename HashFcn, typename Comp, typename NodeAlloc,
          int MAX_HEIGHT>
bool concurrent_skiplist<T, HashFcn, Comp, NodeAlloc, MAX_HEIGHT>::Find(
    T &key) {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  HCL_CPP_FUNCTION_UPDATE("access", "remote");
  uint16_t key_int = static_cast<uint16_t>(serverLocation(key));
  HCL_CPP_FUNCTION_UPDATE("server", key_int);
  return RPC_CALL_WRAPPER("_Find", key_int, bool, key);
}

template <typename T, typename HashFcn, typename Comp, typename NodeAlloc,
          int MAX_HEIGHT>
bool concurrent_skiplist<T, HashFcn, Comp, NodeAlloc, MAX_HEIGHT>::Erase(
    T &key) {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  HCL_CPP_FUNCTION_UPDATE("access", "remote");
  uint16_t key_int = static_cast<uint16_t>(serverLocation(key));
  HCL_CPP_FUNCTION_UPDATE("server", key_int);
  return RPC_CALL_WRAPPER("_Erase", key_int, bool, key);
}

#endif
