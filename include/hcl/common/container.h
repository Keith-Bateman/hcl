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
#if defined(HCL_HAS_CONFIG)
#include <hcl/hcl_config.hpp>
#else
#error "no config"
#endif
#include <hcl/common/typedefs.h>
#include <hcl/communication/rpc_factory.h>
#include <hcl/communication/rpc_lib.h>

#include <cstdint>
#include <memory>

namespace hcl {
class Container {
 protected:
  int comm_size, my_rank, num_servers;
  uint16_t my_server;
  std::shared_ptr<RPC> rpc;
  really_long memory_allocated;
  bool is_server;
  boost::interprocess::managed_mapped_file segment;
  CharStruct name, func_prefix;
  boost::interprocess::interprocess_mutex *mutex;
  CharStruct backed_file;

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
  GetData(MappedType &data);

  template <typename Allocator, typename MappedType, typename SharedType>
  typename std::enable_if_t<!std::is_same<Allocator, nullptr_t>::value,
                            SharedType>
  GetData(MappedType &data);

  virtual ~Container();
  Container(CharStruct name_, uint16_t port);
  void lock();

  void unlock();
};
}  // namespace hcl

#endif  // HCL_CONTAINER_H
