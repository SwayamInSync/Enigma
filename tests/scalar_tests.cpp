#include <gtest/gtest.h>
#include <cmath>
#include <limits>
#include "Scalar.h"

using namespace enigma;

class ScalarTest : public ::testing::Test
{
protected:
    static constexpr double EPSILON = 1e-7;

    bool approxEqual(double a, double b)
    {
        return std::fabs(a - b) < EPSILON;
    }
};

// Construction and Type Tests
TEST_F(ScalarTest, DefaultConstruction)
{
    Scalar s;
    EXPECT_EQ(s.type(), ScalarType::Float64);
    EXPECT_TRUE(approxEqual(s.to<double>(), 0.0));
}

TEST_F(ScalarTest, TypeConstruction)
{
    // Integer construction
    Scalar s1(42);
    EXPECT_EQ(s1.type(), ScalarType::Int64);
    EXPECT_EQ(s1.to<int64_t>(), 42);

    // Float construction
    Scalar s2(3.14);
    EXPECT_EQ(s2.type(), ScalarType::Float64);
    EXPECT_TRUE(approxEqual(s2.to<double>(), 3.14));

    // Boolean construction
    Scalar s3(true);
    EXPECT_EQ(s3.type(), ScalarType::Bool);
    EXPECT_TRUE(s3.to<bool>());

    // Complex construction
    Scalar s4(std::complex<double>(1.0, 2.0));
    EXPECT_EQ(s4.type(), ScalarType::Complex128);
}

// Type Checking Tests
TEST_F(ScalarTest, TypeChecking)
{
    Scalar i(42);
    EXPECT_TRUE(i.isIntegral());
    EXPECT_FALSE(i.isFloatingPoint());
    EXPECT_FALSE(i.isComplex());
    EXPECT_FALSE(i.isBoolean());

    Scalar f(3.14);
    EXPECT_FALSE(f.isIntegral());
    EXPECT_TRUE(f.isFloatingPoint());
    EXPECT_FALSE(f.isComplex());
    EXPECT_FALSE(f.isBoolean());

    Scalar c(std::complex<double>(1.0, 2.0));
    EXPECT_FALSE(c.isIntegral());
    EXPECT_FALSE(c.isFloatingPoint());
    EXPECT_TRUE(c.isComplex());
    EXPECT_FALSE(c.isBoolean());

    Scalar b(true);
    EXPECT_FALSE(b.isIntegral());
    EXPECT_FALSE(b.isFloatingPoint());
    EXPECT_FALSE(b.isComplex());
    EXPECT_TRUE(b.isBoolean());
}

// Type Conversion Tests
TEST_F(ScalarTest, NumericConversions)
{
    // Int to Float
    Scalar i(42);
    EXPECT_TRUE(approxEqual(i.to<double>(), 42.0));

    // Float to Int (when possible)
    Scalar f(42.0);
    EXPECT_EQ(f.to<int64_t>(), 42);

    // Bool to numeric
    Scalar b(true);
    EXPECT_EQ(b.to<int64_t>(), 1);
    EXPECT_TRUE(approxEqual(b.to<double>(), 1.0));

    // Complex to real (when possible)
    Scalar c(std::complex<double>(1.0, 0.0));
    EXPECT_TRUE(approxEqual(c.to<double>(), 1.0));
}

// Conversion Error Tests
TEST_F(ScalarTest, ConversionErrors)
{
    // Float with fractional part to int
    Scalar f(3.14);
    EXPECT_THROW(f.to<int64_t>(), ScalarTypeError);

    // Complex with imaginary part to real
    Scalar c(std::complex<double>(1.0, 2.0));
    EXPECT_THROW(c.to<double>(), ScalarTypeError);

    // Value overflow tests
    Scalar big(std::numeric_limits<int64_t>::max());
    EXPECT_THROW(big.to<int32_t>(), ScalarTypeError);
}

