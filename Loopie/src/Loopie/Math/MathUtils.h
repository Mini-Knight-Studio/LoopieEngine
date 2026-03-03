#pragma once
#include "MathTypes.h"

namespace Loopie
{
    namespace Math
    {
        static constexpr float PI = 3.14159265358979323846f;
        static constexpr float DEG2RAD = PI / 180.0f;
        static constexpr float RAD2DEG = 180.0f / PI;
        static constexpr float EPSILON = 1e-6f;

        inline matrix4 ToMatrix4(const quaternion& q) {
            return toMat4(q);
        }

        inline quaternion ToQuaternion(const matrix3& m) {
            return toQuat(m);
        }

        inline bool Approximately(float a, float b, float epsilon = EPSILON) {
            return abs(a - b) < epsilon;
        }

        inline void DecomposeMatrix(const matrix4& matrix, vec3& position, quaternion& rotation, glm::vec3& scale)
        {
            position = vec3(matrix[3]);
            vec3 x_axis = vec3(matrix[0]);
            vec3 y_axis = vec3(matrix[1]);
            vec3 z_axis = vec3(matrix[2]);

            vec3 magnitudeScale = vec3(glm::length(x_axis), glm::length(y_axis), glm::length(z_axis));

            float determinant = glm::determinant(matrix3(matrix));
            vec3 signScale = vec3(1.0f, 1.0f, 1.0f);

            if (determinant < 0)
                signScale.x = -1.0f;

            const float min_scale = 0.0001f;

            scale.x = signScale.x * glm::max(magnitudeScale.x, min_scale);
            scale.y = signScale.y * glm::max(magnitudeScale.y, min_scale);
            scale.z = signScale.z * glm::max(magnitudeScale.z, min_scale);

            matrix3 rotationMatrix = matrix3(matrix);

            rotationMatrix[0] = x_axis / scale.x;
            rotationMatrix[1] = y_axis / scale.y;
            rotationMatrix[2] = z_axis / scale.z;

            rotation = Math::ToQuaternion(rotationMatrix);
        }

    }
}