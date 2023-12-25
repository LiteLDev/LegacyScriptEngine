#pragma once
#include <functional>
#include <leveldb/c.h>
#include <leveldb/cache.h>
#include <leveldb/db.h>
#include <leveldb/filter_policy.h>
#include <leveldb/iterator.h>
#include <memory>
#include <optional>
#include <string_view>

class KVDB {
  leveldb::DB *db;
  leveldb::ReadOptions rdopt;
  leveldb::WriteOptions wropt;
  leveldb::Options options;
  leveldb::Status status;
  std::string dbpath;

  void _init(const char *path, bool create, bool read_cache, int cache_sz,
             int Bfilter_bit);

public:
  static std::unique_ptr<KVDB> create(const std::string &path,
                                      bool read_cache = true, int cache_sz = 0,
                                      int Bfilter_bit = 0);
  static std::unique_ptr<KVDB> open(const std::string &path, bool create = true,
                                    bool read_cache = true, int cache_sz = 0,
                                    int Bfilter_bit = 0);
  ~KVDB();

  KVDB() = default;
  KVDB(KVDB const &) = delete;
  KVDB &operator=(KVDB const &) = delete;

  bool get(std::string_view key, std::string &val);
  std::optional<std::string> get(std::string_view key);
  bool set(std::string_view key, std::string_view val);
  bool del(std::string_view key);
  void iter(std::function<bool(std::string_view key,
                               std::string_view val)> const &fn);
  void iter(std::function<bool(std::string_view key)> const &);
  std::string error(leveldb::Status status);
  std::vector<std::string> getAllKeys();
  bool isValid();
  operator bool();

  // For Compatibility
  inline bool put(std::string_view key, std::string_view val) {
    return set(key, val);
  }
};