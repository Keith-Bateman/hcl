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

#ifndef HCL_UTIL_H
#define HCL_UTIL_H
#include <hcl/hcl_config.hpp>
/*Includes*/
#include <stdlib.h>
#include <unistd.h>

#include <boost/interprocess/containers/string.hpp>
#include <cmath>
#include <cstdio>
#include <string>

namespace bip = boost::interprocess;

struct KeyType {
  size_t a;
  KeyType() : a(0) {}

  KeyType(const KeyType &t) { a = t.a; }
  KeyType(KeyType &t) { a = t.a; }
  KeyType(KeyType &&t) { a = t.a; }
  KeyType(size_t a_) : a(a_) {}
  /* equal operator for comparing two Matrix. */
  bool operator==(const KeyType &o) const { return a == o.a; }
  KeyType &operator=(const KeyType &other) {
    a = other.a;
    return *this;
  }
  bool operator<(const KeyType &o) const { return a < o.a; }
  bool operator>(const KeyType &o) const { return a > o.a; }
  bool Contains(const KeyType &o) const { return a == o.a; }
};

#if defined(HCL_COMMUNICATION_ENABLE_THALLIUM)
template <typename A>
void serialize(A &ar, KeyType &a) {
  ar &a.a;
}
#endif
namespace std {
template <>
struct hash<KeyType> {
  size_t operator()(const KeyType &k) const { return k.a; }
};
}  // namespace std
// 1,4,16,1000,4000,16000,250000,1000000,4000000,16000000

struct MappedType {
  // char a[62500];
  // double a[62500];
  // std::array<double, 62500> a;
  std::string a;

  MappedType() {
    a = "";
    a.resize(1000000, 'c');
    // for (long i = 0; i < 62500; i++) {
    //     a[i] = 1.1;
    // }
  }

  MappedType(std::string a_) {
    a = a_;
    // for (long i = 0; i < 62500; i++) {
    //     a[i] = a_[i];
    // }
  }

  /* equal operator for comparing two Matrix. */
  bool operator==(const MappedType &o) const {
    if (a == o.a) {
      return false;
    }
    // for (long i = 0; i < 62500; i++) {
    // if (a[i] != o.a[i]) {
    //     return false;
    // }
    // }
    return true;
  }
  MappedType &operator=(const MappedType &other) {
    a = other.a;
    // for (long i = 0; i < 62500; i++) {
    //     a[i] = other.a[i];
    // }
    return *this;
  }
  // bool operator<(const MappedType &o) const {
  //     for (long i = 0; i < 62500; i++) {
  //         if (a[i] < o.a[i]) {
  //             return true;
  //         }
  //     }
  //     return false;
  // }
  // bool Contains(const MappedType &o) const {
  //     for (long i = 0; i < 62500; i++) {
  //         if (a[i] != o.a[i]) {
  //             return false;
  //         }
  //     }
  //     return true;
  // }

  template <typename A>
  void serialize(A &ar) const {
    ar &a;
    // for (long i = 0; i < 62500; i++) {
    //     ar & a[i];
    // }
  }
};
const int MAX = 26;
std::string printRandomString(int n) {
  char alphabet[MAX] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i',
                        'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r',
                        's', 't', 'u', 'v', 'w', 'x', 'y', 'z'};

  std::string res = "";
  for (int i = 0; i < n; i++) res = res + alphabet[rand() % MAX];

  return res;
}

// void bt_sighandler(int sig, struct sigcontext ctx) {
//   void *trace[16];
//   char **messages = (char **)NULL;
//   int i, trace_size = 0;

//   if (sig == SIGSEGV)
//     printf("Got signal %d, faulty address is %p from %p\n", sig, ctx.cr2,
//            ctx.rip);
//   else
//     printf("Got signal %d\n", sig);

//   trace_size = backtrace(trace, 16);
//   /* overwrite sigaction with caller's address */
//   trace[1] = (void *)ctx.rip;
//   messages = backtrace_symbols(trace, trace_size);
//   /* skip first stack frame (points here) */
//   printf("[bt] Execution path:\n");
//   for (i = 1; i < trace_size; ++i) {
//     printf("[bt] #%d %s\n", i, messages[i]);

//     /* find first occurence of '(' or ' ' in message[i] and assume
//      * everything before that is the file name. (Don't go beyond 0 though
//      * (string terminator)*/
//     size_t p = 0;
//     while (messages[i][p] != '(' && messages[i][p] != ' ' &&
//            messages[i][p] != 0)
//       ++p;

//     char syscom[256];
//     sprintf(syscom, "addr2line %p -e %.*s", trace[i], p, messages[i]);
//     // last parameter is the file name of the symbol
//     int res = system(syscom);
//     (void)res;
//   }

//   exit(0);
// }

// void SetSignal() {
//   struct sigaction sa;

//   sa.sa_handler = reinterpret_cast<__sighandler_t>(bt_sighandler);
//   sigemptyset(&sa.sa_mask);
//   sa.sa_flags = SA_RESTART;

//   sigaction(SIGSEGV, &sa, NULL);
//   sigaction(SIGUSR1, &sa, NULL);
//   sigaction(SIGABRT, &sa, NULL);
// }

const uint32_t KB = 1024;
const uint32_t MB = 1024 * 1024;
#ifndef DISABLE_MPI
#define AGGREGATE_TIME(name, comm)              \
  double total_##name = 0.0;                    \
  auto name##_a = name##_time.getElapsedTime(); \
  MPI_Reduce(&name##_a, &total_##name, 1, MPI_DOUBLE, MPI_SUM, 0, comm);
#else
#define AGGREGATE_TIME(name, comm) \
  double total_##name = name##_time.getElapsedTime();
#endif

size_t GetRandomOffset(size_t i, unsigned int offset_seed, size_t stride,
                       size_t total_size) {
  return abs((int)(((i * rand_r(&offset_seed)) % stride) % total_size));
}
inline std::string get_filename(int fd) {
  const int kMaxSize = 256;
  char proclnk[kMaxSize];
  char filename[kMaxSize];
  snprintf(proclnk, kMaxSize, "/proc/self/fd/%d", fd);
  size_t r = readlink(proclnk, filename, kMaxSize);
  filename[r] = '\0';
  return filename;
}

std::string GenRandom(const int len) {
  std::string tmp_s;
  static const char alphanum[] =
      "0123456789"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "abcdefghijklmnopqrstuvwxyz";

  srand(100);

  tmp_s.reserve(len);

  for (int i = 0; i < len; ++i) {
    tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];
  }

  tmp_s[len - 1] = '\n';

  return tmp_s;
}
namespace hcl {
namespace test {
class Timer {
 public:
  Timer() : elapsed_time(0) {}
  void resumeTime() { t1 = std::chrono::high_resolution_clock::now(); }
  double pauseTime() {
    auto t2 = std::chrono::high_resolution_clock::now();
    elapsed_time += std::chrono::duration<double>(t2 - t1).count();
    return elapsed_time;
  }
  double getElapsedTime() { return elapsed_time; }

 private:
  std::chrono::high_resolution_clock::time_point t1;
  double elapsed_time;
};
}  // namespace test
}  // namespace hcl
#endif  // HCL_UTIL_H
