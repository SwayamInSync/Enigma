#pragma once

#include <memory>
#include <atomic>
#include <shared_mutex>
#include <mutex>
#include <variant>
#include <cstdint>
#include <stdexcept>
#include "DataPtr.h"
#include "Storage.h"

namespace enigma::cow
{
  class COWError : public std::runtime_error
  {
    using std::runtime_error::runtime_error;
  };

  class RefCountError : public COWError
  {
  public:
    RefCountError(const std::string &msg) : COWError(msg) {}
    RefCountError(const char *msg) : COWError(msg) {}
  };
  class ReadGuard
  {
    std::shared_lock<std::shared_mutex> lock_;

  public:
    explicit ReadGuard(std::shared_mutex &mutex)
        : lock_(mutex) {}
    ~ReadGuard() = default;
  };

  class WriteGuard
  {
    std::unique_lock<std::shared_mutex> lock_;

  public:
    explicit WriteGuard(std::shared_mutex &mutex)
        : lock_(mutex) {}
    ~WriteGuard() = default;
  };

  using RefCountResult = std::variant<std::shared_lock<std::shared_mutex>, // Still shared
                                      void *>;                             // Last reference

  class COWDeleter
  {
  public:
    static void deleter(DataPtr *d_ptr);
    static uintptr_t identifier()
    {
      return reinterpret_cast<uintptr_t>(&deleter);
    }
  };

  class COWDeleterContext
  {
  private:
    mutable std::shared_mutex mutex_;
    std::atomic<int64_t> refcount_;
    // std::unique_ptr<void, DataPtr::DeleterFn> original_ctx_;
    void *original_ctx_;
    DataPtr::DeleterFn data_deleter_;

    enum class State
    {
      Active,
      PendingDelete,
      Deleted
    };
    std::atomic<State> state_;

  public:
    std::shared_lock<std::shared_mutex> get_shared_lock()
    {
      return std::shared_lock<std::shared_mutex>(mutex_);
    }

    explicit COWDeleterContext(void *ctx, DataPtr::DeleterFn deleter);
    ~COWDeleterContext();

    void increment_refcount(int cnt);
    RefCountResult decrement_refcount();
    int64_t reference_count() const;

    void *get_original_ctx() const;
    DataPtr::DeleterFn get_original_deleter() const;
    std::shared_mutex &mutex() { return mutex_; }

    bool is_active() const;

    // Prevent copying and moving
    COWDeleterContext(const COWDeleterContext &) = delete;
    COWDeleterContext &operator=(const COWDeleterContext &) = delete;
    COWDeleterContext(COWDeleterContext &&) = delete;
    COWDeleterContext &operator=(COWDeleterContext &&) = delete;
  };

  std::unique_ptr<DataPtr> make_cow_data_ptr(
      DataPtr &src_ptr,
      COWDeleterContext &ctx);

  std::unique_ptr<DataPtr> copy_cow_data_ptr(
      DataPtr &src_ptr);

  std::shared_ptr<Storage> lazy_clone_storage(Storage &storage);
  void materialize_cow_storage(Storage &storage);
  bool is_cow_data_ptr(const DataPtr &data_ptr);

} // namespace enigma::cow