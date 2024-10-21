#include "DeviceType.h"

namespace enigma 
{
  std::string device_type_name(DeviceType device_type)
  {
    switch (device_type)
    {
      case DeviceType::CPU:
        return "CPU";
      case DeviceType::CUDA:
        return "CUDA";
      default:
        return "Unknown";
    }
  }

  bool is_valid_device_type(DeviceType device_type) 
  {
    switch (device_type) 
    {
        case DeviceType::CPU:
        case DeviceType::CUDA:
            return true;
        default:
            return false;
    }
  }

}