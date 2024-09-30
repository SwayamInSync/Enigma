import pytest
# import numpy as np

try:
    from enigma import Tensor
except ImportError:
    pytest.skip("Enigma module not found", allow_module_level=True)

def test_tensor_creation():
    # Test 1D tensor
    t1 = Tensor([3], [1.0, 2.0, 3.0])
    assert t1.shape() == [3]
    assert list(t1.data()) == [1.0, 2.0, 3.0]

    # Test 2D tensor
    t2 = Tensor([2, 2], [1.0, 2.0, 3.0, 4.0])
    assert t2.shape() == [2, 2]
    assert list(t2.data()) == [1.0, 2.0, 3.0, 4.0]

def test_tensor_addition():
    t1 = Tensor([2, 2], [1.0, 2.0, 3.0, 4.0])
    t2 = Tensor([2, 2], [5.0, 6.0, 7.0, 8.0])
    result = t1.add(t2)
    assert result.shape() == [2, 2]
    assert list(result.data()) == [6.0, 8.0, 10.0, 12.0]

def test_tensor_multiplication():
    t1 = Tensor([2, 2], [1.0, 2.0, 3.0, 4.0])
    t2 = Tensor([2, 2], [5.0, 6.0, 7.0, 8.0])
    result = t1.multiply(t2)
    assert result.shape() == [2, 2]
    assert list(result.data()) == [5.0, 12.0, 21.0, 32.0]

def test_tensor_shape_mismatch():
    t1 = Tensor([2, 2], [1.0, 2.0, 3.0, 4.0])
    t2 = Tensor([3], [5.0, 6.0, 7.0])
    with pytest.raises(ValueError):
        t1.add(t2)
    with pytest.raises(ValueError):
        t1.multiply(t2)

def test_tensor_to_string():
    t = Tensor([2, 2], [1.0, 2.0, 3.0, 4.0])
    assert str(t) == "Tensor(shape=[2, 2], data=[1.00, 2.00, 3.00, 4.00])"

# @pytest.mark.parametrize("shape, data", [
#     ([2, 2], [1.0, 2.0, 3.0, 4.0]),
#     ([3], [1.0, 2.0, 3.0]),
#     ([2, 3], [1.0, 2.0, 3.0, 4.0, 5.0, 6.0]),
# ])
# def test_tensor_numpy_comparison(shape, data):
#     t = Tensor(shape, data)
#     np_array = np.array(data).reshape(shape)
    
#     assert t.shape() == list(np_array.shape)
#     assert np.allclose(t.data(), np_array.flatten())

#     # Test addition
#     t2 = Tensor(shape, [x + 1 for x in data])
#     np_array2 = np_array + 1
#     result = t.add(t2)
#     assert np.allclose(result.data(), (np_array + np_array2).flatten())

#     # Test multiplication
#     result = t.multiply(t2)
#     assert np.allclose(result.data(), (np_array * np_array2).flatten())