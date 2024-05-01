#ifndef INCLUDE_HCL_INTERNAL_H_
#define INCLUDE_HCL_INTERNAL_H_

#include <cstdint>
#include <hcl/hcl_config.hpp>
/*Internal*/
#include <hcl/common/logging.h>
#include <hcl/common/profiler.h>
#include <hcl/communication/rpc_lib.h>
/*Standard*/
#include <stdint.h>

#include <memory>
#include <stdexcept>
#include <unordered_map>

namespace hcl {

class HCL {
 private:
  boost::mutex file_load;
  std::shared_ptr<hcl ::ConfigurationManager> conf;

  // port. rpc
  std::unordered_map<uint16_t, std::shared_ptr<RPC>> rpcs;

  static std::shared_ptr<HCL> instance;

  std::vector<URI> LoadURI(uint16_t _port, CharStruct _server_list_path,
                           CharStruct _uri, int16_t _my_server_idx,
                           bool _is_server);
  int ConfigureInternalEnv();
  int ConfigureInternal(bool initialize, uint16_t _port, uint16_t _num_servers,
                        int16_t _my_server_idx, really_long _memory_allocated,
                        int16_t _is_server, int16_t _is_server_on_node,
                        CharStruct _backed_file_dir,
                        CharStruct _server_list_path, CharStruct _uri);
  int CreateRPC(uint16_t server_port, std::vector<URI>& uris);

 public:
  HCL(uint16_t _port, uint16_t _num_servers, int16_t _my_server_idx,
      really_long _memory_allocated, int16_t _is_server,
      int16_t _is_server_on_node, CharStruct _backed_file_dir,
      CharStruct _server_list_path, CharStruct _uri); /* default constructor. */
  HCL(const HCL&) = delete; /* deleting copy constructor. */

  int Finalize();

  static std::shared_ptr<HCL> GetInstance(
      bool initialize = true, uint16_t _port = 0, uint16_t _num_servers = 0,
      int32_t _my_server_idx = -1, really_long _memory_allocated = 0,
      int16_t _is_server = -1, int16_t _is_server_on_node = -1,
      CharStruct _backed_file_dir = "", CharStruct _server_list_path = "",
      CharStruct _uri = "") {
    if (instance == nullptr) {
      HCL_LOG_TRACE();
      HCL_CPP_FUNCTION()
      if (initialize) {
        instance = std::make_shared<HCL>(
            _port, _num_servers, _my_server_idx, _memory_allocated, _is_server,
            _is_server_on_node, _backed_file_dir, _server_list_path, _uri);
      } else {
        throw new std::logic_error("HCL was not initialized");
      }
    }
    return instance;
  }
  int ReConfigure(uint16_t _port = 0, uint16_t _num_servers = 0,
                  int32_t _my_server_idx = -1,
                  really_long _memory_allocated = 0, int16_t _is_server = -1,
                  int16_t _is_server_on_node = -1,
                  CharStruct _backed_file_dir = "",
                  CharStruct _server_list_path = "", CharStruct _uri = "");
  std::shared_ptr<RPC> GetRPC(uint16_t server_port);
};

}  // namespace hcl

#endif  // INCLUDE_HCL_INTERNAL_H_
