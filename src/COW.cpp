#include <cassert>
#include <cstring>
#include "COW.h"
#include <iostream>

namespace enigma
{
  namespace cow
  {
    COWDeleterContext::COWDeleterContext(std::unique_ptr<void, DataPtr::DeleterFn> original_ctx) : original_ctx_(std::move(original_ctx)), refcount_(1) {}

    void COWDeleterContext::increment_refcount()
    {
      refcount_.fetch_add(1, std::memory_order_relaxed);
    }

    bool COWDeleterContext::decrement_refcount()
    {
      return refcount_.fetch_sub(1, std::memory_order_acq_rel) == 1;
    }

    void COWDeleter::deleter(void * ctx)
    {
      auto * cow_ctx = static_cast<COWDeleterContext *>(ctx);
      if (cow_ctx->decrement_refcount())
      {
        delete cow_ctx;
      }
    }

    std::unique_ptr<DataPtr> make_cow_data_ptr( DataPtr & src_ptr, COWDeleterContext & ctx)
    {
      ctx.increment_refcount();
      auto new_ptr = std::make_unique<DataPtr>(src_ptr.get(), &ctx, COWDeleter::deleter, src_ptr.device());
      new_ptr->set_deleter_id(COWDeleter::identifier());
      return new_ptr;
    }

    std::unique_ptr<DataPtr> copy_cow_data_ptr(DataPtr& src_ptr)
    {
        if (is_cow_data_ptr(src_ptr)) {
            auto* ctx = static_cast<COWDeleterContext*>(src_ptr.get_context());
            assert(ctx != nullptr);
            ctx->increment_refcount();
            return make_cow_data_ptr(src_ptr, *ctx);
        } else {
            // If it's not a COW DataPtr, create a new COW DataPtr
            auto original_ctx = src_ptr.move_context();
            auto cow_ctx = std::make_unique<COWDeleterContext>(std::move(original_ctx));
            return make_cow_data_ptr(src_ptr, *cow_ctx.release());
        }
    }

    std::shared_ptr<Storage> lazy_clone_storage(Storage & storage)
    {
      auto & data_ptr = storage.data_ptr();
      auto new_storage = std::make_shared<Storage>(storage.size_bytes(), storage.device());

      std::cout << "initial deleter id: " << storage.data_ptr().get_deleter_id() << std::endl;

      if (!is_cow_data_ptr(data_ptr))
      {
        // convert to COW
        auto original_ctx = data_ptr.move_context();
        auto cow_ctx = std::make_unique<COWDeleterContext>(std::move(original_ctx));
        auto new_data_ptr = make_cow_data_ptr(data_ptr, *cow_ctx.release());

        // Update the original storage to use COW
        storage.set_data_ptr(copy_cow_data_ptr(*new_data_ptr));

        // create and return a new Storage with COW
        new_storage->set_data_ptr(std::move(new_data_ptr));
      }
      else
      {
        // already COW so just return a clone
        auto* ctx = static_cast<COWDeleterContext*>(data_ptr.get_context());
        assert(ctx != nullptr);
        ctx->increment_refcount();
        new_storage->set_data_ptr(make_cow_data_ptr(data_ptr, *ctx));
      }

      std::cout << "Final deleter id: " << new_storage->data_ptr().get_deleter_id() << std::endl;
      
      return new_storage;
    }

    void materialize_cow_storage(Storage & storage)
    {
      auto & data_ptr = storage.data_ptr();
      if (!is_cow_data_ptr(data_ptr))
      {
        return;
      }

      auto * ctx = static_cast<COWDeleterContext *>(data_ptr.get_context());
      assert(ctx != nullptr);

      if (ctx->decrement_refcount())
      {
        // Last reference, take ownership of the data
        auto original_deleter = ctx->get_original_deleter();
        auto new_data_ptr = std::make_unique<DataPtr>(data_ptr.get(), ctx->get_original_ctx(), std::move(original_deleter), data_ptr.device());
        storage.set_data_ptr(std::move(new_data_ptr));
        delete ctx;
      }
      else
      {
        // Not the last reference, make a copy
        auto new_data = storage.allocator()->allocate(storage.size_bytes());
        std::memcpy(new_data, data_ptr.get(), storage.size_bytes());
        auto new_data_ptr = std::make_unique<DataPtr>(new_data, nullptr, [allocator = storage.allocator()](void * p){ allocator->deallocate(p); }, data_ptr.device());
        storage.set_data_ptr(std::move(new_data_ptr));
      }
    }

    bool is_cow_data_ptr(const DataPtr & data_ptr)
    {
      return (data_ptr.get_deleter_id() != DataPtr::INVALID_DELETER_ID) && (data_ptr.get_deleter_id() == COWDeleter::identifier());
    }
  }
}