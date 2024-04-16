#include <catch_config.h>
#include <hcl.h>
#ifndef DISABLE_MPI
#include <mpi.h>
#endif
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
  int test_count;
  int rank;
  int comm_size;
  int num_nodes;
  std::shared_ptr<hcl::HCL> hcl;
#ifndef DISABLE_MPI
  /*Client Info*/
  MPI_Comm client_comm;
#endif
  int client_comm_size, client_rank;
  bool is_server;
  bool is_client;
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
#ifndef DISABLE_MPI
  MPI_Init(argc, argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &info.rank);
  MPI_Comm_size(MPI_COMM_WORLD, &info.comm_size);
#else
  info.rank = 0;
  info.comm_size = 1;
#endif
  info.debug_init = false;
#ifndef DISABLE_MPI
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
#endif
  info.is_server = (info.rank + 1) % args.process_per_node == 0;
  info.num_nodes = info.comm_size / args.process_per_node;
  info.is_client = true;
  info.client_comm_size = 1;
  info.client_rank = 0;
#ifndef DISABLE_MPI
  if (info.comm_size > 1) {
    MPI_Comm_split(MPI_COMM_WORLD, !info.is_server, info.rank,
                   &info.client_comm);
    MPI_Comm_size(info.client_comm, &info.client_comm_size);
    MPI_Comm_rank(info.client_comm, &info.client_rank);
    info.is_client = !info.is_server;
  } else {
    info.client_comm = MPI_COMM_WORLD;
  }
#else
  info.is_client = true;
#endif

  configure_hcl(false);
  HCL_LOG_INFO("Initializing the Catch2 Test with args ppn:%d server_path:%s\n",
               args.process_per_node, args.server_path.c_str());
  info.hcl = hcl::HCL::GetInstance(true);
#ifndef DISABLE_MPI
  MPI_Barrier(MPI_COMM_WORLD);
#endif
  return 0;
}
int catch_finalize() {
#ifndef DISABLE_MPI
  MPI_Barrier(MPI_COMM_WORLD);
#endif
  HCL_LOG_INFO("Finalizing the %s Test \n", "Catch2");
  info.hcl.reset();
#ifndef DISABLE_MPI
  MPI_Finalize();
#endif
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
#ifndef DISABLE_MPI
  MPI_Barrier(MPI_COMM_WORLD);
#endif
  return 0;
}
int posttest() {
#ifndef DISABLE_MPI
  MPI_Barrier(MPI_COMM_WORLD);
#endif
  return 0;
}

#include "map.cpp"
#include "multimap.cpp"
#include "priority_queue.cpp"
#include "queue.cpp"
#include "set.cpp"
#include "unordered_map.cpp"
#include "unordered_map_string.cpp"