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

#ifndef INCLUDE_HCL_VECTOR_VECTOR_H_
#define INCLUDE_HCL_VECTOR_VECTOR_H_

#include <hcl/hcl_config.hpp>
/**
 * Include Headers
 */
#include <hcl/common/debug.h>
#include <hcl/common/singleton.h>
#include <hcl/communication/rpc_lib.h>
#include <hcl/hcl_internal.h>

/** Boost Headers **/
#include <boost/algorithm/string.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/managed_mapped_file.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
/** Standard C++ Headers**/
#include <hcl/common/container.h>

#include <boost/interprocess/managed_mapped_file.hpp>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <utility>

/** Namespaces Uses **/
namespace bip = boost::interprocess;

namespace hcl {
/**
 * This is a Distributed Vector Class. It uses shared memory +
 * RPC to achieve the data structure.
 *
 * @tparam MappedType, the value of the Vector
 */
template <typename MappedType, class Allocator = nullptr_t,
          class SharedType = nullptr_t>
class vector : public container {
 private:
  /** Class Typedefs for ease of use **/
  typedef bip::allocator<MappedType, bip::managed_mapped_file::segment_manager>
      ShmemAllocator;
  typedef boost::interprocess::vector<MappedType, ShmemAllocator> Vector;

  /** Class attributes**/
  Vector *my_vector;

 public:
  ~vector();

  void construct_shared_memory() override;

  void open_shared_memory() override;

  void bind_functions() override;

  explicit vector(CharStruct name_ = "TEST_VECTOR",
                  uint16_t port = HCL_CONF->RPC_PORT,
                  uint16_t _num_servers = HCL_CONF->NUM_SERVERS,
                  uint16_t _my_server_idx = HCL_CONF->MY_SERVER,
                  really_long _memory_allocated = HCL_CONF->MEMORY_ALLOCATED,
                  bool _is_server = HCL_CONF->IS_SERVER,
                  bool _is_server_on_node = HCL_CONF->SERVER_ON_NODE,
                  CharStruct _backed_file_dir = HCL_CONF->BACKED_FILE_DIR);
  Vector *data() {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    HCL_CPP_FUNCTION_UPDATE("access", "local");
    if (server_on_node || is_server)
      return my_vector;
    else
      nullptr;
  }
  bool LocalPush(MappedType &data);
  std::pair<bool, MappedType> LocalGet(size_t index);
  size_t LocalSize();

#if defined(HCL_COMMUNICATION_ENABLE_THALLIUM)
  THALLIUM_DEFINE(LocalPush, (data), MappedType &data)
  THALLIUM_DEFINE(LocalGet, (index), size_t index)
  THALLIUM_DEFINE1(LocalSize)
#endif

  bool Push(MappedType &data, uint16_t &key_int);
  std::pair<bool, MappedType> Get(size_t index, uint16_t &key_int);
  size_t Size(uint16_t &key_int);
};

#include "vector.cpp"

}  // namespace hcl

#endif  // INCLUDE_HCL_VECTOR_VECTOR_H_
