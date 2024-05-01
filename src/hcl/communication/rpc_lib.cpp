#include <hcl/communication/rpc_lib.h>

#if defined(HCL_COMMUNICATION_ENABLE_THALLIUM)
tl::endpoint RPC::get_endpoint(URI server_uri) {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  // // We use addr lookup because mercury addresses must be exactly 15 char
  // char ip[16];
  // struct hostent *he = gethostbyname(_server_name.c_str());
  // in_addr **addr_list = (struct in_addr **)he->h_addr_list;
  // strcpy(ip, inet_ntoa(*addr_list[0]));
  return thallium_client->lookup(server_uri.client_uri.c_str());
}
void RPC::init_engine_and_endpoints() {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  thallium_client = hcl::Singleton<tl::engine>::GetInstance(
      uris[my_server_index].endpoint_uri.c_str(), MARGO_CLIENT_MODE);
  auto total_servers = uris.size();
  thallium_endpoints.reserve(total_servers);
  for (std::vector<CharStruct>::size_type i = 0; i < total_servers; ++i) {
    thallium_endpoints.push_back(get_endpoint(uris[i]));
  }
}
#endif

void RPC::Stop() {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  if (HCL_CONF->IS_SERVER) {
    switch (HCL_CONF->RPC_IMPLEMENTATION) {
#ifdef HCL_COMMUNICATION_ENABLE_THALLIUM
      case THALLIUM_TCP:
#endif
#if defined(HCL_COMMUNICATION_ENABLE_THALLIUM)
      {
        // Mercury addresses in endpoints must be freed before
        // finalizing Thallium
        thallium_endpoints.clear();
        thallium_server->finalize();
        break;
      }
#endif
    }
  }
}

RPC::~RPC() {
  HCL_LOG_TRACE();
  Stop();
}

RPC::RPC(bool _is_server, uint16_t _my_server_index, size_t _threads,
         std::vector<URI> _uris)
    : is_server(_is_server),
      my_server_index(_my_server_index),
      threads(_threads),
      uris(_uris) {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  /* if current rank is a server */
  if (is_server) {
    switch (HCL_CONF->RPC_IMPLEMENTATION) {
#ifdef HCL_COMMUNICATION_ENABLE_THALLIUM
      case THALLIUM_TCP: {
        engine_init_str = uris[my_server_index].server_uri;
        break;
      }
#endif
    }
  }
  run();
}

void RPC::run() {
  HCL_LOG_TRACE();
  HCL_CPP_FUNCTION()
  if (is_server) {
    switch (HCL_CONF->RPC_IMPLEMENTATION) {
#ifdef HCL_COMMUNICATION_ENABLE_THALLIUM
      case THALLIUM_TCP:
#endif
#if defined(HCL_COMMUNICATION_ENABLE_THALLIUM)
      {
        thallium_server = hcl::Singleton<tl::engine>::GetInstance(
            engine_init_str.c_str(), THALLIUM_SERVER_MODE, true, threads);
        HCL_LOG_INFO("Running server on URI %s\n", engine_init_str.c_str());
        break;
      }
#endif
    }
  }
  switch (HCL_CONF->RPC_IMPLEMENTATION) {
#ifdef HCL_COMMUNICATION_ENABLE_THALLIUM
    case THALLIUM_TCP: {
      HCL_LOG_INFO("Will run client with end_point URI %s and client URI %s\n",
                   uris[my_server_index].endpoint_uri.c_str(),
                   uris[my_server_index].client_uri.c_str());
      init_engine_and_endpoints();
      break;
    }
#endif
  }
}
