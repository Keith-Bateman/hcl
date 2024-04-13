#include <iostream>
#include <thallium.hpp>

namespace tl = thallium;

void hello(const tl::request& req) { std::cout << "Hello World!" << std::endl; }

int main(int argc, char** argv) {
  tl::engine myEngine("ucx+rc_verbs://mlx5_0:1/192.168.128.211:9999", THALLIUM_SERVER_MODE);
  myEngine.define("hello", hello).disable_response();
  std::cout << "Server running at address " << myEngine.self() << std::endl;

  return 0;
}
