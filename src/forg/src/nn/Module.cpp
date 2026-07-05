#include "forg/nn/Module.h"

namespace forg::nn {

Values Module::Forward(const Values& input) const { return input; }

void Module::Train(bool training) { m_training = training; }

void Module::Eval() { Train(false); }

void Module::ZeroGrad()
{
    for (const ValuePtr& parameter : Parameters())
    {
        if (parameter)
            parameter->SetGrad(0.0);
    }
}

Values Module::Parameters() const { return {}; }

} // namespace forg::nn
