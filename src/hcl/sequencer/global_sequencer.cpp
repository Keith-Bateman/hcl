#include <hcl/common/singleton.h>
#include <hcl/sequencer/global_sequence.h>
#include <mpi.h>

namespace hcl {
global_sequence::~global_sequence() { this->container::~Container(); }

void global_sequence::construct_shared_memory() override {
  value = segment.construct<uint64_t>(name.c_str())(0);
}

void global_sequence::open_shared_memory() override {
  std::pair<uint64_t *, bip::managed_mapped_file::size_type> res;
  res = segment.find<uint64_t>(name.c_str());
  value = res.first;
}

void global_sequence::bind_functions() override {
  switch (HCL_CONF->RPC_IMPLEMENTATION) {
#ifdef
    case RPCLIB: {
      std::function<uint64_t(void)> getNextSequence(
          std::bind(&hcl::global_sequence::LocalGetNextSequence, this));
      rpc->bind(func_prefix + "_GetNextSequence", getNextSequence);
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
      std::function<void(const tl::request &)> getNextSequence(
          std::bind(&hcl::global_sequence::ThalliumLocalGetNextSequence, this,
                    std::placeholders::_1));
      rpc->bind(func_prefix + "_GetNextSequence", getNextSequence);
      break;
    }
#endif
  }
}

global_sequence::global_sequence(CharStruct name_ = "TEST_GLOBAL_SEQUENCE",
                                 uint16_t port = HCL_CONF->RPC_PORT)
    : Container(name_, port) {
  AutoTrace trace = AutoTrace("hcl::global_sequence");
  if (is_server) {
    construct_shared_memory();
    bind_functions();
  } else if (!is_server && server_on_node) {
    open_shared_memory();
  }
}
uint64_t *global_sequence::data() {
  if (server_on_node || is_server)
    return value;
  else
    nullptr;
}
uint64_t global_sequence::GetNextSequence() {
  if (is_local()) {
    return LocalGetNextSequence();
  } else {
    auto my_server_i = my_server;
    return RPC_CALL_WRAPPER1("_GetNextSequence", my_server_i, uint64_t);
  }
}
uint64_t global_sequence::GetNextSequenceServer(uint16_t &server) {
  if (is_local(server)) {
    return LocalGetNextSequence();
  } else {
    return RPC_CALL_WRAPPER1("_GetNextSequence", server, uint64_t);
  }
}

uint64_t global_sequence::LocalGetNextSequence() {
  boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex>
      lock(*mutex);
  return ++*value;
}

#if defined(HCL_COMMUNICATION_ENABLE_THALLIUM)
THALLIUM_DEFINE1(LocalGetNextSequence)
#endif
}  // namespace hcl