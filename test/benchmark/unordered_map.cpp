

#include <array>

int value = 0;

TEMPLATE_TEST_CASE_SIG("unordered_map", "[template]",
                       ((int S, typename K, typename V), S, K, V),
                       (1, int, std::array<int, 1>),
                       (2, int, std::array<int, 1>)) {
  REQUIRE(pretest() == 0);
  typedef K Key;
  typedef V Value;
  MPI_Comm client_comm;
  bool is_client = true;
  int client_comm_size = 1, client_rank = 0;
  if (info.comm_size > 1) {
    MPI_Comm_split(MPI_COMM_WORLD, !info.is_server, info.rank, &client_comm);
    MPI_Comm_size(client_comm, &client_comm_size);
    MPI_Comm_rank(client_comm, &client_rank);
    is_client = !info.is_server;
  } else {
    client_comm = MPI_COMM_WORLD;
  }
  float total_requests = client_comm_size * args.num_request;
  HCL_LOG_INFO("Running Pre Test");

  typedef hcl::unordered_map<Key, Value> MapType;
  std::unordered_map<Key, Value> map = std::unordered_map<Key, Value>();
  if (is_client) {
    hcl::test::Timer put_time = hcl::test::Timer();
    /*Local std::map test*/
    Value v = {10};
    for (int i = 1; i <= args.num_request; i++) {
      Key k = Key(i);
      put_time.resumeTime();
      auto val = map.insert_or_assign(k, v);
      put_time.pauseTime();
      REQUIRE(val.second);
    }
    hcl::test::Timer get_time = hcl::test::Timer();
    /*Local std::map test*/
    for (int i = 1; i <= args.num_request; i++) {
      Key k = Key(i);
      get_time.resumeTime();
      auto iterator = map.find(k);
      get_time.pauseTime();
      REQUIRE(iterator != map.end());
    }
    AGGREGATE_TIME(put, client_comm);
    AGGREGATE_TIME(get, client_comm);
    if (client_rank == 0) {
      printf("stl put throughput: %f\n",
             total_requests / total_put * client_comm_size);
      printf("stl get throughput: %f\n",
             total_requests / total_get * client_comm_size);
    }
  }
  configure_hcl(true);
  std::shared_ptr<MapType> lmap;
  if (info.is_server) {
    lmap = std::make_shared<MapType>("Local" + std::to_string(value));
  }
  MPI_Barrier(MPI_COMM_WORLD);
  if (!info.is_server) {
    lmap = std::make_shared<MapType>("Local" + std::to_string(value));
  }
  if (is_client) {
    hcl::test::Timer put_time = hcl::test::Timer();
    Value v = {10};
    /*Local std::map test*/
    for (int i = 1; i <= args.num_request; i++) {
      Key k = Key(i);
      put_time.resumeTime();
      bool success = lmap->Put(k, v);
      put_time.pauseTime();
      REQUIRE(success);
    }
    hcl::test::Timer get_time = hcl::test::Timer();
    /*Local std::map test*/
    for (int i = 1; i <= args.num_request; i++) {
      Key k = Key(i);
      get_time.resumeTime();
      auto iterator = lmap->Get(k);
      get_time.pauseTime();
      REQUIRE(iterator.first);
    }
    AGGREGATE_TIME(put, client_comm);
    AGGREGATE_TIME(get, client_comm);
    if (client_rank == 0) {
      printf("hcl local put throughput: %f\n",
             total_requests / total_put * client_comm_size);
      printf("hcl local get throughput: %f\n",
             total_requests / total_get * client_comm_size);
    }
  }
  MPI_Barrier(MPI_COMM_WORLD);
  REQUIRE(configure_hcl(false) == 0);
  std::shared_ptr<MapType> rmap;
  if (info.is_server) {
    rmap = std::make_shared<MapType>("Remote" + std::to_string(value));
  }
  MPI_Barrier(MPI_COMM_WORLD);
  if (!info.is_server) {
    rmap = std::make_shared<MapType>("Remote" + std::to_string(value));
  }

  if (is_client) {
    hcl::test::Timer put_time = hcl::test::Timer();
    /*Local std::map test*/
    Value v = {10};
    for (int i = 1; i <= args.num_request; i++) {
      Key k = Key(i);
      HCL_LOG_INFO("Loop for Put %d %d", k, v[0]);
      put_time.resumeTime();
      bool success = rmap->Put(k, v);
      put_time.pauseTime();
      REQUIRE(success);
    }
    hcl::test::Timer get_time = hcl::test::Timer();
    /*Local std::map test*/
    for (int i = 1; i <= args.num_request; i++) {
      Key k = Key(i);
      get_time.resumeTime();
      auto iterator = rmap->Get(k);
      get_time.pauseTime();
      REQUIRE(iterator.first);
    }
    AGGREGATE_TIME(put, client_comm);
    AGGREGATE_TIME(get, client_comm);
    if (client_rank == 0) {
      printf("hcl remote put throughput: %f\n",
             total_requests / total_put * client_comm_size);
      printf("hcl remote get throughput: %f\n",
             total_requests / total_get * client_comm_size);
    }
  }
  MPI_Barrier(MPI_COMM_WORLD);

  HCL_LOG_INFO("Running Post Test");
  REQUIRE(posttest() == 0);
  value++;
}