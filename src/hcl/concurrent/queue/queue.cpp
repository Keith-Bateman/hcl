#include <hcl/concurrent/queue/queue.h>
/** Include Headers **/
#include <hcl/common/debug.h>
#include <hcl/common/macros.h>
#include <hcl/common/singleton.h>

namespace hcl {

template <class ValueT>
concurrent_queue<ValueT>::~concurrent_queue() {
  if (queue != nullptr) delete queue;
}
template <class ValueT>
void concurrent_queue<ValueT>::construct_shared_memory() {}
template <class ValueT>
void concurrent_queue<ValueT>::open_shared_memory() {}
template <class ValueT>
void concurrent_queue<ValueT>::bind_functions() {
  switch (HCL_CONF->RPC_IMPLEMENTATION) {
#ifdef HCL_COMMUNICATION_ENABLE_RPCLIB
    case RPCLIB: {
      std::function<bool(ValueT &)> pushFunc(std::bind(
          &concurrent_queue<ValueT>::LocalPush, this, std::placeholders::_1));
      std::function<std::pair<bool, ValueT>(void)> popFunc(
          std::bind(&concurrent_queue<ValueT>::LocalPop, this));

      rpc->bind(func_prefix + "_Push", pushFunc);
      rpc->bind(func_prefix + "_Pop", popFunc);
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

      std::function<void(const tl::request &, ValueT &)> pushFunc(
          std::bind(&concurrent_queue<ValueT>::ThalliumLocalPush, this,
                    std::placeholders::_1, std::placeholders::_2));
      std::function<void(const tl::request &)> popFunc(
          std::bind(&concurrent_queue<ValueT>::ThalliumLocalPop, this,
                    std::placeholders::_1));

      rpc->bind(func_prefix + "_Push", pushFunc);
      rpc->bind(func_prefix + "_Pop", popFunc);
      break;
    }
#endif
  }
}
template <class ValueT>
concurrent_queue<ValueT>::concurrent_queue(CharStruct name_, uint16_t port)
    : Container(name_, port) {
  queue = nullptr;
  AutoTrace trace = AutoTrace("hcl::concurrent_queue");
  if (is_server) {
    queue = new queue_type(128);
    bind_functions();
  } else if (!is_server && server_on_node) {
  }
}
template <class ValueT>
boost::lockfree::queue<ValueT> *concurrent_queue<ValueT>::data() {
  return queue;
}

template <class ValueT>
bool concurrent_queue<ValueT>::LocalPush(ValueT &v) {
  return queue->push(v);
}
template <class ValueT>
std::pair<bool, ValueT> concurrent_queue<ValueT>::LocalPop() {
  ValueT v;
  bool b = queue->pop(v);
  if (b)
    return std::pair<bool, ValueT>(true, v);
  else
    return std::pair<bool, ValueT>(false, ValueT());
}

template <typename ValueT>
bool concurrent_queue<ValueT>::Push(uint64_t &s, ValueT &data) {
  uint16_t key_int = static_cast<uint16_t>(s);
  AutoTrace trace = AutoTrace("hcl::concurrent_queue::Push(remote)", data);
  return RPC_CALL_WRAPPER("_Push", key_int, bool, data);
}

template <typename ValueT>
std::pair<bool, ValueT> concurrent_queue<ValueT>::Pop(uint64_t &s) {
  uint16_t key_int = static_cast<uint16_t>(s);
  AutoTrace trace = AutoTrace("hcl::concurrent_queue::Pop(remote)");
  typedef std::pair<bool, ValueT> ret_type;
  return RPC_CALL_WRAPPER1("_Pop", key_int, ret_type);
}

}  // namespace hcl