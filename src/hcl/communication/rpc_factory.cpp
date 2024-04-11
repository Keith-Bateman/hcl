#include <hcl/common/macros.h>
#include <hcl/communication/rpc_factory.h>
namespace hcl {
std::shared_ptr<RPC> RPCFactory::GetRPC(uint16_t server_port) {
  auto iter = rpcs.find(server_port);
  if (iter != rpcs.end()) return iter->second;
  auto temp = HCL_CONF->RPC_PORT;
  HCL_CONF->RPC_PORT = server_port;
  auto rpc = std::make_shared<RPC>();
  rpcs.emplace(server_port, rpc);
  HCL_CONF->RPC_PORT = temp;
  return rpc;
}
}  // namespace hcl
