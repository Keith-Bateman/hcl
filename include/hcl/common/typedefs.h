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

#ifndef INCLUDE_HCL_COMMON_TYPEDEFS_H_
#define INCLUDE_HCL_COMMON_TYPEDEFS_H_
#if defined(HCL_HAS_CONFIG)
#include <hcl/hcl_config.hpp>
#else
#error "no config"
#endif
#include <stdint.h>

#include <chrono>

typedef uint64_t t_mili;
typedef uint64_t HTime;
typedef uint64_t really_long;
typedef std::chrono::high_resolution_clock::time_point chrono_time;
#endif  // INCLUDE_HCL_COMMON_TYPEDEFS_H_
