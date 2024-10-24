#include <stdexcept>
#include "Device.h"

namespace enigma
{
  Device::Device() : type_(DeviceType::INVALID_TYPE), index_(-1) {}
  Device::Device(DeviceType type, int index) : type_(type), index_(index)
  {
    if (type_ == DeviceType::CPU && index_ != -1 && index_ != 0)
    {
      throw std::invalid_argument("CPU device index must be -1 or 0");
    }
    else if (!is_valid_device_type(type_))
    {
      throw std::invalid_argument("Invalid device type");
    }
  }

  Device::Device(const std::string &device_string)
  {
    if (device_string.substr(0, 3) == "cpu")
    {
      type_ = DeviceType::CPU;
      index_ = 0;
    }
    else if (device_string.substr(0, 4) == "cuda")
    {
      type_ = DeviceType::CUDA;
      if (device_string.length() > 5 && device_string[4] == ':')
      {
        index_ = std::stoi(device_string.substr(5));
      }
      else
      {
        index_ = -1;
      }
    }
    else
    {
      throw std::invalid_argument("Invalid device string");
    }
  }

  std::string Device::to_string() const
  {
    std::string result = device_type_name(type_);
    if (has_index())
    {
      result += ":" + std::to_string(index_);
    }

    return result;
  }

} // namespace enigma