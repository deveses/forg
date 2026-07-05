#include "forg/nn/Loss.h"

#include <cstddef>
#include <limits>

namespace forg::nn {

ValuePtr MSELoss(const Values& prediction, const Values& target)
{
    if (prediction.empty() || prediction.size() != target.size())
        return nullptr;

    ValuePtr loss = MakeValue(0.0);
    for (std::size_t index = 0; index < prediction.size(); ++index)
    {
        if (!prediction[index] || !target[index])
            return nullptr;

        const ValuePtr error = prediction[index] - target[index];
        loss = loss + error * error;
        if (!loss)
            return nullptr;
    }
    return loss / static_cast<double>(prediction.size());
}

Values Softmax(const Values& logits)
{
    if (logits.empty())
        return {};

    for (const ValuePtr& logit : logits)
    {
        if (!logit)
            return {};
    }

    double max_logit = logits.front()->GetData();
    for (const ValuePtr& logit : logits)
    {
        if (logit->GetData() > max_logit)
            max_logit = logit->GetData();
    }

    Values exps;
    exps.reserve(logits.size());
    for (const ValuePtr& logit : logits)
    {
        exps.push_back(Exp(logit - max_logit));
        if (!exps.back())
            return {};
    }

    ValuePtr sum = exps.front();
    for (std::size_t index = 1; index < exps.size(); ++index)
    {
        sum = sum + exps[index];
        if (!sum)
            return {};
    }

    Values probabilities;
    probabilities.reserve(exps.size());
    for (const ValuePtr& value : exps)
    {
        probabilities.push_back(value / sum);
        if (!probabilities.back())
            return {};
    }
    return probabilities;
}

ValuePtr CrossEntropyLoss(const Values& logits, std::size_t target_index)
{
    if (target_index >= logits.size())
        return nullptr;

    const Values probabilities = Softmax(logits);
    if (probabilities.empty())
        return nullptr;

    return -Log(probabilities[target_index]);
}

ValuePtr CrossEntropyLoss(const Values& logits, const Values& target)
{
    if (logits.empty() || logits.size() != target.size())
        return nullptr;

    const Values probabilities = Softmax(logits);
    if (probabilities.empty())
        return nullptr;

    ValuePtr loss = MakeValue(0.0);
    for (std::size_t index = 0; index < target.size(); ++index)
    {
        if (!target[index])
            return nullptr;

        loss = loss - target[index] * Log(probabilities[index]);
        if (!loss)
            return nullptr;
    }
    return loss;
}

Values OneHot(std::size_t class_count, std::size_t index)
{
    Values output;
    OneHotInto(class_count, index, output);
    return output;
}

bool OneHotInto(std::size_t class_count, std::size_t index, Values& output)
{
    if (class_count == 0 || index >= class_count)
    {
        output.clear();
        return false;
    }

    if (output.size() != class_count)
    {
        output.clear();
        output.reserve(class_count);
        for (std::size_t class_index = 0; class_index < class_count;
             ++class_index)
        {
            output.push_back(MakeValue(class_index == index ? 1.0 : 0.0));
        }
        return true;
    }

    for (std::size_t class_index = 0; class_index < class_count; ++class_index)
    {
        if (!output[class_index])
            output[class_index] = MakeValue(class_index == index ? 1.0 : 0.0);
        else
            output[class_index]->SetData(class_index == index ? 1.0 : 0.0);

        output[class_index]->SetGrad(0.0);
    }
    return true;
}

std::size_t ArgMax(const Values& values)
{
    if (values.empty())
        return std::numeric_limits<std::size_t>::max();

    std::size_t best_index = 0;
    for (std::size_t index = 1; index < values.size(); ++index)
    {
        if (!values[index])
            continue;

        if (!values[best_index] ||
            values[index]->GetData() > values[best_index]->GetData())
        {
            best_index = index;
        }
    }
    return best_index;
}

} // namespace forg::nn