// Arithmetic Operation Tests
TEST_F(ScalarTest, BasicArithmetic)
{
    // Integer arithmetic
    Scalar i1(42), i2(8);
    EXPECT_EQ((i1 + i2).to<int64_t>(), 50);
    EXPECT_EQ((i1 - i2).to<int64_t>(), 34);
    EXPECT_EQ((i1 * i2).to<int64_t>(), 336);
    EXPECT_TRUE(approxEqual((i1 / i2).to<double>(), 5.25));

    // Floating point arithmetic
    Scalar f1(3.14), f2(2.0);
    EXPECT_TRUE(approxEqual((f1 + f2).to<double>(), 5.14));
    EXPECT_TRUE(approxEqual((f1 - f2).to<double>(), 1.14));
    EXPECT_TRUE(approxEqual((f1 * f2).to<double>(), 6.28));
    EXPECT_TRUE(approxEqual((f1 / f2).to<double>(), 1.57));

    // Complex arithmetic
    Scalar c1(std::complex<double>(1.0, 2.0));
    Scalar c2(std::complex<double>(2.0, -1.0));
    auto sum = (c1 + c2).to<std::complex<double>>();
    EXPECT_TRUE(approxEqual(sum.real(), 3.0));
    EXPECT_TRUE(approxEqual(sum.imag(), 1.0));
}

// Mixed Type Operation Tests
TEST_F(ScalarTest, MixedTypeOperations)
{
    Scalar i(42);                             // Int
    Scalar f(3.14);                           // Float
    Scalar c(std::complex<double>(1.0, 2.0)); // Complex

    // Int + Float
    auto res1 = (i + f).to<double>();
    EXPECT_TRUE(approxEqual(res1, 45.14));

    // Float + Complex
    auto res2 = (f + c).to<std::complex<double>>();
    EXPECT_TRUE(approxEqual(res2.real(), 4.14));
    EXPECT_TRUE(approxEqual(res2.imag(), 2.0));

    // Int * Float
    auto res3 = (i * f).to<double>();
    EXPECT_TRUE(approxEqual(res3, 131.88));
}

TEST_F(ScalarTest, MultiplicationOperations)
{
    // Basic multiplication
    EXPECT_EQ((Scalar(5) * Scalar(3)).to<int64_t>(), 15);
    EXPECT_TRUE(approxEqual((Scalar(3.14) * Scalar(2.0)).to<double>(), 6.28));

    // Mixed type multiplication
    EXPECT_TRUE(approxEqual((Scalar(5) * Scalar(3.14)).to<double>(), 15.7));

    // Complex multiplication
    auto c1 = Scalar(std::complex<double>(3.0, 2.0));
    auto c2 = Scalar(std::complex<double>(1.0, 1.0));
    auto prod = (c1 * c2).to<std::complex<double>>();
    EXPECT_TRUE(approxEqual(prod.real(), 1.0)); // 3*1 - 2*1 = 1
    EXPECT_TRUE(approxEqual(prod.imag(), 5.0)); // 3*1 + 2*1 = 5

    // Overflow tests
    Scalar big(std::numeric_limits<int64_t>::max());
    EXPECT_THROW(big * Scalar(2), ScalarTypeError);

    // Boolean multiplication
    EXPECT_TRUE((Scalar(true) * Scalar(true)).to<bool>());
    EXPECT_FALSE((Scalar(true) * Scalar(false)).to<bool>());
}

TEST_F(ScalarTest, DivisionOperations)
{
    // Basic division
    EXPECT_TRUE(approxEqual((Scalar(6) / Scalar(2)).to<double>(), 3.0));
    EXPECT_TRUE(approxEqual((Scalar(3.14) / Scalar(2.0)).to<double>(), 1.57));

    // Mixed type division
    EXPECT_TRUE(approxEqual((Scalar(5) / Scalar(2.0)).to<double>(), 2.5));

    // Complex division
    auto c1 = Scalar(std::complex<double>(3.0, 2.0));
    auto c2 = Scalar(std::complex<double>(1.0, 1.0));
    auto quot = (c1 / c2).to<std::complex<double>>();
    EXPECT_TRUE(approxEqual(quot.real(), 2.5));
    EXPECT_TRUE(approxEqual(quot.imag(), -0.5));

    // Division by zero tests
    EXPECT_THROW(Scalar(1) / Scalar(0), ScalarTypeError);
    EXPECT_THROW(Scalar(1.0) / Scalar(0.0), ScalarTypeError);
    EXPECT_THROW(
        Scalar(1) / Scalar(std::complex<double>(0.0, 0.0)),
        ScalarTypeError);

    // Integer division that results in floating point
    auto result = (Scalar(5) / Scalar(2)).to<double>();
    EXPECT_TRUE(approxEqual(result, 2.5));
}

