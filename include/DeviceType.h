#pragma once

#include <cstdint>
#include <string>

namespace enigma
{
  enum class DeviceType : int8_t
  {
    INVALID_TYPE = -1,
    CPU = 0,
    CUDA
  };

  std::string device_type_name(DeviceType device_type);
  bool is_valid_device_type(DeviceType device_type);
}; // namespace enigma
