#include <iostream>
// save diagnostic state
#pragma GCC diagnostic push

// turn off the specific warning. Can also use "-Wall"
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#pragma GCC diagnostic ignored "-Wpedantic"
#include <thallium.hpp>
#pragma GCC diagnostic pop

namespace tl = thallium;

void hello(const tl::request& req) { std::cout << "Hello World!" << std::endl; }

int main(int argc, char** argv) {
  tl::engine myEngine("ucx+rc_verbs://mlx5_0:1/192.168.128.211:9999",
                      THALLIUM_SERVER_MODE);
  myEngine.define("hello", hello).disable_response();
  std::cout << "Server running at address " << myEngine.self() << std::endl;

  return 0;
}
