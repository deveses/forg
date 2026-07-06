#include "forg/nn/Sequential.h"

#include <memory>
#include <utility>

namespace forg::nn {

Sequential::Sequential(std::vector<std::shared_ptr<Module>> modules)
    : m_modules(std::move(modules))
{
}

void Sequential::Add(std::shared_ptr<Module> module)
{
    if (module)
        m_modules.push_back(std::move(module));
}

Values Sequential::Forward(const Values& input) const
{
    Values output = input;
    for (const std::shared_ptr<Module>& module : m_modules)
    {
        if (!module)
            return {};

        output = module->Forward(output);
        if (output.empty())
            return {};
    }
    return output;
}

void Sequential::Train(bool training)
{
    Module::Train(training);
    for (const std::shared_ptr<Module>& module : m_modules)
    {
        if (module)
            module->Train(training);
    }
}

Values Sequential::Parameters() const
{
    Values parameters;
    for (const std::shared_ptr<Module>& module : m_modules)
    {
        if (!module)
            continue;

        Values module_parameters = module->Parameters();
        parameters.insert(parameters.end(), module_parameters.begin(),
                          module_parameters.end());
    }
    return parameters;
}

} // namespace forg::nn