// Edge Case Tests
TEST_F(ScalarTest, EdgeCases)
{
    // Division by zero
    Scalar num(1.0);
    Scalar zero(0.0);
    EXPECT_THROW(num / zero, ScalarTypeError);

    // Maximum values
    Scalar maxInt(std::numeric_limits<int64_t>::max());
    EXPECT_THROW(maxInt + Scalar(1), ScalarTypeError);

    // Minimum values
    Scalar minInt(std::numeric_limits<int64_t>::min());
    EXPECT_THROW(minInt - Scalar(1), ScalarTypeError);
}

TEST_F(ScalarTest, ComparisonOperations)
{
    // Same type comparisons
    EXPECT_TRUE(Scalar(42) == Scalar(42));
    EXPECT_TRUE(Scalar(3.14159) == Scalar(3.14159));
    EXPECT_TRUE(Scalar(true) == Scalar(true));
    EXPECT_TRUE(Scalar(false) == Scalar(false));

    // Floating point comparisons with epsilon
    EXPECT_TRUE(Scalar(0.1 + 0.2) == Scalar(0.3));
    EXPECT_TRUE(Scalar(1.0 + 1e-8) == Scalar(1.0));

    // Mixed type comparisons
    EXPECT_TRUE(Scalar(42) == Scalar(42.0));

    // Boolean comparisons (strict)
    EXPECT_FALSE(Scalar(1) == Scalar(true));  // No implicit conversion
    EXPECT_FALSE(Scalar(0) == Scalar(false)); // No implicit conversion
    EXPECT_FALSE(Scalar(true) == Scalar(1));  // No implicit conversion
    EXPECT_FALSE(Scalar(false) == Scalar(0)); // No implicit conversion

    // Complex number comparisons
    auto c1 = Scalar(std::complex<double>(1.0, 0.0));
    auto c2 = Scalar(1.0);
    EXPECT_TRUE(c1 == c2); // Complex with zero imaginary equals real

    auto c3 = Scalar(std::complex<double>(1.0, 1e-8));
    auto c4 = Scalar(std::complex<double>(1.0, 0.0));
    EXPECT_TRUE(c3 == c4); // Small imaginary part within epsilon

    // Different type comparisons
    EXPECT_FALSE(Scalar(std::complex<double>(1.0, 1.0)) == Scalar(1.0));
    EXPECT_FALSE(Scalar(42.5) == Scalar(42));

    // Edge cases
    EXPECT_TRUE(Scalar(0.0) == Scalar(0));
    EXPECT_TRUE(Scalar(-0.0) == Scalar(0.0));

    // Near zero comparisons
    EXPECT_TRUE(Scalar(1e-8) == Scalar(0.0));
    EXPECT_FALSE(Scalar(1e-6) == Scalar(0.0));
}

