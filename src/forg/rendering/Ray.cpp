#include "forg_pch.h"

#include "rendering/Ray.h"

namespace forg {

    // Ray-Triangle intersection using barycentric coordinates
    // based on "Tomas Möller and Ben Trumbor - Fast, Minimum Storage Ray-Triangle Intersection"
    // R(t) = O + tD
    // O = ray's origin, D = normalized direction
    // T(u,v) = (1 - u - v)V0 + uV1 + vV2
    // (u,v) - barycentric coordinates where u,v >=0 and u+v<=1
    // we need to resolve R(t) = T(u,v) =>
    // O + tD = (1 - u - v)V0 + uV1 + vV2
    // [-D, V1 - V0, V2 - V0] [t u v]^T = O - V0
    // so Det = |-D, E1, E2| = -D . (E1 x E2) = E1.(D x E2) = E1.P,
    // Det_x  = | T, E1, E2| =  T . (E1 x E2) = E2.(T x E1) = E2.Q,
    // Det_y  = |-D,  T, E2| = -D . ( T x E2) =  T.(D x E2) = T.P,
    // Det_z  = |-D, E1,  T| = -D . (E1 x  T) =  D.(T x E1) = D.Q
    // where E1 = V1-V0, E2 = V2-V0, T = O-V0, P = DxE2, Q = TxE1
    // notice A.(BxC) = B.(CxA), AxB = -BxA
    bool Ray::TriangleIntersection(
        const Vector3& vert0,
        const Vector3& vert1,
        const Vector3& vert2,
        float epsilon,
        bool culling,
        float& t,
        float& u,
        float& v) const
    {
        Vector3 edge1, edge2, tvec, pvec, qvec;
        float det, inv_det;

        t = u = v = 0.0f;

        Vector3::Substract(edge1, vert1, vert0);
        Vector3::Substract(edge2, vert2, vert0);

        Vector3::Cross(pvec, Direction, edge2);
        det = Vector3::Dot(edge1, pvec);
        inv_det = 1.0f/det;

        if (culling)
        {
            if (det<epsilon) return false;

            tvec = Origin - vert0;

            u = Vector3::Dot(tvec, pvec);
            if (u < 0.0f || u > det)
                return false;

            Vector3::Cross(qvec, tvec, edge1);
            v = Vector3::Dot(Direction, qvec);
            if (v < 0.0f || u + v > det)
                return false;

            t = Vector3::Dot(edge2, qvec);

            t *= inv_det;
            u *= inv_det;
            v *= inv_det;
        }
        else
        {
            if (det > -epsilon && det < epsilon)
                return false;

            tvec = Origin - vert0;

            u = Vector3::Dot(tvec, pvec)*inv_det;
            if (u < 0.0f || u > 1.0f)
                return false;

            Vector3::Cross(qvec, tvec, edge1);
            v = Vector3::Dot(Direction, qvec)*inv_det;
            if (v < 0.0f || u + v > 1.0f)
                return false;

            t = Vector3::Dot(edge2, qvec)*inv_det;
        }

        return true;
    }


}
