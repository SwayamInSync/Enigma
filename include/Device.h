#pragma once

#include <string>
#include "DeviceType.h"

namespace enigma
{
  class Device
  {
  private:
    DeviceType type_;
    int index_;

  public:
    // Constructors
    Device();
    Device(DeviceType type, int index = -1);
    explicit Device(const std::string &device_string);

    // Getters
    DeviceType type() const { return type_; };
    int index() const { return index_; };

    // methods
    bool has_index() const { return index_ != -1; };
    bool is_cpu() const { return type_ == DeviceType::CPU; };
    bool is_cuda() const { return type_ == DeviceType::CUDA; };

    std::string to_string() const;
    bool operator==(const Device &other) const = default;
  };
} // namespace enigma
