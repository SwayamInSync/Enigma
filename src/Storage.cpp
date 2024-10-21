#include <stdexcept>
#include "Storage.h"

namespace enigma
{
  Storage::Storage(size_t size_bytes, const Device & device) : size_bytes_(size_bytes), device_(device)
  {
    allocate();
  }

  Storage::Storage(size_t size_bytes, void * data, const Device & device) : size_bytes_(size_bytes), data_(data), device_(device), allocator_(get_allocator(device))
  {
    if (data_ == nullptr)
    {
      throw std::invalid_argument("Data pointer cannot be null");
    }
  }

  Storage::~Storage()
  {
    deallocate();
  }

  void Storage::allocate()
  {
    if (size_bytes_ > 0)
    {
      data_ = allocator_->allocate(size_bytes_);
      if(data_ == nullptr) throw std::bad_alloc();
    }
    else 
    {
      data_ = nullptr;
    }
  }

  void Storage::deallocate() 
  {
    if (data_) 
    {
        allocator_->deallocate(data_);
        data_ = nullptr;
    }
  }

  void Storage::resize(size_t new_size_bytes) 
  {
    if (new_size_bytes == size_bytes_) return;

    deallocate();
    size_bytes_ = new_size_bytes;
    allocate();
  }

} // namespace enigma
