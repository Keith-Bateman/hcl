#include <hcl/common/container.h>
#include <hcl/hcl_internal.h>
namespace hcl {
bool container::is_local(uint16_t &key_int) {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  return key_int == my_server_idx && server_on_node;
}
bool container::is_local() {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  return server_on_node;
}

container::~container() {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  if (is_server) boost::interprocess::file_mapping::remove(backed_file.c_str());
}
container::container(CharStruct _name, uint16_t _port, uint16_t _num_servers,
                     uint16_t _my_server_idx, really_long _memory_allocated,
                     bool _is_server, bool _is_server_on_node,
                     CharStruct _backed_file_dir)
    : num_servers(_num_servers),
      my_server_idx(_my_server_idx),
      memory_allocated(_memory_allocated),
      is_server(_is_server),
      segment(),
      name(_name),
      func_prefix(_name),
      backed_file(_backed_file_dir + PATH_SEPARATOR + _name + "_" +
                  std::to_string(_my_server_idx)),
      port(_port),
      server_on_node(_is_server_on_node) {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  /* create per server name for shared memory. Needed if multiple servers are
     spawned on one node*/
  this->name += "_" + std::to_string(my_server_idx);
  /* if current rank is a server */
  auto rpc = hcl::HCL::GetInstance(false)->GetRPC(port);
  if (is_server) {
    /* Delete existing instance of shared memory space*/
    boost::interprocess::file_mapping::remove(backed_file.c_str());
    /* allocate new shared memory space */
    segment = boost::interprocess::managed_mapped_file(
        boost::interprocess::create_only, backed_file.c_str(),
        memory_allocated);
    mutex = segment.construct<boost::interprocess::interprocess_mutex>("mtx")();
  } else if (!is_server && server_on_node) {
    /* Map the clients to their respective memory pools */
    segment = boost::interprocess::managed_mapped_file(
        boost::interprocess::open_only, backed_file.c_str());
    std::pair<boost::interprocess::interprocess_mutex *,
              boost::interprocess::managed_mapped_file::size_type>
        res2;
    res2 = segment.find<boost::interprocess::interprocess_mutex>("mtx");
    mutex = res2.first;
  }
}
void container::lock() {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  if (server_on_node || is_server) mutex->lock();
}

void container::unlock() {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  if (server_on_node || is_server) mutex->unlock();
}
}  // namespace hcl