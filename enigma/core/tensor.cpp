#include "tensor.h"
#include <numeric>
#include <algorithm>
#include <stdexcept>
#include <random>
#include <sstream>
#include <iomanip>
#include <vector>

Tensor::Tensor(const std::vector<long> &shape) : shape_(shape)
{
    long total_size = compute_size();
    data_.resize(total_size);
}

Tensor::Tensor(const std::vector<long> &shape, const std::vector<float> &data)
    : shape_(shape), data_(data)
{
    if (data.size() != compute_size())
    {
        throw std::invalid_argument("Data size does not match the specified shape");
    }
}

Tensor Tensor::add(const Tensor &other) const
{
    if (shape_ != other.shape_)
    {
        throw std::invalid_argument("Tensor shapes do not match for addition");
    }

    Tensor result(shape_);
    for (size_t i = 0; i < data_.size(); ++i)
    {
        result.data_[i] = data_[i] + other.data_[i];
    }
    return result;
}

Tensor Tensor::multiply(const Tensor &other) const
{
    if (shape_ != other.shape_)
    {
        throw std::invalid_argument("Tensor shapes do not match for multiplication");
    }

    Tensor result(shape_);
    for (size_t i = 0; i < data_.size(); ++i)
    {
        result.data_[i] = data_[i] * other.data_[i];
    }
    return result;
}

void Tensor::fill(float value)
{
    std::fill(data_.begin(), data_.end(), value);
}

void Tensor::randn()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<> d(0, 1);
    for (auto &elem : data_)
    {
        elem = static_cast<float>(d(gen));
    }
}

long Tensor::size() const
{
    return data_.size();
}

const std::vector<long> &Tensor::shape() const
{
    return shape_;
}

const std::vector<float> &Tensor::data() const
{
    return data_;
}

std::string Tensor::repr() const
{
    std::ostringstream oss;
    oss << "Tensor(shape=[";
    for (size_t i = 0; i < shape_.size(); ++i)
    {
        oss << shape_[i];
        if (i < shape_.size() - 1)
            oss << ", ";
    }
    oss << "], data=[";
    for (size_t i = 0; i < data_.size(); ++i)
    {
        oss << std::fixed << std::setprecision(2) << data_[i];
        if (i < data_.size() - 1)
            oss << ", ";
    }
    oss << "])";
    return oss.str();
}

std::string Tensor::str() const
{
    return repr(); // For now only
}

long Tensor::compute_size() const
{
    return std::accumulate(shape_.begin(), shape_.end(), 1L, std::multiplies<long>());
}