#include "forg/nn/Parameters.h"

#include "ModuleUtils.h"

#include <cstddef>
#include <fstream>
#include <iomanip>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

namespace forg::nn {
using namespace detail;

bool SaveParameters(const Module& module, const std::string& filename,
                    std::string* error)
{
    const Values parameters = module.Parameters();
    for (const ValuePtr& parameter : parameters)
    {
        if (!parameter)
        {
            SetError(error, "Cannot save null model parameter");
            return false;
        }
    }

    std::ofstream stream(filename);
    if (!stream)
    {
        SetError(error, "Unable to open parameter file for writing");
        return false;
    }

    stream << "FORG_NN_PARAMETERS 1\n";
    stream << parameters.size() << '\n';
    stream << std::setprecision(std::numeric_limits<double>::max_digits10);
    for (const ValuePtr& parameter : parameters)
    {
        stream << parameter->GetData() << '\n';
    }

    if (!stream)
    {
        SetError(error, "Unable to write parameter file");
        return false;
    }

    SetError(error, {});
    return true;
}

bool LoadParameters(Module& module, const std::string& filename,
                    std::string* error)
{
    std::ifstream stream(filename);
    if (!stream)
    {
        SetError(error, "Unable to open parameter file for reading");
        return false;
    }

    std::string magic;
    int version = 0;
    if (!(stream >> magic >> version) || magic != "FORG_NN_PARAMETERS" ||
        version != 1)
    {
        SetError(error, "Invalid parameter file header");
        return false;
    }

    std::size_t count = 0;
    if (!(stream >> count))
    {
        SetError(error, "Invalid parameter count");
        return false;
    }

    const Values parameters = module.Parameters();
    if (parameters.size() != count)
    {
        std::ostringstream message;
        message << "Parameter count mismatch: file has " << count
                << ", model expects " << parameters.size();
        SetError(error, message.str());
        return false;
    }

    std::vector<double> values;
    values.reserve(count);
    for (std::size_t index = 0; index < count; ++index)
    {
        double value = 0.0;
        if (!(stream >> value))
        {
            SetError(error, "Invalid parameter value");
            return false;
        }
        values.push_back(value);
    }

    for (const ValuePtr& parameter : parameters)
    {
        if (!parameter)
        {
            SetError(error, "Cannot load null model parameter");
            return false;
        }
    }

    for (std::size_t index = 0; index < parameters.size(); ++index)
    {
        parameters[index]->SetData(values[index]);
        parameters[index]->SetGrad(0.0);
    }

    SetError(error, {});
    return true;
}

} // namespace forg::nn