// Type Promotion Tests
TEST_F(ScalarTest, TypePromotion)
{
    // Same type promotion
    EXPECT_EQ(Scalar::promoteTypes(ScalarType::Int64, ScalarType::Int64),
              ScalarType::Int64);

    // Complex promotion
    EXPECT_EQ(Scalar::promoteTypes(ScalarType::Float64, ScalarType::Complex64),
              ScalarType::Complex128);
    EXPECT_EQ(Scalar::promoteTypes(ScalarType::Int64, ScalarType::Complex64),
              ScalarType::Complex128);

    // Float promotion
    EXPECT_EQ(Scalar::promoteTypes(ScalarType::Int64, ScalarType::Float64),
              ScalarType::Float64);
    EXPECT_EQ(Scalar::promoteTypes(ScalarType::Float32, ScalarType::Float64),
              ScalarType::Float64);

    // Integer promotion
    EXPECT_EQ(Scalar::promoteTypes(ScalarType::Int32, ScalarType::Int64),
              ScalarType::Int64);
    EXPECT_EQ(Scalar::promoteTypes(ScalarType::UInt32, ScalarType::Int64),
              ScalarType::Int64);
    EXPECT_EQ(Scalar::promoteTypes(ScalarType::UInt64, ScalarType::Int64),
              ScalarType::UInt64);

    // Boolean promotion
    EXPECT_EQ(Scalar::promoteTypes(ScalarType::Bool, ScalarType::Int64),
              ScalarType::Int64);
}

TEST_F(ScalarTest, TypeCasting)
{
    // Same type casting
    EXPECT_TRUE(Scalar::canCast(ScalarType::Int64, ScalarType::Int64));

    // Complex casting
    EXPECT_FALSE(Scalar::canCast(ScalarType::Complex64, ScalarType::Float64));
    EXPECT_TRUE(Scalar::canCast(ScalarType::Float64, ScalarType::Complex64));

    // Float casting
    EXPECT_FALSE(Scalar::canCast(ScalarType::Float64, ScalarType::Int64));
    EXPECT_TRUE(Scalar::canCast(ScalarType::Int64, ScalarType::Float64));

    // Integer casting
    EXPECT_TRUE(Scalar::canCast(ScalarType::Int32, ScalarType::Int64));
    EXPECT_FALSE(Scalar::canCast(ScalarType::UInt64, ScalarType::Int64));

    // Boolean casting
    EXPECT_TRUE(Scalar::canCast(ScalarType::Bool, ScalarType::Int64));
    EXPECT_TRUE(Scalar::canCast(ScalarType::Int64, ScalarType::Bool));
}

// String Representation Tests
TEST_F(ScalarTest, StringRepresentation)
{
    EXPECT_EQ(Scalar(42).toString(), "42");
    EXPECT_EQ(Scalar(true).toString(), "true");

    // Floating point should have full precision
    Scalar f(3.14159265359);
    EXPECT_TRUE(f.toString().find("3.14159") != std::string::npos);

    // Complex numbers
    Scalar c(std::complex<double>(1.0, 2.0));
    EXPECT_TRUE(c.toString().find("1") != std::string::npos);
    EXPECT_TRUE(c.toString().find("2") != std::string::npos);
}

// Device Support Tests
TEST_F(ScalarTest, DeviceSupport)
{
    Scalar s(42);
    EXPECT_EQ(s.device().type(), DeviceType::CPU);

    // Test device movement (currently only CPU is supported)
    auto moved = s.to(Device(DeviceType::CPU));
    EXPECT_EQ(moved.device().type(), DeviceType::CPU);
    EXPECT_EQ(moved.to<int64_t>(), 42);
}

// Memory Safety Tests
TEST_F(ScalarTest, MemorySafety)
{
    // Test copy construction
    Scalar original(42);
    Scalar copy = original;
    EXPECT_EQ(copy.to<int64_t>(), 42);

    // Modify copy should not affect original
    copy = Scalar(43);
    EXPECT_EQ(original.to<int64_t>(), 42);
    EXPECT_EQ(copy.to<int64_t>(), 43);
}

// Performance Tests (Basic)
TEST_F(ScalarTest, BasicPerformance)
{
    const int iterations = 1000000;

    // Measure time for basic operations
    auto start = std::chrono::high_resolution_clock::now();

    Scalar s(1.0);
    for (int i = 0; i < iterations; ++i)
    {
        s = s + Scalar(1.0);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // This is not a strict test, but helps catch major performance regressions
    EXPECT_LT(duration.count(), 1000); // Should complete within 1 second
}