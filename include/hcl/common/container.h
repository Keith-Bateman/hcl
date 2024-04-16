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
#include <hcl/hcl_internal.h>

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

  inline bool is_local(uint16_t &key_int) {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    return key_int == my_server_idx && server_on_node;
  }
  inline bool is_local() {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    return server_on_node;
  }

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

  virtual ~container() {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    if (is_server)
      boost::interprocess::file_mapping::remove(backed_file.c_str());
  }
  container(CharStruct _name, uint16_t _port, uint16_t _num_servers,
            uint16_t _my_server_idx, really_long _memory_allocated,
            bool _is_server, bool _is_server_on_node,
            CharStruct _backed_file_dir)
      : num_servers(_num_servers),
        my_server_idx(_my_server_idx),
        memory_allocated(_memory_allocated),
        is_server(_is_server),
        segment(),
        name(_name),
        func_prefix(_name),
        backed_file(_backed_file_dir + PATH_SEPARATOR + _name + "_" +
                    std::to_string(_my_server_idx)),
        port(_port),
        server_on_node(_is_server_on_node) {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    /* create per server name for shared memory. Needed if multiple servers are
       spawned on one node*/
    this->name += "_" + std::to_string(my_server_idx);
    /* if current rank is a server */
    auto rpc = hcl::HCL::GetInstance(false)->GetRPC(port);
    if (is_server) {
      /* Delete existing instance of shared memory space*/
      boost::interprocess::file_mapping::remove(backed_file.c_str());
      /* allocate new shared memory space */
      segment = boost::interprocess::managed_mapped_file(
          boost::interprocess::create_only, backed_file.c_str(),
          memory_allocated);
      mutex =
          segment.construct<boost::interprocess::interprocess_mutex>("mtx")();
    } else if (!is_server && server_on_node) {
      /* Map the clients to their respective memory pools */
      segment = boost::interprocess::managed_mapped_file(
          boost::interprocess::open_only, backed_file.c_str());
      std::pair<boost::interprocess::interprocess_mutex *,
                boost::interprocess::managed_mapped_file::size_type>
          res2;
      res2 = segment.find<boost::interprocess::interprocess_mutex>("mtx");
      mutex = res2.first;
    }
  }
  void lock() {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    if (server_on_node || is_server) mutex->lock();
  }

  void unlock() {
    HCL_LOG_TRACE();
    HCL_CPP_FUNCTION()
    if (server_on_node || is_server) mutex->unlock();
  }
};
}  // namespace hcl

#endif  // HCL_CONTAINER_H
