

#include <array>

TEMPLATE_TEST_CASE_SIG("unordered_map_string", "[unordered_map_string]",
                       ((int I, typename K, int S), I, K, S), (1, int, 1),
                       (2, int, 4096), (3, int, 16 * 1024)) {
  HCL_LOG_INFO("Starting Test %d", info.test_count + 1);
  REQUIRE(pretest() == 0);
  typedef K Key;

  typedef boost::interprocess::allocator<
      char, boost::interprocess::managed_mapped_file::segment_manager>
      CharAllocator;
  typedef bip::basic_string<char, std::char_traits<char>, CharAllocator>
      MappedUnitString;

  float total_requests = info.client_comm_size * args.num_request;
  typedef hcl::unordered_map<Key, std::string, std::hash<Key>, CharAllocator,
                             MappedUnitString>
      MapType;
  HCL_LOG_INFO("Ran Pre Test");
  SECTION("stl") {
    std::unordered_map<KeyType, std::string> map =
        std::unordered_map<KeyType, std::string>();
    if (info.is_client) {
      hcl::test::Timer put_time = hcl::test::Timer();

      std::string v(S, 'x');
      for (int i = 1; i <= args.num_request; i++) {
        Key k = Key(i);
        put_time.resumeTime();
        auto val = map.insert_or_assign(k, v);
        put_time.pauseTime();
        REQUIRE(val.second);
      }
      hcl::test::Timer get_time = hcl::test::Timer();

      for (int i = 1; i <= args.num_request; i++) {
        Key k = Key(i);
        get_time.resumeTime();
        auto iterator = map.find(k);
        get_time.pauseTime();
        REQUIRE(iterator != map.end());
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
    std::shared_ptr<MapType> lmap;
    if (info.is_server) {
      lmap =
          std::make_shared<MapType>("Local" + std::to_string(info.test_count));
    }
#ifndef DISABLE_MPI
    MPI_Barrier(MPI_COMM_WORLD);
    if (!info.is_server) {
      lmap =
          std::make_shared<MapType>("Local" + std::to_string(info.test_count));
    }
#endif
    if (info.is_client) {
      hcl::test::Timer put_time = hcl::test::Timer();
      std::string v(S, 'x');

      for (int i = 1; i <= args.num_request; i++) {
        Key k = Key(i);
        put_time.resumeTime();
        bool success = lmap->Put(k, v);
        put_time.pauseTime();
        REQUIRE(success);
      }
      hcl::test::Timer get_time = hcl::test::Timer();

      for (int i = 1; i <= args.num_request; i++) {
        Key k = Key(i);
        get_time.resumeTime();
        auto iterator = lmap->Get(k);
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
    std::shared_ptr<MapType> rmap;
    if (info.is_server) {
      rmap =
          std::make_shared<MapType>("Remote" + std::to_string(info.test_count));
    }
#ifndef DISABLE_MPI
    MPI_Barrier(MPI_COMM_WORLD);
    if (!info.is_server) {
      rmap =
          std::make_shared<MapType>("Remote" + std::to_string(info.test_count));
    }
#endif
    if (info.is_client) {
      hcl::test::Timer put_time = hcl::test::Timer();

      std::string v(S, 'x');
      for (int i = 1; i <= args.num_request; i++) {
        Key k = Key(i);
        HCL_LOG_DEBUG("Loop for Put %d %d", k, v[0]);
        put_time.resumeTime();
        bool success = rmap->Put(k, v);
        put_time.pauseTime();
        REQUIRE(success);
      }
      hcl::test::Timer get_time = hcl::test::Timer();

      for (int i = 1; i <= args.num_request; i++) {
        Key k = Key(i);
        get_time.resumeTime();
        auto iterator = rmap->Get(k);
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