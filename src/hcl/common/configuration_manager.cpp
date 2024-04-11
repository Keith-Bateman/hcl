#include <hcl/common/configuration_manager.h>
#include <hcl/common/debug.h>
#include <hcl/common/singleton.h>

namespace hcl {
ConfigurationManager::ConfigurationManager()
    : RPC_PORT(9000),
      RPC_THREADS(1),
#if defined(HCL_COMMUNICATION_ENABLE_RPCLIB)
      RPC_IMPLEMENTATION(RPCLIB),
#elif defined(HCL_COMMUNICATION_ENABLE_THALLIUM)
      RPC_IMPLEMENTATION(THALLIUM_TCP),
#elif defined(HCL_COMMUNICATION_ENABLE_THALLIUM) && \
    defined(HCL_COMMUNICATION_PROTOCOL_ENABLE_VERBS)
      RPC_IMPLEMENTATION(THALLIUM_ROCE),
#endif
      TCP_CONF("ofi+sockets"),
      VERBS_CONF("ofi-verbs"),
      VERBS_DOMAIN("mlx5_0"),
      MEMORY_ALLOCATED(1024ULL * 1024ULL * 128ULL),
      IS_SERVER(false),
      MY_SERVER(0),
      NUM_SERVERS(1),
      SERVER_ON_NODE(true),
      SERVER_LIST_PATH(""),
      SERVER_LIST(),
      BACKED_FILE_DIR("/dev/shm"),
      DYN_CONFIG(false) {
  AutoTrace trace = AutoTrace("ConfigurationManager");
}

std::vector<CharStruct> ConfigurationManager::LoadServers() {
  file_load.lock();
  SERVER_LIST = std::vector<CharStruct>();
  fstream file;
  file.open(SERVER_LIST_PATH.c_str(), ios::in);
  if (file.is_open()) {
    std::string file_line;

    int count;
    while (getline(file, file_line)) {
      CharStruct server_node_name;
      if (!file_line.empty()) {
        auto split_loc = file_line.find(":");
        auto split_loc2 = file_line.find(':');  // split to node and net
        // int split_loc = file_line.find(" slots=");
        // int split_loc2 = file_line.find('='); // split to node and net
        if (split_loc != std::string::npos) {
          server_node_name = file_line.substr(0, split_loc);
          count =
              atoi(file_line.substr(split_loc2 + 1, std::string::npos).c_str());
        } else {
          // no special network
          server_node_name = file_line;
          count = 1;
        }
        // server list is list of network interfaces
        for (int i = 0; i < count; ++i) {
          SERVER_LIST.emplace_back(server_node_name);
        }
      }
    }
  } else {
    printf("Error: Can't open server list file %s\n", SERVER_LIST_PATH.c_str());
  }
  NUM_SERVERS = SERVER_LIST.size();
  file.close();
  file_load.unlock();
  return SERVER_LIST;
}

void ConfigurationManager::ConfigureDefaultClient(
    std::string server_list_path) {
  if (server_list_path != "") SERVER_LIST_PATH = server_list_path;
  LoadServers();
  IS_SERVER = false;
  MY_SERVER = false;
  SERVER_ON_NODE = false;
}
void ConfigurationManager::ConfigureDefaultServer(
    std::string server_list_path) {
  if (server_list_path != "") SERVER_LIST_PATH = server_list_path;
  LoadServers();
  IS_SERVER = true;
  MY_SERVER = false;
  SERVER_ON_NODE = true;
}
}  // namespace hcl