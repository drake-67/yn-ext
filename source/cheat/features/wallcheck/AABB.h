#pragma once
#include "../../../utils/math/math.hpp"

namespace rbx {
    struct OBB final {
        math::vector3_t center;
        math::vector3_t halfSize;
        math::vector3_t axes[3];

        OBB(const  math::vector3_t& center, const  math::vector3_t& size, const math::coordinate_frame_t& cf)
            : center(center), halfSize(size * 0.5f)
        {
            axes[0] = math::vector3_t(cf.right_vector.x, cf.up_vector.x, cf.back_vector.x);
            axes[1] = math::vector3_t(cf.right_vector.y, cf.up_vector.y, cf.back_vector.y);
            axes[2] = math::vector3_t(cf.right_vector.z, cf.up_vector.z, cf.back_vector.z);
        }

        bool intersectsRay(const math::vector3_t& rayOrigin, const math::vector3_t& rayDir, float maxDistance) const {
            float tmin = 0.0f;
            float tmax = maxDistance;

            bool originInside = true;
            math::vector3_t p = center - rayOrigin;

            for (int i = 0; i < 3; ++i) {
                float e = axes[i].dot(p);
                float f = axes[i].dot(rayDir);

                const float EPSILON = 1e-6f;
                if (fabs(f) > EPSILON) {
                    float t1 = (e + halfSize[i]) / f;
                    float t2 = (e - halfSize[i]) / f;

                    if (t1 > t2) std::swap(t1, t2);

                    tmin = max(tmin, t1);
                    tmax = min(tmax, t2);

                    if (tmax < tmin) {
                        return false;
                    }
                }
                else {
                    if (-e - halfSize[i] > 0.0f || -e + halfSize[i] < 0.0f) {
                        return false;
                    }
                }
            }

            return true;
        }
    };
}
