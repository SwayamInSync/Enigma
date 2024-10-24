#pragma once

#include <cstdint>
#include <complex>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include "Device.h"

namespace enigma
{
    class ScalarError : public std::runtime_error
    {
        using std::runtime_error::runtime_error;
    };

    class ScalarTypeError : public ScalarError
    {
    public:
        ScalarTypeError(const std::string &msg) : ScalarError(msg) {}
        ScalarTypeError(const char *msg) : ScalarError(msg) {}
    };

    enum class ScalarType : int8_t
    {
        Int8,
        Int16,
        Int32,
        Int64,
        UInt8,
        UInt16,
        UInt32,
        UInt64,
        Float32,
        Float64, // Default type
        Complex64,
        Complex128,
        Bool,
        Invalid
    };

    // Type trait system for mapping C++ types to ScalarTypes
    template <typename T>
    struct CPPTypeToScalar
    {
        static constexpr ScalarType value = ScalarType::Invalid;
    };

    // Specializations for core types
    template <>
    struct CPPTypeToScalar<int8_t>
    {
        static constexpr ScalarType value = ScalarType::Int8;
    };
    template <>
    struct CPPTypeToScalar<int16_t>
    {
        static constexpr ScalarType value = ScalarType::Int16;
    };
    template <>
    struct CPPTypeToScalar<int32_t>
    {
        static constexpr ScalarType value = ScalarType::Int32;
    };
    template <>
    struct CPPTypeToScalar<int64_t>
    {
        static constexpr ScalarType value = ScalarType::Int64;
    };
    template <>
    struct CPPTypeToScalar<uint8_t>
    {
        static constexpr ScalarType value = ScalarType::UInt8;
    };
    template <>
    struct CPPTypeToScalar<uint16_t>
    {
        static constexpr ScalarType value = ScalarType::UInt16;
    };
    template <>
    struct CPPTypeToScalar<uint32_t>
    {
        static constexpr ScalarType value = ScalarType::UInt32;
    };
    template <>
    struct CPPTypeToScalar<uint64_t>
    {
        static constexpr ScalarType value = ScalarType::UInt64;
    };
    template <>
    struct CPPTypeToScalar<float>
    {
        static constexpr ScalarType value = ScalarType::Float32;
    };
    template <>
    struct CPPTypeToScalar<double>
    {
        static constexpr ScalarType value = ScalarType::Float64;
    };
    template <>
    struct CPPTypeToScalar<std::complex<float>>
    {
        static constexpr ScalarType value = ScalarType::Complex64;
    };
    template <>
    struct CPPTypeToScalar<std::complex<double>>
    {
        static constexpr ScalarType value = ScalarType::Complex128;
    };
    template <>
    struct CPPTypeToScalar<bool>
    {
        static constexpr ScalarType value = ScalarType::Bool;
    };

    // Forward mapping from ScalarType to C++ type
    template <ScalarType>
    struct ScalarToCPPType;
    template <>
    struct ScalarToCPPType<ScalarType::Int8>
    {
        using type = int8_t;
    };
    template <>
    struct ScalarToCPPType<ScalarType::Int16>
    {
        using type = int16_t;
    };
    template <>
    struct ScalarToCPPType<ScalarType::Int32>
    {
        using type = int32_t;
    };
    template <>
    struct ScalarToCPPType<ScalarType::Int64>
    {
        using type = int64_t;
    };
    template <>
    struct ScalarToCPPType<ScalarType::UInt8>
    {
        using type = uint8_t;
    };
    template <>
    struct ScalarToCPPType<ScalarType::UInt16>
    {
        using type = uint16_t;
    };
    template <>
    struct ScalarToCPPType<ScalarType::UInt32>
    {
        using type = uint32_t;
    };
    template <>
    struct ScalarToCPPType<ScalarType::UInt64>
    {
        using type = uint64_t;
    };
    template <>
    struct ScalarToCPPType<ScalarType::Float32>
    {
        using type = float;
    };
    template <>
    struct ScalarToCPPType<ScalarType::Float64>
    {
        using type = double;
    };
    template <>
    struct ScalarToCPPType<ScalarType::Complex64>
    {
        using type = std::complex<float>;
    };
    template <>
    struct ScalarToCPPType<ScalarType::Complex128>
    {
        using type = std::complex<double>;
    };
    template <>
    struct ScalarToCPPType<ScalarType::Bool>
    {
        using type = bool;
    };

