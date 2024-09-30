#pragma once
#include <vector>
#include <Python.h>

class Tensor {
public:
    Tensor(const std::vector<long>& shape);
    Tensor(const std::vector<long>& shape, const std::vector<float>& data);


    Tensor add(const Tensor& other) const;
    Tensor multiply(const Tensor& other) const;


    void fill(float value);
    void randn();
    long size() const;
    const std::vector<long>& shape() const;
    const std::vector<float>& data() const;

private:
    std::vector<long> shape_;
    std::vector<float> data_;

    long compute_size() const;
};