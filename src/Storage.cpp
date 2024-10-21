#include <stdexcept>
#include "COW.h"
#include "Storage.h"

#include <iostream>

namespace enigma
{
  Storage::Storage(size_t size_bytes, const Device & device) : size_bytes_(size_bytes), device_(device)
  {
    allocator_ = get_allocator(device);
    if (!allocator_) {
        throw std::runtime_error("Failed to get allocator for device");
    }
    allocate();
  }

  Storage::Storage(size_t size_bytes, void * data, const Device & device) :  size_bytes_(size_bytes), device_(device), allocator_(get_allocator(device))
  {
    if (data == nullptr)
    {
      throw std::invalid_argument("Data pointer cannot be null");
    }

    data_ptr_ = std::make_unique<DataPtr>(data, nullptr, nullptr, device);
  }

  Storage::~Storage()
  {
    deallocate();
  }

  void Storage::allocate()
  {
    if (size_bytes_ > 0)
    {
      void * ptr = allocator_->allocate(size_bytes_);
      if(ptr == nullptr) throw std::bad_alloc();

      data_ptr_ = std::make_unique<DataPtr>(ptr, nullptr, [this](void * p){ allocator_->deallocate(p); }, device_); // data, ctx, deleter, device
    }
    else 
    {
      data_ptr_ = std::make_unique<DataPtr>();
    }
  }

  void Storage::deallocate() 
  {
    data_ptr_.reset();
  }

  void Storage::resize(size_t new_size_bytes) 
  {
    if (new_size_bytes == size_bytes_) return;

    deallocate();
    size_bytes_ = new_size_bytes;
    allocate();
  }

  void Storage::set_data_ptr(std::unique_ptr<DataPtr> new_data_ptr) 
  {
    data_ptr_ = std::move(new_data_ptr);
  }

  std::shared_ptr<Storage> Storage::lazy_clone(Storage& src) 
  {
    return cow::lazy_clone_storage(src);
  }

  void Storage::materialize() 
  {
    cow::materialize_cow_storage(*this);
  }

  bool Storage::is_cow() const 
  {
    return cow::is_cow_data_ptr(*data_ptr_);
  }

} // namespace enigma
