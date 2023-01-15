#include <exception>
#include <filesystem>
#include <iostream>
#include <random>

#include <benchmark/benchmark.h>

#include <SQLiteCpp/SQLiteCpp.h>

namespace {

constexpr auto N = 1024;
constexpr auto SEED = 831;

struct row {
  std::string path;
  int x;
  double y;
  float z;
};

std::vector<row> generateData(auto &randDevice) {
  std::vector<row> result;
  std::uniform_int_distribution<int> someIntDistribution;
  std::uniform_real_distribution<float> someFloatDistribution;
  std::uniform_real_distribution<double> someDoubleDistribution;

  for (size_t i = 0; i < N; ++i) {
    result.push_back({"/Users/family/Devel/vcpkg/scripts/buildsystems",
                      someIntDistribution(randDevice),
                      someDoubleDistribution(randDevice),
                      someFloatDistribution(randDevice)});
  }
  return result;
}

bool validateDb(std::string const &dbPath) {
  SQLite::Database db(dbPath, SQLite::OPEN_READONLY);
  SQLite::Statement statement(db, "select count(*) from test_table");
  while (statement.executeStep()) {
    unsigned sz = statement.getColumn(0);
    if (sz != N)
      return false;
  }
  return true;
}

void BM_singleAddInstruction_noPrepare(benchmark::State &state) {
  std::mt19937_64 device(SEED);
  auto data = generateData(device);

  std::filesystem::path tmp_dir_path{std::filesystem::temp_directory_path() /=
                                     std::tmpnam(nullptr)};
  std::filesystem::create_directories(tmp_dir_path);
  auto dbPath = tmp_dir_path / "test.db";

  for (auto _ : state) {
    try {
      SQLite::Database db(dbPath, SQLite::OPEN_CREATE | SQLite::OPEN_READWRITE);

      db.exec("PRAGMA synchronous = 0;");

      db.exec("begin transaction");

      db.exec(R"sql(
                    create table if not exists test_table (
                        path text not null,
                        x int,
                        y real,
                        z real
                    ) strict
                )sql");

      for (auto &&r : data) {
        SQLite::Statement insertQuery(
            db, "insert into test_table values (?, ?, ?, ?)");

        insertQuery.bind(1, r.path);
        insertQuery.bind(2, r.x);
        insertQuery.bind(3, r.y);
        insertQuery.bind(4, r.z);

        insertQuery.exec();
      }

      db.exec("end transaction");
    } catch (std::exception const &e) {
      std::cerr << "Error occurred: " << e.what() << std::endl;
    }

    assert(validateDb(dbPath));

    std::filesystem::remove(dbPath);
  }
  std::filesystem::remove_all(tmp_dir_path);
}

BENCHMARK(BM_singleAddInstruction_noPrepare);

void BM_singleAddInstruction_preparedInsert(benchmark::State &state) {
  std::mt19937_64 device(SEED);
  auto data = generateData(device);

  std::filesystem::path tmp_dir_path{std::filesystem::temp_directory_path() /=
                                     std::tmpnam(nullptr)};
  std::filesystem::create_directories(tmp_dir_path);
  auto dbPath = tmp_dir_path / "test.db";

  for (auto _ : state) {
    try {
      SQLite::Database db(dbPath, SQLite::OPEN_CREATE | SQLite::OPEN_READWRITE);

      db.exec("PRAGMA synchronous = 0;");

      db.exec("begin transaction");

      db.exec(R"sql(
                    create table if not exists test_table (
                        path text not null,
                        x int,
                        y real,
                        z real
                    ) strict
                )sql");

      SQLite::Statement insertQuery(
          db, "insert into test_table values (?, ?, ?, ?)");

      for (auto &&r : data) {
        insertQuery.bind(1, r.path);
        insertQuery.bind(2, r.x);
        insertQuery.bind(3, r.y);
        insertQuery.bind(4, r.z);

        insertQuery.exec();

        insertQuery.reset();
      }

      db.exec("end transaction");
    } catch (std::exception const &e) {
      std::cerr << "Error occurred: " << e.what() << std::endl;
    }

    assert(validateDb(dbPath));
    std::filesystem::remove(dbPath);
  }
  std::filesystem::remove_all(tmp_dir_path);
}

BENCHMARK(BM_singleAddInstruction_preparedInsert);

#define singleInsert(idx)                                                      \
  do {                                                                         \
    insertQuery.bind(idx * 4 + 1, data[i + idx].path);                         \
    insertQuery.bind(idx * 4 + 2, data[i + idx].x);                            \
    insertQuery.bind(idx * 4 + 3, data[i + idx].y);                            \
    insertQuery.bind(idx * 4 + 4, data[i + idx].z);                            \
  } while (0)

void BM_multiAddInstruction_noPrepare(benchmark::State &state) {
  std::mt19937_64 device(SEED);
  auto data = generateData(device);

  std::filesystem::path tmp_dir_path{std::filesystem::temp_directory_path() /=
                                     std::tmpnam(nullptr)};
  std::filesystem::create_directories(tmp_dir_path);
  auto dbPath = tmp_dir_path / "test.db";

  for (auto _ : state) {
    try {
      SQLite::Database db(dbPath, SQLite::OPEN_CREATE | SQLite::OPEN_READWRITE);

      db.exec("PRAGMA synchronous = 0;");

      db.exec("begin transaction");

      db.exec(R"sql(
                    create table if not exists test_table (
                        path text not null,
                        x int,
                        y real,
                        z real
                    ) strict
                )sql");

      for (size_t i = 0; i + 7 < data.size(); i += 8) {
        SQLite::Statement insertQuery(
            db, "insert into test_table values "
                "(?, ?, ?, ?), (?, ?, ?, ?), (?, ?, ?, ?), "
                "(?, ?, ?, ?), (?, ?, ?, ?), (?, ?, ?, ?), "
                "(?, ?, ?, ?), (?, ?, ?, ?)");

        for (size_t j = 0; j < 8; ++j)
          singleInsert(j);

        insertQuery.exec();
      }

      db.exec("end transaction");
    } catch (std::exception const &e) {
      std::cerr << "Error occurred: " << e.what() << std::endl;
    }

    assert(validateDb(dbPath));
    std::filesystem::remove(dbPath);
  }
  std::filesystem::remove_all(tmp_dir_path);
}

BENCHMARK(BM_multiAddInstruction_noPrepare);

void BM_multiAddInstruction_prepared8(benchmark::State &state) {
  std::mt19937_64 device(SEED);
  auto data = generateData(device);

  std::filesystem::path tmp_dir_path{std::filesystem::temp_directory_path() /=
                                     std::tmpnam(nullptr)};
  std::filesystem::create_directories(tmp_dir_path);
  auto dbPath = tmp_dir_path / "test.db";

  for (auto _ : state) {
    try {
      SQLite::Database db(dbPath, SQLite::OPEN_CREATE | SQLite::OPEN_READWRITE);

      db.exec("PRAGMA synchronous = 0;");

      db.exec("begin transaction");

      db.exec(R"sql(
                    create table if not exists test_table (
                        path text not null,
                        x int,
                        y real,
                        z real
                    ) strict
                )sql");

      SQLite::Statement insertQuery(db,
                                    "insert into test_table values "
                                    "(?, ?, ?, ?), (?, ?, ?, ?), (?, ?, ?, ?), "
                                    "(?, ?, ?, ?), (?, ?, ?, ?), (?, ?, ?, ?), "
                                    "(?, ?, ?, ?), (?, ?, ?, ?)");

      for (size_t i = 0; i + 7 < data.size(); i += 8) {
        for (size_t j = 0; j < 8; ++j)
          singleInsert(j);

        insertQuery.exec();
        insertQuery.reset();
      }

      db.exec("end transaction");
    } catch (std::exception const &e) {
      std::cerr << "Error occurred: " << e.what() << std::endl;
    }

    assert(validateDb(dbPath));
    std::filesystem::remove(dbPath);
  }
  std::filesystem::remove_all(tmp_dir_path);
}

BENCHMARK(BM_multiAddInstruction_prepared8);

void BM_multiAddInstruction_prepared16(benchmark::State &state) {
  std::mt19937_64 device(SEED);
  auto data = generateData(device);

  std::filesystem::path tmp_dir_path{std::filesystem::temp_directory_path() /=
                                     std::tmpnam(nullptr)};
  std::filesystem::create_directories(tmp_dir_path);
  auto dbPath = tmp_dir_path / "test.db";

  for (auto _ : state) {
    try {
      SQLite::Database db(dbPath, SQLite::OPEN_CREATE | SQLite::OPEN_READWRITE);

      db.exec("PRAGMA synchronous = 0;");

      db.exec("begin transaction");

      db.exec(R"sql(
                    create table if not exists test_table (
                        path text not null,
                        x int,
                        y real,
                        z real
                    ) strict
                )sql");

      SQLite::Statement insertQuery(db,
                                    "insert into test_table values "
                                    "(?, ?, ?, ?), (?, ?, ?, ?), (?, ?, ?, ?), "
                                    "(?, ?, ?, ?), (?, ?, ?, ?), (?, ?, ?, ?), "
                                    "(?, ?, ?, ?), (?, ?, ?, ?), (?, ?, ?, ?), "
                                    "(?, ?, ?, ?), (?, ?, ?, ?), (?, ?, ?, ?), "
                                    "(?, ?, ?, ?), (?, ?, ?, ?), (?, ?, ?, ?), "
                                    "(?, ?, ?, ?)");

      for (size_t i = 0; i + 15 < data.size(); i += 16) {
        for (size_t j = 0; j < 16; ++j)
          singleInsert(j);

        insertQuery.exec();
        insertQuery.reset();
      }

      db.exec("end transaction");
    } catch (std::exception const &e) {
      std::cerr << "Error occurred: " << e.what() << std::endl;
    }

    assert(validateDb(dbPath));
    std::filesystem::remove(dbPath);
  }
  std::filesystem::remove_all(tmp_dir_path);
}

BENCHMARK(BM_multiAddInstruction_prepared16);

void BM_multiAddInstruction_prepared32(benchmark::State &state) {
  std::mt19937_64 device(SEED);
  auto data = generateData(device);

  std::filesystem::path tmp_dir_path{std::filesystem::temp_directory_path() /=
                                     std::tmpnam(nullptr)};
  std::filesystem::create_directories(tmp_dir_path);
  auto dbPath = tmp_dir_path / "test.db";

  for (auto _ : state) {
    try {
      SQLite::Database db(dbPath, SQLite::OPEN_CREATE | SQLite::OPEN_READWRITE);

      db.exec("PRAGMA synchronous = 0;");

      db.exec("begin transaction");

      db.exec(R"sql(
                    create table if not exists test_table (
                        path text not null,
                        x int,
                        y real,
                        z real
                    ) strict
                )sql");

      SQLite::Statement insertQuery(db,
                                    "insert into test_table values "
                                    "(?, ?, ?, ?), (?, ?, ?, ?), (?, ?, ?, ?), "
                                    "(?, ?, ?, ?), (?, ?, ?, ?), (?, ?, ?, ?), "
                                    "(?, ?, ?, ?), (?, ?, ?, ?), (?, ?, ?, ?), "
                                    "(?, ?, ?, ?), (?, ?, ?, ?), (?, ?, ?, ?), "
                                    "(?, ?, ?, ?), (?, ?, ?, ?), (?, ?, ?, ?), "
                                    "(?, ?, ?, ?), (?, ?, ?, ?), (?, ?, ?, ?), "
                                    "(?, ?, ?, ?), (?, ?, ?, ?), (?, ?, ?, ?), "
                                    "(?, ?, ?, ?), (?, ?, ?, ?), (?, ?, ?, ?), "
                                    "(?, ?, ?, ?), (?, ?, ?, ?), (?, ?, ?, ?), "
                                    "(?, ?, ?, ?), (?, ?, ?, ?), (?, ?, ?, ?), "
                                    "(?, ?, ?, ?), (?, ?, ?, ?)");

      for (size_t i = 0; i + 31 < data.size(); i += 32) {
        for (size_t j = 0; j < 32; ++j)
          singleInsert(j);

        insertQuery.exec();
        insertQuery.reset();
      }

      db.exec("end transaction");
    } catch (std::exception const &e) {
      std::cerr << "Error occurred: " << e.what() << std::endl;
    }

    assert(validateDb(dbPath));
    std::filesystem::remove(dbPath);
  }
  std::filesystem::remove_all(tmp_dir_path);
}

BENCHMARK(BM_multiAddInstruction_prepared32);
} // namespace

BENCHMARK_MAIN();
