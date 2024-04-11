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

#ifndef INCLUDE_HCL_SEQUENCER_GLOBAL_SEQUENCE_H_
#define INCLUDE_HCL_SEQUENCER_GLOBAL_SEQUENCE_H_
#if defined(HCL_HAS_CONFIG)
#include <hcl/hcl_config.hpp>
#else
#error "no config"
#endif
/**
 * Include Headers
 */
#include <hcl/common/container.h>
#include <hcl/communication/rpc_factory.h>
#include <hcl/communication/rpc_lib.h>
/** Boost Headers **/
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/managed_mapped_file.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
/** Standard C++ Headers**/
#include <stdint-gcc.h>

#include <memory>
#include <string>
#include <utility>

namespace bip = boost::interprocess;

namespace hcl {
class global_sequence : public Container {
 private:
  uint64_t *value;

 public:
  ~global_sequence();

  void construct_shared_memory();

  void open_shared_memory() override;

  void bind_functions() override;

  global_sequence(CharStruct name_ = "TEST_GLOBAL_SEQUENCE",
                  uint16_t port = -1);
  uint64_t *data();
  uint64_t GetNextSequence();
  uint64_t GetNextSequenceServer(uint16_t &server);

  uint64_t LocalGetNextSequence();
};

}  // namespace hcl

#endif  // INCLUDE_HCL_SEQUENCER_GLOBAL_SEQUENCE_H_
