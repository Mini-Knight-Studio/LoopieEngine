#include "BillBoardComponent.h"
#include "Loopie/Components/Camera.h"

namespace Loopie
{
	Billboard::Billboard(vec3 pos, BillboardType t)
	{
		m_Btype = t;
		m_position = pos;
		m_transform = matrix4(1.0f);
	}
	matrix4  Billboard::UpdateCalc(Camera* cam)
	{
		//camera aligned
		vec3 billboardPos = m_position;
		vec3 cameraPos = cam->GetPosition();
		vec3 cameraUp = cam->GetUp();

		matrix4 lookAtMatrix = glm::lookAt(billboardPos, cameraPos, cameraUp);

		m_transform = glm::inverse(lookAtMatrix);
		
		return m_transform;
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