#pragma once

#include "DataPtr.h"
#include "Allocator.h"
#include "Device.h"
#include <memory>
#include <cstddef>

namespace enigma 
{

  class Storage 
  {
    private:
      std::unique_ptr<DataPtr> data_ptr_;
      size_t size_bytes_;
      Device device_;
      std::shared_ptr<Allocator> allocator_;

      void allocate();
      void deallocate();

    public:
      Storage(size_t size_bytes, const Device& device);
      Storage(size_t size_bytes, void* data, const Device& device);
      ~Storage();

      void* data() const { return data_ptr_ ? data_ptr_->get() : nullptr; }
      size_t size_bytes() const { return size_bytes_; }
      const Device& device() const { return device_; }
      DataPtr& data_ptr() { return *data_ptr_; }
      std::shared_ptr<Allocator> allocator() const { return allocator_; }

      void resize(size_t new_size_bytes);

      // Methods for COW support
      static std::shared_ptr<Storage> lazy_clone(Storage& src);
      void materialize();
      bool is_cow() const;

      // Method to allow setting a new DataPtr
      void set_data_ptr(std::unique_ptr<DataPtr> new_data_ptr);
  };

} // namespace enigma