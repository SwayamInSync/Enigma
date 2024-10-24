#include <cmath>
#include <sstream>
#include <iomanip>
#include "Scalar.h"
#include <iostream>

namespace enigma
{

    namespace
    {
        bool isIntegralType(ScalarType type)
        {
            return type == ScalarType::Int8 || type == ScalarType::Int16 ||
                   type == ScalarType::Int32 || type == ScalarType::Int64 ||
                   type == ScalarType::UInt8 || type == ScalarType::UInt16 ||
                   type == ScalarType::UInt32 || type == ScalarType::UInt64;
        }

        bool isFloatingType(ScalarType type)
        {
            return type == ScalarType::Float32 || type == ScalarType::Float64;
        }

        bool isComplexType(ScalarType type)
        {
            return type == ScalarType::Complex64 || type == ScalarType::Complex128;
        }

        // Helper for checking if a double is effectively an integer
        bool isIntegralDouble(double val)
        {
            return std::fabs(val - std::round(val)) < 1e-7;
        }

        // Helper for safe numeric casting
        template <typename To, typename From>
        To safeCast(From value)
        {
            if constexpr (std::is_floating_point_v<From> && std::is_integral_v<To>)
            {
                if (!isIntegralDouble(value))
                {
                    throw ScalarTypeError("Cannot convert non-integer floating point to integral type");
                }
            }

            if constexpr (std::is_integral_v<To>)
            {
                if (value > static_cast<From>(std::numeric_limits<To>::max()) ||
                    value < static_cast<From>(std::numeric_limits<To>::min()))
                {
                    throw ScalarTypeError("Value out of range for target type");
                }
            }

            return static_cast<To>(value);
        }

        bool would_overflow(int64_t a, int64_t b, bool is_addition)
        {
            if (is_addition)
            {
                if (b > 0 && a > std::numeric_limits<int64_t>::max() - b)
                    return true;
                if (b < 0 && a < std::numeric_limits<int64_t>::min() - b)
                    return true;
            }
            else
            {
                if (b < 0 && a > std::numeric_limits<int64_t>::max() + b)
                    return true;
                if (b > 0 && a < std::numeric_limits<int64_t>::min() + b)
                    return true;
            }
            return false;
        }
    } // namespace

    // Type conversion implementations
    template <>
    double Scalar::to<double>() const
    {
        switch (type_)
        {
        case ScalarType::Float64:
            return data_.d;
        case ScalarType::Float32:
            return data_.d;
        case ScalarType::Int64:
            return static_cast<double>(data_.i);
        case ScalarType::UInt64:
            return static_cast<double>(data_.u);
        case ScalarType::Bool:
            return data_.b ? 1.0 : 0.0;
        case ScalarType::Complex128:
        case ScalarType::Complex64:
            if (data_.z.imag() != 0.0)
            {
                throw ScalarTypeError("Cannot convert complex with non-zero imaginary part to double");
            }
            return data_.z.real();
        default:
            throw ScalarTypeError("Unsupported type conversion to double");
        }
    }

    template <>
    int64_t Scalar::to<int64_t>() const
    {
        switch (type_)
        {
        case ScalarType::Int64:
            return data_.i;
        case ScalarType::UInt64:
            if (data_.u > static_cast<uint64_t>(std::numeric_limits<int64_t>::max()))
            {
                throw ScalarTypeError("UInt64 value too large for Int64");
            }
            return static_cast<int64_t>(data_.u);
        case ScalarType::Float64:
        case ScalarType::Float32:
            if (!isIntegralDouble(data_.d))
            {
                throw ScalarTypeError("Cannot convert non-integer float to int64_t");
            }
            return safeCast<int64_t>(data_.d);
        case ScalarType::Bool:
            return data_.b ? 1 : 0;
        case ScalarType::Complex128:
        case ScalarType::Complex64:
            if (data_.z.imag() != 0.0)
            {
                throw ScalarTypeError("Cannot convert complex with non-zero imaginary part to int64_t");
            }
            return safeCast<int64_t>(data_.z.real());
        default:
            throw ScalarTypeError("Unsupported type conversion to int64_t");
        }
    }

    template <>
    int32_t Scalar::to<int32_t>() const
    {
        switch (type_)
        {
        case ScalarType::Int64:
            if (data_.i > static_cast<int64_t>(std::numeric_limits<int32_t>::max()) ||
                data_.i < static_cast<int64_t>(std::numeric_limits<int32_t>::min()))
            {
                throw ScalarTypeError("Int64 value out of range for int32");
            }
            return static_cast<int32_t>(data_.i);
        case ScalarType::UInt64:
            if (data_.u > static_cast<uint64_t>(std::numeric_limits<int32_t>::max()))
            {
                throw ScalarTypeError("UInt64 value out of range for int32");
            }
            return static_cast<int32_t>(data_.u);
        case ScalarType::Float64:
        case ScalarType::Float32:
            if (!isIntegralDouble(data_.d))
            {
                throw ScalarTypeError("Cannot convert non-integer float to int32");
            }
            if (data_.d > static_cast<double>(std::numeric_limits<int32_t>::max()) ||
                data_.d < static_cast<double>(std::numeric_limits<int32_t>::min()))
            {
                throw ScalarTypeError("Float value out of range for int32");
            }
            return static_cast<int32_t>(data_.d);
        case ScalarType::Bool:
            return data_.b ? 1 : 0;
        case ScalarType::Complex128:
        case ScalarType::Complex64:
            if (data_.z.imag() != 0.0)
            {
                throw ScalarTypeError("Cannot convert complex with non-zero imaginary part to int32");
            }
            return to<int32_t>(); // Convert the real part
        default:
            throw ScalarTypeError("Unsupported type conversion to int32");
        }
    }

