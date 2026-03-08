#pragma once
#include "Loopie/Math/MathTypes.h"
#include <vector>

namespace Loopie
{
	class Ray
	{
	public:
		Ray(vec3 startPoint, vec3 endPoint);
		Ray(vec3 startPoint, vec3 direction, float length);
		vec3 StartPoint() const { return startPoint; }
		vec3 EndPoint() const { return endPoint; }
		vec3 Direction() const { return direction; }
        bool Intersects(const std::vector<vec3>& vertex, bool bounded_by_vertex, vec3& intersection_point) const;
	private:
		bool IntersectsLine(const vec3 lineStart, const vec3 lineEnd, vec3& intersectionPoint, bool only_hits_segment = false)const;
		bool IntersectsPlane(const std::vector<vec3>& vertex_array, vec3& intersectionPoint, bool only_hits_polygon = false)const;
	private:
		vec3 startPoint = vec3(0);
		vec3 endPoint = vec3(0);
		vec3 direction = vec3(0);
	};
}