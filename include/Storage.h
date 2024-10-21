#pragma once

#include <memory>
#include <cstddef>
#include "Allocator.h"

namespace enigma
{
  class Storage
  {
    private:
      void * data_;
      size_t size_bytes_;
      Device device_;
      std::shared_ptr<Allocator> allocator_;

      void allocate();
      void deallocate();

    public:
      Storage(size_t size_bytes, const Device & device);
      Storage(size_t size_bytes, void * data, const Device & device);
      ~Storage();

      void * data() const { return data_; }
      size_t size_bytes() const { return size_bytes_; }
      const Device device() const { return device_; }

      void resize(size_t new_size_bytes);
  };
} // namespace enigma
