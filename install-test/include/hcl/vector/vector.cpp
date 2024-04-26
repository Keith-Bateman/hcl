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

template <typename MappedType, typename Allocator, typename SharedType>
vector<MappedType, Allocator, SharedType>::~vector() {
  this->container::~container();
}
template <typename MappedType, typename Allocator, typename SharedType>
vector<MappedType, Allocator, SharedType>::vector(
    CharStruct name_, uint16_t port, uint16_t _num_servers,
    uint16_t _my_server_idx, really_long _memory_allocated, bool _is_server,
    bool _is_server_on_node, CharStruct _backed_file_dir)
    : container(name_, port, _num_servers, _my_server_idx, _memory_allocated,
                _is_server, _is_server_on_node, _backed_file_dir),
      my_vector() {
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
 * Push the data into the local vector.
 * @param key, the key for put
 * @param data, the value for put
 * @return bool, true if Put was successful else false.
 */
template <typename MappedType, typename Allocator, typename SharedType>
bool vector<MappedType, Allocator, SharedType>::LocalPush(MappedType &data) {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  HCL_CPP_FUNCTION_UPDATE("access", "local");
  bip::scoped_lock<bip::interprocess_mutex> lock(*mutex);
  auto value = GetData<Allocator, MappedType, SharedType>(data);
  my_vector->push_back(std::move(value));
  return true;
}

/**
 * Push the data into the vector. Uses key to decide the server to hash it to,
 * @param key, the key for put
 * @param data, the value for put
 * @return bool, true if Put was successful else false.
 */
template <typename MappedType, typename Allocator, typename SharedType>
bool vector<MappedType, Allocator, SharedType>::Push(MappedType &data,
                                                     uint16_t &key_int) {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  if (is_local(key_int)) {
    HCL_CPP_FUNCTION_UPDATE("access", "local");
    return LocalPush(data);
  } else {
    HCL_CPP_FUNCTION_UPDATE("access", "remote");
    HCL_CPP_FUNCTION_UPDATE("access", key_int);
    return RPC_CALL_WRAPPER("_Push", key_int, bool, data);
  }
}

/**
 * Get the local data from the vector.
 * @param key_int, key_int to know which server
 * @return return a pair of bool and Value. If bool is true then data was
 * found and is present in value part else bool is set to false
 */
template <typename MappedType, typename Allocator, typename SharedType>
std::pair<bool, MappedType> vector<MappedType, Allocator, SharedType>::LocalGet(
    size_t index) {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  HCL_CPP_FUNCTION_UPDATE("access", "local");
  bip::scoped_lock<bip::interprocess_mutex> lock(*mutex);
  if (my_vector->size() > index) {
    MappedType value = my_vector->at(index);
    return std::pair<bool, MappedType>(true, value);
  }
  return std::pair<bool, MappedType>(false, MappedType());
}

/**
 * Get the data from the vector. Uses key_int to decide the server to hash it
 * to,
 * @param key_int, key_int to know which server
 * @return return a pair of bool and Value. If bool is true then data was
 * found and is present in value part else bool is set to false
 */
template <typename MappedType, typename Allocator, typename SharedType>
std::pair<bool, MappedType> vector<MappedType, Allocator, SharedType>::Get(
    size_t index, uint16_t &key_int) {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  if (is_local(key_int)) {
    HCL_CPP_FUNCTION_UPDATE("access", "local");
    return LocalGet(index);
  } else {
    HCL_CPP_FUNCTION_UPDATE("access", "remote");
    HCL_CPP_FUNCTION_UPDATE("access", key_int);
    typedef std::pair<bool, MappedType> ret_type;
    return RPC_CALL_WRAPPER("_Get", key_int, ret_type, index);
  }
}

/**
 * Get the size of the local vector.
 * @param key_int, key_int to know which server
 * @return return a size of the vector
 */
template <typename MappedType, typename Allocator, typename SharedType>
size_t vector<MappedType, Allocator, SharedType>::LocalSize() {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  bip::scoped_lock<bip::interprocess_mutex> lock(*mutex);
  size_t value = my_vector->size();
  return value;
}

/**
 * Get the size of the vector. Uses key_int to decide the server to hash it to,
 * @param key_int, key_int to know which server
 * @return return a size of the vector
 */
template <typename MappedType, typename Allocator, typename SharedType>
size_t vector<MappedType, Allocator, SharedType>::Size(uint16_t &key_int) {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  if (is_local(key_int)) {
    HCL_CPP_FUNCTION_UPDATE("access", "local");
    return LocalSize();
  } else {
    HCL_CPP_FUNCTION_UPDATE("access", "remote");
    HCL_CPP_FUNCTION_UPDATE("access", key_int);
    return RPC_CALL_WRAPPER1("_Size", key_int, size_t);
  }
}

template <typename MappedType, typename Allocator, typename SharedType>
void vector<MappedType, Allocator, SharedType>::construct_shared_memory() {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  HCL_CPP_FUNCTION_UPDATE("access", "local");
  ShmemAllocator alloc_inst(segment.get_segment_manager());
  /* Construct vector in the shared memory space. */
  my_vector = segment.construct<Vector>("Vector")(alloc_inst);
}

template <typename MappedType, typename Allocator, typename SharedType>
void vector<MappedType, Allocator, SharedType>::open_shared_memory() {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  HCL_CPP_FUNCTION_UPDATE("access", "local");
  std::pair<Vector *, bip::managed_mapped_file::size_type> res;
  res = segment.find<Vector>("Vector");
  my_vector = res.first;
}

template <typename MappedType, typename Allocator, typename SharedType>
void vector<MappedType, Allocator, SharedType>::bind_functions() {
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
      std::function<void(const tl::request &, MappedType &)> pushFunc(std::bind(
          &hcl::vector<MappedType, Allocator, SharedType>::ThalliumLocalPush,
          this, std::placeholders::_1, std::placeholders::_2));
      std::function<void(const tl::request &, size_t index)> getFunc(std::bind(
          &hcl::vector<MappedType, Allocator, SharedType>::ThalliumLocalGet,
          this, std::placeholders::_1, std::placeholders::_2));
      std::function<void(const tl::request &)> sizeFunc(std::bind(
          &hcl::vector<MappedType, Allocator, SharedType>::ThalliumLocalSize,
          this, std::placeholders::_1));
      rpc->bind(func_prefix + "_Push", pushFunc);
      rpc->bind(func_prefix + "_Get", getFunc);
      rpc->bind(func_prefix + "_Size", sizeFunc);
      break;
    }
#endif
  }
}
// template class vector<int>;
