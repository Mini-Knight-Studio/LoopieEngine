#pragma once
#include "Loopie/Math/MathTypes.h"
#include "Loopie/Math/AABB.h"
namespace Loopie
{
	class Camera;
	enum BillboardType
	{   CAMERA_FACING, 
		AXIS_ALIGNED, 
		SCREEN_ALIGNED
	};
	class Billboard  
	{               
		
		private: 
			BillboardType m_Btype;
			vec3 m_position;
			matrix4 m_transform;
			AABB m_Bbox;

		public:
			Billboard(vec3 pos, BillboardType t);
			matrix4  UpdateCalc(Camera* cam);
			void DrawDebug();

			BillboardType GetType()const;
			void SetType(BillboardType t);

			vec3 GetPosition();
			void SetPosition(vec3 pos);
			AABB* GetAABB();
			void SetAABB(AABB& box);
			
	};
}