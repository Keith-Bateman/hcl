
#include <hcl/common/container.h>
/** Include Headers **/
#include <hcl/common/constants.h>
#include <hcl/common/debug.h>
#include <hcl/common/macros.h>
namespace hcl {
bool Container::is_local(uint16_t &key_int) {
  return key_int == my_server && server_on_node;
}
bool Container::is_local() { return server_on_node; }

template <typename Allocator, typename MappedType, typename SharedType>
typename std::enable_if_t<std::is_same<Allocator, nullptr_t>::value, MappedType>
Container::GetData(MappedType &data) {
  return std::move(data);
}

template <typename Allocator, typename MappedType, typename SharedType>
typename std::enable_if_t<!std::is_same<Allocator, nullptr_t>::value,
                          SharedType>
Container::GetData(MappedType &data) {
  Allocator allocator(segment.get_segment_manager());
  SharedType value(allocator);
  value.assign(data);
  return std::move(value);
}

Container::~Container() {
  if (is_server) boost::interprocess::file_mapping::remove(backed_file.c_str());
}
Container::Container(CharStruct name_, uint16_t port)
    : comm_size(1),
      my_rank(0),
      num_servers(HCL_CONF->NUM_SERVERS),
      my_server(HCL_CONF->MY_SERVER),
      memory_allocated(HCL_CONF->MEMORY_ALLOCATED),
      is_server(HCL_CONF->IS_SERVER),
      segment(),
      name(name_),
      func_prefix(name_),
      backed_file(HCL_CONF->BACKED_FILE_DIR + PATH_SEPARATOR + name_ + "_" +
                  std::to_string(my_server)),
      server_on_node(HCL_CONF->SERVER_ON_NODE) {
  if (port == 0) port = HCL_CONF->RPC_PORT;
  AutoTrace trace = AutoTrace("hcl::container");
  /* Initialize MPI rank and size of world */
  MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  /* create per server name for shared memory. Needed if multiple servers are
     spawned on one node*/
  this->name += "_" + std::to_string(my_server);
  /* if current rank is a server */
  rpc = hcl::Singleton<RPCFactory>::GetInstance()->GetRPC(port);
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
void Container::lock() {
  if (server_on_node || is_server) mutex->lock();
}

void Container::unlock() {
  if (server_on_node || is_server) mutex->unlock();
}
}  // namespace hcl