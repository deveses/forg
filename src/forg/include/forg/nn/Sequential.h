/*******************************************************************************
    This source file is part of FORG library.

    Sequential scalar module composition.
*******************************************************************************/

#pragma once
#include "forg/nn/Module.h"

#include <memory>
#include <vector>

namespace forg::nn {

/// Runs a list of modules in order.
///
/// Parameters are returned in module order. Train() and Eval() propagate to
/// child modules.
class Sequential : public Module
{
  public:
    Sequential() = default;
    explicit Sequential(std::vector<std::shared_ptr<Module>> modules);

    void Add(std::shared_ptr<Module> module);
    Values Forward(const Values& input) const override;
    void Train(bool training = true) override;
    Values Parameters() const override;

    const std::vector<std::shared_ptr<Module>>& Modules() const noexcept
    {
        return m_modules;
    }

  private:
    std::vector<std::shared_ptr<Module>> m_modules;
};

} // namespace forg::nn
