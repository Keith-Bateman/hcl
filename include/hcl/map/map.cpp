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

#ifndef INCLUDE_HCL_MAP_MAP_CPP_
#define INCLUDE_HCL_MAP_MAP_CPP_

/**
 * Put the data into the local map.
 * @param key, the key for put
 * @param data, the value for put
 * @return bool, true if Put was successful else false.
 */
#include <string>
template <typename KeyType, typename MappedType, typename Compare,
          typename Allocator, typename SharedType>
bool map<KeyType, MappedType, Compare, Allocator, SharedType>::LocalPut(
    KeyType &key, MappedType &data) {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex>
      lock(*mutex);
  auto value = GetData<Allocator, MappedType, SharedType>(data);
  mymap->insert_or_assign(key, value);
  HCL_CPP_FUNCTION_UPDATE("access", "local");
  return true;
}

/**
 * Put the data into the map. Uses key to decide the server to hash it to,
 * @param key, the key for put
 * @param data, the value for put
 * @return bool, true if Put was successful else false.
 */
template <typename KeyType, typename MappedType, typename Compare,
          typename Allocator, typename SharedType>
bool map<KeyType, MappedType, Compare, Allocator, SharedType>::Put(
    KeyType &key, MappedType &data) {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  size_t key_hash = keyHash(key);
  uint16_t key_int = static_cast<uint16_t>(key_hash % num_servers);
  if (is_local(key_int)) {
    HCL_CPP_FUNCTION_UPDATE("access", "local");
    return LocalPut(key, data);
  } else {
    HCL_CPP_FUNCTION_UPDATE("access", "remote");
    HCL_CPP_FUNCTION_UPDATE("server", key_int);
    return RPC_CALL_WRAPPER("_Put", key_int, bool, key, data);
  }
}

/**
 * Get the data in the local map.
 * @param key, key to get
 * @return return a pair of bool and Value. If bool is true then
 * data was found and is present in value part else bool is set to false
 */
template <typename KeyType, typename MappedType, typename Compare,
          typename Allocator, typename SharedType>
std::pair<bool, MappedType> map<KeyType, MappedType, Compare, Allocator,
                                SharedType>::LocalGet(KeyType &key) {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  HCL_CPP_FUNCTION_UPDATE("access", "local");
  boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex>
      lock(*mutex);
  typename MyMap::iterator iterator = mymap->find(key);
  if (iterator != mymap->end()) {
    return std::pair<bool, MappedType>(true, iterator->second);
  } else {
    return std::pair<bool, MappedType>(false, MappedType());
  }
}

/**
 * Get the data in the map. Uses key to decide the server to hash it to,
 * @param key, key to get
 * @return return a pair of bool and Value. If bool is true then
 * data was found and is present in value part else bool is set to false
 */
template <typename KeyType, typename MappedType, typename Compare,
          typename Allocator, typename SharedType>
std::pair<bool, MappedType>
map<KeyType, MappedType, Compare, Allocator, SharedType>::Get(KeyType &key) {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  size_t key_hash = keyHash(key);
  uint16_t key_int = key_hash % num_servers;
  if (is_local(key_int)) {
    HCL_CPP_FUNCTION_UPDATE("access", "local");
    return LocalGet(key);
  } else {
    HCL_CPP_FUNCTION_UPDATE("access", "remote");
    HCL_CPP_FUNCTION_UPDATE("server", key_int);
    typedef std::pair<bool, MappedType> ret_type;
    return RPC_CALL_WRAPPER("_Get", key_int, ret_type, key);
  }
}

template <typename KeyType, typename MappedType, typename Compare,
          typename Allocator, typename SharedType>
std::pair<bool, MappedType> map<KeyType, MappedType, Compare, Allocator,
                                SharedType>::LocalErase(KeyType &key) {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex>
      lock(*mutex);
  size_t s = mymap->erase(key);
  HCL_CPP_FUNCTION_UPDATE("access", "local");
  return std::pair<bool, MappedType>(s > 0, MappedType());
}

template <typename KeyType, typename MappedType, typename Compare,
          typename Allocator, typename SharedType>
std::pair<bool, MappedType>
map<KeyType, MappedType, Compare, Allocator, SharedType>::Erase(KeyType &key) {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  size_t key_hash = keyHash(key);
  uint16_t key_int = key_hash % num_servers;
  if (is_local(key_int)) {
    HCL_CPP_FUNCTION_UPDATE("access", "local");
    return LocalErase(key);
  } else {
    HCL_CPP_FUNCTION_UPDATE("access", "remote");
    HCL_CPP_FUNCTION_UPDATE("server", key_int);
    typedef std::pair<bool, MappedType> ret_type;
    return RPC_CALL_WRAPPER("_Erase", key_int, ret_type, key);
  }
}

/**
 * Get the data into the map. Uses key to decide the server to hash it to,
 * @param key, key to get
 * @return return a pair of bool and Value. If bool is true then data was
 * found and is present in value part else bool is set to false
 */
template <typename KeyType, typename MappedType, typename Compare,
          typename Allocator, typename SharedType>
