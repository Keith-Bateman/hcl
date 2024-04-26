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

#ifndef HCL_CONTAINER_H
#define HCL_CONTAINER_H

#include <hcl/common/logging.h>
#include <hcl/common/profiler.h>
#include <hcl/communication/rpc_lib.h>

#include <cstdint>
#include <hcl/hcl_config.hpp>
#include <memory>

#include "data_structures.h"
#include "typedefs.h"

namespace hcl {
class container {
 protected:
  int num_servers;
  uint16_t my_server_idx;
  really_long memory_allocated;
  bool is_server;
  boost::interprocess::managed_mapped_file segment;
  CharStruct name, func_prefix;
  boost::interprocess::interprocess_mutex *mutex;
  CharStruct backed_file;
  uint16_t port;

 public:
  bool server_on_node;
  virtual void construct_shared_memory() = 0;
  virtual void open_shared_memory() = 0;
  virtual void bind_functions() = 0;

  bool is_local(uint16_t &key_int);
  bool is_local();

  template <typename Allocator, typename MappedType, typename SharedType>
  typename std::enable_if_t<std::is_same<Allocator, nullptr_t>::value,
                            MappedType>
  GetData(MappedType &data) {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    return std::move(data);
  }

  template <typename Allocator, typename MappedType, typename SharedType>
  typename std::enable_if_t<!std::is_same<Allocator, nullptr_t>::value,
                            SharedType>
  GetData(MappedType &data) {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    Allocator allocator(segment.get_segment_manager());
    SharedType value(allocator);
    value.assign(data);
    return value;
  }

  virtual ~container();
  container(CharStruct _name, uint16_t _port, uint16_t _num_servers,
            uint16_t _my_server_idx, really_long _memory_allocated,
            bool _is_server, bool _is_server_on_node,
            CharStruct _backed_file_dir);
  void lock();

  void unlock();
};
}  // namespace hcl

#endif  // HCL_CONTAINER_H
