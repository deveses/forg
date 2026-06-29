#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "forg/nn.h"

#include <cmath>
#include <filesystem>
#include <memory>
#include <random>
#include <string>
#include <vector>

using Catch::Approx;

namespace {

std::filesystem::path MnistDataPath(const char* name)
{
    return std::filesystem::path(FORG_TEST_DATA_DIR) / "mnist" / name;
}

} // namespace

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

TEST_CASE("Value exp log and sigmoid compute gradients", "[nn][value]")
{
    using namespace forg::nn;

    const ValuePtr x = MakeValue(2.0);
    const ValuePtr y = Log(Exp(x));
    Backward(y);

    REQUIRE(y->GetData() == Approx(2.0));
    REQUIRE(x->GetGrad() == Approx(1.0));

    const ValuePtr z = MakeValue(0.0);
    const ValuePtr sigmoid = Sigmoid(z);
    Backward(sigmoid);

    REQUIRE(sigmoid->GetData() == Approx(0.5));
    REQUIRE(z->GetGrad() == Approx(0.25));

    REQUIRE_FALSE(Exp(nullptr));
    REQUIRE_FALSE(Log(nullptr));
    REQUIRE_FALSE(Sigmoid(nullptr));
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

TEST_CASE("Linear and Sequential compose scalar modules", "[nn][module]")
{
    using namespace forg::nn;

    std::mt19937 rng(42);
    Linear linear(3, 2, rng);
    const Values input = {MakeValue(1.0), MakeValue(-2.0), MakeValue(0.5)};
    const Values output = linear.Forward(input);

    REQUIRE(output.size() == 2);
    REQUIRE(linear.Parameters().size() == 8);

    Sequential model({
        std::make_shared<Linear>(3, 4, rng),
        std::make_shared<ReLU>(),
        std::make_shared<Linear>(4, 1, rng),
    });
    const Values prediction = model.Forward(input);

    REQUIRE(prediction.size() == 1);
    REQUIRE(model.Parameters().size() == 21);
}

TEST_CASE("Flatten converts numeric inputs into Values", "[nn][module]")
{
    using namespace forg::nn;

    const Values flat = Flatten::From({0.0, 0.5, 1.0});
    REQUIRE(flat.size() == 3);
    REQUIRE(flat[0]->GetData() == Approx(0.0));
    REQUIRE(flat[1]->GetData() == Approx(0.5));
    REQUIRE(flat[2]->GetData() == Approx(1.0));

    const Values image = Flatten::FromImage({
        {0.25, 0.75},
        {1.0}
    });
    REQUIRE(image.size() == 3);
    REQUIRE(image[0]->GetData() == Approx(0.25));
    REQUIRE(image[1]->GetData() == Approx(0.75));
    REQUIRE(image[2]->GetData() == Approx(1.0));
}

TEST_CASE("Flatten and OneHot can reuse Value storage", "[nn][module]")
{
    using namespace forg::nn;

    Values input;
    REQUIRE(Flatten::Into({0.0, 0.5, 1.0}, input));
    REQUIRE(input.size() == 3);

    const ValuePtr first = input[0];
    const ValuePtr second = input[1];
    const ValuePtr third = input[2];

    input[0]->SetGrad(4.0);
    REQUIRE(Flatten::Into({1.0, 0.25, 0.0}, input));
    REQUIRE(input[0] == first);
    REQUIRE(input[1] == second);
    REQUIRE(input[2] == third);
    REQUIRE(input[0]->GetData() == Approx(1.0));
    REQUIRE(input[1]->GetData() == Approx(0.25));
    REQUIRE(input[2]->GetData() == Approx(0.0));
    REQUIRE(input[0]->GetGrad() == Approx(0.0));

    Values target;
    REQUIRE(OneHotInto(3, 2, target));
    REQUIRE(target.size() == 3);
    const ValuePtr hot = target[2];
    target[2]->SetGrad(7.0);

    REQUIRE(OneHotInto(3, 0, target));
    REQUIRE(target[2] == hot);
    REQUIRE(target[0]->GetData() == Approx(1.0));
    REQUIRE(target[2]->GetData() == Approx(0.0));
    REQUIRE(target[2]->GetGrad() == Approx(0.0));
}

TEST_CASE("Loss, classification helpers, and SGD support training loops",
          "[nn][module]")
{
    using namespace forg::nn;

    const Values target = OneHot(3, 1);
    REQUIRE(target.size() == 3);
    REQUIRE(target[0]->GetData() == Approx(0.0));
    REQUIRE(target[1]->GetData() == Approx(1.0));
    REQUIRE(target[2]->GetData() == Approx(0.0));

    const Values scores = {MakeValue(-1.0), MakeValue(2.0), MakeValue(0.5)};
    REQUIRE(ArgMax(scores) == 1);
    REQUIRE(OneHot(0, 0).empty());
    REQUIRE(OneHot(3, 4).empty());

    const ValuePtr weight = MakeValue(0.0);
    const Values prediction = {weight * 2.0};
    const Values expected = {MakeValue(4.0)};
    const ValuePtr loss = MSELoss(prediction, expected);

    REQUIRE(loss);
    REQUIRE(loss->GetData() == Approx(16.0));

    SGD optimizer({weight}, 0.1);
    optimizer.ZeroGrad();
    Backward(loss);
    REQUIRE(weight->GetGrad() == Approx(-16.0));

    optimizer.Step();
    REQUIRE(weight->GetData() == Approx(1.6));

    optimizer.ZeroGrad();
    REQUIRE(weight->GetGrad() == Approx(0.0));
}

TEST_CASE("Softmax and cross entropy support multiclass training",
          "[nn][module]")
{
    using namespace forg::nn;

    const Values logits = {MakeValue(1.0), MakeValue(2.0), MakeValue(3.0)};
    const Values probabilities = Softmax(logits);

    REQUIRE(probabilities.size() == 3);
    const double denominator = std::exp(-2.0) + std::exp(-1.0) + 1.0;
    REQUIRE(probabilities[0]->GetData() ==
            Approx(std::exp(-2.0) / denominator));
    REQUIRE(probabilities[1]->GetData() ==
            Approx(std::exp(-1.0) / denominator));
    REQUIRE(probabilities[2]->GetData() == Approx(1.0 / denominator));

    const ValuePtr loss = CrossEntropyLoss(logits, 2);
    REQUIRE(loss);
    REQUIRE(loss->GetData() == Approx(-std::log(1.0 / denominator)));

    Backward(loss);
    REQUIRE(logits[0]->GetGrad() == Approx(probabilities[0]->GetData()));
    REQUIRE(logits[1]->GetGrad() == Approx(probabilities[1]->GetData()));
    REQUIRE(logits[2]->GetGrad() == Approx(probabilities[2]->GetData() - 1.0));
}

TEST_CASE("Cross entropy accepts one-hot targets and rejects invalid input",
          "[nn][module]")
{
    using namespace forg::nn;

    const Values logits = {MakeValue(1.0), MakeValue(2.0), MakeValue(3.0)};
    const Values target = OneHot(3, 2);
    const ValuePtr loss = CrossEntropyLoss(logits, target);
    REQUIRE(loss);

    REQUIRE(CrossEntropyLoss(logits, 3) == nullptr);
    REQUIRE(CrossEntropyLoss({}, 0) == nullptr);
    REQUIRE(CrossEntropyLoss(logits, Values{MakeValue(1.0)}) == nullptr);
    REQUIRE(Softmax({}).empty());
    REQUIRE(Softmax({MakeValue(1.0), nullptr}).empty());
}

TEST_CASE("Backward scratch can be reused across passes", "[nn][value]")
{
    using namespace forg::nn;

    const ValuePtr weight = MakeValue(2.0);
    BackwardScratch scratch;

    ValuePtr loss = Pow(weight * 3.0 - 1.0, 2.0);
    Backward(loss, scratch);
    REQUIRE(weight->GetGrad() == Approx(30.0));
    REQUIRE_FALSE(scratch.topo.empty());
    REQUIRE_FALSE(scratch.visited.empty());

    weight->SetGrad(0.0);
    loss = Pow(weight * 4.0 - 1.0, 2.0);
    Backward(loss, scratch);
    REQUIRE(weight->GetGrad() == Approx(56.0));
    REQUIRE_FALSE(scratch.topo.empty());
    REQUIRE_FALSE(scratch.visited.empty());
}

TEST_CASE("MNIST dataset reads valid IDX image and label files", "[nn][mnist]")
{
    const std::filesystem::path images =
        MnistDataPath("valid-images.idx3-ubyte");
    const std::filesystem::path labels =
        MnistDataPath("valid-labels.idx1-ubyte");

    forg::nn::MnistDataset dataset;
    REQUIRE(dataset.Load(images.string(), labels.string()));
    REQUIRE(dataset.Rows() == 2);
    REQUIRE(dataset.Columns() == 2);
    REQUIRE(dataset.ImageSize() == 4);
    REQUIRE(dataset.Samples().size() == 2);
    REQUIRE(dataset.Samples()[0].label == 3);
    REQUIRE(dataset.Samples()[1].label == 9);
    REQUIRE(dataset.Samples()[0].pixels[0] == Approx(0.0));
    REQUIRE(dataset.Samples()[0].pixels[2] == Approx(1.0));
    REQUIRE(dataset.Samples()[1].pixels[2] == Approx(128.0 / 255.0));
}

TEST_CASE("MNIST dataset rejects invalid magic numbers", "[nn][mnist]")
{
    const std::filesystem::path images =
        MnistDataPath("invalid-magic-images.idx3-ubyte");
    const std::filesystem::path labels =
        MnistDataPath("invalid-magic-labels.idx1-ubyte");

    forg::nn::MnistDataset dataset;
    REQUIRE_FALSE(dataset.Load(images.string(), labels.string()));
    REQUIRE_FALSE(dataset.Error().empty());
}

TEST_CASE("MNIST dataset rejects mismatched image and label counts",
          "[nn][mnist]")
{
    const std::filesystem::path images =
        MnistDataPath("mismatch-images.idx3-ubyte");
    const std::filesystem::path labels =
        MnistDataPath("mismatch-labels.idx1-ubyte");

    forg::nn::MnistDataset dataset;
    REQUIRE_FALSE(dataset.Load(images.string(), labels.string()));
    REQUIRE_FALSE(dataset.Error().empty());
}
