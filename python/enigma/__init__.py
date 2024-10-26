from ._enigma import (
    Scalar,
    ScalarType,
    ScalarError,
    ScalarTypeError,
    get_dtype,
    promote_types,
    can_cast,
)


int8 = ScalarType.int8
int16 = ScalarType.int16
int32 = ScalarType.int32
int64 = ScalarType.int64
uint8 = ScalarType.uint8
uint16 = ScalarType.uint16
uint32 = ScalarType.uint32
uint64 = ScalarType.uint64
float32 = ScalarType.float32
float64 = ScalarType.float64
complex64 = ScalarType.complex64
complex128 = ScalarType.complex128
bool_ = ScalarType.bool

# Version info
__version__ = "0.0.1"