    // Helper alias for getting C++ type from ScalarType
    template <ScalarType S>
    using scalar_t = typename ScalarToCPPType<S>::type;

    class Scalar
    {
    private:
        union Data
        {
            int64_t i;
            uint64_t u;
            double d;
            std::complex<double> z;
            bool b;
            Data() : d(0.0) {} // Initialize to 0.0 as Float64 is default
        };

        ScalarType type_;
        Data data_;
        Device device_;

    public:
        // Constructors
        Scalar() : type_(ScalarType::Float64), device_(DeviceType::CPU) {}

        // Type-specific constructors
        explicit Scalar(double v) : type_(ScalarType::Float64), device_(DeviceType::CPU) { data_.d = v; }
        explicit Scalar(int64_t v) : type_(ScalarType::Int64), device_(DeviceType::CPU) { data_.i = v; }
        explicit Scalar(uint64_t v) : type_(ScalarType::UInt64), device_(DeviceType::CPU) { data_.u = v; }
        explicit Scalar(bool v) : type_(ScalarType::Bool), device_(DeviceType::CPU) { data_.b = v; }
        explicit Scalar(const std::complex<double> &v) : type_(ScalarType::Complex128), device_(DeviceType::CPU) { data_.z = v; }

        // Template constructor for other numeric types
        template <typename T,
                  typename = std::enable_if_t<std::is_arithmetic_v<T> || std::is_same_v<T, std::complex<float>>>>
        explicit Scalar(T value) : device_(DeviceType::CPU)
        {
            type_ = CPPTypeToScalar<T>::value;
            if constexpr (std::is_floating_point_v<T>)
            {
                data_.d = static_cast<double>(value);
                type_ = ScalarType::Float64;
            }
            else if constexpr (std::is_integral_v<T>)
            {
                if constexpr (std::is_unsigned_v<T>)
                {
                    data_.u = static_cast<uint64_t>(value);
                    type_ = ScalarType::UInt64;
                }
                else
                {
                    data_.i = static_cast<int64_t>(value);
                    type_ = ScalarType::Int64;
                }
            }
            else if constexpr (std::is_same_v<T, std::complex<float>>)
            {
                data_.z = static_cast<std::complex<double>>(value);
            }
        }

        // Type checking methods
        bool isFloatingPoint() const
        {
            return type_ == ScalarType::Float32 || type_ == ScalarType::Float64;
        }

        bool isIntegral() const
        {
            return type_ == ScalarType::Int8 || type_ == ScalarType::Int16 ||
                   type_ == ScalarType::Int32 || type_ == ScalarType::Int64 ||
                   type_ == ScalarType::UInt8 || type_ == ScalarType::UInt16 ||
                   type_ == ScalarType::UInt32 || type_ == ScalarType::UInt64;
        }

        bool isComplex() const
        {
            return type_ == ScalarType::Complex64 || type_ == ScalarType::Complex128;
        }

        bool isBoolean() const { return type_ == ScalarType::Bool; }

        // Accessors
        ScalarType type() const { return type_; }
        Device device() const { return device_; }

        // Type conversion with checks
        template <typename T>
        T to() const;

        // Basic arithmetic operations
        Scalar operator-() const;
        Scalar operator+(const Scalar &other) const;
        Scalar operator-(const Scalar &other) const;
        Scalar operator*(const Scalar &other) const;
        Scalar operator/(const Scalar &other) const;

        // Comparison operators
        bool operator==(const Scalar &other) const;
        bool operator!=(const Scalar &other) const { return !(*this == other); }

        // Static methods for type handling
        static ScalarType promoteTypes(ScalarType a, ScalarType b);
        static std::string typeName(ScalarType type);
        static bool canCast(ScalarType from, ScalarType to);

        // Device movement (for future CUDA support)
        Scalar to(Device device) const;

        // String representation
        std::string toString() const;
    };

} // namespace enigma