std::vector<std::pair<KeyType, MappedType>>
map<KeyType, MappedType, Compare, Allocator, SharedType>::Contains(
    KeyType &key_start, KeyType &key_end) {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  auto final_values = std::vector<std::pair<KeyType, MappedType>>();
  auto current_server = ContainsInServer(key_start, key_end);
  final_values.insert(final_values.end(), current_server.begin(),
                      current_server.end());
  for (int i = 0; i < num_servers; ++i) {
    if (i != my_server) {
      HCL_CPP_REGION(ContainsServer);
      HCL_CPP_REGION_UPDATE(ContainsServer, "access", "remote");
      HCL_CPP_REGION_UPDATE(ContainsServer, "server", i);
      typedef std::vector<std::pair<KeyType, MappedType>> ret_type;
      auto server =
          RPC_CALL_WRAPPER("_Contains", i, ret_type, key_start, key_end);
      final_values.insert(final_values.end(), server.begin(), server.end());
    }
  }
  return final_values;
}

template <typename KeyType, typename MappedType, typename Compare,
          typename Allocator, typename SharedType>
std::vector<std::pair<KeyType, MappedType>>
map<KeyType, MappedType, Compare, Allocator, SharedType>::GetAllData() {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  auto final_values = std::vector<std::pair<KeyType, MappedType>>();
  auto current_server = GetAllDataInServer();
  final_values.insert(final_values.end(), current_server.begin(),
                      current_server.end());
  for (int i = 0; i < num_servers; ++i) {
    if (i != my_server) {
      HCL_CPP_REGION(GetAllDataServer);
      HCL_CPP_REGION_UPDATE(GetAllDataServer, "access", "remote");
      HCL_CPP_REGION_UPDATE(GetAllDataServer, "server", i);
      typedef std::vector<std::pair<KeyType, MappedType>> ret_type;
      auto server = RPC_CALL_WRAPPER1("_GetAllData", i, ret_type);
      final_values.insert(final_values.end(), server.begin(), server.end());
    }
  }
  return final_values;
}

template <typename KeyType, typename MappedType, typename Compare,
          typename Allocator, typename SharedType>
std::vector<std::pair<KeyType, MappedType>>
map<KeyType, MappedType, Compare, Allocator, SharedType>::LocalContainsInServer(
    KeyType &key_start, KeyType &key_end) {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  HCL_CPP_FUNCTION_UPDATE("access", "local");
  auto final_values = std::vector<std::pair<KeyType, MappedType>>();
  {
    boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex>
        lock(*mutex);
    typename MyMap::iterator lower_bound;
    size_t size = mymap->size();
    if (size == 0) {
    } else if (size == 1) {
      lower_bound = mymap->begin();

      if (lower_bound->first > key_start)
        final_values.insert(final_values.end(),
                            std::pair<KeyType, MappedType>(
                                lower_bound->first, lower_bound->second));
    } else {
      lower_bound = mymap->lower_bound(key_start);
      if (lower_bound == mymap->end()) return final_values;
      if (lower_bound != mymap->begin()) {
        --lower_bound;
        if (key_start > lower_bound->first) lower_bound++;
      }
      while (lower_bound != mymap->end()) {
        if (lower_bound->first > key_end) break;
        final_values.insert(final_values.end(),
                            std::pair<KeyType, MappedType>(
                                lower_bound->first, lower_bound->second));
        lower_bound++;
      }
    }
  }
  return final_values;
}

template <typename KeyType, typename MappedType, typename Compare,
          typename Allocator, typename SharedType>
std::vector<std::pair<KeyType, MappedType>>
map<KeyType, MappedType, Compare, Allocator, SharedType>::ContainsInServer(
    KeyType &key_start, KeyType &key_end) {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  if (is_local()) {
    HCL_CPP_FUNCTION_UPDATE("access", "local");
    return LocalContainsInServer(key_start, key_end);
  } else {
    typedef std::vector<std::pair<KeyType, MappedType>> ret_type;
    auto my_server_i = my_server;
    HCL_CPP_FUNCTION_UPDATE("access", "remote");
    HCL_CPP_FUNCTION_UPDATE("server", my_server_i);
    return RPC_CALL_WRAPPER("_Contains", my_server_i, ret_type, key_start,
                            key_end);
  }
}

template <typename KeyType, typename MappedType, typename Compare,
          typename Allocator, typename SharedType>
std::vector<std::pair<KeyType, MappedType>>
map<KeyType, MappedType, Compare, Allocator,
    SharedType>::LocalGetAllDataInServer() {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  HCL_CPP_FUNCTION_UPDATE("access", "local");
  auto final_values = std::vector<std::pair<KeyType, MappedType>>();
  {
    boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex>
        lock(*mutex);
    typename MyMap::iterator lower_bound;
    lower_bound = mymap->begin();
    while (lower_bound != mymap->end()) {
      final_values.insert(final_values.end(),
                          std::pair<KeyType, MappedType>(lower_bound->first,
                                                         lower_bound->second));
      lower_bound++;
    }
  }
  return final_values;
}

template <typename KeyType, typename MappedType, typename Compare,
          typename Allocator, typename SharedType>
std::vector<std::pair<KeyType, MappedType>>
map<KeyType, MappedType, Compare, Allocator, SharedType>::GetAllDataInServer() {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  if (is_local()) {
    HCL_CPP_FUNCTION_UPDATE("access", "local");
    return LocalGetAllDataInServer();
  } else {
    typedef std::vector<std::pair<KeyType, MappedType>> ret_type;
    auto my_server_i = my_server;
    HCL_CPP_FUNCTION_UPDATE("access", "remote");
    HCL_CPP_FUNCTION_UPDATE("server", my_server_i);
    return RPC_CALL_WRAPPER1("_GetAllData", my_server_i, ret_type);
  }
}

#endif  // INCLUDE_HCL_MAP_MAP_CPP_
