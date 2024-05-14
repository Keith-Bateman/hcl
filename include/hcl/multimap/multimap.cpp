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

#ifndef INCLUDE_HCL_MULTIMAP_MULTIMAP_CPP_
#define INCLUDE_HCL_MULTIMAP_MULTIMAP_CPP_

/* Constructor to deallocate the shared memory*/
template <typename KeyType, typename MappedType, typename Compare,
          typename Allocator, typename SharedType>
multimap<KeyType, MappedType, Compare, Allocator, SharedType>::~multimap() {
  this->container::~container();
}

template <typename KeyType, typename MappedType, typename Compare,
          typename Allocator, typename SharedType>
multimap<KeyType, MappedType, Compare, Allocator, SharedType>::multimap(
    CharStruct name_, uint16_t port, uint16_t _num_servers,
    uint16_t _my_server_idx, really_long _memory_allocated, bool _is_server,
    bool _is_server_on_node, CharStruct _backed_file_dir)
    : container(name_, port, _num_servers, _my_server_idx, _memory_allocated,
                _is_server, _is_server_on_node, _backed_file_dir),
      mymap() {
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
 * Put the data into the local multimap.
 * @param key, the key for put
 * @param data, the value for put
 * @return bool, true if Put was successful else false.
 */
template <typename KeyType, typename MappedType, typename Compare,
          typename Allocator, typename SharedType>
bool multimap<KeyType, MappedType, Compare, Allocator, SharedType>::LocalPut(
    KeyType &key, MappedType &data) {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  HCL_CPP_FUNCTION_UPDATE("access", "local");
  boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex>
      lock(*mutex);
  typename MyMap::iterator iterator = mymap->find(key);
  if (iterator != mymap->end()) {
    mymap->erase(iterator);
  }
  auto value = GetData<Allocator, MappedType, SharedType>(data);
  mymap->insert(std::pair<KeyType, MappedType>(key, value));
  return true;
}

/**
 * Put the data into the multimap. Uses key to decide the server to hash it
 * to,
 * @param key, the key for put
 * @param data, the value for put
 * @return bool, true if Put was successful else false.
 */
template <typename KeyType, typename MappedType, typename Compare,
          typename Allocator, typename SharedType>
bool multimap<KeyType, MappedType, Compare, Allocator, SharedType>::Put(
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
 * Get the data in the local multimap.
 * @param key, key to get
 * @return return a pair of bool and Value. If bool is true then data was
 * found and is present in value part else bool is set to false
 */
template <typename KeyType, typename MappedType, typename Compare,
          typename Allocator, typename SharedType>
std::pair<bool, MappedType> multimap<KeyType, MappedType, Compare, Allocator,
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
 * Get the data into the multimap. Uses key to decide the server to hash it
 * to,
 * @param key, key to get
 * @return return a pair of bool and Value. If bool is true then data was
 * found and is present in value part else bool is set to false
 */
template <typename KeyType, typename MappedType, typename Compare,
          typename Allocator, typename SharedType>
std::pair<bool, MappedType> multimap<KeyType, MappedType, Compare, Allocator,
                                     SharedType>::Get(KeyType &key) {
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
std::pair<bool, MappedType> multimap<KeyType, MappedType, Compare, Allocator,
                                     SharedType>::LocalErase(KeyType &key) {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex>
      lock(*mutex);
  size_t s = mymap->erase(key);
  return std::pair<bool, MappedType>(s > 0, MappedType());
}

template <typename KeyType, typename MappedType, typename Compare,
          typename Allocator, typename SharedType>
std::pair<bool, MappedType> multimap<KeyType, MappedType, Compare, Allocator,
                                     SharedType>::Erase(KeyType &key) {
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
 * Get the data in the multimap. Uses key to decide the server to hash it
 * to,
 * @param key, key to get
 * @return return a pair of bool and Value. If bool is true then data was
 * found and is present in value part else bool is set to false
 */
template <typename KeyType, typename MappedType, typename Compare,
          typename Allocator, typename SharedType>
std::vector<std::pair<KeyType, MappedType>>
multimap<KeyType, MappedType, Compare, Allocator, SharedType>::Contains(
    KeyType &key) {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  std::vector<std::pair<KeyType, MappedType>> final_values =
      std::vector<std::pair<KeyType, MappedType>>();
  auto current_server = ContainsInServer(key);
  final_values.insert(final_values.end(), current_server.begin(),
                      current_server.end());
  for (int i = 0; i < num_servers; ++i) {
    if (i != my_server_idx) {
      HCL_CPP_REGION(ContainsServer);
      HCL_CPP_REGION_UPDATE(ContainsServer, "access", "remote");
      HCL_CPP_REGION_UPDATE(ContainsServer, "server", i);
      typedef std::vector<std::pair<KeyType, MappedType>> ret_type;
      auto server = RPC_CALL_WRAPPER("_Contains", i, ret_type, key);
      final_values.insert(final_values.end(), server.begin(), server.end());
    }
  }
  return final_values;
}

template <typename KeyType, typename MappedType, typename Compare,
          typename Allocator, typename SharedType>
std::vector<std::pair<KeyType, MappedType>>
multimap<KeyType, MappedType, Compare, Allocator, SharedType>::GetAllData() {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  std::vector<std::pair<KeyType, MappedType>> final_values =
      std::vector<std::pair<KeyType, MappedType>>();
  auto current_server = GetAllDataInServer();
  final_values.insert(final_values.end(), current_server.begin(),
                      current_server.end());
  for (int i = 0; i < num_servers; ++i) {
    if (i != my_server_idx) {
      HCL_CPP_REGION(ContainsGetAllData);
      HCL_CPP_REGION_UPDATE(ContainsGetAllData, "access", "remote");
      HCL_CPP_REGION_UPDATE(ContainsGetAllData, "server", i);
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
multimap<KeyType, MappedType, Compare, Allocator,
         SharedType>::LocalContainsInServer(KeyType &key) {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION();
  HCL_CPP_FUNCTION_UPDATE("access", "local");
  std::vector<std::pair<KeyType, MappedType>> final_values =
      std::vector<std::pair<KeyType, MappedType>>();
  {
    boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex>
        lock(*mutex);
    typename MyMap::iterator lower_bound;
    size_t size = mymap->size();
    if (size == 0) {
    } else if (size == 1) {
      lower_bound = mymap->begin();
      final_values.insert(final_values.end(),
                          std::pair<KeyType, MappedType>(lower_bound->first,
                                                         lower_bound->second));
    } else {
      lower_bound = mymap->lower_bound(key);
      if (lower_bound == mymap->end()) return final_values;
      if (lower_bound != mymap->begin()) {
        --lower_bound;
        if (!key.Contains(lower_bound->first)) lower_bound++;
      }
      while (lower_bound != mymap->end()) {
        if (!(key.Contains(lower_bound->first) ||
              lower_bound->first.Contains(key)))
          break;
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
multimap<KeyType, MappedType, Compare, Allocator, SharedType>::ContainsInServer(
    KeyType &key) {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  if (is_local()) {
    HCL_CPP_FUNCTION_UPDATE("access", "local");
    return LocalContainsInServer(key);
  } else {
    typedef std::vector<std::pair<KeyType, MappedType>> ret_type;
    auto my_server_i = my_server_idx;
    HCL_CPP_FUNCTION_UPDATE("access", "remote");
    HCL_CPP_FUNCTION_UPDATE("server", my_server_i);
    return RPC_CALL_WRAPPER("_Contains", my_server_i, ret_type, key);
  }
}

template <typename KeyType, typename MappedType, typename Compare,
          typename Allocator, typename SharedType>
std::vector<std::pair<KeyType, MappedType>>
multimap<KeyType, MappedType, Compare, Allocator,
         SharedType>::LocalGetAllDataInServer() {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  HCL_CPP_FUNCTION_UPDATE("access", "local");
  std::vector<std::pair<KeyType, MappedType>> final_values =
      std::vector<std::pair<KeyType, MappedType>>();
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
std::vector<std::pair<KeyType, MappedType>> multimap<
    KeyType, MappedType, Compare, Allocator, SharedType>::GetAllDataInServer() {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  if (is_local()) {
    HCL_CPP_FUNCTION_UPDATE("access", "local");
    return LocalGetAllDataInServer();
  } else {
    typedef std::vector<std::pair<KeyType, MappedType>> ret_type;
    auto my_server_i = my_server_idx;
    HCL_CPP_FUNCTION_UPDATE("access", "remote");
    HCL_CPP_FUNCTION_UPDATE("server", my_server_i);
    return RPC_CALL_WRAPPER1("_GetAllData", my_server_i, ret_type);
  }
}

template <typename KeyType, typename MappedType, typename Compare,
          typename Allocator, typename SharedType>
void multimap<KeyType, MappedType, Compare, Allocator,
              SharedType>::construct_shared_memory() {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  ShmemAllocator alloc_inst(segment.get_segment_manager());
  /* Construct Multimap in the shared memory space. */
  mymap = segment.construct<MyMap>(name.c_str())(Compare(), alloc_inst);
}

template <typename KeyType, typename MappedType, typename Compare,
          typename Allocator, typename SharedType>
void multimap<KeyType, MappedType, Compare, Allocator,
              SharedType>::open_shared_memory() {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  std::pair<MyMap *, boost::interprocess::managed_mapped_file::size_type> res;
  res = segment.find<MyMap>(name.c_str());
  mymap = res.first;
}

template <typename KeyType, typename MappedType, typename Compare,
          typename Allocator, typename SharedType>
void multimap<KeyType, MappedType, Compare, Allocator,
              SharedType>::bind_functions() {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()

  auto rpc = hcl::HCL::GetInstance(false)->GetRPC(port);
  /* Create a RPC server and map the methods to it. */
  switch (HCL_CONF->RPC_IMPLEMENTATION) {
#ifdef HCL_COMMUNICATION_ENABLE_THALLIUM
    case THALLIUM:
#endif
#if defined(HCL_COMMUNICATION_ENABLE_THALLIUM)
    {

      std::function<void(const tl::request &, KeyType &, MappedType &)> putFunc(
          std::bind(&multimap<KeyType, MappedType, Compare, Allocator,
                              SharedType>::ThalliumLocalPut,
                    this, std::placeholders::_1, std::placeholders::_2,
                    std::placeholders::_3));
      std::function<void(const tl::request &, KeyType &)> getFunc(
          std::bind(&multimap<KeyType, MappedType, Compare, Allocator,
                              SharedType>::ThalliumLocalGet,
                    this, std::placeholders::_1, std::placeholders::_2));
      std::function<void(const tl::request &, KeyType &)> eraseFunc(
          std::bind(&multimap<KeyType, MappedType, Compare, Allocator,
                              SharedType>::ThalliumLocalErase,
                    this, std::placeholders::_1, std::placeholders::_2));
      std::function<void(const tl::request &)> getAllDataInServerFunc(
          std::bind(&multimap<KeyType, MappedType, Compare, Allocator,
                              SharedType>::ThalliumLocalGetAllDataInServer,
                    this, std::placeholders::_1));
      std::function<void(const tl::request &, KeyType &)> containsInServerFunc(
          std::bind(&multimap<KeyType, MappedType,
                              Compare>::ThalliumLocalContainsInServer,
                    this, std::placeholders::_1, std::placeholders::_2));

      rpc->bind(func_prefix + "_Put", putFunc);
      rpc->bind(func_prefix + "_Get", getFunc);
      rpc->bind(func_prefix + "_Erase", eraseFunc);
      rpc->bind(func_prefix + "_GetAllData", getAllDataInServerFunc);
      rpc->bind(func_prefix + "_Contains", containsInServerFunc);
      break;
    }
#endif
  }
}

#endif  // INCLUDE_HCL_MULTIMAP_MULTIMAP_CPP_
