#include <cassert>
#include <cstring>
#include "COW.h"
#include "DEBUG.h"

namespace enigma::cow
{

  COWDeleterContext::COWDeleterContext(void *ctx, DataPtr::DeleterFn deleter)
      : refcount_(0), original_ctx_(ctx), data_deleter_(std::move(deleter)), state_(State::Active) {}

  COWDeleterContext::~COWDeleterContext()
  {
    assert(refcount_.load(std::memory_order_relaxed) == 0);
    assert(state_.load(std::memory_order_relaxed) == State::PendingDelete);
    state_.store(State::Deleted, std::memory_order_release);
  }

  void COWDeleterContext::increment_refcount(int cnt = 1)
  {
    refcount_.fetch_add(cnt, std::memory_order_relaxed);
  }

  RefCountResult COWDeleterContext::decrement_refcount()
  {
    auto prev_count = refcount_.fetch_sub(1, std::memory_order_acq_rel);
    if (prev_count == 1)
    {
      // Last reference
      std::unique_lock lock(mutex_);
      state_.store(State::PendingDelete, std::memory_order_release);
      return original_ctx_;
    }

    // Still shared - return read lock
    return std::shared_lock<std::shared_mutex>(mutex_);
  }

  int64_t COWDeleterContext::reference_count() const
  {
    ReadGuard guard(mutex_);
    return refcount_.load(std::memory_order_acquire);
  }

  void *COWDeleterContext::get_original_ctx() const
  {
    ReadGuard guard(mutex_);
    return original_ctx_;
  }

  DataPtr::DeleterFn COWDeleterContext::get_original_deleter() const
  {
    ReadGuard guard(mutex_);
    return data_deleter_;
  }

  bool COWDeleterContext::is_active() const
  {
    return state_.load(std::memory_order_acquire) == State::Active;
  }

  void COWDeleter::deleter(DataPtr *data_ptr)
  {

    if (!data_ptr || !data_ptr->get_context())
      return;

    auto *cow_ctx = static_cast<COWDeleterContext *>(data_ptr->get_context());

    int64_t current_count = cow_ctx->reference_count();

    if (current_count > 0)
    {

      auto result = cow_ctx->decrement_refcount();

      if (auto *original_ctx = std::get_if<void *>(&result))
      {

        auto data_deleter = cow_ctx->get_original_deleter();
        delete cow_ctx;
        data_deleter(data_ptr);
      }
    }
  }

  // Helper functions
  std::unique_ptr<DataPtr> make_cow_data_ptr(DataPtr &src_ptr, COWDeleterContext &ctx)
  {
    if (!ctx.is_active())
    {
      throw COWError("Attempting to use inactive COW context");
    }
    // also increment the ptr_refcount_
    ctx.increment_refcount();
    auto new_ptr = std::make_unique<DataPtr>(
        src_ptr.get(),
        &ctx,
        COWDeleter::deleter,
        src_ptr.device(),
        COWDeleter::identifier());
    return new_ptr;
  }

  std::unique_ptr<DataPtr> copy_cow_data_ptr(DataPtr &src_ptr)
  {
    if (!is_cow_data_ptr(src_ptr))
    {
      throw std::runtime_error("Must be a COW data ptr to make copy");
      return nullptr;
    }

    auto *ctx = static_cast<COWDeleterContext *>(src_ptr.get_context());
    if (!ctx)
    {
      throw COWError("Null context in COW DataPtr");
    }
    return make_cow_data_ptr(src_ptr, *ctx);

    // Convert to COW
    // void *original_ctx = src_ptr.get_context();
    // auto original_deleter = src_ptr.get_deleter();
    // src_ptr.release_context();
    // auto *cow_ctx = new COWDeleterContext(original_ctx, std::move(original_deleter));
    // return make_cow_data_ptr(src_ptr, *cow_ctx);
  }

  bool is_cow_data_ptr(const DataPtr &data_ptr)
  {
    return (data_ptr.get_deleter_id() != DataPtr::INVALID_DELETER_ID) &&
           (data_ptr.get_deleter_id() == COWDeleter::identifier());
  }

  std::shared_ptr<Storage> lazy_clone_storage(Storage &storage)
  {

    auto &data_ptr = storage.data_ptr();
    auto new_storage = std::make_shared<Storage>(0, storage.device());
    if (!is_cow_data_ptr(data_ptr))
    {
      // First conversion to COW

      void *original_ctx = data_ptr.get_context();
      auto original_deleter = data_ptr.get_deleter();
      void *data = data_ptr.get();

      auto *cow_ctx = new COWDeleterContext(original_ctx, std::move(original_deleter));
      cow_ctx->increment_refcount(2); // since orig + laxy_clone

      // Update original and new storage
      data_ptr.set_context(cow_ctx);
      data_ptr.set_deleter(COWDeleter::deleter);
      data_ptr.set_deleter_id(COWDeleter::identifier());

      new_storage->set_size_bytes(storage.size_bytes());
      new_storage->set_data_ptr(std::make_unique<DataPtr>(
          data,    // share same memory location
          cow_ctx, // Share same context
          COWDeleter::deleter,
          storage.device(),
          COWDeleter::identifier()));
    }
    else
    {
      // Already COW, just clone
      new_storage->set_size_bytes(storage.size_bytes());
      new_storage->set_data_ptr(copy_cow_data_ptr(data_ptr));
    }

    return new_storage;
  }

  void materialize_cow_storage(Storage &storage)
  {
    auto &data_ptr = storage.data_ptr();
    if (!is_cow_data_ptr(data_ptr))
      return;

    auto *ctx = static_cast<COWDeleterContext *>(data_ptr.get_context());
    if (!ctx)
    {
      throw COWError("Null context during materialization");
    }

    // Prepare new data under lock
    void *new_data = nullptr;
    {
      auto result = ctx->decrement_refcount(); // (either void or a lock)

      if (auto *original_ctx = std::get_if<void *>(&result))
      {
        // Last reference - take ownership
        auto original_deleter = ctx->get_original_deleter();

        auto new_data_ptr = std::make_unique<DataPtr>(
            data_ptr.get(),
            *original_ctx,
            original_deleter,
            data_ptr.device());
        data_ptr.release_context();
        storage.set_data_ptr(std::move(new_data_ptr));
        delete ctx; // since this is last reference, so delete the ctx
        return;
      }

      // Still shared - make copy under lock
      [[maybe_unused]] auto &lock = std::get<std::shared_lock<std::shared_mutex>>(result);

      new_data = storage.allocator()->allocate(storage.size_bytes());
      std::memcpy(new_data, data_ptr.get(), storage.size_bytes());
    } // Lock is released here

    // Create and set new DataPtr after lock is released
    data_ptr.release_context();
    auto new_data_ptr = std::make_unique<DataPtr>(
        new_data,
        nullptr,
        [allocator = storage.allocator()](DataPtr *p)
        {
          allocator->deallocate(p->get());
        },
        data_ptr.device());
    storage.set_data_ptr(std::move(new_data_ptr));
  }

} // namespace enigma::cow