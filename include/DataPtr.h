#pragma once

#include "Device.h"
#include <functional>
#include <memory>

namespace enigma 
{

  class DataPtr 
  {
    public:
      using DeleterFn = std::function<void(void*)>; // will be used for custom deletion of data
      
    private:
      void* data_;
      void* ctx_;
      DeleterFn deleter_;
      Device device_;
      uintptr_t deleter_id_;

    public:
      static constexpr uintptr_t INVALID_DELETER_ID = 0;
      // Constructors
      DataPtr() : data_(nullptr), ctx_(nullptr), deleter_(nullptr), device_(DeviceType::CPU), deleter_id_(INVALID_DELETER_ID) {}
      DataPtr(void* data, void* ctx, DeleterFn deleter, Device device)
          : data_(data), ctx_(ctx), deleter_(std::move(deleter)), device_(device), deleter_id_(INVALID_DELETER_ID) {}

      // Getters
      void* get() const { return data_; }
      void* get_context() const { return ctx_; }
      const DeleterFn& get_deleter() const { return deleter_; }
      Device device() const { return device_; }
      uintptr_t get_deleter_id() const { return deleter_id_; }

      void set_deleter_id(uintptr_t id) { deleter_id_ = id; }

      // Methods
      void clear() 
      {
          if (deleter_ && data_) 
          {
              deleter_(ctx_ ? ctx_ : data_);
          }
          data_ = nullptr;
          ctx_ = nullptr;
          deleter_ = nullptr;
          deleter_id_ = 0;
      }

      std::unique_ptr<void, DeleterFn> move_context() 
      {
        auto ctx = ctx_;
        ctx_ = nullptr;
        return std::unique_ptr<void, DeleterFn>(ctx, std::move(deleter_));
      }

      void* release_context() 
      {
          void* released_ctx = ctx_;
          ctx_ = nullptr;
          return released_ctx;
      }

      explicit operator bool() const 
      {
          return data_ != nullptr;
      }

      void set_deleter(DeleterFn new_deleter) 
      {
          deleter_ = std::move(new_deleter);
      }
  };

} // namespace enigma