    template <>
    bool Scalar::to<bool>() const
    {
        switch (type_)
        {
        case ScalarType::Bool:
            return data_.b;
        case ScalarType::Int64:
            return data_.i != 0;
        case ScalarType::UInt64:
            return data_.u != 0;
        case ScalarType::Float64:
        case ScalarType::Float32:
            return data_.d != 0.0;
        case ScalarType::Complex128:
        case ScalarType::Complex64:
            return data_.z.real() != 0.0 || data_.z.imag() != 0.0;
        default:
            throw ScalarTypeError("Unsupported type conversion to bool");
        }
    }

    template <>
    std::complex<double> Scalar::to<std::complex<double>>() const
    {
        switch (type_)
        {
        case ScalarType::Complex128:
            return data_.z;
        case ScalarType::Complex64:
            return static_cast<std::complex<double>>(data_.z);
        case ScalarType::Float64:
        case ScalarType::Float32:
            return std::complex<double>(data_.d, 0.0);
        case ScalarType::Int64:
            return std::complex<double>(static_cast<double>(data_.i), 0.0);
        case ScalarType::UInt64:
            return std::complex<double>(static_cast<double>(data_.u), 0.0);
        case ScalarType::Bool:
            return std::complex<double>(data_.b ? 1.0 : 0.0, 0.0);
        default:
            throw ScalarTypeError("Unsupported type conversion to complex<double>");
        }
    }

    // Arithmetic operations
    Scalar Scalar::operator-() const
    {
        switch (type_)
        {
        case ScalarType::Float64:
        case ScalarType::Float32:
            return Scalar(-data_.d);
        case ScalarType::Int64:
            return Scalar(-data_.i);
        case ScalarType::UInt64:
            if (data_.u > 0)
            {
                throw ScalarTypeError("Cannot negate unsigned value");
            }
            return Scalar(0ull);
        case ScalarType::Complex128:
        case ScalarType::Complex64:
            return Scalar(-data_.z);
        case ScalarType::Bool:
            throw ScalarTypeError("Cannot negate boolean value");
        default:
            throw ScalarTypeError("Unsupported type for negation");
        }
    }

    Scalar Scalar::operator+(const Scalar &other) const
    {
        // Handle same-type operations efficiently
        if (type_ == other.type_)
        {
            switch (type_)
            {
            case ScalarType::Float64:
                return Scalar(data_.d + other.data_.d);
            case ScalarType::Int64:
                if (would_overflow(data_.i, other.data_.i, true))
                {
                    throw ScalarTypeError("Integer overflow in addition");
                }
                return Scalar(data_.i + other.data_.i);
            case ScalarType::UInt64:
                if (data_.u > std::numeric_limits<uint64_t>::max() - other.data_.u)
                {
                    throw ScalarTypeError("Unsigned integer overflow in addition");
                }
                return Scalar(data_.u + other.data_.u);
            case ScalarType::Complex128:
                return Scalar(data_.z + other.data_.z);
            case ScalarType::Bool:
                throw ScalarTypeError("Cannot add boolean values");
            default:
                break;
            }
        }

        // Handle mixed-type operations through promotion
        if (isComplex() || other.isComplex())
        {
            std::complex<double> lhs = isComplex() ? data_.z : std::complex<double>(to<double>(), 0.0);
            std::complex<double> rhs = other.isComplex() ? other.data_.z : std::complex<double>(other.to<double>(), 0.0);
            return Scalar(lhs + rhs);
        }
        else if (isFloatingPoint() || other.isFloatingPoint())
        {
            return Scalar(to<double>() + other.to<double>());
        }
        else
        {
            // Integer promotion with overflow check
            if (would_overflow(to<int64_t>(), other.to<int64_t>(), true))
            {
                throw ScalarTypeError("Integer overflow in addition");
            }
            return Scalar(to<int64_t>() + other.to<int64_t>());
        }
    }

    Scalar Scalar::operator-(const Scalar &other) const
    {
        return *this + (-other);
    }

