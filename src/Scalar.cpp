#include <cmath>
#include <sstream>
#include <iomanip>
#include "Scalar.h"
#include <iostream>

namespace enigma
{

    namespace
    {
        bool almost_equal(double a, double b, double epsilon = 1e-7)
        {
            if (a == b)
                return true;

            // Handle comparisons near zero
            if (std::abs(a) < epsilon && std::abs(b) < epsilon)
                return true;

            // Relative comparison for larger numbers
            double diff = std::abs(a - b);
            a = std::abs(a);
            b = std::abs(b);
            double largest = (b > a) ? b : a;
            return diff <= largest * epsilon;
        }

        // Helper for complex comparison
        bool complex_almost_equal(const std::complex<double> &a,
                                  const std::complex<double> &b,
                                  double epsilon = 1e-7)
        {
            return almost_equal(a.real(), b.real(), epsilon) &&
                   almost_equal(a.imag(), b.imag(), epsilon);
        }

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

        bool isUnsignedType(ScalarType type)
        {
            switch (type)
            {
            case ScalarType::UInt8:
            case ScalarType::UInt16:
            case ScalarType::UInt32:
            case ScalarType::UInt64:
                return true;
            default:
                return false;
            }
        }

        int getTypeWidth(ScalarType type)
        {
            switch (type)
            {
            case ScalarType::Int8:
            case ScalarType::UInt8:
                return 8;
            case ScalarType::Int16:
            case ScalarType::UInt16:
                return 16;
            case ScalarType::Int32:
            case ScalarType::UInt32:
                return 32;
            case ScalarType::Int64:
            case ScalarType::UInt64:
                return 64;
            default:
                return 0;
            }
        }

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
    uint64_t Scalar::to<uint64_t>() const
    {
        switch (type_)
        {
        case ScalarType::UInt64:
            return data_.u;

        case ScalarType::Int64:
            if (data_.i < 0)
            {
                throw ScalarTypeError("Cannot convert negative integer to unsigned");
            }
            return static_cast<uint64_t>(data_.i);

        case ScalarType::Float64:
        case ScalarType::Float32:
            if (!isIntegralDouble(data_.d) || data_.d < 0)
            {
                throw ScalarTypeError("Cannot convert non-integral or negative float to uint64_t");
            }
            if (data_.d > static_cast<double>(std::numeric_limits<uint64_t>::max()))
            {
                throw ScalarTypeError("Float value too large for uint64_t");
            }
            return static_cast<uint64_t>(data_.d);

        case ScalarType::Bool:
            return data_.b ? 1ULL : 0ULL;

        case ScalarType::Complex128:
        case ScalarType::Complex64:
            if (data_.z.imag() != 0.0)
            {
                throw ScalarTypeError("Cannot convert complex with non-zero imaginary part to uint64_t");
            }
            if (data_.z.real() < 0 || !isIntegralDouble(data_.z.real()))
            {
                throw ScalarTypeError("Cannot convert complex to uint64_t");
            }
            return static_cast<uint64_t>(data_.z.real());

        default:
            throw ScalarTypeError("Unsupported type conversion to uint64_t");
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
        // Handle same-type operations efficiently
        if (type_ == other.type_)
        {
            switch (type_)
            {
            case ScalarType::Float64:
                return Scalar(data_.d - other.data_.d);

            case ScalarType::Int64:
                if (would_overflow(data_.i, other.data_.i, false))
                { // false for subtraction
                    throw ScalarTypeError("Integer overflow in subtraction");
                }
                return Scalar(data_.i - other.data_.i);

            case ScalarType::UInt64:
                if (data_.u < other.data_.u)
                { // Underflow check for unsigned
                    throw ScalarTypeError("Unsigned integer underflow in subtraction");
                }
                return Scalar(data_.u - other.data_.u);

            case ScalarType::Complex128:
                return Scalar(data_.z - other.data_.z);

            case ScalarType::Bool:
                throw ScalarTypeError("Cannot subtract boolean values");

            default:
                break;
            }
        }

        // Handle mixed-type operations through promotion
        if (isComplex() || other.isComplex())
        {
            std::complex<double> lhs = isComplex() ? data_.z : std::complex<double>(to<double>(), 0.0);
            std::complex<double> rhs = other.isComplex() ? other.data_.z : std::complex<double>(other.to<double>(), 0.0);
            return Scalar(lhs - rhs);
        }
        else if (isFloatingPoint() || other.isFloatingPoint())
        {
            return Scalar(to<double>() - other.to<double>());
        }
        else
        {
            // Integer promotion with overflow check
            if (would_overflow(to<int64_t>(), other.to<int64_t>(), false))
            {
                throw ScalarTypeError("Integer overflow in subtraction");
            }
            return Scalar(to<int64_t>() - other.to<int64_t>());
        }
    }

    namespace
    {
        // Helper for multiplication overflow check
        bool would_multiply_overflow(int64_t a, int64_t b)
        {
            if (a > 0)
            {
                if (b > 0)
                {
                    if (a > std::numeric_limits<int64_t>::max() / b)
                        return true;
                }
                else
                {
                    if (b < std::numeric_limits<int64_t>::min() / a)
                        return true;
                }
            }
            else
            {
                if (b > 0)
                {
                    if (a < std::numeric_limits<int64_t>::min() / b)
                        return true;
                }
                else
                {
                    if (a != 0 && b < std::numeric_limits<int64_t>::max() / a)
                        return true;
                }
            }
            return false;
        }

        bool would_multiply_overflow_unsigned(uint64_t a, uint64_t b)
        {
            if (b != 0 && a > std::numeric_limits<uint64_t>::max() / b)
                return true;
            return false;
        }
    }

    Scalar Scalar::operator*(const Scalar &other) const
    {
        // Handle same-type operations efficiently
        if (type_ == other.type_)
        {
            switch (type_)
            {
            case ScalarType::Float64:
                return Scalar(data_.d * other.data_.d);

            case ScalarType::Int64:
                if (would_multiply_overflow(data_.i, other.data_.i))
                {
                    throw ScalarTypeError("Integer overflow in multiplication");
                }
                return Scalar(data_.i * other.data_.i);

            case ScalarType::UInt64:
                if (would_multiply_overflow_unsigned(data_.u, other.data_.u))
                {
                    throw ScalarTypeError("Unsigned integer overflow in multiplication");
                }
                return Scalar(data_.u * other.data_.u);

            case ScalarType::Complex128:
                return Scalar(data_.z * other.data_.z);

            case ScalarType::Bool:
                return Scalar(static_cast<bool>(data_.b & other.data_.b));

            default:
                throw ScalarTypeError("Unsupported type for multiplication");
            }
        }

        // Handle mixed-type operations
        if (isComplex() || other.isComplex())
        {
            std::complex<double> lhs = isComplex() ? data_.z : std::complex<double>(to<double>(), 0.0);
            std::complex<double> rhs = other.isComplex() ? other.data_.z : std::complex<double>(other.to<double>(), 0.0);
            return Scalar(lhs * rhs);
        }

        // Then floating point
        if (isFloatingPoint() || other.isFloatingPoint())
        {
            return Scalar(to<double>() * other.to<double>());
        }

        // Integer multiplication with overflow check
        if (would_multiply_overflow(to<int64_t>(), other.to<int64_t>()))
        {
            throw ScalarTypeError("Integer overflow in multiplication");
        }
        return Scalar(to<int64_t>() * other.to<int64_t>());
    }

    Scalar Scalar::operator/(const Scalar &other) const
    {
        // Handle division by zero with type-specific checks
        if (other.type_ == ScalarType::Complex128)
        {
            if (other.data_.z == std::complex<double>(0.0, 0.0))
            {
                throw ScalarTypeError("Division by complex zero");
            }
        }
        else
        {
            double val = other.to<double>();
            if (std::abs(val) < std::numeric_limits<double>::epsilon())
            {
                throw ScalarTypeError("Division by zero");
            }
        }

        // Handle division based on types
        if (isComplex() || other.isComplex())
        {
            std::complex<double> lhs = isComplex() ? data_.z : std::complex<double>(to<double>(), 0.0);
            std::complex<double> rhs = other.isComplex() ? other.data_.z : std::complex<double>(other.to<double>(), 0.0);
            return Scalar(lhs / rhs);
        }

        // Integer division
        if (isIntegral() && other.isIntegral())
        {
            // Check if division would be exact
            double quotient = static_cast<double>(to<int64_t>()) / other.to<double>();
            if (std::floor(quotient) != quotient)
            {
                // If not exact, promote to floating point
                return Scalar(quotient);
            }
            // If exact and within bounds, keep as integer
            if (quotient <= std::numeric_limits<int64_t>::max() &&
                quotient >= std::numeric_limits<int64_t>::min())
            {
                return Scalar(static_cast<int64_t>(quotient));
            }
        }

        // Default to floating point division for other cases
        return Scalar(to<double>() / other.to<double>());
    }

    bool Scalar::operator==(const Scalar &other) const
    {
        // Same type comparisons (most common case)
        if (type_ == other.type_)
        {
            switch (type_)
            {
            case ScalarType::Float64:
                return almost_equal(data_.d, other.data_.d);

            case ScalarType::Int64:
                return data_.i == other.data_.i;

            case ScalarType::UInt64:
                return data_.u == other.data_.u;

            case ScalarType::Complex128:
                return complex_almost_equal(data_.z, other.data_.z);

            case ScalarType::Bool:
                return data_.b == other.data_.b;

            default:
                return false;
            }
        }

        // Mixed-type comparisons
        try
        {
            // If either is boolean, require exact boolean comparison
            if (isBoolean() || other.isBoolean())
            {
                // Only allow bool == bool, not bool == number
                return false;
            }

            // If either is complex
            if (isComplex() || other.isComplex())
            {
                // Only compare complex numbers if both can be converted to complex
                if (!isComplex())
                {
                    // Convert non-complex to complex for comparison
                    return complex_almost_equal(
                        std::complex<double>(to<double>(), 0.0),
                        other.data_.z);
                }
                else if (!other.isComplex())
                {
                    return complex_almost_equal(
                        data_.z,
                        std::complex<double>(other.to<double>(), 0.0));
                }
                return false;
            }

            // If either is floating point
            if (isFloatingPoint() || other.isFloatingPoint())
            {
                return almost_equal(to<double>(), other.to<double>());
            }

            // If both are integer types (signed or unsigned)
            if (isIntegral() && other.isIntegral())
            {
                // Handle potential overflow in conversion
                try
                {
                    return to<int64_t>() == other.to<int64_t>();
                }
                catch (const ScalarTypeError &)
                {
                    // If conversion fails, try unsigned comparison
                    return to<uint64_t>() == other.to<uint64_t>();
                }
            }

            return false;
        }
        catch (const ScalarTypeError &)
        {
            return false; // If conversion fails, values are not equal
        }
    }

    ScalarType Scalar::promoteTypes(ScalarType a, ScalarType b)
    {
        // Same type, no promotion needed
        if (a == b)
            return a;

        // Handle invalid types
        if (a == ScalarType::Invalid || b == ScalarType::Invalid)
        {
            return ScalarType::Invalid;
        }

        // Special handling for boolean
        if (a == ScalarType::Bool)
            return b;
        if (b == ScalarType::Bool)
            return a;

        // Complex type promotion
        if (isComplexType(a) || isComplexType(b))
        {
            // Always promote to Complex128 for maximum precision
            return ScalarType::Complex128;
        }

        // Floating point promotion
        if (isFloatingType(a) || isFloatingType(b))
        {
            // If either is Float64, promote to Float64
            if (a == ScalarType::Float64 || b == ScalarType::Float64)
            {
                return ScalarType::Float64;
            }
            return ScalarType::Float32;
        }

        // Integer promotion rules
        if (isIntegralType(a) && isIntegralType(b))
        {
            bool aUnsigned = isUnsignedType(a);
            bool bUnsigned = isUnsignedType(b);
            int aWidth = getTypeWidth(a);
            int bWidth = getTypeWidth(b);

            // If both unsigned or both signed, use the wider type
            if (aUnsigned == bUnsigned)
            {
                return (aWidth >= bWidth) ? a : b;
            }

            // One signed, one unsigned
            ScalarType unsignedType = aUnsigned ? a : b;
            ScalarType signedType = aUnsigned ? b : a;

            // If unsigned type is wider or equal, use it
            if (getTypeWidth(unsignedType) >= getTypeWidth(signedType))
            {
                return unsignedType;
            }

            // Otherwise use the signed type of the next size up
            switch (getTypeWidth(signedType))
            {
            case 8:
                return ScalarType::Int16;
            case 16:
                return ScalarType::Int32;
            case 32:
                return ScalarType::Int64;
            default:
                return ScalarType::Int64; // Can't go higher than 64 bits
            }
        }

        // Default to Float64 for any other combination
        return ScalarType::Float64;
    }

    // Type conversion and string utilities
    bool Scalar::canCast(ScalarType from, ScalarType to)
    {
        // Same type is always allowed
        if (from == to)
            return true;

        // Invalid type cannot be cast
        if (from == ScalarType::Invalid || to == ScalarType::Invalid)
        {
            return false;
        }

        // Boolean can be cast to/from anything
        if (from == ScalarType::Bool || to == ScalarType::Bool)
        {
            return true;
        }

        // Complex to non-complex only allowed if imaginary part is zero
        if (isComplexType(from) && !isComplexType(to))
        {
            return false;
        }

        // Floating point to integral requires explicit cast
        if (isFloatingType(from) && isIntegralType(to))
        {
            return false;
        }

        // Unsigned to signed of same or smaller width not allowed
        if (isUnsignedType(from) && !isUnsignedType(to))
        {
            return getTypeWidth(to) > getTypeWidth(from);
        }

        // All other casts are allowed but might lose precision
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