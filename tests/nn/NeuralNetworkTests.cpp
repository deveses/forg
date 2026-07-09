#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "forg/nn.h"

#include <cmath>
#include <fstream>
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

std::filesystem::path TempDataPath(const std::string& name)
{
    return std::filesystem::temp_directory_path() /
           (name + "-" + std::to_string(std::random_device{}()));
}

bool HasAnyGradient(const forg::nn::Values& values)
{
    for (const forg::nn::ValuePtr& value : values)
    {
        if (value && value->GetGrad() != 0.0)
            return true;
    }
    return false;
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

TEST_CASE("Value exp log sigmoid and tanh compute gradients", "[nn][value]")
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

    const ValuePtr t = MakeValue(0.5);
    const ValuePtr tanh = Tanh(t);
    Backward(tanh);

    REQUIRE(tanh->GetData() == Approx(std::tanh(0.5)));
    REQUIRE(t->GetGrad() == Approx(1.0 - std::tanh(0.5) * std::tanh(0.5)));

    REQUIRE_FALSE(Exp(nullptr));
    REQUIRE_FALSE(Log(nullptr));
    REQUIRE_FALSE(Sigmoid(nullptr));
    REQUIRE_FALSE(Tanh(nullptr));
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
    REQUIRE(forg::nn::RNN(0, 2, 3, rng).Parameters().empty());
    REQUIRE(forg::nn::RNN(2, 0, 3, rng).Parameters().empty());
    REQUIRE(forg::nn::RNN(2, 3, 0, rng).Parameters().empty());
    REQUIRE(forg::nn::LSTM(0, 2, 3, rng).Parameters().empty());
    REQUIRE(forg::nn::LSTM(2, 0, 3, rng).Parameters().empty());
    REQUIRE(forg::nn::LSTM(2, 3, 0, rng).Parameters().empty());
    REQUIRE(forg::nn::GRU(0, 2, 3, rng).Parameters().empty());
    REQUIRE(forg::nn::GRU(2, 0, 3, rng).Parameters().empty());
    REQUIRE(forg::nn::GRU(2, 3, 0, rng).Parameters().empty());

    forg::nn::Layer layer(2, 1, rng);
    REQUIRE(layer.Forward({forg::nn::MakeValue(1.0)}).empty());
    REQUIRE(layer.Forward({forg::nn::MakeValue(1.0), nullptr}).empty());

    forg::nn::Embedding invalid_embedding(0, 3, rng);
    REQUIRE(invalid_embedding.Parameters().empty());
    REQUIRE(invalid_embedding.Forward(std::vector<std::size_t>{0}).empty());

    forg::nn::Embedding embedding(2, 3, rng);
    REQUIRE(embedding.Forward(std::vector<std::size_t>{2}).empty());
    REQUIRE(embedding.Forward({forg::nn::MakeValue(-1.0)}).empty());
    REQUIRE(embedding.Forward({forg::nn::MakeValue(0.5)}).empty());
    REQUIRE(embedding.Forward({nullptr}).empty());

    forg::nn::RNN rnn(2, 3, 4, rng);
    REQUIRE(rnn.Forward({forg::nn::MakeValue(1.0)}).empty());
    REQUIRE(rnn.Forward({forg::nn::MakeValue(1.0), nullptr}).empty());
    REQUIRE(rnn.Forward(
                   std::vector<forg::nn::ValuePtr>{
                       forg::nn::MakeValue(1.0),
                       forg::nn::MakeValue(2.0),
                       forg::nn::MakeValue(3.0),
                       forg::nn::MakeValue(4.0),
                       forg::nn::MakeValue(5.0),
                       forg::nn::MakeValue(6.0),
                       forg::nn::MakeValue(7.0),
                       forg::nn::MakeValue(8.0),
                   },
                   {forg::nn::MakeValue(0.0)})
                .empty());

    forg::nn::LSTM lstm(2, 3, 4, rng);
    REQUIRE(lstm.Forward({forg::nn::MakeValue(1.0)}).empty());
    REQUIRE(lstm.Forward({forg::nn::MakeValue(1.0), nullptr}).empty());
    const std::vector<forg::nn::ValuePtr> sequence = {
        forg::nn::MakeValue(1.0), forg::nn::MakeValue(2.0),
        forg::nn::MakeValue(3.0), forg::nn::MakeValue(4.0),
        forg::nn::MakeValue(5.0), forg::nn::MakeValue(6.0),
        forg::nn::MakeValue(7.0), forg::nn::MakeValue(8.0),
    };
    REQUIRE(lstm.Forward(sequence, {forg::nn::MakeValue(0.0)},
                         {forg::nn::MakeValue(0.0), forg::nn::MakeValue(0.0),
                          forg::nn::MakeValue(0.0)})
                .empty());
    REQUIRE(lstm.Forward(sequence,
                         {forg::nn::MakeValue(0.0), forg::nn::MakeValue(0.0),
                          forg::nn::MakeValue(0.0)},
                         {forg::nn::MakeValue(0.0)})
                .empty());

    forg::nn::GRU gru(2, 3, 4, rng);
    REQUIRE(gru.Forward({forg::nn::MakeValue(1.0)}).empty());
    REQUIRE(gru.Forward({forg::nn::MakeValue(1.0), nullptr}).empty());
    REQUIRE(gru.Forward(sequence, {forg::nn::MakeValue(0.0)}).empty());
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

TEST_CASE("RNN returns full hidden sequence and exposes parameters",
          "[nn][module]")
{
    using namespace forg::nn;

    std::mt19937 rng(45);
    RNN rnn(2, 3, 4, rng);

    REQUIRE(rnn.InputSize() == 2);
    REQUIRE(rnn.HiddenSize() == 3);
    REQUIRE(rnn.SequenceLength() == 4);
    REQUIRE(rnn.InputWeights().size() == 6);
    REQUIRE(rnn.HiddenWeights().size() == 9);
    REQUIRE(rnn.Biases().size() == 3);
    REQUIRE(rnn.Parameters().size() == 18);

    const Values input = {
        MakeValue(1.0), MakeValue(2.0), MakeValue(3.0), MakeValue(4.0),
        MakeValue(5.0), MakeValue(6.0), MakeValue(7.0), MakeValue(8.0),
    };
    const Values output = rnn.Forward(input);

    REQUIRE(output.size() == 12);
}

TEST_CASE("RNN applies recurrent tanh dynamics across timesteps",
          "[nn][module]")
{
    using namespace forg::nn;

    std::mt19937 rng(46);
    RNN rnn(1, 1, 2, rng);
    rnn.InputWeights()[0]->SetData(1.0);
    rnn.HiddenWeights()[0]->SetData(1.0);
    rnn.Biases()[0]->SetData(0.0);

    const Values output = rnn.Forward({MakeValue(1.0), MakeValue(2.0)});
    const double first = std::tanh(1.0);
    const double second = std::tanh(2.0 + first);

    REQUIRE(output.size() == 2);
    REQUIRE(output[0]->GetData() == Approx(first));
    REQUIRE(output[1]->GetData() == Approx(second));
}

TEST_CASE("RNN gradients flow through inputs recurrent weights and biases",
          "[nn][module]")
{
    using namespace forg::nn;

    std::mt19937 rng(47);
    RNN rnn(1, 1, 2, rng);
    rnn.InputWeights()[0]->SetData(0.8);
    rnn.HiddenWeights()[0]->SetData(0.6);
    rnn.Biases()[0]->SetData(0.1);

    const Values input = {MakeValue(0.5), MakeValue(-0.25)};
    const Values output = rnn.Forward(input);
    REQUIRE(output.size() == 2);

    Backward(output[1]);

    REQUIRE(input[0]->GetGrad() != Approx(0.0));
    REQUIRE(input[1]->GetGrad() != Approx(0.0));
    REQUIRE(rnn.InputWeights()[0]->GetGrad() != Approx(0.0));
    REQUIRE(rnn.HiddenWeights()[0]->GetGrad() != Approx(0.0));
    REQUIRE(rnn.Biases()[0]->GetGrad() != Approx(0.0));
}

TEST_CASE("RNN accepts explicit initial hidden state with gradients",
          "[nn][module]")
{
    using namespace forg::nn;

    std::mt19937 rng(48);
    RNN rnn(1, 1, 1, rng);
    rnn.InputWeights()[0]->SetData(0.0);
    rnn.HiddenWeights()[0]->SetData(1.0);
    rnn.Biases()[0]->SetData(0.0);

    const ValuePtr initial = MakeValue(0.25);
    const Values output = rnn.Forward({MakeValue(0.0)}, {initial});

    REQUIRE(output.size() == 1);
    REQUIRE(output[0]->GetData() == Approx(std::tanh(0.25)));

    Backward(output[0]);
    REQUIRE(initial->GetGrad() ==
            Approx(1.0 - std::tanh(0.25) * std::tanh(0.25)));
}

TEST_CASE("RNN composes with Sequential using flattened full sequence",
          "[nn][module]")
{
    using namespace forg::nn;

    std::mt19937 rng(49);
    const auto rnn = std::make_shared<RNN>(1, 2, 2, rng);
    const auto linear = std::make_shared<Linear>(4, 1, rng);
    Sequential model({rnn, linear});

    const Values output = model.Forward({MakeValue(0.5), MakeValue(-0.5)});

    REQUIRE(output.size() == 1);
    REQUIRE(model.Parameters().size() ==
            rnn->Parameters().size() + linear->Parameters().size());
}

TEST_CASE("LSTM returns full hidden sequence and exposes parameters",
          "[nn][module]")
{
    using namespace forg::nn;

    std::mt19937 rng(50);
    LSTM lstm(2, 3, 4, rng);

    REQUIRE(lstm.InputSize() == 2);
    REQUIRE(lstm.HiddenSize() == 3);
    REQUIRE(lstm.SequenceLength() == 4);
    REQUIRE(lstm.InputWeights().size() == 24);
    REQUIRE(lstm.HiddenWeights().size() == 36);
    REQUIRE(lstm.Biases().size() == 12);
    REQUIRE(lstm.Parameters().size() == 72);

    const Values input = {
        MakeValue(1.0), MakeValue(2.0), MakeValue(3.0), MakeValue(4.0),
        MakeValue(5.0), MakeValue(6.0), MakeValue(7.0), MakeValue(8.0),
    };
    const Values output = lstm.Forward(input);

    REQUIRE(output.size() == 12);
}

TEST_CASE("LSTM applies gated cell dynamics across timesteps", "[nn][module]")
{
    using namespace forg::nn;

    std::mt19937 rng(51);
    LSTM lstm(1, 1, 2, rng);
    for (const ValuePtr& parameter : lstm.Parameters())
    {
        parameter->SetData(0.0);
    }
    lstm.InputWeights()[2]->SetData(1.0);

    const Values output = lstm.Forward({MakeValue(1.0), MakeValue(2.0)});
    const double first_cell = 0.5 * std::tanh(1.0);
    const double first_hidden = 0.5 * std::tanh(first_cell);
    const double second_cell = 0.5 * first_cell + 0.5 * std::tanh(2.0);
    const double second_hidden = 0.5 * std::tanh(second_cell);

    REQUIRE(output.size() == 2);
    REQUIRE(output[0]->GetData() == Approx(first_hidden));
    REQUIRE(output[1]->GetData() == Approx(second_hidden));
}

TEST_CASE("LSTM gradients flow through sequence parameters and states",
          "[nn][module]")
{
    using namespace forg::nn;

    std::mt19937 rng(52);
    LSTM lstm(1, 1, 2, rng);
    for (std::size_t index = 0; index < lstm.InputWeights().size(); ++index)
    {
        lstm.InputWeights()[index]->SetData(0.2 + 0.1 * index);
    }
    for (std::size_t index = 0; index < lstm.HiddenWeights().size(); ++index)
    {
        lstm.HiddenWeights()[index]->SetData(0.1 + 0.05 * index);
    }
    for (const ValuePtr& bias : lstm.Biases())
    {
        bias->SetData(0.1);
    }

    const Values input = {MakeValue(0.5), MakeValue(-0.25)};
    const ValuePtr initial_hidden = MakeValue(0.2);
    const ValuePtr initial_cell = MakeValue(-0.1);
    const Values output = lstm.Forward(input, {initial_hidden}, {initial_cell});
    REQUIRE(output.size() == 2);

    Backward(output[1]);

    REQUIRE(input[0]->GetGrad() != Approx(0.0));
    REQUIRE(input[1]->GetGrad() != Approx(0.0));
    REQUIRE(initial_hidden->GetGrad() != Approx(0.0));
    REQUIRE(initial_cell->GetGrad() != Approx(0.0));
    REQUIRE(HasAnyGradient(lstm.InputWeights()));
    REQUIRE(HasAnyGradient(lstm.HiddenWeights()));
    REQUIRE(HasAnyGradient(lstm.Biases()));
}

TEST_CASE("LSTM composes with Sequential using flattened full sequence",
          "[nn][module]")
{
    using namespace forg::nn;

    std::mt19937 rng(53);
    const auto lstm = std::make_shared<LSTM>(1, 2, 2, rng);
    const auto linear = std::make_shared<Linear>(4, 1, rng);
    Sequential model({lstm, linear});

    const Values output = model.Forward({MakeValue(0.5), MakeValue(-0.5)});

    REQUIRE(output.size() == 1);
    REQUIRE(model.Parameters().size() ==
            lstm->Parameters().size() + linear->Parameters().size());
}

TEST_CASE("GRU returns full hidden sequence and exposes parameters",
          "[nn][module]")
{
    using namespace forg::nn;

    std::mt19937 rng(54);
    GRU gru(2, 3, 4, rng);

    REQUIRE(gru.InputSize() == 2);
    REQUIRE(gru.HiddenSize() == 3);
    REQUIRE(gru.SequenceLength() == 4);
    REQUIRE(gru.InputWeights().size() == 18);
    REQUIRE(gru.HiddenWeights().size() == 27);
    REQUIRE(gru.Biases().size() == 9);
    REQUIRE(gru.Parameters().size() == 54);

    const Values input = {
        MakeValue(1.0), MakeValue(2.0), MakeValue(3.0), MakeValue(4.0),
        MakeValue(5.0), MakeValue(6.0), MakeValue(7.0), MakeValue(8.0),
    };
    const Values output = gru.Forward(input);

    REQUIRE(output.size() == 12);
}

TEST_CASE("GRU applies gated hidden dynamics across timesteps", "[nn][module]")
{
    using namespace forg::nn;

    std::mt19937 rng(55);
    GRU gru(1, 1, 2, rng);
    for (const ValuePtr& parameter : gru.Parameters())
    {
        parameter->SetData(0.0);
    }
    gru.InputWeights()[2]->SetData(1.0);

    const Values output = gru.Forward({MakeValue(1.0), MakeValue(2.0)});
    const double first_hidden = 0.5 * std::tanh(1.0);
    const double second_hidden = 0.5 * std::tanh(2.0) + 0.5 * first_hidden;

    REQUIRE(output.size() == 2);
    REQUIRE(output[0]->GetData() == Approx(first_hidden));
    REQUIRE(output[1]->GetData() == Approx(second_hidden));
}

TEST_CASE("GRU gradients flow through sequence parameters and initial state",
          "[nn][module]")
{
    using namespace forg::nn;

    std::mt19937 rng(56);
    GRU gru(1, 1, 2, rng);
    for (std::size_t index = 0; index < gru.InputWeights().size(); ++index)
    {
        gru.InputWeights()[index]->SetData(0.2 + 0.1 * index);
    }
    for (std::size_t index = 0; index < gru.HiddenWeights().size(); ++index)
    {
        gru.HiddenWeights()[index]->SetData(0.1 + 0.05 * index);
    }
    for (const ValuePtr& bias : gru.Biases())
    {
        bias->SetData(0.1);
    }

    const Values input = {MakeValue(0.5), MakeValue(-0.25)};
    const ValuePtr initial_hidden = MakeValue(0.2);
    const Values output = gru.Forward(input, {initial_hidden});
    REQUIRE(output.size() == 2);

    Backward(output[1]);

    REQUIRE(input[0]->GetGrad() != Approx(0.0));
    REQUIRE(input[1]->GetGrad() != Approx(0.0));
    REQUIRE(initial_hidden->GetGrad() != Approx(0.0));
    REQUIRE(HasAnyGradient(gru.InputWeights()));
    REQUIRE(HasAnyGradient(gru.HiddenWeights()));
    REQUIRE(HasAnyGradient(gru.Biases()));
}

TEST_CASE("GRU composes with Sequential using flattened full sequence",
          "[nn][module]")
{
    using namespace forg::nn;

    std::mt19937 rng(57);
    const auto gru = std::make_shared<GRU>(1, 2, 2, rng);
    const auto linear = std::make_shared<Linear>(4, 1, rng);
    Sequential model({gru, linear});

    const Values output = model.Forward({MakeValue(0.5), MakeValue(-0.5)});

    REQUIRE(output.size() == 1);
    REQUIRE(model.Parameters().size() ==
            gru->Parameters().size() + linear->Parameters().size());
}

TEST_CASE("Recurrent layers expose final state alongside full sequence",
          "[nn][module]")
{
    using namespace forg::nn;

    std::mt19937 rng(58);

    RNN rnn(1, 2, 3, rng);
    const RecurrentState rnn_state =
        rnn.ForwardState({MakeValue(0.1), MakeValue(0.2), MakeValue(0.3)});
    REQUIRE(rnn_state.sequence.size() == 6);
    REQUIRE(rnn_state.hidden.size() == 2);
    REQUIRE(rnn_state.cell.empty());
    REQUIRE(rnn_state.hidden[0] == rnn_state.sequence[4]);
    REQUIRE(rnn_state.hidden[1] == rnn_state.sequence[5]);

    LSTM lstm(1, 2, 3, rng);
    const RecurrentState lstm_state =
        lstm.ForwardState({MakeValue(0.1), MakeValue(0.2), MakeValue(0.3)});
    REQUIRE(lstm_state.sequence.size() == 6);
    REQUIRE(lstm_state.hidden.size() == 2);
    REQUIRE(lstm_state.cell.size() == 2);
    REQUIRE(lstm_state.hidden[0] == lstm_state.sequence[4]);
    REQUIRE(lstm_state.hidden[1] == lstm_state.sequence[5]);

    GRU gru(1, 2, 3, rng);
    const RecurrentState gru_state =
        gru.ForwardState({MakeValue(0.1), MakeValue(0.2), MakeValue(0.3)});
    REQUIRE(gru_state.sequence.size() == 6);
    REQUIRE(gru_state.hidden.size() == 2);
    REQUIRE(gru_state.cell.empty());
    REQUIRE(gru_state.hidden[0] == gru_state.sequence[4]);
    REQUIRE(gru_state.hidden[1] == gru_state.sequence[5]);
}

TEST_CASE("EncoderDecoder exposes dimensions shapes and parameters",
          "[nn][module]")
{
    using namespace forg::nn;

    std::mt19937 rng(59);
    const Values encoder_input = {
        MakeValue(0.1), MakeValue(0.2), MakeValue(0.3), MakeValue(0.4),
    };
    const Values decoder_input = {
        MakeValue(0.5), MakeValue(0.6), MakeValue(0.7), MakeValue(0.8),
        MakeValue(0.9), MakeValue(1.0),
    };

    EncoderDecoder rnn(2, 2, 3, 2, 3, RecurrentCellType::RNN, rng);
    REQUIRE(rnn.EncoderInputSize() == 2);
    REQUIRE(rnn.DecoderInputSize() == 2);
    REQUIRE(rnn.HiddenSize() == 3);
    REQUIRE(rnn.EncoderLength() == 2);
    REQUIRE(rnn.DecoderLength() == 3);
    REQUIRE(rnn.CellType() == RecurrentCellType::RNN);
    REQUIRE(rnn.Forward(encoder_input, decoder_input).size() == 9);
    REQUIRE(rnn.Parameters().size() == 36);

    EncoderDecoder lstm(2, 2, 3, 2, 3, RecurrentCellType::LSTM, rng);
    REQUIRE(lstm.Forward(encoder_input, decoder_input).size() == 9);
    REQUIRE(lstm.Parameters().size() == 144);

    EncoderDecoder gru(2, 2, 3, 2, 3, RecurrentCellType::GRU, rng);
    REQUIRE(gru.Forward(encoder_input, decoder_input).size() == 9);
    REQUIRE(gru.Parameters().size() == 108);
}

TEST_CASE("EncoderDecoder passes RNN encoder state to decoder",
          "[nn][module]")
{
    using namespace forg::nn;

    std::mt19937 rng(60);
    EncoderDecoder model(1, 1, 1, 1, 1, RecurrentCellType::RNN, rng);
    const Values parameters = model.Parameters();
    REQUIRE(parameters.size() == 6);
    for (const ValuePtr& parameter : parameters)
    {
        parameter->SetData(0.0);
    }
    parameters[0]->SetData(1.0);
    parameters[4]->SetData(1.0);

    const Values output = model.Forward({MakeValue(1.0)}, {MakeValue(0.0)});
    REQUIRE(output.size() == 1);
    REQUIRE(output[0]->GetData() == Approx(std::tanh(std::tanh(1.0))));
}

TEST_CASE("LSTM EncoderDecoder gradients flow through encoder and decoder",
          "[nn][module]")
{
    using namespace forg::nn;

    std::mt19937 rng(61);
    EncoderDecoder model(1, 1, 1, 2, 2, RecurrentCellType::LSTM, rng);
    for (const ValuePtr& parameter : model.Parameters())
    {
        parameter->SetData(0.2);
    }

    const Values encoder_input = {MakeValue(0.5), MakeValue(-0.25)};
    const Values decoder_input = {MakeValue(0.75), MakeValue(0.1)};
    const RecurrentState encoder_state = model.Encode(encoder_input);
    const RecurrentState decoder_state =
        model.Decode(decoder_input, encoder_state);
    REQUIRE(decoder_state.sequence.size() == 2);

    Backward(decoder_state.sequence.back());

    REQUIRE(encoder_input[0]->GetGrad() != Approx(0.0));
    REQUIRE(encoder_input[1]->GetGrad() != Approx(0.0));
    REQUIRE(decoder_input[0]->GetGrad() != Approx(0.0));
    REQUIRE(decoder_input[1]->GetGrad() != Approx(0.0));
    REQUIRE(encoder_state.hidden[0]->GetGrad() != Approx(0.0));
    REQUIRE(encoder_state.cell[0]->GetGrad() != Approx(0.0));
    REQUIRE(HasAnyGradient(model.Parameters()));
}

TEST_CASE("Seq2Seq projects decoder states at each timestep", "[nn][module]")
{
    using namespace forg::nn;

    std::mt19937 rng(62);
    Seq2Seq model(2, 2, 3, 4, 2, 3, RecurrentCellType::GRU, rng);
    const Values encoder_input = {
        MakeValue(0.1), MakeValue(0.2), MakeValue(0.3), MakeValue(0.4),
    };
    const Values decoder_input = {
        MakeValue(0.5), MakeValue(0.6), MakeValue(0.7), MakeValue(0.8),
        MakeValue(0.9), MakeValue(1.0),
    };

    const Values output = model.Forward(encoder_input, decoder_input);
    REQUIRE(output.size() == 12);
    REQUIRE(model.OutputSize() == 4);
    REQUIRE(model.Parameters().size() == 124);

    Values concatenated = encoder_input;
    concatenated.insert(concatenated.end(), decoder_input.begin(),
                        decoder_input.end());
    REQUIRE(model.Forward(concatenated).size() == 12);
}

TEST_CASE("EncoderDecoder and Seq2Seq reject invalid inputs", "[nn][module]")
{
    using namespace forg::nn;

    std::mt19937 rng(63);
    EncoderDecoder encoder_decoder(1, 1, 2, 2, 2, RecurrentCellType::GRU, rng);
    REQUIRE(encoder_decoder.Forward({MakeValue(1.0)}, {MakeValue(2.0)})
                .empty());
    REQUIRE(encoder_decoder.Forward(
                               {MakeValue(1.0), nullptr, MakeValue(2.0),
                                MakeValue(3.0)})
                .empty());
    REQUIRE(EncoderDecoder(0, 1, 1, 1, 1, RecurrentCellType::RNN, rng)
                .Parameters()
                .empty());

    Seq2Seq seq2seq(1, 1, 2, 2, 2, 2, RecurrentCellType::RNN, rng);
    REQUIRE(seq2seq.Forward({MakeValue(1.0)}, {MakeValue(2.0)}).empty());
    REQUIRE(seq2seq.Forward({MakeValue(1.0), MakeValue(2.0), nullptr,
                             MakeValue(3.0)})
                .empty());
    REQUIRE(Seq2Seq(1, 1, 1, 0, 1, 1, RecurrentCellType::RNN, rng)
                .Parameters()
                .empty());
}

TEST_CASE("LayerNorm normalizes each flat sequence token", "[nn][module]")
{
    using namespace forg::nn;

    LayerNorm norm(2, 2);
    const Values input = {
        MakeValue(1.0),
        MakeValue(3.0),
        MakeValue(2.0),
        MakeValue(6.0),
    };

    const Values output = norm.Forward(input);
    REQUIRE(output.size() == 4);
    REQUIRE(norm.Parameters().size() == 4);
    REQUIRE(output[0]->GetData() == Approx(-1.0).margin(1e-4));
    REQUIRE(output[1]->GetData() == Approx(1.0).margin(1e-4));
    REQUIRE(output[2]->GetData() == Approx(-1.0).margin(1e-4));
    REQUIRE(output[3]->GetData() == Approx(1.0).margin(1e-4));
    REQUIRE(norm.Forward({MakeValue(1.0), nullptr, MakeValue(2.0),
                          MakeValue(3.0)})
                .empty());
}

TEST_CASE("ScaledDotProductAttention supports causal masking",
          "[nn][module]")
{
    using namespace forg::nn;

    ScaledDotProductAttention attention(1, 1, 2, 2, AttentionMask::Causal);
    const Values query = {MakeValue(1.0), MakeValue(1.0)};
    const Values key = {MakeValue(1.0), MakeValue(1.0)};
    const Values value = {MakeValue(2.0), MakeValue(8.0)};

    const Values output = attention.Forward(query, key, value);
    REQUIRE(output.size() == 2);
    REQUIRE(output[0]->GetData() == Approx(2.0));
    REQUIRE(output[1]->GetData() == Approx(5.0));

    Backward(output[0]);
    REQUIRE(value[0]->GetGrad() == Approx(1.0));
    REQUIRE(value[1]->GetGrad() == Approx(0.0));
    REQUIRE(attention.Forward({query[0]}, key, value).empty());
}

TEST_CASE("MultiHeadAttention exposes parameters and gradients",
          "[nn][module]")
{
    using namespace forg::nn;

    std::mt19937 rng(64);
    MultiHeadAttention attention(4, 2, 3, rng);
    const Values input = {
        MakeValue(0.1), MakeValue(0.2), MakeValue(0.3), MakeValue(0.4),
        MakeValue(0.5), MakeValue(0.6), MakeValue(0.7), MakeValue(0.8),
        MakeValue(0.9), MakeValue(1.0), MakeValue(1.1), MakeValue(1.2),
    };

    const Values output = attention.Forward(input);
    REQUIRE(output.size() == input.size());
    REQUIRE(attention.Parameters().size() == 80);

    Backward(output.front());
    REQUIRE(HasAnyGradient(input));
    REQUIRE(HasAnyGradient(attention.Parameters()));
    REQUIRE(MultiHeadAttention(3, 2, 3, rng).Parameters().empty());
    REQUIRE(attention.Forward({MakeValue(1.0)}).empty());
}

TEST_CASE("Transformer blocks preserve sequence shape", "[nn][module]")
{
    using namespace forg::nn;

    std::mt19937 rng(65);
    TransformerEncoderBlock encoder_block(4, 2, 2, 5, rng);
    const Values encoder_input = {
        MakeValue(0.1), MakeValue(0.2), MakeValue(0.3), MakeValue(0.4),
        MakeValue(0.5), MakeValue(0.6), MakeValue(0.7), MakeValue(0.8),
    };

    const Values encoder_output = encoder_block.Forward(encoder_input);
    REQUIRE(encoder_output.size() == encoder_input.size());
    REQUIRE(encoder_block.Parameters().size() == 145);

    TransformerDecoderBlock decoder_block(4, 2, 2, 3, 5, rng);
    const Values decoder_input = {
        MakeValue(0.2), MakeValue(0.3), MakeValue(0.4), MakeValue(0.5),
        MakeValue(0.6), MakeValue(0.7), MakeValue(0.8), MakeValue(0.9),
    };
    const Values encoder_memory = {
        MakeValue(0.1), MakeValue(0.2), MakeValue(0.3), MakeValue(0.4),
        MakeValue(0.5), MakeValue(0.6), MakeValue(0.7), MakeValue(0.8),
        MakeValue(0.9), MakeValue(1.0), MakeValue(1.1), MakeValue(1.2),
    };

    const Values decoder_output =
        decoder_block.Forward(decoder_input, encoder_memory);
    REQUIRE(decoder_output.size() == decoder_input.size());

    TransformerEncoder encoder(2, 4, 2, 2, 5, rng);
    REQUIRE(encoder.LayerCount() == 2);
    REQUIRE(encoder.Forward(encoder_input).size() == encoder_input.size());
    REQUIRE(encoder.Parameters().size() == 290);
}

TEST_CASE("AttentionSeq2Seq attends over encoder hidden sequence",
          "[nn][module]")
{
    using namespace forg::nn;

    std::mt19937 rng(66);
    AttentionSeq2Seq model(1, 1, 2, 2, 2, 1, RecurrentCellType::RNN, rng);
    const Values encoder_input = {MakeValue(0.1), MakeValue(0.2)};
    const Values decoder_input = {MakeValue(0.3)};

    const Values output = model.Forward(encoder_input, decoder_input);
    REQUIRE(output.size() == 2);
    REQUIRE(model.OutputSize() == 2);
    REQUIRE(model.HeadCount() == 1);
    REQUIRE(model.Parameters().size() == 46);

    const RecurrentState encoder_state =
        model.InnerEncoderDecoder()->Encode(encoder_input);
    const RecurrentState decoder_state =
        model.InnerEncoderDecoder()->Decode(decoder_input, encoder_state);
    const Values context = model.Attention()->Forward(
        decoder_state.sequence, encoder_state.sequence, encoder_state.sequence);
    REQUIRE(context.size() == 2);

    Backward(context.front());
    REQUIRE(encoder_state.sequence.front()->GetGrad() != Approx(0.0));
    REQUIRE(HasAnyGradient(model.Attention()->Parameters()));
    REQUIRE(AttentionSeq2Seq(1, 1, 3, 2, 2, 1, RecurrentCellType::RNN, rng, 2)
                .Parameters()
                .empty());
}

TEST_CASE("Embedding maps token indices to trainable rows", "[nn][module]")
{
    using namespace forg::nn;

    std::mt19937 rng(43);
    Embedding embedding(4, 3, rng);
    REQUIRE(embedding.NumEmbeddings() == 4);
    REQUIRE(embedding.EmbeddingDim() == 3);
    REQUIRE(embedding.Parameters().size() == 12);

    for (std::size_t index = 0; index < embedding.Weights().size(); ++index)
    {
        embedding.Weights()[index]->SetData(static_cast<double>(index) + 0.5);
    }

    const Values output = embedding.Forward(std::vector<std::size_t>{2, 0});
    REQUIRE(output.size() == 6);
    REQUIRE(output[0] == embedding.Weights()[6]);
    REQUIRE(output[1] == embedding.Weights()[7]);
    REQUIRE(output[2] == embedding.Weights()[8]);
    REQUIRE(output[3] == embedding.Weights()[0]);
    REQUIRE(output[4] == embedding.Weights()[1]);
    REQUIRE(output[5] == embedding.Weights()[2]);
    REQUIRE(output[0]->GetData() == Approx(6.5));
    REQUIRE(output[5]->GetData() == Approx(2.5));

    Backward(output[0] + output[5]);
    REQUIRE(embedding.Weights()[6]->GetGrad() == Approx(1.0));
    REQUIRE(embedding.Weights()[2]->GetGrad() == Approx(1.0));
    REQUIRE(embedding.Weights()[7]->GetGrad() == Approx(0.0));
}

TEST_CASE("Embedding accepts scalar Value indices in module pipelines",
          "[nn][module]")
{
    using namespace forg::nn;

    std::mt19937 rng(44);
    const auto embedding = std::make_shared<Embedding>(5, 2, rng);
    for (std::size_t index = 0; index < embedding->Weights().size(); ++index)
    {
        embedding->Weights()[index]->SetData(static_cast<double>(index));
    }

    Sequential model({
        embedding,
        std::make_shared<Linear>(4, 1, rng),
    });
    Values linear_parameters = model.Modules()[1]->Parameters();
    for (const ValuePtr& parameter : linear_parameters)
    {
        parameter->SetData(1.0);
    }

    const Values prediction = model.Forward({MakeValue(1.0), MakeValue(3.0)});
    REQUIRE(prediction.size() == 1);
    REQUIRE(prediction[0]->GetData() == Approx(19.0));

    Backward(prediction[0]);
    REQUIRE(embedding->Weights()[2]->GetGrad() == Approx(1.0));
    REQUIRE(embedding->Weights()[3]->GetGrad() == Approx(1.0));
    REQUIRE(embedding->Weights()[6]->GetGrad() == Approx(1.0));
    REQUIRE(embedding->Weights()[7]->GetGrad() == Approx(1.0));
    REQUIRE(embedding->Weights()[0]->GetGrad() == Approx(0.0));
}

TEST_CASE("Matrix stores row-major numeric data", "[nn][matrix]")
{
    forg::nn::Matrix matrix(2, 3, 1.0);
    REQUIRE(matrix.Rows() == 2);
    REQUIRE(matrix.Columns() == 3);
    REQUIRE(matrix.Size() == 6);

    matrix(1, 2) = 5.0;
    REQUIRE(matrix(0, 0) == Approx(1.0));
    REQUIRE(matrix(1, 2) == Approx(5.0));
    REQUIRE(matrix.Data()[5] == Approx(5.0));
}

TEST_CASE("MatrixMLP trains batched dense classifier", "[nn][matrix]")
{
    std::mt19937 rng(9);
    forg::nn::MatrixMLP model(2, 4, 2, rng);
    forg::nn::Matrix input(4, 2);
    input(0, 0) = 1.0;
    input(0, 1) = 0.0;
    input(1, 0) = 0.9;
    input(1, 1) = 0.1;
    input(2, 0) = 0.0;
    input(2, 1) = 1.0;
    input(3, 0) = 0.1;
    input(3, 1) = 0.9;
    const std::vector<std::size_t> labels = {0, 0, 1, 1};

    const double first_loss = model.TrainBatch(input, labels, 0.1);
    double last_loss = first_loss;
    for (std::size_t step = 0; step < 200; ++step)
    {
        last_loss = model.TrainBatch(input, labels, 0.1);
    }

    REQUIRE(first_loss > 0.0);
    REQUIRE(last_loss < first_loss);
    REQUIRE(model.Predict({1.0, 0.0}) == 0);
    REQUIRE(model.Predict({0.0, 1.0}) == 1);
}

TEST_CASE("MatrixMLP multithreaded training matches single-thread training",
          "[nn][matrix]")
{
    std::mt19937 single_rng(12);
    std::mt19937 threaded_rng(12);
    forg::nn::MatrixMLP single_threaded(2, 4, 2, single_rng);
    forg::nn::MatrixMLP multithreaded(2, 4, 2, threaded_rng);
    single_threaded.SetThreadCount(1);
    multithreaded.SetThreadCount(2);
    REQUIRE(single_threaded.ThreadCount() == 1);
    REQUIRE(multithreaded.ThreadCount() == 2);

    forg::nn::Matrix input(6, 2);
    input(0, 0) = 1.0;
    input(0, 1) = 0.0;
    input(1, 0) = 0.9;
    input(1, 1) = 0.1;
    input(2, 0) = 0.8;
    input(2, 1) = 0.2;
    input(3, 0) = 0.0;
    input(3, 1) = 1.0;
    input(4, 0) = 0.1;
    input(4, 1) = 0.9;
    input(5, 0) = 0.2;
    input(5, 1) = 0.8;
    const std::vector<std::size_t> labels = {0, 0, 0, 1, 1, 1};

    const double single_loss = single_threaded.TrainBatch(input, labels, 0.05);
    const double threaded_loss = multithreaded.TrainBatch(input, labels, 0.05);
    REQUIRE(threaded_loss == Approx(single_loss));

    const forg::nn::Matrix single_output = single_threaded.Forward(input);
    const forg::nn::Matrix threaded_output = multithreaded.Forward(input);
    REQUIRE(threaded_output.Size() == single_output.Size());
    for (std::size_t index = 0; index < single_output.Size(); ++index)
    {
        REQUIRE(threaded_output.Data()[index] ==
                Approx(single_output.Data()[index]).epsilon(1e-12));
    }
}

TEST_CASE("MatrixMLP parameters can be saved and loaded", "[nn][matrix]")
{
    std::mt19937 source_rng(7);
    std::mt19937 target_rng(8);
    forg::nn::MatrixMLP source(2, 3, 2, source_rng);
    forg::nn::MatrixMLP target(2, 3, 2, target_rng);
    forg::nn::Matrix input(1, 2);
    input(0, 0) = 0.25;
    input(0, 1) = 0.75;

    const std::filesystem::path filename = TempDataPath("forg-nn-matrix");
    std::string error = "not cleared";
    REQUIRE(source.SaveParameters(filename.string(), &error));
    REQUIRE(error.empty());
    REQUIRE(target.LoadParameters(filename.string(), &error));
    REQUIRE(error.empty());

    const forg::nn::Matrix source_output = source.Forward(input);
    const forg::nn::Matrix target_output = target.Forward(input);
    REQUIRE(source_output.Rows() == target_output.Rows());
    REQUIRE(source_output.Columns() == target_output.Columns());
    for (std::size_t index = 0; index < source_output.Size(); ++index)
    {
        REQUIRE(target_output.Data()[index] ==
                Approx(source_output.Data()[index]));
    }

    forg::nn::MatrixMLP mismatched(2, 4, 2, target_rng);
    REQUIRE_FALSE(mismatched.LoadParameters(filename.string(), &error));
    REQUIRE_FALSE(error.empty());

    std::filesystem::remove(filename);
}

TEST_CASE("MatrixGPT validates config and returns batched logits",
          "[nn][matrix]")
{
    std::mt19937 rng(91);
    forg::nn::MatrixGPTConfig config;
    config.vocab_size = 5;
    config.block_size = 3;
    config.model_size = 8;
    config.head_count = 2;
    config.layer_count = 1;
    config.feed_forward_size = 12;

    forg::nn::MatrixGPT model(config, rng);
    REQUIRE(model.Valid());
    REQUIRE(model.ParameterCount() > 0);
    REQUIRE(model.Config().vocab_size == 5);

    const std::vector<std::size_t> input = {0, 1, 2, 2, 3, 4};
    const forg::nn::Matrix logits = model.Forward(input, 2);
    REQUIRE(logits.Rows() == 6);
    REQUIRE(logits.Columns() == 5);

    config.model_size = 7;
    forg::nn::MatrixGPT invalid(config, rng);
    REQUIRE_FALSE(invalid.Valid());
    REQUIRE(invalid.ParameterCount() == 0);
    REQUIRE(invalid.Forward(input, 2).Empty());
}

TEST_CASE("MatrixGPT training lowers tiny language-model loss",
          "[nn][matrix]")
{
    std::mt19937 rng(92);
    forg::nn::MatrixGPTConfig config;
    config.vocab_size = 3;
    config.block_size = 3;
    config.model_size = 8;
    config.head_count = 2;
    config.layer_count = 1;
    config.feed_forward_size = 16;

    forg::nn::MatrixGPT model(config, rng);
    const std::vector<std::size_t> input = {
        0, 1, 0,
        0, 1, 0,
        0, 1, 0,
        0, 1, 0,
    };
    const std::vector<std::size_t> target = {
        1, 0, 1,
        1, 0, 1,
        1, 0, 1,
        1, 0, 1,
    };

    const double first_loss = model.TrainBatch(input, target, 4, 0.01);
    double last_loss = first_loss;
    for (std::size_t step = 0; step < 80; ++step)
        last_loss = model.TrainBatch(input, target, 4, 0.01);

    REQUIRE(first_loss > 0.0);
    REQUIRE(last_loss < first_loss);
}

TEST_CASE("MatrixGPT causal attention ignores future tokens",
          "[nn][matrix]")
{
    std::mt19937 rng(93);
    forg::nn::MatrixGPTConfig config;
    config.vocab_size = 6;
    config.block_size = 4;
    config.model_size = 8;
    config.head_count = 2;
    config.layer_count = 1;
    config.feed_forward_size = 12;

    forg::nn::MatrixGPT model(config, rng);
    const forg::nn::Matrix first =
        model.Forward(std::vector<std::size_t>{1, 2, 3, 4}, 1);
    const forg::nn::Matrix second =
        model.Forward(std::vector<std::size_t>{1, 5, 5, 5}, 1);
    REQUIRE(first.Rows() == second.Rows());
    REQUIRE(first.Columns() == second.Columns());
    for (std::size_t column = 0; column < first.Columns(); ++column)
        REQUIRE(second(0, column) == Approx(first(0, column)));
}

TEST_CASE("MatrixGPT parameters can be saved and loaded", "[nn][matrix]")
{
    std::mt19937 source_rng(94);
    std::mt19937 target_rng(95);
    forg::nn::MatrixGPTConfig config;
    config.vocab_size = 5;
    config.block_size = 3;
    config.model_size = 8;
    config.head_count = 2;
    config.layer_count = 1;
    config.feed_forward_size = 12;

    forg::nn::MatrixGPT source(config, source_rng);
    forg::nn::MatrixGPT target(config, target_rng);
    const std::vector<std::size_t> input = {0, 1, 2};
    const forg::nn::Matrix source_before = source.Forward(input, 1);
    const forg::nn::Matrix target_before = target.Forward(input, 1);
    REQUIRE(source_before.Data() != target_before.Data());

    const std::filesystem::path filename = TempDataPath("forg-nn-matrix-gpt");
    std::string error = "not cleared";
    REQUIRE(source.SaveParameters(filename.string(), &error));
    REQUIRE(error.empty());
    REQUIRE(target.LoadParameters(filename.string(), &error));
    REQUIRE(error.empty());

    const forg::nn::Matrix target_after = target.Forward(input, 1);
    REQUIRE(source_before.Rows() == target_after.Rows());
    REQUIRE(source_before.Columns() == target_after.Columns());
    for (std::size_t index = 0; index < source_before.Size(); ++index)
    {
        REQUIRE(target_after.Data()[index] ==
                Approx(source_before.Data()[index]));
    }

    std::filesystem::remove(filename);
}

TEST_CASE("MatrixGPT threaded training matches single-thread training",
          "[nn][matrix]")
{
    std::mt19937 single_rng(96);
    std::mt19937 threaded_rng(96);
    forg::nn::MatrixGPTConfig config;
    config.vocab_size = 4;
    config.block_size = 3;
    config.model_size = 8;
    config.head_count = 2;
    config.layer_count = 1;
    config.feed_forward_size = 12;

    forg::nn::MatrixGPT single(config, single_rng);
    forg::nn::MatrixGPT threaded(config, threaded_rng);
    single.SetThreadCount(1);
    threaded.SetThreadCount(2);
    const std::vector<std::size_t> input = {0, 1, 2, 1, 2, 3};
    const std::vector<std::size_t> target = {1, 2, 3, 2, 3, 0};

    const double single_loss = single.TrainBatch(input, target, 2, 0.005);
    const double threaded_loss =
        threaded.TrainBatch(input, target, 2, 0.005);
    REQUIRE(threaded_loss == Approx(single_loss));

    const forg::nn::Matrix single_output = single.Forward(input, 2);
    const forg::nn::Matrix threaded_output = threaded.Forward(input, 2);
    REQUIRE(threaded_output.Size() == single_output.Size());
    for (std::size_t index = 0; index < single_output.Size(); ++index)
    {
        REQUIRE(threaded_output.Data()[index] ==
                Approx(single_output.Data()[index]).epsilon(1e-12));
    }
}

TEST_CASE("Model parameters can be saved and loaded", "[nn][module]")
{
    using namespace forg::nn;

    std::mt19937 source_rng(11);
    std::mt19937 target_rng(22);
    Linear source(2, 2, source_rng);
    Linear target(2, 2, target_rng);
    const Values source_parameters = source.Parameters();
    const Values target_parameters = target.Parameters();
    REQUIRE(source_parameters.size() == target_parameters.size());

    for (std::size_t index = 0; index < source_parameters.size(); ++index)
    {
        source_parameters[index]->SetData(static_cast<double>(index) + 0.25);
        target_parameters[index]->SetData(-10.0);
        target_parameters[index]->SetGrad(3.0);
    }

    const std::filesystem::path filename = TempDataPath("forg-nn-params");
    std::string error = "not cleared";
    REQUIRE(SaveParameters(source, filename.string(), &error));
    REQUIRE(error.empty());
    REQUIRE(LoadParameters(target, filename.string(), &error));
    REQUIRE(error.empty());

    for (std::size_t index = 0; index < source_parameters.size(); ++index)
    {
        REQUIRE(target_parameters[index]->GetData() ==
                Approx(source_parameters[index]->GetData()));
        REQUIRE(target_parameters[index]->GetGrad() == Approx(0.0));
    }

    std::filesystem::remove(filename);
}

TEST_CASE("Model parameter loading rejects invalid files", "[nn][module]")
{
    using namespace forg::nn;

    std::mt19937 rng(3);
    Linear source(2, 2, rng);
    Linear mismatched(2, 1, rng);
    const Values mismatched_parameters = mismatched.Parameters();
    for (const ValuePtr& parameter : mismatched_parameters)
    {
        parameter->SetData(7.0);
    }

    const std::filesystem::path valid_filename =
        TempDataPath("forg-nn-valid-params");
    std::string error;
    REQUIRE(SaveParameters(source, valid_filename.string(), &error));
    REQUIRE_FALSE(LoadParameters(mismatched, valid_filename.string(), &error));
    REQUIRE_FALSE(error.empty());
    for (const ValuePtr& parameter : mismatched_parameters)
    {
        REQUIRE(parameter->GetData() == Approx(7.0));
    }

    const std::filesystem::path invalid_filename =
        TempDataPath("forg-nn-invalid-params");
    {
        std::ofstream stream(invalid_filename);
        stream << "not a forg parameter file\n";
    }
    REQUIRE_FALSE(LoadParameters(source, invalid_filename.string(), &error));
    REQUIRE_FALSE(error.empty());

    std::filesystem::remove(valid_filename);
    std::filesystem::remove(invalid_filename);
}

TEST_CASE("Dropout respects training and evaluation modes", "[nn][module]")
{
    using namespace forg::nn;

    const Values input = {MakeValue(2.0), MakeValue(-3.0)};
    Dropout dropout(1.0);
    Values output = dropout.Forward(input);

    REQUIRE(output.size() == 2);
    REQUIRE(output[0]->GetData() == Approx(0.0));
    REQUIRE(output[1]->GetData() == Approx(0.0));

    Backward(output[0] + output[1]);
    REQUIRE(input[0]->GetGrad() == Approx(0.0));
    REQUIRE(input[1]->GetGrad() == Approx(0.0));

    dropout.Eval();
    output = dropout.Forward(input);
    REQUIRE(output.size() == 2);
    REQUIRE(output[0] == input[0]);
    REQUIRE(output[1] == input[1]);

    const auto shared_dropout = std::make_shared<Dropout>(1.0);
    Sequential model({shared_dropout});
    model.Eval();
    REQUIRE_FALSE(model.Training());
    REQUIRE_FALSE(shared_dropout->Training());
}

TEST_CASE("Conv2d applies channel-first scalar convolution", "[nn][module]")
{
    using namespace forg::nn;

    std::mt19937 rng(5);
    Conv2d conv(1, 1, 3, 3, 2, 2, 1, 0, rng);
    REQUIRE(conv.OutputHeight() == 2);
    REQUIRE(conv.OutputWidth() == 2);
    REQUIRE(conv.Parameters().size() == 5);

    for (const ValuePtr& weight : conv.Weights())
    {
        weight->SetData(1.0);
    }
    conv.Biases()[0]->SetData(0.0);

    const Values input = {
        MakeValue(1.0), MakeValue(2.0), MakeValue(3.0),
        MakeValue(4.0), MakeValue(5.0), MakeValue(6.0),
        MakeValue(7.0), MakeValue(8.0), MakeValue(9.0),
    };
    const Values output = conv.Forward(input);

    REQUIRE(output.size() == 4);
    REQUIRE(output[0]->GetData() == Approx(12.0));
    REQUIRE(output[1]->GetData() == Approx(16.0));
    REQUIRE(output[2]->GetData() == Approx(24.0));
    REQUIRE(output[3]->GetData() == Approx(28.0));

    Backward(output[0]);
    REQUIRE(input[0]->GetGrad() == Approx(1.0));
    REQUIRE(input[1]->GetGrad() == Approx(1.0));
    REQUIRE(input[3]->GetGrad() == Approx(1.0));
    REQUIRE(input[4]->GetGrad() == Approx(1.0));
    REQUIRE(input[8]->GetGrad() == Approx(0.0));
    REQUIRE(conv.Weights()[0]->GetGrad() == Approx(1.0));
    REQUIRE(conv.Weights()[1]->GetGrad() == Approx(2.0));
    REQUIRE(conv.Weights()[2]->GetGrad() == Approx(4.0));
    REQUIRE(conv.Weights()[3]->GetGrad() == Approx(5.0));
    REQUIRE(conv.Biases()[0]->GetGrad() == Approx(1.0));
}

TEST_CASE("MaxPool2d forwards the maximum value with gradient routing",
          "[nn][module]")
{
    using namespace forg::nn;

    MaxPool2d pool(1, 2, 2, 2, 2);
    const Values input = {
        MakeValue(1.0),
        MakeValue(3.0),
        MakeValue(2.0),
        MakeValue(4.0),
    };
    const Values output = pool.Forward(input);

    REQUIRE(output.size() == 1);
    REQUIRE(output[0] == input[3]);
    REQUIRE(output[0]->GetData() == Approx(4.0));

    Backward(output[0]);
    REQUIRE(input[0]->GetGrad() == Approx(0.0));
    REQUIRE(input[1]->GetGrad() == Approx(0.0));
    REQUIRE(input[2]->GetGrad() == Approx(0.0));
    REQUIRE(input[3]->GetGrad() == Approx(1.0));
}

TEST_CASE("BatchNorm normalizes vectors and exposes affine parameters",
          "[nn][module]")
{
    using namespace forg::nn;

    BatchNorm norm(3);
    const Values input = {MakeValue(1.0), MakeValue(2.0), MakeValue(3.0)};
    const Values output = norm.Forward(input);

    REQUIRE(output.size() == 3);
    REQUIRE(norm.Parameters().size() == 6);
    REQUIRE(output[0]->GetData() + output[1]->GetData() +
                output[2]->GetData() ==
            Approx(0.0));

    Backward(output[0]);
    REQUIRE(norm.Scale()[0]->GetGrad() != Approx(0.0));
    REQUIRE(norm.Bias()[0]->GetGrad() == Approx(1.0));
}

TEST_CASE("Image-shaped modules reject invalid shapes", "[nn][module]")
{
    using namespace forg::nn;

    Conv2d invalid_conv(1, 1, 2, 2, 3, 3);
    REQUIRE(invalid_conv.Parameters().empty());
    REQUIRE(invalid_conv.Forward({MakeValue(1.0)}).empty());

    MaxPool2d invalid_pool(1, 2, 2, 3, 3);
    REQUIRE(invalid_pool
                .Forward({
                    MakeValue(1.0),
                    MakeValue(2.0),
                    MakeValue(3.0),
                    MakeValue(4.0),
                })
                .empty());

    BatchNorm norm(2);
    REQUIRE(norm.Forward({MakeValue(1.0)}).empty());
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

TEST_CASE("SGD can scale accumulated gradients for batches", "[nn][module]")
{
    using namespace forg::nn;

    const ValuePtr weight = MakeValue(0.0);
    SGD optimizer({weight}, 0.1);

    optimizer.ZeroGrad();
    Backward(Pow(weight * 2.0 - 4.0, 2.0));
    Backward(Pow(weight * 2.0 - 4.0, 2.0));
    REQUIRE(weight->GetGrad() == Approx(-32.0));

    optimizer.Step(0.5);
    REQUIRE(weight->GetData() == Approx(1.6));
}

TEST_CASE("MomentumSGD accumulates velocity across steps", "[nn][module]")
{
    using namespace forg::nn;

    const ValuePtr weight = MakeValue(0.0);
    MomentumSGD optimizer({weight}, 0.1, 0.9);

    weight->SetGrad(2.0);
    optimizer.Step();
    REQUIRE(weight->GetData() == Approx(-0.2));
    REQUIRE(optimizer.Velocity()[0] == Approx(2.0));

    weight->SetGrad(3.0);
    optimizer.Step(0.5);
    REQUIRE(optimizer.Velocity()[0] == Approx(3.3));
    REQUIRE(weight->GetData() == Approx(-0.53));

    optimizer.ZeroGrad();
    REQUIRE(weight->GetGrad() == Approx(0.0));
}

TEST_CASE("Adam applies bias-corrected adaptive updates", "[nn][module]")
{
    using namespace forg::nn;

    const ValuePtr weight = MakeValue(0.0);
    Adam optimizer({weight}, 0.1);

    weight->SetGrad(4.0);
    optimizer.Step(0.5);

    REQUIRE(optimizer.StepCount() == 1);
    REQUIRE(optimizer.FirstMoment()[0] == Approx(0.2));
    REQUIRE(optimizer.SecondMoment()[0] == Approx(0.004));
    REQUIRE(weight->GetData() == Approx(-0.1).epsilon(1e-6));

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
