#include <hcl/clock/global_clock.h>
/** Include Headers **/
#include <hcl/common/constants.h>
#include <hcl/common/debug.h>
#include <hcl/common/macros.h>
#include <hcl/common/singleton.h>

namespace hcl {

global_clock::~global_clock() {
  AutoTrace trace = AutoTrace("hcl::~global_clock", NULL);
  if (is_server) bip::file_mapping::remove(backed_file.c_str());
}

global_clock::global_clock(CharStruct name_, uint16_t port)
    : is_server(HCL_CONF->IS_SERVER),
      memory_allocated(1024ULL * 1024ULL * 128ULL),
      my_rank(0),
      comm_size(1),
      num_servers(HCL_CONF->NUM_SERVERS),
      my_server(HCL_CONF->MY_SERVER),
      segment(),
      name(name_),
      func_prefix(name_),
      server_on_node(HCL_CONF->SERVER_ON_NODE),
      backed_file(HCL_CONF->BACKED_FILE_DIR + PATH_SEPARATOR + name_) {
  AutoTrace trace = AutoTrace("hcl::global_clock");
  MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  name = name + "_" + std::to_string(my_server);
  rpc = Singleton<RPCFactory>::GetInstance()->GetRPC(port);
  if (is_server) {
    switch (HCL_CONF->RPC_IMPLEMENTATION) {
#ifdef HCL_COMMUNICATION_ENABLE_RPCLIB
      case RPCLIB: {
        std::function<HTime(void)> getTimeFunction(
            std::bind(&global_clock::LocalGetTime, this));
        rpc->bind(func_prefix + "_GetTime", getTimeFunction);
        break;
      }
#endif
#ifdef HCL_COMMUNICATION_ENABLE_THALLIUM
      case THALLIUM_TCP:
#endif
#ifdef HCL_ENABLE_THALLIUM_ROCE
      case THALLIUM_ROCE:
#endif
#if defined(HCL_COMMUNICATION_ENABLE_THALLIUM)
      {
        std::function<void(const tl::request &)> getTimeFunction(std::bind(
            &global_clock::ThalliumLocalGetTime, this, std::placeholders::_1));
        rpc->bind(func_prefix + "_GetTime", getTimeFunction);
        break;
      }
#endif
    }

    bip::file_mapping::remove(backed_file.c_str());
    segment =
        bip::managed_mapped_file(bip::create_only, backed_file.c_str(), 65536);
    start = segment.construct<chrono_time>("Time")(
        std::chrono::high_resolution_clock::now());
    mutex = segment.construct<boost::interprocess::interprocess_mutex>("mtx")();
  } else if (!is_server && server_on_node) {
    segment = bip::managed_mapped_file(bip::open_only, backed_file.c_str());
    std::pair<chrono_time *, bip::managed_mapped_file::size_type> res;
    res = segment.find<chrono_time>("Time");
    start = res.first;
    std::pair<bip::interprocess_mutex *, bip::managed_mapped_file::size_type>
        res2;
    res2 = segment.find<bip::interprocess_mutex>("mtx");
    mutex = res2.first;
  }
}
chrono_time *global_clock::data() {
  if (server_on_node || is_server)
    return start;
  else
    nullptr;
}
void global_clock::lock() {
  if (server_on_node || is_server) mutex->lock();
}
void global_clock::unlock() {
  if (server_on_node || is_server) mutex->unlock();
}
HTime global_clock::GetTime() {
  if (server_on_node) {
    return LocalGetTime();
  } else {
    auto my_server_i = my_server;
    return RPC_CALL_WRAPPER1("_GetTime", my_server_i, HTime);
  }
}
HTime global_clock::GetTimeServer(uint16_t &server) {
  AutoTrace trace = AutoTrace("hcl::global_clock::GetTimeServer", server);
  if (my_server == server && server_on_node) {
    return LocalGetTime();
  } else {
    return RPC_CALL_WRAPPER1("_GetTime", server, HTime);
  }
}
HTime global_clock::LocalGetTime() {
  AutoTrace trace = AutoTrace("hcl::global_clock::GetTime", NULL);
  boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex>
      lock(*mutex);
  auto t2 = std::chrono::high_resolution_clock::now();
  auto t = std::chrono::duration_cast<std::chrono::microseconds>(t2 - *start)
               .count();
  return t;
}
}  // namespace hcl