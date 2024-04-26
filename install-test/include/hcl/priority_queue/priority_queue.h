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

#ifndef INCLUDE_HCL_PRIORITY_QUEUE_PRIORITY_QUEUE_H_
#define INCLUDE_HCL_PRIORITY_QUEUE_PRIORITY_QUEUE_H_

#include <hcl/hcl_config.hpp>
/**
 * Include Headers
 */
#include <hcl/common/container.h>
#include <hcl/common/debug.h>
#include <hcl/common/singleton.h>
#include <hcl/common/typedefs.h>
#include <hcl/communication/rpc_lib.h>
#include <hcl/hcl_internal.h>

/** Boost Headers **/
#include <boost/algorithm/string.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
/** Standard C++ Headers**/

#include <functional>
#include <iostream>
#include <memory>
#include <queue>
#include <scoped_allocator>
#include <string>
#include <utility>
#include <vector>

/** Namespaces Uses **/
namespace bip = boost::interprocess;

/** Global Typedefs **/

namespace hcl {
/**
 * This is a Distributed priority_queue Class. It uses shared memory + RPC
 * to achieve the data structure.
 *
 * @tparam MappedType, the value of the priority_queue
 */
template <typename MappedType, typename Compare = std::less<MappedType>,
          class Allocator = nullptr_t, class SharedType = nullptr_t>
class priority_queue : public container {
 private:
  /** Class Typedefs for ease of use **/
  typedef std::scoped_allocator_adaptor<
      bip::allocator<MappedType, bip::managed_mapped_file::segment_manager>>
      ShmemAllocator;
  typedef std::priority_queue<MappedType,
                              std::vector<MappedType, ShmemAllocator>, Compare>
      Queue;

  /** Class attributes**/
  Queue *queue;

 public:
  ~priority_queue();

  void construct_shared_memory() override;

  void open_shared_memory() override;

  void bind_functions() override;

  explicit priority_queue(
      CharStruct name_ = "TEST_PRIORITY_QUEUE",
      uint16_t port = HCL_CONF->RPC_PORT,
      uint16_t _num_servers = HCL_CONF->NUM_SERVERS,
      uint16_t _my_server_idx = HCL_CONF->MY_SERVER,
      really_long _memory_allocated = HCL_CONF->MEMORY_ALLOCATED,
      bool _is_server = HCL_CONF->IS_SERVER,
      bool _is_server_on_node = HCL_CONF->SERVER_ON_NODE,
      CharStruct _backed_file_dir = HCL_CONF->BACKED_FILE_DIR);
  Queue *data() {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    HCL_CPP_FUNCTION_UPDATE("access", "local");
    if (server_on_node || is_server)
      return queue;
    else
      nullptr;
  }
  bool LocalPush(MappedType &data);
  std::pair<bool, MappedType> LocalPop();
  std::pair<bool, MappedType> LocalTop();
  size_t LocalSize();

#if defined(HCL_COMMUNICATION_ENABLE_THALLIUM)
  THALLIUM_DEFINE(LocalPush, (data), MappedType &data)
  THALLIUM_DEFINE1(LocalPop)
  THALLIUM_DEFINE1(LocalTop)
  THALLIUM_DEFINE1(LocalSize)
#endif

  bool Push(MappedType &data, uint16_t &key_int);
  std::pair<bool, MappedType> Pop(uint16_t &key_int);
  std::pair<bool, MappedType> Top(uint16_t &key_int);
  size_t Size(uint16_t &key_int);
};

#include "priority_queue.cpp"

}  // namespace hcl

#endif  // INCLUDE_HCL_PRIORITY_QUEUE_PRIORITY_QUEUE_H_
