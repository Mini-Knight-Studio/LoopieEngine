#include "BillBoardComponent.h"
#include "Loopie/Components/Camera.h"

namespace Loopie
{
    Billboard::Billboard() {
        m_Btype = BillboardType::CAMERA_FACING;
        m_position = vec3(0);
        m_transform = matrix4(1.0f);
    }
	Billboard::Billboard(vec3 pos, BillboardType t)
	{
		m_Btype = t;
		m_position = pos;
		m_transform = matrix4(1.0f);
	}
    matrix4 Billboard::UpdateCalc(Camera* cam)
    {
        vec3 billboardPos = m_position;
        vec3 cameraPos = cam->GetPosition();
        vec3 cameraUp = cam->GetUp();

        switch (m_Btype)
        {
        case CAMERA_FACING:
        {
            matrix4 lookAtMatrix = glm::lookAt(billboardPos, cameraPos, cameraUp);
            m_transform = glm::inverse(lookAtMatrix);
            break;
        }
        case AXIS_ALIGNED:
        {
            vec3 axis = glm::normalize(m_axisAlignedAxis);
            vec3 toCamera = cameraPos - billboardPos;

            toCamera -= glm::dot(toCamera, axis) * axis;

            if (glm::length(toCamera) < 0.0001f)
            {
                break;
            }

            vec3 forward = glm::normalize(toCamera);
            vec3 right = glm::normalize(glm::cross(axis, forward));
            vec3 up = glm::normalize(glm::cross(forward, right));

            m_transform = matrix4(1.0f);
            m_transform[0] = vec4(right, 0.0f);
            m_transform[1] = vec4(up, 0.0f);
            m_transform[2] = vec4(forward, 0.0f);
            m_transform[3] = vec4(billboardPos, 1.0f);
            break;
        }
        case SCREEN_ALIGNED:
        {

            break;
        }
        }

        return m_transform;
    }

    matrix4 Billboard::UpdateCalcRotation(Camera* cam, const quaternion& emitterRot)
    {
        vec3 billboardPos = m_position;
        vec3 cameraPos = cam->GetPosition();
        vec3 cameraUp = cam->GetUp();

        matrix4 rotOnly = matrix4(1.0f);

        switch (m_Btype)
        {
        case CAMERA_FACING:
        {
            matrix4 lookAtMatrix = glm::lookAt(billboardPos, cameraPos, cameraUp);
            matrix4 fullInverse = glm::inverse(lookAtMatrix);
            rotOnly[0] = vec4(vec3(fullInverse[0]), 0.0f);
            rotOnly[1] = vec4(vec3(fullInverse[1]), 0.0f);
            rotOnly[2] = vec4(vec3(fullInverse[2]), 0.0f);
            break;
        }
        case AXIS_ALIGNED:
        {
            vec3 axis = glm::normalize(m_axisAlignedAxis);
            vec3 toCamera = cameraPos - billboardPos;
            toCamera -= glm::dot(toCamera, axis) * axis;

            if (glm::length(toCamera) < 0.0001f)
                break;

            vec3 forward = glm::normalize(toCamera);
            vec3 right = glm::normalize(glm::cross(axis, forward));
            vec3 up = axis;

            rotOnly[0] = vec4(right, 0.0f);
            rotOnly[1] = vec4(up, 0.0f);
            rotOnly[2] = vec4(forward, 0.0f);
            break;
        }
        case SCREEN_ALIGNED:
        {
            vec3 camForward = glm::normalize(billboardPos - cameraPos); 
            matrix4 lookAtMatrix = glm::lookAt(vec3(0.0f), -camForward, cameraUp);
            matrix4 fullInverse = glm::inverse(lookAtMatrix);
            rotOnly[0] = vec4(vec3(fullInverse[0]), 0.0f);
            rotOnly[1] = vec4(vec3(fullInverse[1]), 0.0f);
            rotOnly[2] = vec4(vec3(fullInverse[2]), 0.0f);
            break;
        }
        case NONE:
        {
            rotOnly = glm::mat4_cast(emitterRot);
            break;
        }
        }
        rotOnly[3] = vec4(0.0f, 0.0f, 0.0f, 1.0f);
        return rotOnly;
    }

	
	BillboardType Billboard::GetType()const
	{
		return m_Btype;
	}
	void Billboard::SetType(BillboardType t)
	{
		m_Btype = t;
	}
	vec3 Billboard::GetPosition()
	{
		return m_position;
	}
	void Billboard::SetPosition(vec3 pos)
	{
		m_position = pos;
	}
	AABB* Billboard::GetAABB()
	{
		return &m_Bbox;
	}
	void Billboard::SetAABB(AABB& box)
	{
		m_Bbox = box;
	}
	
}