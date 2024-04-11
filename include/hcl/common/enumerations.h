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

#ifndef INCLUDE_HCL_COMMON_ENUMERATIONS_H
#define INCLUDE_HCL_COMMON_ENUMERATIONS_H
#if defined(HCL_HAS_CONFIG)
#include <hcl/hcl_config.hpp>
#else
#error "no config"
#endif
typedef enum RPCImplementation {
#if defined(HCL_COMMUNICATION_ENABLE_RPCLIB)
  RPCLIB = 0,
#elif defined(HCL_COMMUNICATION_ENABLE_THALLIUM)
  THALLIUM_TCP = 1,
#elif defined(HCL_COMMUNICATION_ENABLE_THALLIUM) && \
    defined(HCL_COMMUNICATION_PROTOCOL_ENABLE_VERBS)
  THALLIUM_ROCE = 2
#endif
} RPCImplementation;

#endif  // INCLUDE_HCL_COMMON_ENUMERATIONS_H
