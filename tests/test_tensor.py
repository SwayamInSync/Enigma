import pytest

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
    expected = "Tensor(shape=[2, 2], data=[1.00, 2.00, 3.00, 4.00])"
    assert str(t) == expected
    assert repr(t) == expected


def test_tensor_operations_chain():
    t1 = Tensor([2, 2], [1.0, 2.0, 3.0, 4.0])
    t2 = Tensor([2, 2], [5.0, 6.0, 7.0, 8.0])
    t3 = Tensor([2, 2], [0.1, 0.2, 0.3, 0.4])

    result = t1.add(t2).multiply(t3)
    expected_data = [0.6, 1.6, 3.0, 4.8]
    assert result.shape() == [2, 2]
    assert all(abs(a - b) < 1e-6 for a, b in zip(result.data(), expected_data))


def test_tensor_fill():
    t = Tensor([2, 3], [0.0] * 6)
    t.fill(5.0)
    assert all(abs(x - 5.0) < 1e-6 for x in t.data())


def test_tensor_randn():
    t = Tensor([2, 3], [0.0] * 6)
    t.randn()
    # Check that values have changed and are different
    assert not all(abs(x) < 1e-6 for x in t.data())
    assert len(set(t.data())) > 1


if __name__ == "__main__":
    pytest.main([__file__, "-v"])
