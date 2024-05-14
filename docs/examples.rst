****************
Example Programs
****************

The example shows how to use HCL data structures. 
The template can be used for any data structure supported by HCL.

.. code-block:: c
   :linenos:

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


The test shows an end to end usage of hcl within an application.
To run this test, we have two components server and client which can switched using args.

To run the server 

.. code-block:: Bash
    
    simple_test 1

This will start a server and host the hcl data structure on that process.

Then run a client which can access this data structure,

.. code-block:: Bash
    
    simple_test 0