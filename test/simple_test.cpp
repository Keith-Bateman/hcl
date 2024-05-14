/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Distributed under BSD 3-Clause license.                                   *
 * Copyright by The HDF Group.                                               *
 * Copyright by the Illinois Institute of Technology.                        *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of Hermes. The full Hermes copyright notice, including  *
 * terms governing use, modification, and redistribution, is contained in    *
 * the COPYING file, which can be found at the top directory. If you do not  *
 * have access to the file, you may request a copy from help@hdfgroup.org.   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <hcl.h>

int main(int argc, char *argv[]) {
  bool is_server = true;
  if (argc == 1) {
    fprintf(stderr, "call `executable <server>`\n");
    exit(1);
  }
  if (argc > 1) is_server = atoi(argv[1]);
  size_t my_server = 0;
  size_t num_servers = 1;
  bool server_on_node = false;
  auto hcl =
      hcl::HCL::GetInstance(true, 10000, num_servers, my_server, 0, is_server,
                            server_on_node, "", "./server_list");
  auto map = hcl::unordered_map<int, int>();
  if (is_server) {
    fprintf(stderr, "Press any key to exit server\n");
    fflush(stdout);
    getchar();
    fprintf(stderr, "Finished server\n");
  } else {
    for (int i = 0; i < 128; i++) {
      auto result = map.Put(i, i);
      assert(result);
    }
    fprintf(stderr, "Finished client\n");
  }
  hcl->Finalize();
  exit(EXIT_SUCCESS);
}
