// save diagnostic state
#pragma GCC diagnostic push

// turn off the specific warning. Can also use "-Wall"
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#pragma GCC diagnostic ignored "-Wpedantic"
#include <thallium.hpp>
#pragma GCC diagnostic pop
namespace tl = thallium;

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <address>" << std::endl;
    exit(0);
  }

  tl::engine myEngine("ucx+rc_verbs://mlx5_0:1/hsi0", THALLIUM_CLIENT_MODE);
  tl::remote_procedure hello = myEngine.define("hello").disable_response();
  tl::endpoint server = myEngine.lookup(argv[1]);
  hello.on(server)();

  return 0;
}