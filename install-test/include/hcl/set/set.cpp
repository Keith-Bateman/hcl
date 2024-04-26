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

#ifndef INCLUDE_HCL_SET_SET_CPP_
#define INCLUDE_HCL_SET_SET_CPP_

/* Constructor to deallocate the shared memory*/
#include <cstdint>
template <typename KeyType, typename Hash, typename Compare, typename Allocator,
          typename SharedType>
set<KeyType, Hash, Compare, Allocator, SharedType>::~set() {
  this->container::~container();
}

template <typename KeyType, typename Hash, typename Compare, typename Allocator,
          typename SharedType>
set<KeyType, Hash, Compare, Allocator, SharedType>::set(
    CharStruct name_, uint16_t port, uint16_t _num_servers,
    uint16_t _my_server_idx, really_long _memory_allocated, bool _is_server,
    bool _is_server_on_node, CharStruct _backed_file_dir)
    : container(name_, port, _num_servers, _my_server_idx, _memory_allocated,
                _is_server, _is_server_on_node, _backed_file_dir),
      myset() {
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
 * Put the data into the local set.
 * @param key, the key for put
 * @param data, the value for put
 * @return bool, true if Put was successful else false.
 */
template <typename KeyType, typename Hash, typename Compare, typename Allocator,
          typename SharedType>
bool set<KeyType, Hash, Compare, Allocator, SharedType>::LocalPut(
    KeyType &key) {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  HCL_CPP_FUNCTION_UPDATE("access", "local");
  boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex>
      lock(*mutex);
  auto value = GetData<Allocator, KeyType, SharedType>(key);
  myset->insert(value);

  return true;
}

/**
 * Put the data into the set. Uses key to decide the server to hash it to,
 * @param key, the key for put
 * @param data, the value for put
 * @return bool, true if Put was successful else false.
 */
template <typename KeyType, typename Hash, typename Compare, typename Allocator,
          typename SharedType>
bool set<KeyType, Hash, Compare, Allocator, SharedType>::Put(KeyType &key) {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  size_t key_hash = keyHash(key);
  uint16_t key_int = static_cast<uint16_t>(key_hash % num_servers);
  if (is_local(key_int)) {
    HCL_CPP_FUNCTION_UPDATE("access", "local");
    return LocalPut(key);
  } else {
    HCL_CPP_FUNCTION_UPDATE("access", "remote");
    HCL_CPP_FUNCTION_UPDATE("access", key_int);
    return RPC_CALL_WRAPPER("_Put", key_int, bool, key);
  }
}

/**
 * Get the data in the local set.
 * @param key, key to get
 * @return return a pair of bool and Value. If bool is true then
 * data was found and is present in value part else bool is set to false
 */
template <typename KeyType, typename Hash, typename Compare, typename Allocator,
          typename SharedType>
bool set<KeyType, Hash, Compare, Allocator, SharedType>::LocalGet(
    KeyType &key) {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  HCL_CPP_FUNCTION_UPDATE("access", "local");
  boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex>
      lock(*mutex);
  typename MySet::iterator iterator = myset->find(key);
  if (iterator != myset->end()) {
    return true;
  } else {
    return false;
  }
}

/**
 * Get the data in the set. Uses key to decide the server to hash it to,
 * @param key, key to get
 * @return return a pair of bool and Value. If bool is true then
 * data was found and is present in value part else bool is set to false
 */
template <typename KeyType, typename Hash, typename Compare, typename Allocator,
          typename SharedType>
bool set<KeyType, Hash, Compare, Allocator, SharedType>::Get(KeyType &key) {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  size_t key_hash = keyHash(key);
  uint16_t key_int = key_hash % num_servers;
  if (is_local(key_int)) {
    HCL_CPP_FUNCTION_UPDATE("access", "local");
    return LocalGet(key);
  } else {
    HCL_CPP_FUNCTION_UPDATE("access", "remote");
    HCL_CPP_FUNCTION_UPDATE("access", key_int);
    typedef bool ret_type;
    return RPC_CALL_WRAPPER("_Get", key_int, ret_type, key);
  }
}

template <typename KeyType, typename Hash, typename Compare, typename Allocator,
          typename SharedType>
bool set<KeyType, Hash, Compare, Allocator, SharedType>::LocalErase(
    KeyType &key) {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  HCL_CPP_FUNCTION_UPDATE("access", "local");
  boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex>
      lock(*mutex);
  size_t s = myset->erase(key);

  return s > 0;
}

template <typename KeyType, typename Hash, typename Compare, typename Allocator,
          typename SharedType>
bool set<KeyType, Hash, Compare, Allocator, SharedType>::Erase(KeyType &key) {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  size_t key_hash = keyHash(key);
  uint16_t key_int = key_hash % num_servers;
  if (is_local(key_int)) {
    HCL_CPP_FUNCTION_UPDATE("access", "local");
    return LocalErase(key);
  } else {
    HCL_CPP_FUNCTION_UPDATE("access", "remote");
    HCL_CPP_FUNCTION_UPDATE("access", key_int);
    typedef bool ret_type;
    return RPC_CALL_WRAPPER("_Erase", key_int, ret_type, key);
  }
}

/**
 * Get the data into the set. Uses key to decide the server to hash it to,
 * @param key, key to get
 * @return return a pair of bool and Value. If bool is true then data was
 * found and is present in value part else bool is set to false
 */
template <typename KeyType, typename Hash, typename Compare, typename Allocator,
          typename SharedType>
std::vector<KeyType>
set<KeyType, Hash, Compare, Allocator, SharedType>::Contains(KeyType &key_start,
                                                             KeyType &key_end) {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  std::vector<KeyType> final_values = std::vector<KeyType>();
  auto current_server = ContainsInServer(key_start, key_end);
  final_values.insert(final_values.end(), current_server.begin(),
                      current_server.end());
  for (int i = 0; i < num_servers; ++i) {
    if (i != my_server_idx) {
      HCL_CPP_REGION(ContainsInServerServer)
      HCL_CPP_REGION_UPDATE(ContainsInServerServer, "access", "remote");
      HCL_CPP_REGION_UPDATE(ContainsInServerServer, "access", i);
      typedef std::vector<KeyType> ret_type;
      auto server =
          RPC_CALL_WRAPPER("_Contains", i, ret_type, key_start, key_end);
      final_values.insert(final_values.end(), server.begin(), server.end());
    }
  }
  return final_values;
}

template <typename KeyType, typename Hash, typename Compare, typename Allocator,
          typename SharedType>
std::vector<KeyType>
set<KeyType, Hash, Compare, Allocator, SharedType>::GetAllData() {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  std::vector<KeyType> final_values = std::vector<KeyType>();
  auto current_server = GetAllDataInServer();
  final_values.insert(final_values.end(), current_server.begin(),
                      current_server.end());
  for (int i = 0; i < num_servers; ++i) {
    if (i != my_server_idx) {
      HCL_CPP_REGION(GetAllDataServer)
      HCL_CPP_REGION_UPDATE(GetAllDataServer, "access", "remote");
      HCL_CPP_REGION_UPDATE(GetAllDataServer, "access", i);
      typedef std::vector<KeyType> ret_type;
      auto server = RPC_CALL_WRAPPER1("_GetAllData", i, ret_type);
      final_values.insert(final_values.end(), server.begin(), server.end());
    }
  }
  return final_values;
}

template <typename KeyType, typename Hash, typename Compare, typename Allocator,
          typename SharedType>
std::vector<KeyType> set<KeyType, Hash, Compare, Allocator,
                         SharedType>::LocalContainsInServer(KeyType &key_start,
                                                            KeyType &key_end) {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  HCL_CPP_FUNCTION_UPDATE("access", "local");
  std::vector<KeyType> final_values = std::vector<KeyType>();
  {
    boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex>
        lock(*mutex);
    typename MySet::iterator lower_bound;
    size_t size = myset->size();
    if (size == 0) {
    } else if (size == 1) {
      lower_bound = myset->begin();
      if (*lower_bound >= key_start)
        final_values.insert(final_values.end(), *lower_bound);
    } else {
      lower_bound = myset->lower_bound(key_start);
      /*KeyType k=*lower_bound;*/
      if (lower_bound == myset->end()) return final_values;
      if (lower_bound != myset->begin()) {
        --lower_bound;
        /*k=*lower_bound;*/
        if (key_start > *lower_bound) lower_bound++;
      }
      /*k=*lower_bound;*/
      while (lower_bound != myset->end()) {
        if (*lower_bound > key_end) break;
        final_values.insert(final_values.end(), *lower_bound);
        lower_bound++;
      }
    }
  }
  return final_values;
}

template <typename KeyType, typename Hash, typename Compare, typename Allocator,
          typename SharedType>
std::vector<KeyType> set<KeyType, Hash, Compare, Allocator,
                         SharedType>::ContainsInServer(KeyType &key_start,
                                                       KeyType &key_end) {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  if (is_local()) {
    HCL_CPP_FUNCTION_UPDATE("access", "local");
    return LocalContainsInServer(key_start, key_end);
  } else {
    typedef std::vector<KeyType> ret_type;
    auto my_server_i = my_server_idx;
    HCL_CPP_FUNCTION_UPDATE("access", "remote");
    HCL_CPP_FUNCTION_UPDATE("server", my_server_i);
    return RPC_CALL_WRAPPER("_Contains", my_server_i, ret_type, key_start,
                            key_end);
  }
}

template <typename KeyType, typename Hash, typename Compare, typename Allocator,
          typename SharedType>
std::vector<KeyType>
set<KeyType, Hash, Compare, Allocator, SharedType>::LocalGetAllDataInServer() {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  HCL_CPP_FUNCTION_UPDATE("access", "local");
  std::vector<KeyType> final_values = std::vector<KeyType>();
  {
    boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex>
        lock(*mutex);
    typename MySet::iterator lower_bound;
    lower_bound = myset->begin();
    while (lower_bound != myset->end()) {
      final_values.insert(final_values.end(), *lower_bound);
      lower_bound++;
    }
  }
  return final_values;
}

template <typename KeyType, typename Hash, typename Compare, typename Allocator,
          typename SharedType>
std::vector<KeyType>
set<KeyType, Hash, Compare, Allocator, SharedType>::GetAllDataInServer() {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  if (is_local()) {
    HCL_CPP_FUNCTION_UPDATE("access", "local");
    return LocalGetAllDataInServer();
  } else {
    typedef std::vector<KeyType> ret_type;
    auto my_server_i = my_server_idx;
    HCL_CPP_FUNCTION_UPDATE("access", "remote");
    HCL_CPP_FUNCTION_UPDATE("server", my_server_i);
    return RPC_CALL_WRAPPER1("_GetAllData", my_server_i, ret_type);
  }
}

template <typename KeyType, typename Hash, typename Compare, typename Allocator,
          typename SharedType>
std::pair<bool, KeyType>
set<KeyType, Hash, Compare, Allocator, SharedType>::LocalSeekFirst() {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  bip::scoped_lock<bip::interprocess_mutex> lock(*mutex);
  if (myset->size() > 0) {
    auto iterator = myset->begin();  // We want First (smallest) value in set
    KeyType value = *iterator;
    return std::pair<bool, KeyType>(true, value);
  }
  return std::pair<bool, KeyType>(false, KeyType());
}

template <typename KeyType, typename Hash, typename Compare, typename Allocator,
          typename SharedType>
std::pair<bool, KeyType> set<KeyType, Hash, Compare, Allocator,
                             SharedType>::SeekFirst(uint16_t &key_int) {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  if (is_local(key_int)) {
    HCL_CPP_FUNCTION_UPDATE("access", "local");
    return LocalSeekFirst();
  } else {
    HCL_CPP_FUNCTION_UPDATE("access", "remote");
    HCL_CPP_FUNCTION_UPDATE("server", key_int);
    typedef std::pair<bool, KeyType> ret_type;
    return RPC_CALL_WRAPPER1("_SeekFirst", key_int, ret_type);
  }
}

template <typename KeyType, typename Hash, typename Compare, typename Allocator,
          typename SharedType>
std::pair<bool, std::vector<KeyType>>
set<KeyType, Hash, Compare, Allocator, SharedType>::LocalSeekFirstN(
    uint32_t n) {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  HCL_CPP_FUNCTION_UPDATE("access", "local");
  bip::scoped_lock<bip::interprocess_mutex> lock(*mutex);
  auto keys = std::vector<KeyType>();
  auto iterator = myset->begin();
  uint32_t i = 0;
  while (iterator != myset->end() && i < n) {
    keys.push_back(*iterator);
    i++;
    iterator++;
  }
  return std::pair<bool, std::vector<KeyType>>(i > 0, keys);
}

template <typename KeyType, typename Hash, typename Compare, typename Allocator,
          typename SharedType>
std::pair<bool, std::vector<KeyType>>
set<KeyType, Hash, Compare, Allocator, SharedType>::SeekFirstN(
    uint16_t &key_int, uint32_t n) {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  if (is_local(key_int)) {
    HCL_CPP_FUNCTION_UPDATE("access", "local");
    return LocalSeekFirstN(n);
  } else {
    HCL_CPP_FUNCTION_UPDATE("access", "remote");
    HCL_CPP_FUNCTION_UPDATE("server", key_int);
    typedef std::pair<bool, KeyType> ret_type;
    return RPC_CALL_WRAPPER("_SeekFirstN", key_int, ret_type, n);
  }
}

template <typename KeyType, typename Hash, typename Compare, typename Allocator,
          typename SharedType>
std::pair<bool, KeyType>
set<KeyType, Hash, Compare, Allocator, SharedType>::LocalPopFirst() {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  HCL_CPP_FUNCTION_UPDATE("access", "local");
  bip::scoped_lock<bip::interprocess_mutex> lock(*mutex);
  if (myset->size() > 0) {
    auto iterator = myset->begin();  // We want First (smallest) value in set
    KeyType value = *iterator;
    myset->erase(iterator);
    return std::pair<bool, KeyType>(true, value);
  }
  return std::pair<bool, KeyType>(false, KeyType());
}

template <typename KeyType, typename Hash, typename Compare, typename Allocator,
          typename SharedType>
std::pair<bool, KeyType> set<KeyType, Hash, Compare, Allocator,
                             SharedType>::PopFirst(uint16_t &key_int) {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  if (is_local(key_int)) {
    HCL_CPP_FUNCTION_UPDATE("access", "local");
    return LocalPopFirst();
  } else {
    HCL_CPP_FUNCTION_UPDATE("access", "remote");
    HCL_CPP_FUNCTION_UPDATE("server", key_int);
    typedef std::pair<bool, KeyType> ret_type;
    return RPC_CALL_WRAPPER1("_PopFirst", key_int, ret_type);
  }
}

template <typename KeyType, typename Hash, typename Compare, typename Allocator,
          typename SharedType>
size_t set<KeyType, Hash, Compare, Allocator, SharedType>::LocalSize() {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  HCL_CPP_FUNCTION_UPDATE("access", "local");
  return myset->size();
}

template <typename KeyType, typename Hash, typename Compare, typename Allocator,
          typename SharedType>
size_t set<KeyType, Hash, Compare, Allocator, SharedType>::Size(
    uint16_t &key_int) {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  if (is_local(key_int)) {
    HCL_CPP_FUNCTION_UPDATE("access", "local");
    return LocalSize();
  } else {
    HCL_CPP_FUNCTION_UPDATE("access", "remote");
    HCL_CPP_FUNCTION_UPDATE("server", key_int);
    typedef size_t ret_type;
    return RPC_CALL_WRAPPER1("_Size", key_int, ret_type);
  }
}

template <typename KeyType, typename Hash, typename Compare, typename Allocator,
          typename SharedType>
void set<KeyType, Hash, Compare, Allocator,
         SharedType>::construct_shared_memory() {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  HCL_CPP_FUNCTION_UPDATE("access", "local");
  ShmemAllocator alloc_inst(segment.get_segment_manager());
  /* Construct set in the shared memory space. */
  myset = segment.construct<MySet>(name.c_str())(Compare(), alloc_inst);
}

template <typename KeyType, typename Hash, typename Compare, typename Allocator,
          typename SharedType>
void set<KeyType, Hash, Compare, Allocator, SharedType>::open_shared_memory() {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  HCL_CPP_FUNCTION_UPDATE("access", "local");
  std::pair<MySet *, boost::interprocess::managed_mapped_file::size_type> res;
  res = segment.find<MySet>(name.c_str());
  myset = res.first;
}

template <typename KeyType, typename Hash, typename Compare, typename Allocator,
          typename SharedType>
void set<KeyType, Hash, Compare, Allocator, SharedType>::bind_functions() {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  HCL_CPP_FUNCTION_UPDATE("access", "local");

  auto rpc = hcl::HCL::GetInstance(false)->GetRPC(port);
  /* Create a RPC server and map the methods to it. */
  switch (HCL_CONF->RPC_IMPLEMENTATION) {
#ifdef HCL_COMMUNICATION_ENABLE_THALLIUM
    case THALLIUM_TCP:
#endif
#if defined(HCL_COMMUNICATION_ENABLE_THALLIUM)
    {

      std::function<void(const tl::request &, KeyType &)> putFunc(std::bind(
          &set<KeyType, Hash, Compare, Allocator, SharedType>::ThalliumLocalPut,
          this, std::placeholders::_1, std::placeholders::_2));
      std::function<void(const tl::request &, KeyType &)> getFunc(std::bind(
          &set<KeyType, Hash, Compare, Allocator, SharedType>::ThalliumLocalGet,
          this, std::placeholders::_1, std::placeholders::_2));
      std::function<void(const tl::request &, KeyType &)> eraseFunc(
          std::bind(&set<KeyType, Hash, Compare, Allocator,
                         SharedType>::ThalliumLocalErase,
                    this, std::placeholders::_1, std::placeholders::_2));
      std::function<void(const tl::request &)> getAllDataInServerFunc(
          std::bind(&set<KeyType, Hash, Compare, Allocator,
                         SharedType>::ThalliumLocalGetAllDataInServer,
                    this, std::placeholders::_1));
      std::function<void(const tl::request &, KeyType &, KeyType &)>
          containsInServerFunc(
              std::bind(&set<KeyType, Hash, Compare, Allocator,
                             SharedType>::ThalliumLocalContainsInServer,
                        this, std::placeholders::_1, std::placeholders::_2,
                        std::placeholders::_3));
      std::function<void(const tl::request &)> seekFirstFunc(
          std::bind(&set<KeyType, Hash, Compare, Allocator,
                         SharedType>::ThalliumLocalSeekFirst,
                    this, std::placeholders::_1));
      std::function<void(const tl::request &)> popFirstFunc(
          std::bind(&set<KeyType, Hash, Compare, Allocator,
                         SharedType>::ThalliumLocalPopFirst,
                    this, std::placeholders::_1));
      std::function<void(const tl::request &)> sizeFunc(
          std::bind(&set<KeyType, Hash, Compare, Allocator,
                         SharedType>::ThalliumLocalSize,
                    this, std::placeholders::_1));
      std::function<void(const tl::request &, uint32_t)> localSeekFirstNFunc(
          std::bind(&set<KeyType, Hash, Compare, Allocator,
                         SharedType>::ThalliumLocalSeekFirstN,
                    this, std::placeholders::_1, std::placeholders::_2));
      rpc->bind(func_prefix + "_Put", putFunc);
      rpc->bind(func_prefix + "_Get", getFunc);
      rpc->bind(func_prefix + "_Erase", eraseFunc);
      rpc->bind(func_prefix + "_GetAllData", getAllDataInServerFunc);
      rpc->bind(func_prefix + "_Contains", containsInServerFunc);

      rpc->bind(func_prefix + "_SeekFirst", seekFirstFunc);
      rpc->bind(func_prefix + "_PopFirst", popFirstFunc);
      // rpc->bind(func_prefix+"_SeekFirstN", localSeekFirstNFunc);
      rpc->bind(func_prefix + "_Size", sizeFunc);
      break;
    }
#endif
  }
}

#endif  // INCLUDE_HCL_SET_SET_CPP_
