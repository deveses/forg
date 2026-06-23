#include "forg/nn/Value.h"

#include <cmath>
#include <functional>
#include <unordered_set>
#include <utility>

namespace forg::nn {

struct ValueGraphAccess
{
    static const std::vector<ValuePtr>& Previous(const Value& value)
    {
        return value.Previous();
    }

    static void ApplyBackward(Value& value) { value.ApplyBackward(); }
};

namespace {

void BuildTopo(const ValuePtr& value, std::unordered_set<const Value*>& visited,
               std::vector<ValuePtr>& topo)
{
    if (!value)
        return;

    if (visited.contains(value.get()))
        return;

    visited.insert(value.get());
    for (const ValuePtr& previous : ValueGraphAccess::Previous(*value))
    {
        BuildTopo(previous, visited, topo);
    }
    topo.push_back(value);
}

} // namespace

Value::Value(double value) : m_data(value) {}

Value::Value(double value, std::vector<ValuePtr> previous)
    : m_data(value), m_previous(std::move(previous))
{
}

void Value::ApplyBackward()
{
    if (m_backward)
        m_backward();
}

ValuePtr MakeValue(double value) { return ValuePtr(new Value(value)); }

ValuePtr operator+(const ValuePtr& lhs, const ValuePtr& rhs)
{
    if (!lhs || !rhs)
        return nullptr;

    ValuePtr out(
        new Value(lhs->m_data + rhs->m_data, std::vector<ValuePtr>{lhs, rhs}));
    std::weak_ptr<Value> weak_out = out;
    out->m_backward = [lhs, rhs, weak_out]()
    {
        if (auto out = weak_out.lock())
        {
            lhs->m_grad += out->m_grad;
            rhs->m_grad += out->m_grad;
        }
    };
    return out;
}

ValuePtr operator+(const ValuePtr& lhs, double rhs)
{
    return lhs + MakeValue(rhs);
}

ValuePtr operator+(double lhs, const ValuePtr& rhs)
{
    return MakeValue(lhs) + rhs;
}

ValuePtr operator-(const ValuePtr& value) { return value * -1.0; }

ValuePtr operator-(const ValuePtr& lhs, const ValuePtr& rhs)
{
    return lhs + (-rhs);
}

ValuePtr operator-(const ValuePtr& lhs, double rhs)
{
    return lhs - MakeValue(rhs);
}

ValuePtr operator-(double lhs, const ValuePtr& rhs)
{
    return MakeValue(lhs) - rhs;
}

ValuePtr operator*(const ValuePtr& lhs, const ValuePtr& rhs)
{
    if (!lhs || !rhs)
        return nullptr;

    ValuePtr out(
        new Value(lhs->m_data * rhs->m_data, std::vector<ValuePtr>{lhs, rhs}));
    std::weak_ptr<Value> weak_out = out;
    out->m_backward = [lhs, rhs, weak_out]()
    {
        if (auto out = weak_out.lock())
        {
            lhs->m_grad += rhs->m_data * out->m_grad;
            rhs->m_grad += lhs->m_data * out->m_grad;
        }
    };
    return out;
}

ValuePtr operator*(const ValuePtr& lhs, double rhs)
{
    return lhs * MakeValue(rhs);
}

ValuePtr operator*(double lhs, const ValuePtr& rhs)
{
    return MakeValue(lhs) * rhs;
}

ValuePtr operator/(const ValuePtr& lhs, const ValuePtr& rhs)
{
    return lhs * Pow(rhs, -1.0);
}

ValuePtr operator/(const ValuePtr& lhs, double rhs)
{
    return lhs / MakeValue(rhs);
}

ValuePtr operator/(double lhs, const ValuePtr& rhs)
{
    return MakeValue(lhs) / rhs;
}

ValuePtr Pow(const ValuePtr& value, double exponent)
{
    if (!value)
        return nullptr;

    ValuePtr out(new Value(std::pow(value->m_data, exponent),
                           std::vector<ValuePtr>{value}));
    std::weak_ptr<Value> weak_out = out;
    out->m_backward = [value, exponent, weak_out]()
    {
        if (auto out = weak_out.lock())
        {
            value->m_grad += exponent *
                             std::pow(value->m_data, exponent - 1.0) *
                             out->m_grad;
        }
    };
    return out;
}

ValuePtr Relu(const ValuePtr& value)
{
    if (!value)
        return nullptr;

    ValuePtr out(new Value(value->m_data < 0.0 ? 0.0 : value->m_data,
                           std::vector<ValuePtr>{value}));
    std::weak_ptr<Value> weak_out = out;
    out->m_backward = [value, weak_out]()
    {
        if (auto out = weak_out.lock())
        {
            value->m_grad += (out->m_data > 0.0 ? 1.0 : 0.0) * out->m_grad;
        }
    };
    return out;
}

void Backward(const ValuePtr& root)
{
    if (!root)
        return;

    std::unordered_set<const Value*> visited;
    std::vector<ValuePtr> topo;
    BuildTopo(root, visited, topo);

    root->SetGrad(1.0);
    for (auto it = topo.rbegin(); it != topo.rend(); ++it)
    {
        ValueGraphAccess::ApplyBackward(**it);
    }
}

} // namespace forg::nn