    Scalar Scalar::operator*(const Scalar &other) const
    {
        if (type_ == other.type_)
        {
            switch (type_)
            {
            case ScalarType::Float64:
                return Scalar(data_.d * other.data_.d);
            case ScalarType::Int64:
                return Scalar(data_.i * other.data_.i);
            case ScalarType::UInt64:
                return Scalar(data_.u * other.data_.u);
            case ScalarType::Complex128:
                return Scalar(data_.z * other.data_.z);
            case ScalarType::Bool:
                return Scalar(static_cast<bool>(data_.b & other.data_.b));
            default:
                break;
            }
        }

        // Handle mixed-type operations
        if (isFloatingPoint() || other.isFloatingPoint())
        {
            return Scalar(to<double>() * other.to<double>());
        }
        else if (isComplex() || other.isComplex())
        {
            return Scalar(std::complex<double>(to<double>(), 0) *
                          std::complex<double>(other.to<double>(), 0));
        }
        else
        {
            return Scalar(to<int64_t>() * other.to<int64_t>());
        }
    }

    Scalar Scalar::operator/(const Scalar &other) const
    {
        // Check for division by zero
        if ((other.isFloatingPoint() && other.to<double>() == 0.0) ||
            (other.isIntegral() && other.to<int64_t>() == 0))
        {
            throw ScalarTypeError("Division by zero");
        }

        // Always promote to floating point for division
        return Scalar(to<double>() / other.to<double>());
    }

    // Comparison operations
    bool Scalar::operator==(const Scalar &other) const
    {
        if (type_ == other.type_)
        {
            switch (type_)
            {
            case ScalarType::Float64:
                return std::fabs(data_.d - other.data_.d) < 1e-7;
            case ScalarType::Int64:
                return data_.i == other.data_.i;
            case ScalarType::UInt64:
                return data_.u == other.data_.u;
            case ScalarType::Complex128:
                return data_.z == other.data_.z;
            case ScalarType::Bool:
                return data_.b == other.data_.b;
            default:
                return false;
            }
        }

        // Handle mixed-type comparisons
        try
        {
            if (isFloatingPoint() || other.isFloatingPoint())
            {
                return std::fabs(to<double>() - other.to<double>()) < 1e-7;
            }
            else if (isComplex() || other.isComplex())
            {
                return false; // Complex numbers can only be compared with same type
            }
            else
            {
                return to<int64_t>() == other.to<int64_t>();
            }
        }
        catch (const ScalarTypeError &)
        {
            return false;
        }
    }

    // Type promotion implementation
    ScalarType Scalar::promoteTypes(ScalarType a, ScalarType b)
    {
        if (a == b)
            return a;

        // Complex dominates all
        if (a == ScalarType::Complex128 || b == ScalarType::Complex128)
            return ScalarType::Complex128;
        if (a == ScalarType::Complex64 || b == ScalarType::Complex64)
            return ScalarType::Complex64;

        // Float dominates integral
        if (a == ScalarType::Float64 || b == ScalarType::Float64)
            return ScalarType::Float64;
        if (a == ScalarType::Float32 || b == ScalarType::Float32)
            return ScalarType::Float32;

        // Integer promotion rules
        if (isIntegralType(a) && isIntegralType(b))
        {
            // If either is unsigned and can't fit in signed, promote to unsigned
            if ((a == ScalarType::UInt64 || b == ScalarType::UInt64))
                return ScalarType::UInt64;
            return ScalarType::Int64;
        }

        return ScalarType::Float64; // Default promotion
    }

    // Type conversion and string utilities
    bool Scalar::canCast(ScalarType from, ScalarType to)
    {
        if (from == to)
            return true;

        // Can't cast complex to non-complex
        if ((from == ScalarType::Complex64 || from == ScalarType::Complex128) &&
            (to != ScalarType::Complex64 && to != ScalarType::Complex128))
        {
            return false;
        }

        // Can't cast floating point to integral
        if ((from == ScalarType::Float32 || from == ScalarType::Float64) &&
            (to == ScalarType::Int64 || to == ScalarType::UInt64))
        {
            return false;
        }

        return true;
    }

    std::string Scalar::typeName(ScalarType type)
    {
        switch (type)
        {
        case ScalarType::Float64:
            return "Float64";
        case ScalarType::Float32:
            return "Float32";
        case ScalarType::Int64:
            return "Int64";
        case ScalarType::Int32:
            return "Int32";
        case ScalarType::UInt64:
            return "UInt64";
        case ScalarType::UInt32:
            return "UInt32";
        case ScalarType::Complex128:
            return "Complex128";
        case ScalarType::Complex64:
            return "Complex64";
        case ScalarType::Bool:
            return "Bool";
        default:
            return "Unknown";
        }
    }

    std::string Scalar::toString() const
    {
        std::ostringstream ss;
        ss << std::setprecision(17);
        switch (type_)
        {
        case ScalarType::Float64:
        case ScalarType::Float32:
            ss << data_.d;
            break;
        case ScalarType::Int64:
            ss << data_.i;
            break;
        case ScalarType::UInt64:
            ss << data_.u;
            break;
        case ScalarType::Complex128:
        case ScalarType::Complex64:
            ss << data_.z.real() << "+" << data_.z.imag() << "j";
            break;
        case ScalarType::Bool:
            ss << (data_.b ? "true" : "false");
            break;
        default:
            ss << "Unknown";
        }
        return ss.str();
    }

    // Device support
    Scalar Scalar::to(Device device) const
    {
        Scalar result(*this);
        result.device_ = device;
        return result;
    }

} // namespace enigma