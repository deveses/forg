#include "forg_pch.h"

#include "mesh/xfile/xtmembers.h"

namespace forg { namespace xfile {

    xstring XTemplatePrimitive::ToString() const
    {
        xstring output;

        output.append(GetPrimitiveTypeName(m_iType));
        output.append(" ");
        output.append(m_sOptionalName);

        return output;
    }

    xstring XTemplateReference::ToString() const
    {
        xstring output;

        output.append(m_sTemplateName);
        output.append(" ");
        output.append(m_sOptionalName);

        return output;
    }

    xstring XTemplateArray::ToString() const
    {
        xstring output;

        output.append("array ");

        if (m_bPrimitive)
            output.append(GetPrimitiveTypeName(m_iPrimitiveType));
        else
            output.append(m_sTypeName);

        output.append(" ");
        output.append(m_sName);

        for (size_t k=0; k<m_dimension_list.size(); k++)
        {
            const XArrayDimension& dim = m_dimension_list[k];
            if (dim.IsConstant())
            {
                char buf[128];

                sprintf(buf, "[]");
                sprintf(buf, "[%d]", dim.GetValue());

                output.append(buf);
            }
            else
            {
                output.append("[");
                output.append(dim.GetVariableName());
                output.append("]");
            }
        }

        return output;
    }

}}
