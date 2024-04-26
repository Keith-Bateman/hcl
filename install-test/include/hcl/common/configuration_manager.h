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

#ifndef INCLUDE_HCL_COMMON_CONFIGURATION_MANAGER_H
#define INCLUDE_HCL_COMMON_CONFIGURATION_MANAGER_H

#include <hcl/common/constants.h>
#include <hcl/common/data_structures.h>
#include <hcl/common/debug.h>
#include <hcl/common/enumerations.h>
#include <hcl/common/logging.h>
#include <hcl/common/profiler.h>
#include <hcl/common/singleton.h>

#include <boost/thread/mutex.hpp>
#include <fstream>
#include <hcl/hcl_config.hpp>
#include <vector>

#include "typedefs.h"

namespace hcl {

class ConfigurationManager {
 private:
  boost::mutex file_load;

 public:
  uint16_t RPC_PORT;
  uint16_t RPC_THREADS;
  RPCImplementation RPC_IMPLEMENTATION;
  CharStruct URI;
  really_long MEMORY_ALLOCATED;

  bool IS_SERVER;
  uint16_t MY_SERVER;
  uint32_t NUM_SERVERS;
  bool SERVER_ON_NODE;
  CharStruct SERVER_LIST_PATH;
  std::vector<CharStruct> SERVER_LIST;
  CharStruct BACKED_FILE_DIR;

  bool DYN_CONFIG;  // Does not do anything (yet)

  ConfigurationManager();

  std::vector<CharStruct> LoadServers();

  void ConfigureDefaultClient(std::string server_list_path = "");

  void ConfigureDefaultServer(std::string server_list_path = "");

  ~ConfigurationManager();
};

}  // namespace hcl

#endif  // INCLUDE_HCL_COMMON_CONFIGURATION_MANAGER_H
