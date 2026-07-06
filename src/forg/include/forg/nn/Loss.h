/*******************************************************************************
    This source file is part of FORG library.

    Scalar loss and classification helpers.
*******************************************************************************/

#pragma once
#include "forg/nn/Value.h"

#include <cstddef>

namespace forg::nn {

/// Mean squared error over prediction and target vectors.
ValuePtr MSELoss(const Values& prediction, const Values& target);

/// Numerically shifted softmax over raw logits.
Values Softmax(const Values& logits);

/// Cross-entropy loss for raw logits and a class index target.
ValuePtr CrossEntropyLoss(const Values& logits, std::size_t target_index);

/// Cross-entropy loss for raw logits and a one-hot or soft target vector.
ValuePtr CrossEntropyLoss(const Values& logits, const Values& target);

/// Creates a one-hot vector with class_count entries.
Values OneHot(std::size_t class_count, std::size_t index);

/// Reuses or creates one-hot Value storage in output.
bool OneHotInto(std::size_t class_count, std::size_t index, Values& output);

/// Returns the index of the largest data value, or max size_t for empty input.
std::size_t ArgMax(const Values& values);

} // namespace forg::nn
