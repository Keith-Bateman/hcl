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

#ifndef INCLUDE_HCL_UNORDERED_MAP_UNORDERED_MAP_CPP_
#define INCLUDE_HCL_UNORDERED_MAP_UNORDERED_MAP_CPP_

#include <boost/interprocess/file_mapping.hpp>

/* Constructor to deallocate the shared memory*/
template <typename KeyType, typename MappedType, typename Hash,
          typename Allocator, typename SharedType>
unordered_map<KeyType, MappedType, Hash, Allocator,
              SharedType>::~unordered_map() {
  this->container::~container();
}

template <typename KeyType, typename MappedType, typename Hash,
          typename Allocator, typename SharedType>
unordered_map<KeyType, MappedType, Hash, Allocator, SharedType>::unordered_map(
    CharStruct name_, uint16_t port)
    : container(name_, port), myHashMap(), size_occupied(0) {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  if (is_server) {
    construct_shared_memory();
    bind_functions();
  } else if (!is_server && server_on_node) {
    open_shared_memory();
  }
}

/**
 * Put the data into the local unordered map.
 * @param key, the key for put
 * @param data, the value for put
 * @return bool, true if Put was successful else false.
 */
template <typename KeyType, typename MappedType, typename Hash,
          typename Allocator, typename SharedType>
bool unordered_map<KeyType, MappedType, Hash, Allocator, SharedType>::LocalPut(
    KeyType &key, MappedType &data) {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  if (is_server && !server_on_node) {
    boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex>
        lock(*mutex);
    auto value = GetData<Allocator, MappedType, SharedType>(data);
    auto iter = myHashMap->insert_or_assign(key, value);
    if (iter.second)
      size_occupied += CalculateSize<KeyType>().GetSize(key) +
                       CalculateSize<MappedType>().GetSize(data);
  } else {
    auto value = GetData<Allocator, MappedType, SharedType>(data);
    auto iter = myHashMap->insert_or_assign(key, value);
    if (iter.second)
      size_occupied += CalculateSize<KeyType>().GetSize(key) +
                       CalculateSize<MappedType>().GetSize(data);
  }
  return true;
}
/**
 * Put the data into the unordered map. Uses key to decide the server to hash it
 * to,
 * @param key, the key for put
 * @param data, the value for put
 * @return bool, true if Put was successful else false.
 */
template <typename KeyType, typename MappedType, typename Hash,
          typename Allocator, typename SharedType>
