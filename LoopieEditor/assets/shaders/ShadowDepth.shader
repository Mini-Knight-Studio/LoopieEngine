[vertex]
#version 460 core

layout (location = 0) in vec3 a_Position;

// GPU Skinning
layout (location = 5) in vec4 a_BoneIDs;
layout (location = 6) in vec4 a_Weights;

layout (std430, binding = 2) readonly buffer BoneMatrices
{
    mat4 lp_Bones[];
};

layout (std140, binding = 3) uniform Shadows
{
    mat4 lp_LightSpaceMatrix[4];
};

uniform mat4 lp_Transform;
uniform int lp_ShadowSlotIndex;
uniform bool lp_Skinned;

void main() 
{
	mat4 skinMatrix = mat4(1.0);

    if (lp_Skinned)
    {
        skinMatrix =
              a_Weights[0] * lp_Bones[int(a_BoneIDs[0])] +
              a_Weights[1] * lp_Bones[int(a_BoneIDs[1])] +
              a_Weights[2] * lp_Bones[int(a_BoneIDs[2])] +
              a_Weights[3] * lp_Bones[int(a_BoneIDs[3])];
    }

	vec4 localPos = skinMatrix * vec4(a_Position, 1.0);

    vec4 worldPos = lp_Transform * localPos;
    gl_Position = lp_LightSpaceMatrix[lp_ShadowSlotIndex] * worldPos;
} 

[fragment]
#version 460 core
void main() {} // It's supposed to be empty.