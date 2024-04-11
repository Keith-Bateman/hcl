
#include <hcl/common/constants.h>
#include <hcl/common/debug.h>
#include <hcl/common/macros.h>
#include <hcl/common/singleton.h>
#include <hcl/communication/rpc_lib.h>
#include <mpi.h>
namespace hcl {
#if defined(HCL_COMMUNICATION_ENABLE_THALLIUM)
tl::endpoint RPC::get_endpoint(CharStruct protocol, CharStruct server_name,
                               uint16_t server_port) {
  // We use addr lookup because mercury addresses must be exactly 15 char
  char ip[16];
  struct hostent *he = gethostbyname(server_name.c_str());
  in_addr **addr_list = (struct in_addr **)he->h_addr_list;
  strcpy(ip, inet_ntoa(*addr_list[0]));
  CharStruct lookup_str =
      protocol + "://" + std::string(ip) + ":" + std::to_string(server_port);
  return thallium_client->lookup(lookup_str.c_str());
}
void RPC::init_engine_and_endpoints(CharStruct protocol) {
  thallium_client = hcl::Singleton<tl::engine>::GetInstance(protocol.c_str(),
                                                            MARGO_CLIENT_MODE);
  thallium_endpoints.reserve(server_list.size());
  for (std::vector<CharStruct>::size_type i = 0; i < server_list.size(); ++i) {
    thallium_endpoints.push_back(
        get_endpoint(protocol, server_list[i], server_port + i));
  }
}

/*std::promise<void> thallium_exit_signal;

  void RPC::runThalliumServer(std::future<void> futureObj){

  while(futureObj.wait_for(std::chrono::milliseconds(1)) ==
  std::future_status::timeout){} thallium_engine->wait_for_finalize();
  }*/

#endif
#if defined(HCL_COMMUNICATION_ENABLE_THALLIUM) && \
    defined(HCL_COMMUNICATION_PROTOCOL_ENABLE_VERBS)
template <typename MappedType>
MappedType RPC::prep_rdma_server(tl::endpoint endpoint, tl::bulk &bulk_handle);

template <typename MappedType>
tl::bulk RPC::prep_rdma_client(MappedType &data);
#endif

void RPC::Stop() {
  if (HCL_CONF->IS_SERVER) {
    switch (HCL_CONF->RPC_IMPLEMENTATION) {
#ifdef HCL_COMMUNICATION_ENABLE_RPCLIB
      case RPCLIB: {
        // Twiddle thumbs
        break;
      }
#endif
#ifdef HCL_COMMUNICATION_ENABLE_THALLIUM
      case THALLIUM_TCP:
#endif
#if defined(HCL_COMMUNICATION_ENABLE_THALLIUM) && \
    defined(HCL_COMMUNICATION_PROTOCOL_ENABLE_VERBS)
      case THALLIUM_ROCE:
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
RPC::~RPC() { Stop(); }

RPC::RPC()
    : server_port(HCL_CONF->RPC_PORT), server_list(HCL_CONF->SERVER_LIST) {
  AutoTrace trace = AutoTrace("RPC");
  if (server_list.empty() && HCL_CONF->SERVER_LIST_PATH.size() > 0) {
    server_list = HCL_CONF->LoadServers();
  }
  /* if current rank is a server */
  if (HCL_CONF->IS_SERVER) {
    switch (HCL_CONF->RPC_IMPLEMENTATION) {
#ifdef HCL_COMMUNICATION_ENABLE_RPCLIB
      case RPCLIB: {
        rpclib_server =
            std::make_shared<rpc::server>(server_port + HCL_CONF->MY_SERVER);
        rpclib_server->suppress_exceptions(false);
        break;
      }
#endif
#ifdef HCL_COMMUNICATION_ENABLE_THALLIUM
      case THALLIUM_TCP: {
        engine_init_str = HCL_CONF->TCP_CONF + "://" +
                          HCL_CONF->SERVER_LIST[HCL_CONF->MY_SERVER] + ":" +
                          std::to_string(server_port + HCL_CONF->MY_SERVER);
        break;
      }
#endif
#if defined(HCL_COMMUNICATION_ENABLE_THALLIUM) && \
    defined(HCL_COMMUNICATION_PROTOCOL_ENABLE_VERBS)
      case THALLIUM_ROCE: {
        engine_init_str = HCL_CONF->VERBS_CONF + ";" + HCL_CONF->VERBS_DOMAIN +
                          "://" + HCL_CONF->SERVER_LIST[HCL_CONF->MY_SERVER] +
                          ":" +
                          std::to_string(server_port + HCL_CONF->MY_SERVER);
        break;
      }
#endif
    }
  }
#ifdef HCL_COMMUNICATION_ENABLE_RPCLIB
  for (std::vector<rpc::client>::size_type i = 0; i < server_list.size(); ++i) {
    rpclib_clients.push_back(
        std::make_unique<rpc::client>(server_list[i].c_str(), server_port + i));
  }
#endif
  run(HCL_CONF->RPC_THREADS);
}

void RPC::run(size_t workers) {
  if (workers == 0) workers = HCL_CONF->RPC_THREADS;
  AutoTrace trace = AutoTrace("RPC::run", workers);
  if (HCL_CONF->IS_SERVER) {
    switch (HCL_CONF->RPC_IMPLEMENTATION) {
#ifdef HCL_COMMUNICATION_ENABLE_RPCLIB
      case RPCLIB: {
        rpclib_server->async_run(workers);
        break;
      }
#endif
#ifdef HCL_COMMUNICATION_ENABLE_THALLIUM
      case THALLIUM_TCP:
#endif
#if defined(HCL_COMMUNICATION_ENABLE_THALLIUM) && \
    defined(HCL_COMMUNICATION_PROTOCOL_ENABLE_VERBS)
      case THALLIUM_ROCE:
#endif
#if defined(HCL_COMMUNICATION_ENABLE_THALLIUM)
      {
        thallium_server = hcl::Singleton<tl::engine>::GetInstance(
            engine_init_str.c_str(), THALLIUM_SERVER_MODE, true,
            HCL_CONF->RPC_THREADS);
        break;
      }
#endif
    }
  }
  switch (HCL_CONF->RPC_IMPLEMENTATION) {
#ifdef HCL_COMMUNICATION_ENABLE_RPCLIB
    case RPCLIB: {
      break;
    }
#endif
#ifdef HCL_COMMUNICATION_ENABLE_THALLIUM
    case THALLIUM_TCP: {
      init_engine_and_endpoints(HCL_CONF->TCP_CONF);
      break;
    }
#endif
#if defined(HCL_COMMUNICATION_ENABLE_THALLIUM) && \
    defined(HCL_COMMUNICATION_PROTOCOL_ENABLE_VERBS)
    case THALLIUM_ROCE: {
      init_engine_and_endpoints(HCL_CONF->VERBS_CONF);
      break;
    }
#endif
  }
}



#ifdef HCL_ENABLE_THALLIUM_ROCE
// These are still experimental for using RDMA bulk transfers
template <typename MappedType>
MappedType RPC::prep_rdma_server(tl::endpoint endpoint, tl::bulk &bulk_handle) {
  // MappedType buffer;
  std::string buffer;
  buffer.resize(1000000, 'a');
  std::vector<std::pair<void *, size_t>> segments(1);
  segments[0].first = (void *)(&buffer[0]);
  segments[0].second = 1000000 + 1;
  tl::bulk local = thallium_server->expose(segments, tl::bulk_mode::write_only);
  bulk_handle.on(endpoint) >> local;
  return buffer;
}

template <typename MappedType>
tl::bulk RPC::prep_rdma_client(MappedType &data) {
  MappedType my_buffer = data;
  std::vector<std::pair<void *, std::size_t>> segments(1);
  segments[0].first = (void *)&my_buffer[0];
  segments[0].second = 1000000 + 1;
  return thallium_client->expose(segments, tl::bulk_mode::read_only);
}
#endif


}  // namespace hcl
