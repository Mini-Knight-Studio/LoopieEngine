#include "OctreeNode.h"


namespace Loopie
{
	OctreeNode::OctreeNode(const AABB& aabb)
	{
		m_aabb = aabb;
	}
}