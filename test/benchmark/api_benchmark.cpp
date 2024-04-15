#include <catch_config.h>
#include <hcl.h>
#include <mpi.h>
#include <unistd.h>
#include <util.h>

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

/**
 * Test data structures
 */
namespace hcl::test {
struct Info {
  int rank;
  int comm_size;
  int num_nodes;
  bool is_server;
  std::shared_ptr<RPC> rpc;

  bool debug_init;
};
struct Arguments {
  // MPI Configurations
  size_t process_per_node = 1;
  // Test Parameters
  std::string server_path = "./server_list";
  int num_request = 8;
  bool debug = false;
};
}  // namespace hcl::test

hcl::test::Arguments args;
hcl::test::Info info;
/**
 * Overridden methods for catch
 */
int configure_hcl(bool is_server_on_node) {
  HCL_CONF->IS_SERVER = info.is_server;
  HCL_CONF->MY_SERVER = info.rank / args.process_per_node;
  HCL_CONF->NUM_SERVERS = info.comm_size / args.process_per_node;
  HCL_CONF->SERVER_ON_NODE = is_server_on_node;
  HCL_CONF->SERVER_LIST_PATH = args.server_path;
  return 0;
}
int catch_init(int* argc, char*** argv) {
  //  fprintf(stdout, "Initializing MPI\n");
  MPI_Init(argc, argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &info.rank);
  MPI_Comm_size(MPI_COMM_WORLD, &info.comm_size);
  info.debug_init = false;

  if (!info.debug_init && args.debug) {
    const int HOSTNAME_SIZE = 256;
    char hostname[HOSTNAME_SIZE];
    gethostname(hostname, HOSTNAME_SIZE);
    int pid = getpid();
    char* start_port_str = getenv("VSC_DEBUG_START_PORT");
    int start_port = 10000;
    if (start_port_str != nullptr) {
      start_port = atoi(start_port_str);
    }
    const char* conf_dir = getenv("VSC_DEBUG_CONF_DIR");
    if (conf_dir == nullptr) {
      conf_dir = ".";
    }
    char conf_file[4096];
    sprintf(conf_file, "%s/debug.conf", conf_dir);

    char exe[1024];
    int ret = readlink("/proc/self/exe", exe, sizeof(exe) - 1);
    REQUIRE(ret != -1);
    exe[ret] = 0;
    if (info.rank == 0) {
      remove(conf_file);
    }
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_File mpi_fh;
    int status_orig = MPI_File_open(MPI_COMM_WORLD, conf_file,
                                    MPI_MODE_WRONLY | MPI_MODE_CREATE,
                                    MPI_INFO_NULL, &mpi_fh);
    REQUIRE(status_orig == MPI_SUCCESS);
    const int buf_len = 16 * 1024;
    char buffer[buf_len];
    int size;
    if (info.rank == 0) {
      size = sprintf(buffer, "%d\n%s:%d:%s:%d:%d\n", info.comm_size, exe,
                     info.rank, hostname, start_port + info.rank, pid);
    } else {
      size = sprintf(buffer, "%s:%d:%s:%d:%d\n", exe, info.rank, hostname,
                     start_port + info.rank, pid);
    }
    MPI_Status status;
    MPI_File_write_ordered(mpi_fh, buffer, size, MPI_CHAR, &status);
    int written_bytes;
    MPI_Get_count(&status, MPI_CHAR, &written_bytes);
    REQUIRE(written_bytes == size);
    MPI_File_close(&mpi_fh);
    MPI_Barrier(MPI_COMM_WORLD);
    if (info.rank == 0) {
      printf("%d ready for attach\n", info.comm_size);
      fflush(stdout);
      sleep(30);
    }
    info.debug_init = true;
  }
  info.is_server = (info.rank + 1) % args.process_per_node == 0;
  info.num_nodes = info.comm_size / args.process_per_node;
  configure_hcl(false);
  HCL_LOG_INFO("Initializing the Catch2 Test with args ppn:%d server_path:%s\n",
               args.process_per_node, args.server_path.c_str());
  info.rpc =
      hcl::Singleton<RPCFactory>::GetInstance()->GetRPC(HCL_CONF->RPC_PORT);
  MPI_Barrier(MPI_COMM_WORLD);
  return 0;
}
int catch_finalize() {
  MPI_Barrier(MPI_COMM_WORLD);
  HCL_LOG_INFO("Finalizing the Catch2 Test \n");
  info.rpc.reset();
  MPI_Finalize();
  return 0;
}
cl::Parser define_options() {
  return cl::Opt(args.server_path, "server_path")["-s"]["--sp"](
             "Path to describe the server list file") |
         cl::Opt(args.process_per_node,
                 "process_per_node")["-p"]["--ppn"]("Processes per node") |
         cl::Opt(args.num_request,
                 "iteration")["-i"]["--iter"]("Number of iterations") |
         cl::Opt(args.debug)["-d"]["--debug"]("debug");
}

int pretest() {
  MPI_Barrier(MPI_COMM_WORLD);
  return 0;
}
int posttest() {
  MPI_Barrier(MPI_COMM_WORLD);
  return 0;
}

#include "unordered_map.cpp"