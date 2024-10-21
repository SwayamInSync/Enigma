#pragma once

#include <memory>
#include <atomic>
#include <cstdint>
#include "DataPtr.h"
#include "Storage.h"

namespace enigma
{
  namespace cow
  {
    class COWDeleter
    {
      public:
        static void deleter(void* ctx);
        static uintptr_t identifier() 
        {
            return reinterpret_cast<uintptr_t>(&deleter);
        }
    };

    class COWDeleterContext
    {
      private:
        std::unique_ptr<void, DataPtr::DeleterFn> original_ctx_;
        std::atomic<int> refcount_;

      public:
        COWDeleterContext(std::unique_ptr<void, DataPtr::DeleterFn> original_ctx);
        void increment_refcount();
        bool decrement_refcount();
        void* get_original_ctx() const { return original_ctx_.get(); }
        DataPtr::DeleterFn get_original_deleter() const { return original_ctx_.get_deleter(); }
    };

    std::shared_ptr<Storage> lazy_clone_storage(Storage & storage);
    void materialize_cow_storage(Storage & storage);
    bool is_cow_data_ptr(const DataPtr& data_ptr);

    std::unique_ptr<DataPtr> make_cow_data_ptr(DataPtr & src_ptr, COWDeleterContext & ctx);
    std::unique_ptr<DataPtr> copy_cow_data_ptr(DataPtr & src_ptr);
    
  } // namespace cow
  
} // namespace enigma