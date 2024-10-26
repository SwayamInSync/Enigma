# python/tests/test_scalar.py
import pytest
import enigma
import math
import time
from typing import Callable


class TestScalar:
    # Helper constant
    EPSILON = 1e-7

    def approx_equal(self, a: float, b: float) -> bool:
        return abs(a - b) < self.EPSILON

    def test_default_construction(self):
        """Test default construction of Scalar"""
        s = enigma.Scalar()
        assert s.dtype == enigma.float64
        assert self.approx_equal(s.to_float(), 0.0)

    def test_type_construction(self):
        """Test construction with different types"""
        # Integer construction
        s1 = enigma.Scalar(42)
        assert s1.dtype == enigma.int64
        assert s1.to_int() == 42

        # Float construction
        s2 = enigma.Scalar(3.14)
        assert s2.dtype == enigma.float64
        assert self.approx_equal(s2.to_float(), 3.14)

        # Boolean construction
        s3 = enigma.Scalar(True)
        assert s3.dtype == enigma.bool_
        assert s3.to_bool()

        # Complex construction
        s4 = enigma.Scalar(1.0 + 2.0j)
        assert s4.dtype == enigma.complex128

    def test_type_checking(self):
        """Test type checking methods"""
        i = enigma.Scalar(42)
        assert i.is_integral()
        assert not i.is_floating_point()
        assert not i.is_complex()
        assert not i.is_bool()

        f = enigma.Scalar(3.14)
        assert not f.is_integral()
        assert f.is_floating_point()
        assert not f.is_complex()
        assert not f.is_bool()

        c = enigma.Scalar(1.0 + 2.0j)
        assert not c.is_integral()
        assert not c.is_floating_point()
        assert c.is_complex()
        assert not c.is_bool()

        b = enigma.Scalar(True)
        assert not b.is_integral()
        assert not b.is_floating_point()
        assert not b.is_complex()
        assert b.is_bool()

    def test_numeric_conversions(self):
        """Test numeric type conversions"""
        # Int to Float
        i = enigma.Scalar(42)
        assert self.approx_equal(i.to_float(), 42.0)

        # Float to Int (when possible)
        f = enigma.Scalar(42.0)
        assert f.to_int() == 42

        # Bool to numeric
        b = enigma.Scalar(True)
        assert b.to_int() == 1
        assert self.approx_equal(b.to_float(), 1.0)

        # Complex to real (when possible)
        c = enigma.Scalar(1.0 + 0.0j)
        assert self.approx_equal(c.to_float(), 1.0)

    def test_conversion_errors(self):
        """Test conversion error cases"""
        # Float with fractional part to int
        f = enigma.Scalar(3.14)
        with pytest.raises(enigma.ScalarTypeError):
            f.to_int()

        # Complex with imaginary part to real
        c = enigma.Scalar(1.0 + 2.0j)
        with pytest.raises(enigma.ScalarTypeError):
            c.to_float()

    def test_basic_arithmetic(self):
        """Test basic arithmetic operations"""
        # Integer arithmetic
        i1, i2 = enigma.Scalar(42), enigma.Scalar(8)
        assert (i1 + i2).to_int() == 50
        assert (i1 - i2).to_int() == 34
        assert (i1 * i2).to_int() == 336
        assert self.approx_equal((i1 / i2).to_float(), 5.25)

        # Floating point arithmetic
        f1, f2 = enigma.Scalar(3.14), enigma.Scalar(2.0)
        assert self.approx_equal((f1 + f2).to_float(), 5.14)
        assert self.approx_equal((f1 - f2).to_float(), 1.14)
        assert self.approx_equal((f1 * f2).to_float(), 6.28)
        assert self.approx_equal((f1 / f2).to_float(), 1.57)

        # Complex arithmetic
        c1 = enigma.Scalar(1.0 + 2.0j)
        c2 = enigma.Scalar(2.0 - 1.0j)
        result = (c1 + c2).to_complex()
        assert self.approx_equal(result.real, 3.0)
        assert self.approx_equal(result.imag, 1.0)

    def test_mixed_type_operations(self):
        """Test operations between different types"""
        i = enigma.Scalar(42)
        f = enigma.Scalar(3.14)
        c = enigma.Scalar(1.0 + 2.0j)

        # Int + Float
        result = (i + f).to_float()
        assert self.approx_equal(result, 45.14)

        # Float + Complex
        result = (f + c).to_complex()
        assert self.approx_equal(result.real, 4.14)
        assert self.approx_equal(result.imag, 2.0)

        # Int * Float
        result = (i * f).to_float()
        assert self.approx_equal(result, 131.88)

    def test_division_operations(self):
        """Test division operations"""
        # Basic division
        assert self.approx_equal(
            (enigma.Scalar(6) / enigma.Scalar(2)).to_float(), 3.0)
        assert self.approx_equal(
            (enigma.Scalar(3.14) / enigma.Scalar(2.0)).to_float(), 1.57)

        # Division by zero
        with pytest.raises(enigma.ScalarTypeError):
            enigma.Scalar(1) / enigma.Scalar(0)
        with pytest.raises(enigma.ScalarTypeError):
            enigma.Scalar(1.0) / enigma.Scalar(0.0)

    def test_comparison_operations(self):
        """Test comparison operations"""
        # Same type comparisons
        assert enigma.Scalar(42) == enigma.Scalar(42)
        assert enigma.Scalar(3.14159) == enigma.Scalar(3.14159)
        assert enigma.Scalar(True) == enigma.Scalar(True)

        # Mixed type comparisons
        assert enigma.Scalar(42) == enigma.Scalar(42.0)

        # Boolean comparisons (strict)
        assert not (enigma.Scalar(1) == enigma.Scalar(True))
        assert not (enigma.Scalar(0) == enigma.Scalar(False))

    def test_type_promotion(self):
        """Test type promotion rules"""
        # Same type promotion
        assert enigma.promote_types(enigma.int64, enigma.int64) == enigma.int64

        # Complex promotion
        assert enigma.promote_types(
            enigma.float64, enigma.complex64) == enigma.complex128
        assert enigma.promote_types(
            enigma.int64, enigma.complex64) == enigma.complex128

        # Float promotion
        assert enigma.promote_types(
            enigma.int64, enigma.float64) == enigma.float64

    def test_type_casting(self):
        """Test type casting capabilities"""
        # Same type casting
        assert enigma.can_cast(enigma.int64, enigma.int64)

        # Complex casting
        assert not enigma.can_cast(enigma.complex64, enigma.float64)
        assert enigma.can_cast(enigma.float64, enigma.complex64)

        # Float casting
        assert not enigma.can_cast(enigma.float64, enigma.int64)
        assert enigma.can_cast(enigma.int64, enigma.float64)

    def test_string_representation(self):
        """Test string conversion"""
        assert str(enigma.Scalar(42)) == "42"
        assert str(enigma.Scalar(True)) == "true"

        # Floating point precision
        f = enigma.Scalar(3.14159265359)
        assert "3.14159" in str(f)


if __name__ == "__main__":
    pytest.main([__file__])
