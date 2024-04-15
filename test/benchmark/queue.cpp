

#include <array>

TEMPLATE_TEST_CASE_SIG("queue", "[queue]", ((int S, typename K), S, K),
                       (1, int), (2, float), (3, char)) {
  HCL_LOG_INFO("Starting Test %d", info.test_count + 1);
  REQUIRE(pretest() == 0);
  typedef K Key;

  float total_requests = info.client_comm_size * args.num_request;
  typedef hcl::queue<Key> Type;
  HCL_LOG_INFO("Ran Pre Test");
  SECTION("stl") {
    std::queue<Key> type = std::queue<Key>();
    if (info.is_client) {
      hcl::test::Timer put_time = hcl::test::Timer();

      for (int i = 1; i <= args.num_request; i++) {
        Key k = Key(i);
        put_time.resumeTime();
        type.push(k);
        put_time.pauseTime();
      }
      hcl::test::Timer get_time = hcl::test::Timer();

      for (int i = 1; i <= args.num_request; i++) {
        get_time.resumeTime();
        type.pop();
        get_time.pauseTime();
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
#ifndef DISABLE_MPI
    MPI_Barrier(MPI_COMM_WORLD);
    if (!info.is_server) {
      type = std::make_shared<Type>("Local" + std::to_string(info.test_count));
    }
#endif
    if (info.is_client) {
      hcl::test::Timer put_time = hcl::test::Timer();
      for (int i = 1; i <= args.num_request; i++) {
        Key k = Key(i);
        uint16_t my_server_key = 0;
        put_time.resumeTime();
        bool success = type->Push(k, my_server_key);
        put_time.pauseTime();
        REQUIRE(success);
      }
      hcl::test::Timer get_time = hcl::test::Timer();
      for (int i = 1; i <= args.num_request; i++) {
        uint16_t my_server_key = 0;
        get_time.resumeTime();
        auto iterator = type->Pop(my_server_key);
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
    std::shared_ptr<Type> type;
    if (info.is_server) {
      type = std::make_shared<Type>("Remote" + std::to_string(info.test_count));
    }
#ifndef DISABLE_MPI
    MPI_Barrier(MPI_COMM_WORLD);
    if (!info.is_server) {
      type = std::make_shared<Type>("Remote" + std::to_string(info.test_count));
    }
#endif
    if (info.is_client) {
      hcl::test::Timer put_time = hcl::test::Timer();

      for (int i = 1; i <= args.num_request; i++) {
        Key k = Key(i);
        uint16_t my_server_key = 0;
        put_time.resumeTime();
        bool success = type->Push(k, my_server_key);
        put_time.pauseTime();
        REQUIRE(success);
      }
      hcl::test::Timer get_time = hcl::test::Timer();

      for (int i = 1; i <= args.num_request; i++) {
        uint16_t my_server_key = 0;
        get_time.resumeTime();
        auto iterator = type->Pop(my_server_key);
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
  HCL_LOG_INFO("Running Post Test");
  REQUIRE(posttest() == 0);
  info.test_count++;
}