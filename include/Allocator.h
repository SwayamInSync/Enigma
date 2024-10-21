#pragma once

#include <cstddef>
#include <memory>
#include "Device.h"

namespace enigma
{
  class Allocator
  {
    public:
      virtual ~Allocator() = default;
      virtual void * allocate(size_t num_bytes) = 0;
      virtual void deallocate(void * ptr) = 0;
      virtual Device device() const = 0;
  };

  class CPUAllocator : public Allocator
  {
    public:
      void * allocate(size_t num_bytes) override;
      void deallocate(void * ptr) override;
      Device device() const override { return Device(DeviceType::CPU); }
  };

  // will implement CUDAAllocator here or in the CUDA folder (depends on my mood) :)

  std:: shared_ptr<Allocator> get_allocator(const Device & device); 

} // namespace enigma
