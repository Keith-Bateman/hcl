

#include <array>

TEMPLATE_TEST_CASE_SIG("map", "[map]",
                       ((int S, typename K, typename V), S, K, V),
                       (1, int, std::array<int, 1>),
                       (2, int, std::array<int, 4096>),
                       (3, int, std::array<int, 16 * 1024>),
                       (4, int, bip::vector<int>)) {
  HCL_LOG_INFO("Starting Test %d", info.test_count + 1);
  REQUIRE(pretest() == 0);
  typedef K Key;
  typedef V Value;

  float total_requests = info.client_comm_size * args.num_request;
  typedef hcl::map<Key, Value> MapType;
  HCL_LOG_INFO("Ran Pre Test %d", info.test_count + 1);
  ;
  SECTION("stl") {
    auto type = std::map<Key, Value>();
    if (info.is_client) {
      hcl::test::Timer put_time = hcl::test::Timer();

      Value v = {10};
      for (int i = 1; i <= args.num_request; i++) {
        Key k = Key(i);
        put_time.resumeTime();
        auto val = type.insert_or_assign(k, v);
        put_time.pauseTime();
        REQUIRE(val.second);
      }
      hcl::test::Timer get_time = hcl::test::Timer();

      for (int i = 1; i <= args.num_request; i++) {
        Key k = Key(i);
        get_time.resumeTime();
        auto iterator = type.find(k);
        get_time.pauseTime();
        REQUIRE(iterator != type.end());
      }
      AGGREGATE_TIME(put, info.client_comm);
      AGGREGATE_TIME(get, info.client_comm);
      if (info.client_rank == 0) {
        HCL_LOG_PRINT("stl put throughput: %f\n",
                      total_requests / total_put * info.client_comm_size);
        HCL_LOG_PRINT("stl get throughput: %f\n",
                      total_requests / total_get * info.client_comm_size);
      }
    }
  }
  SECTION("local") {
    configure_hcl(true);
    std::shared_ptr<MapType> type;
    if (info.is_server) {
      type =
          std::make_shared<MapType>("Local" + std::to_string(info.test_count));
    }
#ifndef DISABLE_MPI
    MPI_Barrier(MPI_COMM_WORLD);
    if (!info.is_server) {
      type =
          std::make_shared<MapType>("Local" + std::to_string(info.test_count));
    }
#endif
    if (info.is_client) {
      hcl::test::Timer put_time = hcl::test::Timer();
      Value v = {10};

      for (int i = 1; i <= args.num_request; i++) {
        Key k = Key(i);
        put_time.resumeTime();
        bool success = type->Put(k, v);
        put_time.pauseTime();
        REQUIRE(success);
      }
      hcl::test::Timer get_time = hcl::test::Timer();

      for (int i = 1; i <= args.num_request; i++) {
        Key k = Key(i);
        get_time.resumeTime();
        auto iterator = type->Get(k);
        get_time.pauseTime();
        REQUIRE(iterator.first);
      }
      AGGREGATE_TIME(put, info.client_comm);
      AGGREGATE_TIME(get, info.client_comm);
      if (info.client_rank == 0) {
        HCL_LOG_PRINT("hcl local put throughput: %f\n",
                      total_requests / total_put * info.client_comm_size);
        HCL_LOG_PRINT("hcl local get throughput: %f\n",
                      total_requests / total_get * info.client_comm_size);
      }
    }
#ifndef DISABLE_MPI
    MPI_Barrier(MPI_COMM_WORLD);
#endif
  }
  SECTION("remote") {
    REQUIRE(configure_hcl(false) == 0);
    std::shared_ptr<MapType> type;
    if (info.is_server) {
      type =
          std::make_shared<MapType>("Remote" + std::to_string(info.test_count));
    }
#ifndef DISABLE_MPI
    MPI_Barrier(MPI_COMM_WORLD);
    if (!info.is_server) {
      type =
          std::make_shared<MapType>("Remote" + std::to_string(info.test_count));
    }
#endif
    if (info.is_client) {
      hcl::test::Timer put_time = hcl::test::Timer();

      Value v = {10};
      for (int i = 1; i <= args.num_request; i++) {
        Key k = Key(i);
        HCL_LOG_DEBUG("Loop for Put %d %d", k, v[0]);
        put_time.resumeTime();
        bool success = type->Put(k, v);
        put_time.pauseTime();
        REQUIRE(success);
      }
      hcl::test::Timer get_time = hcl::test::Timer();

      for (int i = 1; i <= args.num_request; i++) {
        Key k = Key(i);
        get_time.resumeTime();
        auto iterator = type->Get(k);
        get_time.pauseTime();
        REQUIRE(iterator.first);
      }
      AGGREGATE_TIME(put, info.client_comm);
      AGGREGATE_TIME(get, info.client_comm);
      if (info.client_rank == 0) {
        HCL_LOG_PRINT("hcl remote put throughput: %f\n",
                      total_requests / total_put * info.client_comm_size);
        HCL_LOG_PRINT("hcl remote get throughput: %f\n",
                      total_requests / total_get * info.client_comm_size);
      }
    }
#ifndef DISABLE_MPI
    MPI_Barrier(MPI_COMM_WORLD);
#endif
  }
  HCL_LOG_INFO("Running Post %d", info.test_count + 1);
  REQUIRE(posttest() == 0);
  info.test_count++;
}