#include <cstdlib>
#include <stdexcept>
#include "Allocator.h"

namespace enigma
{
  void * CPUAllocator::allocate(size_t num_bytes)
  {
    void * ptr = std::malloc(num_bytes);
    if (ptr == nullptr)
    {
      throw std::bad_alloc();
    }
    return ptr;
  }

  void CPUAllocator::deallocate(void * ptr)
  {
    std::free(ptr);
  }

  std::shared_ptr<Allocator> get_allocator(const Device & device)
  {
    if (device.is_cpu())
    {
      return std::make_shared<CPUAllocator>();
    }
    else if (device.is_cuda())
    {
      // return std::make_shared<CUDAAllocator>();
      throw std::invalid_argument("CUDAAllocator not implemented yet");
    }
    else
    {
      throw std::runtime_error("Invalid device type");
    }
  }

} // namespace enigma