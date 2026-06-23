#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "forg/nn.h"

#include <random>
#include <vector>

using Catch::Approx;

TEST_CASE("Value backward matches micrograd sanity expression", "[nn][value]")
{
    using namespace forg::nn;

    const ValuePtr x = MakeValue(-4.0);
    const ValuePtr z = 2.0 * x + 2.0 + x;
    const ValuePtr q = Relu(z) + z * x;
    const ValuePtr h = Relu(z * z);
    const ValuePtr y = h + q + q * x;

    Backward(y);

    REQUIRE(y->GetData() == Approx(-20.0));
    REQUIRE(x->GetGrad() == Approx(46.0));
}

TEST_CASE("Value backward matches micrograd mixed operator expression",
          "[nn][value]")
{
    using namespace forg::nn;

    const ValuePtr a = MakeValue(-4.0);
    const ValuePtr b = MakeValue(2.0);
    ValuePtr c = a + b;
    ValuePtr d = a * b + Pow(b, 3.0);
    c = c + c + 1.0;
    c = c + 1.0 + c + (-a);
    d = d + d * 2.0 + Relu(b + a);
    d = d + 3.0 * d + Relu(b - a);
    const ValuePtr e = c - d;
    const ValuePtr f = Pow(e, 2.0);
    ValuePtr g = f / 2.0;
    g = g + 10.0 / f;

    Backward(g);

    REQUIRE(g->GetData() == Approx(24.704081632653061));
    REQUIRE(a->GetGrad() == Approx(138.83381924198252));
    REQUIRE(b->GetGrad() == Approx(645.5772594752186));
}

TEST_CASE("Value gradients accumulate and relu gates negative inputs",
          "[nn][value]")
{
    using namespace forg::nn;

    const ValuePtr x = MakeValue(2.0);
    Backward(x * 2.0);
    Backward(x * 3.0);
    REQUIRE(x->GetGrad() == Approx(5.0));

    const ValuePtr negative = MakeValue(-1.0);
    Backward(Relu(negative));
    REQUIRE(negative->GetGrad() == Approx(0.0));

    REQUIRE_FALSE(Relu(nullptr));
    Backward(nullptr);
}

TEST_CASE("Value data can be updated for simple gradient descent",
          "[nn][value]")
{
    using namespace forg::nn;

    const ValuePtr weight = MakeValue(0.0);
    const ValuePtr prediction = weight * 2.0;
    const ValuePtr target = MakeValue(4.0);
    const ValuePtr loss = Pow(prediction - target, 2.0);

    Backward(loss);
    REQUIRE(loss->GetData() == Approx(16.0));
    REQUIRE(weight->GetGrad() == Approx(-16.0));

    weight->SetData(weight->GetData() - 0.1 * weight->GetGrad());
    weight->SetGrad(0.0);

    REQUIRE(weight->GetData() == Approx(1.6));
    REQUIRE(weight->GetGrad() == Approx(0.0));
}

TEST_CASE("Module zeroes parameters and MLP builds deterministic shapes",
          "[nn][module]")
{
    std::mt19937 rng(1234);
    forg::nn::MLP mlp(3, {4, 2, 1}, rng);

    REQUIRE(mlp.Layers().size() == 3);
    REQUIRE(mlp.Layers()[0].Neurons().size() == 4);
    REQUIRE(mlp.Layers()[1].Neurons().size() == 2);
    REQUIRE(mlp.Layers()[2].Neurons().size() == 1);
    REQUIRE(mlp.Layers()[0].Neurons()[0].Nonlinear());
    REQUIRE_FALSE(mlp.Layers()[2].Neurons()[0].Nonlinear());

    const std::vector<forg::nn::ValuePtr> input = {
        forg::nn::MakeValue(1.0),
        forg::nn::MakeValue(-2.0),
        forg::nn::MakeValue(0.5),
    };
    const std::vector<forg::nn::ValuePtr> output = mlp.Forward(input);

    REQUIRE(output.size() == 1);
    REQUIRE(mlp.Parameters().size() == 29);

    forg::nn::Backward(output.front());
    bool has_gradient = false;
    for (const forg::nn::ValuePtr& parameter : mlp.Parameters())
    {
        has_gradient = has_gradient || parameter->GetGrad() != 0.0;
    }
    REQUIRE(has_gradient);

    mlp.ZeroGrad();
    for (const forg::nn::ValuePtr& parameter : mlp.Parameters())
    {
        REQUIRE(parameter->GetGrad() == Approx(0.0));
    }
}

TEST_CASE("Neural modules return empty results for invalid shapes",
          "[nn][module]")
{
    std::mt19937 rng(1);

    REQUIRE(forg::nn::Neuron(0, rng).Parameters().empty());
    REQUIRE(forg::nn::Layer(2, 0, rng).Parameters().empty());
    REQUIRE(forg::nn::MLP(2, {}, rng).Parameters().empty());
    REQUIRE(forg::nn::MLP(2, {3, 0}, rng).Parameters().empty());

    forg::nn::Layer layer(2, 1, rng);
    REQUIRE(layer.Forward({forg::nn::MakeValue(1.0)}).empty());
    REQUIRE(layer.Forward({forg::nn::MakeValue(1.0), nullptr}).empty());
}
