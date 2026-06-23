/*******************************************************************************
    This source file is part of FORG library.

    Micrograd-inspired scalar reverse-mode autograd. The original micrograd
    project is MIT licensed, Copyright (c) 2020 Andrej Karpathy.
*******************************************************************************/

#ifndef FORG_NN_VALUE_H
#define FORG_NN_VALUE_H

#include "forg/base.h"

#include <functional>
#include <memory>
#include <vector>

namespace forg::nn {

class Value;
struct ValueGraphAccess;

using ValuePtr = std::shared_ptr<Value>;

class Value
{
  public:
    double GetData() const noexcept { return m_data; }
    double GetGrad() const noexcept { return m_grad; }
    void SetGrad(double grad) noexcept { m_grad = grad; }

  private:
    friend ValuePtr MakeValue(double value);
    friend ValuePtr operator+(const ValuePtr& lhs, const ValuePtr& rhs);
    friend ValuePtr operator*(const ValuePtr& lhs, const ValuePtr& rhs);
    friend ValuePtr Pow(const ValuePtr& value, double exponent);
    friend ValuePtr Relu(const ValuePtr& value);
    friend struct ValueGraphAccess;

    explicit Value(double value);
    Value(double value, std::vector<ValuePtr> previous);

    const std::vector<ValuePtr>& Previous() const noexcept
    {
        return m_previous;
    }
    void ApplyBackward();

    double m_data = 0.0;
    double m_grad = 0.0;
    std::vector<ValuePtr> m_previous;
    std::function<void()> m_backward;
};

ValuePtr MakeValue(double value);

ValuePtr operator+(const ValuePtr& lhs, const ValuePtr& rhs);
ValuePtr operator+(const ValuePtr& lhs, double rhs);
ValuePtr operator+(double lhs, const ValuePtr& rhs);

ValuePtr operator-(const ValuePtr& value);
ValuePtr operator-(const ValuePtr& lhs, const ValuePtr& rhs);
ValuePtr operator-(const ValuePtr& lhs, double rhs);
ValuePtr operator-(double lhs, const ValuePtr& rhs);

ValuePtr operator*(const ValuePtr& lhs, const ValuePtr& rhs);
ValuePtr operator*(const ValuePtr& lhs, double rhs);
ValuePtr operator*(double lhs, const ValuePtr& rhs);

ValuePtr operator/(const ValuePtr& lhs, const ValuePtr& rhs);
ValuePtr operator/(const ValuePtr& lhs, double rhs);
ValuePtr operator/(double lhs, const ValuePtr& rhs);

ValuePtr Pow(const ValuePtr& value, double exponent);
ValuePtr Relu(const ValuePtr& value);

void Backward(const ValuePtr& root);

} // namespace forg::nn

#endif // FORG_NN_VALUE_H
