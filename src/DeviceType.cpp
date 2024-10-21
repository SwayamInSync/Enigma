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

  bool isValidDeviceType(DeviceType device_type) 
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