bool unordered_map<KeyType, MappedType, Hash, Allocator, SharedType>::Put(
    KeyType key, MappedType data) {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  uint16_t key_int = (uint16_t)keyHash(key) % num_servers;
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
 * Get the data in the local unordered map.
 * @param key, key to get
 * @return return a pair of bool and Value. If bool is true then data was
 * found and is present in value part else bool is set to false
 */
template <typename KeyType, typename MappedType, typename Hash,
          typename Allocator, typename SharedType>
std::pair<bool, MappedType> unordered_map<KeyType, MappedType, Hash, Allocator,
                                          SharedType>::LocalGet(KeyType &key) {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  HCL_CPP_FUNCTION_UPDATE("access", "local");
  if (is_server && !server_on_node) {
    boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex>
        lock(*mutex);
    typename MyHashMap::iterator iterator = myHashMap->find(key);
    if (iterator != myHashMap->end()) {
      return std::pair<bool, MappedType>(true, iterator->second);
    } else {
      return std::pair<bool, MappedType>(false, MappedType());
    }
  } else {
    typename MyHashMap::iterator iterator = myHashMap->find(key);
    if (iterator != myHashMap->end()) {
      return std::pair<bool, MappedType>(true, iterator->second);
    } else {
      return std::pair<bool, MappedType>(false, MappedType());
    }
  }
}

/**
 * Get the data in the unordered map. Uses key to decide the server to hash it
 * to,
 * @param key, key to get
 * @return return a pair of bool and Value. If bool is true then data was
 * found and is present in value part else bool is set to false
 */
template <typename KeyType, typename MappedType, typename Hash,
          typename Allocator, typename SharedType>
std::pair<bool, MappedType> unordered_map<KeyType, MappedType, Hash, Allocator,
                                          SharedType>::Get(KeyType &key) {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  size_t key_hash = keyHash(key);
  uint16_t key_int = static_cast<uint16_t>(key_hash % num_servers);
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

template <typename KeyType, typename MappedType, typename Hash,
          typename Allocator, typename SharedType>
std::pair<bool, MappedType>
unordered_map<KeyType, MappedType, Hash, Allocator, SharedType>::LocalErase(
    KeyType &key) {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  HCL_CPP_FUNCTION_UPDATE("access", "local");
  if (is_server && !server_on_node) {
    boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex>
        lock(*mutex);
    typename MyHashMap::iterator iterator = myHashMap->find(key);
    if (iterator != myHashMap->end()) {
      size_occupied -= CalculateSize<KeyType>().GetSize(key) +
                       CalculateSize<MappedType>().GetSize(iterator->second);
      myHashMap->erase(iterator);
      return std::pair<bool, MappedType>(true, MappedType());
    } else
      return std::pair<bool, MappedType>(false, MappedType());
  } else {
    typename MyHashMap::iterator iterator = myHashMap->find(key);
    if (iterator != myHashMap->end()) {
      size_occupied -= CalculateSize<KeyType>().GetSize(key) +
                       CalculateSize<MappedType>().GetSize(iterator->second);
      myHashMap->erase(iterator);
      return std::pair<bool, MappedType>(true, MappedType());
    } else
      return std::pair<bool, MappedType>(false, MappedType());
  }
}

template <typename KeyType, typename MappedType, typename Hash,
          typename Allocator, typename SharedType>
std::pair<bool, MappedType> unordered_map<KeyType, MappedType, Hash, Allocator,
                                          SharedType>::Erase(KeyType &key) {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  size_t key_hash = keyHash(key);
  uint16_t key_int = static_cast<uint16_t>(key_hash % num_servers);
  if (is_local(key_int)) {
    HCL_CPP_FUNCTION_UPDATE("access", "local");
    return LocalErase(key);
  } else {
    HCL_CPP_FUNCTION_UPDATE("access", "remote");
    HCL_CPP_FUNCTION_UPDATE("server", key_int);
    typedef std::pair<bool, MappedType> ret_type;
    return RPC_CALL_WRAPPER("_Erase", key_int, ret_type, key);
    // return rpc->call(key_int, func_prefix+"_Erase",
    //                  key).template as<std::pair<bool, MappedType>>();
  }
}

template <typename KeyType, typename MappedType, typename Hash,
          typename Allocator, typename SharedType>
std::vector<std::pair<KeyType, MappedType>>
unordered_map<KeyType, MappedType, Hash, Allocator, SharedType>::GetAllData() {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  std::vector<std::pair<KeyType, MappedType>> final_values =
      std::vector<std::pair<KeyType, MappedType>>();
  auto current_server = GetAllDataInServer();
  final_values.insert(final_values.end(), current_server.begin(),
                      current_server.end());
  for (int i = 0; i < num_servers; ++i) {
    if (i != my_server) {
      HCL_CPP_REGION(GetAllDataServer)
      HCL_CPP_REGION_UPDATE(GetAllDataServer, "access", "remote");
      HCL_CPP_REGION_UPDATE(GetAllDataServer, "server", i);
      typedef std::vector<std::pair<KeyType, MappedType>> ret_type;
      auto server = RPC_CALL_WRAPPER1("_GetAllData", i, ret_type);
      final_values.insert(final_values.end(), server.begin(), server.end());
    }
  }
  return final_values;
}

template <typename KeyType, typename MappedType, typename Hash,
          typename Allocator, typename SharedType>
std::vector<std::pair<KeyType, MappedType>>
unordered_map<KeyType, MappedType, Hash, Allocator,
              SharedType>::LocalGetAllDataInServer() {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  HCL_CPP_FUNCTION_UPDATE("access", "local");
  std::vector<std::pair<KeyType, MappedType>> final_values =
      std::vector<std::pair<KeyType, MappedType>>();
  {
    boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex>
        lock(*mutex);
    typename MyHashMap::iterator lower_bound;
    if (myHashMap->size() > 0) {
      lower_bound = myHashMap->begin();
      while (lower_bound != myHashMap->end()) {
        final_values.push_back(std::pair<KeyType, MappedType>(
            lower_bound->first, lower_bound->second));
        lower_bound++;
      }
    }
  }
  return final_values;
}

template <typename KeyType, typename MappedType, typename Hash,
          typename Allocator, typename SharedType>
std::vector<std::pair<KeyType, MappedType>> unordered_map<
    KeyType, MappedType, Hash, Allocator, SharedType>::GetAllDataInServer() {
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

template <typename KeyType, typename MappedType, typename Hash,
          typename Allocator, typename SharedType>
void unordered_map<KeyType, MappedType, Hash, Allocator,
                   SharedType>::open_shared_memory() {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  HCL_CPP_FUNCTION_UPDATE("access", "local");
  std::pair<MyHashMap *, boost::interprocess::managed_mapped_file::size_type>
      res;
  res = segment.find<MyHashMap>(name.c_str());
  myHashMap = res.first;
}

template <typename KeyType, typename MappedType, typename Hash,
          typename Allocator, typename SharedType>
void unordered_map<KeyType, MappedType, Hash, Allocator,
                   SharedType>::bind_functions() {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  HCL_CPP_FUNCTION_UPDATE("access", "local");
  switch (HCL_CONF->RPC_IMPLEMENTATION) {
#ifdef HCL_COMMUNICATION_ENABLE_THALLIUM
    case THALLIUM_TCP:
#endif
#if defined(HCL_COMMUNICATION_ENABLE_THALLIUM)
    {

      std::function<void(const tl::request &, KeyType &, MappedType &)> putFunc(
          std::bind(&unordered_map<KeyType, MappedType, Hash, Allocator,
                                   SharedType>::ThalliumLocalPut,
                    this, std::placeholders::_1, std::placeholders::_2,
                    std::placeholders::_3));
      // std::function<void(const tl::request &, tl::bulk &, KeyType &)>
      // putFunc(
      //     std::bind(&unordered_map<KeyType, MappedType, Hash, Allocator,
      //     SharedType>::ThalliumLocalPut, this,
      //               std::placeholders::_1, std::placeholders::_2,
      //               std::placeholders::_3));
      std::function<void(const tl::request &, KeyType &)> getFunc(
          std::bind(&unordered_map<KeyType, MappedType, Hash, Allocator,
                                   SharedType>::ThalliumLocalGet,
                    this, std::placeholders::_1, std::placeholders::_2));
      std::function<void(const tl::request &, KeyType &)> eraseFunc(
          std::bind(&unordered_map<KeyType, MappedType, Hash, Allocator,
                                   SharedType>::ThalliumLocalErase,
                    this, std::placeholders::_1, std::placeholders::_2));
      std::function<void(const tl::request &)> getAllDataInServerFunc(
          std::bind(&unordered_map<KeyType, MappedType, Hash, Allocator,
                                   SharedType>::ThalliumLocalGetAllDataInServer,
                    this, std::placeholders::_1));

      rpc->bind(func_prefix + "_Put", putFunc);
      rpc->bind(func_prefix + "_Get", getFunc);
      rpc->bind(func_prefix + "_Erase", eraseFunc);
      rpc->bind(func_prefix + "_GetAllData", getAllDataInServerFunc);
      break;
    }
#endif
  }
}

#endif  // INCLUDE_HCL_UNORDERED_MAP_UNORDERED_MAP_CPP_
