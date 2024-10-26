import enigma
from typing import Any


def print_section(title: str) -> None:
    """Helper to print formatted section titles."""
    print(f"\n{'='*50}")
    print(f"  {title}")
    print(f"{'='*50}\n")


def demonstrate_basic_creation():
    """Demonstrate basic scalar creation and type inference."""
    print_section("Basic Scalar Creation")

    # Different ways to create scalars
    examples = [
        (42, "Integer"),
        (3.14, "Float"),
        (True, "Boolean"),
        (1 + 2j, "Complex"),
    ]

    for value, name in examples:
        scalar = enigma.Scalar(value)
        print(f"{name:8} | Value: {scalar} | Type: {scalar.dtype}")

    # Show default construction
    default_scalar = enigma.Scalar()
    print(f"\nDefault | Value: {default_scalar} | Type: {
          default_scalar.dtype}")


def demonstrate_type_checking():
    """Demonstrate type checking methods."""
    print_section("Type Checking")

    scalars = {
        "Integer": enigma.Scalar(42),
        "Float": enigma.Scalar(3.14),
        "Complex": enigma.Scalar(1 + 2j),
        "Boolean": enigma.Scalar(True)
    }

    for name, scalar in scalars.items():
        print(f"{name:8} is:")
        print(f"  Integral? {scalar.is_integral()}")
        print(f"  Floating? {scalar.is_floating_point()}")
        print(f"  Complex?  {scalar.is_complex()}")
        print(f"  Boolean?  {scalar.is_bool()}\n")


def demonstrate_arithmetic():
    """Demonstrate arithmetic operations."""
    print_section("Arithmetic Operations")

    # Basic arithmetic
    a = enigma.Scalar(10)
    b = enigma.Scalar(3)

    print("Integer operations:")
    print(f"10 + 3 = {a + b}")
    print(f"10 - 3 = {a - b}")
    print(f"10 * 3 = {a * b}")
    print(f"10 / 3 = {a / b}")  # Note: Division promotes to float

    # Mixed-type arithmetic
    c = enigma.Scalar(3.14)
    print("\nMixed-type operations:")
    print(f"10 + 3.14 = {a + c}")
    print(f"10 * 3.14 = {a * c}")

    # Complex arithmetic
    d = enigma.Scalar(1 + 1j)
    print("\nComplex operations:")
    print(f"(1 + 1j) * 3.14 = {d * c}")


def demonstrate_type_conversion():
    """Demonstrate type conversion capabilities."""
    print_section("Type Conversion")

    # Safe conversions
    scalar = enigma.Scalar(42)
    print(f"Original: {scalar} (type: {scalar.dtype})")
    print(f"To float: {scalar.to_float()} (explicit conversion)")
    print(f"To int  : {scalar.to_int()}")
    print(f"To bool : {scalar.to_bool()}")

    # Conversion errors
    print("\nDemonstrating conversion errors:")
    try:
        enigma.Scalar(3.14).to_int()
    except enigma.ScalarTypeError as e:
        print(f"Expected error: {e}")


def demonstrate_type_promotion():
    """Demonstrate type promotion rules."""
    print_section("Type Promotion")

    cases = [
        (enigma.int64, enigma.float64),
        (enigma.float32, enigma.float64),
        (enigma.int32, enigma.complex64),
    ]

    for type1, type2 in cases:
        promoted = enigma.promote_types(type1, type2)
        print(f"{type1} + {type2} → {promoted}")


def demonstrate_error_handling():
    """Demonstrate error handling."""
    print_section("Error Handling")
    examples = [
        # Division by zero
        lambda: enigma.Scalar(1) / enigma.Scalar(0),
        # Invalid conversion
        lambda: enigma.Scalar(3.14).to_int(),
        # Complex to real conversion with imaginary part
        lambda: enigma.Scalar(1 + 1j).to_float(),
    ]

    for i, example in enumerate(examples, 1):
        try:
            example()
        except enigma.ScalarTypeError as e:
            print(f"Example {i}: {type(e).__name__}: {e}")


def demonstrate_comparisons():
    """Demonstrate comparison operations."""
    print_section("Comparisons")

    a = enigma.Scalar(42)
    b = enigma.Scalar(42.0)
    c = enigma.Scalar(43)

    print(f"42 == 42.0: {a == b}")
    print(f"42 != 43  : {a != c}")

    # Show type-aware comparisons
    print(f"\nType-aware comparisons:")
    print(f"Scalar(1) == Scalar(True): {
          enigma.Scalar(1) == enigma.Scalar(True)}")


def main():
    """Run all demonstrations."""
    print("\nEnigma Scalar Library Demonstration")
    print("Version:", enigma.__version__, "\n")

    demonstrate_basic_creation()
    demonstrate_type_checking()
    demonstrate_arithmetic()
    demonstrate_type_conversion()
    demonstrate_type_promotion()
    demonstrate_error_handling()
    demonstrate_comparisons()


if __name__ == "__main__":
    main()
