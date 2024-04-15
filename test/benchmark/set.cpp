

#include <array>

TEMPLATE_TEST_CASE_SIG("set", "[set]", ((int S, typename K), S, K), (1, int),
                       (2, float), (3, char)) {
  HCL_LOG_INFO("Starting Test %d", info.test_count + 1);
  REQUIRE(pretest() == 0);
  typedef K Key;

  float total_requests = info.client_comm_size * args.num_request;
  typedef hcl::set<Key> Type;
  HCL_LOG_INFO("Ran Pre Test");
  SECTION("stl") {
    std::set<Key> type = std::set<Key>();
    if (info.is_client) {
      hcl::test::Timer put_time = hcl::test::Timer();

      for (int i = 1; i <= args.num_request; i++) {
        Key k = Key(i);
        put_time.resumeTime();
        auto val = type.insert(k);
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
    std::shared_ptr<Type> type;
    if (info.is_server) {
      type = std::make_shared<Type>("Local" + std::to_string(info.test_count));
    }
    MPI_Barrier(MPI_COMM_WORLD);
    if (!info.is_server) {
      type = std::make_shared<Type>("Local" + std::to_string(info.test_count));
    }
    if (info.is_client) {
      hcl::test::Timer put_time = hcl::test::Timer();

      for (int i = 1; i <= args.num_request; i++) {
        Key k = Key(i);
        put_time.resumeTime();
        bool success = type->Put(k);
        put_time.pauseTime();
        REQUIRE(success);
      }
      hcl::test::Timer get_time = hcl::test::Timer();

      for (int i = 1; i <= args.num_request; i++) {
        Key k = Key(i);
        get_time.resumeTime();
        auto iterator = type->Get(k);
        get_time.pauseTime();
        REQUIRE(iterator);
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
    MPI_Barrier(MPI_COMM_WORLD);
  }
  SECTION("remote") {
    REQUIRE(configure_hcl(false) == 0);
    std::shared_ptr<Type> type;
    if (info.is_server) {
      type = std::make_shared<Type>("Remote" + std::to_string(info.test_count));
    }
    MPI_Barrier(MPI_COMM_WORLD);
    if (!info.is_server) {
      type = std::make_shared<Type>("Remote" + std::to_string(info.test_count));
    }

    if (info.is_client) {
      hcl::test::Timer put_time = hcl::test::Timer();
      for (int i = 1; i <= args.num_request; i++) {
        Key k = Key(i);
        put_time.resumeTime();
        bool success = type->Put(k);
        put_time.pauseTime();
        REQUIRE(success);
      }
      hcl::test::Timer get_time = hcl::test::Timer();
      for (int i = 1; i <= args.num_request; i++) {
        Key k = Key(i);
        get_time.resumeTime();
        auto iterator = type->Get(k);
        get_time.pauseTime();
        REQUIRE(iterator);
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
    MPI_Barrier(MPI_COMM_WORLD);
  }
  HCL_LOG_INFO("Running Post Test");
  REQUIRE(posttest() == 0);
  info.test_count++;
}