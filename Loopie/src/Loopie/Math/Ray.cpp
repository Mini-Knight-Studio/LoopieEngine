#include "Ray.h"
#include "MathUtils.h"
#include "Loopie/Core/Log.h"
namespace Loopie
{
    Ray::Ray(vec3 startPoint, vec3 endPoint)
    {
        this->startPoint = startPoint;
        this->endPoint = endPoint;
		this->direction = normalize(endPoint - startPoint);
    }

    Ray::Ray(vec3 startPoint, vec3 direction, float length)
    {
        this->startPoint = startPoint;
        this->endPoint = startPoint + length * direction;
		this->direction = normalize(direction);
    }

    bool Ray::Intersects(const std::vector<vec3>& vertex, bool bounded_by_vertex, vec3& intersection_point)const
    {
        if (vertex.size() < 2)
        {
            Log::Error("Intersects function does not allow less than 2 vertex, current vertex ammount: {0}", vertex.size());
            return false;
        }
        else if (vertex.size() > 2)
        {
            return IntersectsPlane(vertex, intersection_point, bounded_by_vertex);
        }
        else
        {
            return IntersectsLine(vertex[0], vertex[1], intersection_point, bounded_by_vertex);
        }
    }

    bool Ray::IntersectsLine(const vec3 lineStart, const vec3 lineEnd, vec3& intersectionPoint, bool only_hits_segment)const
    {
        vec3 ldir = lineEnd - lineStart;
        vec3 rdir = endPoint - startPoint;
        vec3 r = startPoint - lineStart;

        float a = dot(rdir, rdir);
        float b = dot(rdir, ldir);
        float c = dot(ldir, ldir);
        float d = dot(rdir, r);
        float e = dot(ldir, r);

        float denom = a * c - b * b;

        if (fabs(denom) < Math::EPSILON)
            return false;

        float t = (b * e - c * d) / denom;

        if (t < 0.0f) return false;
        if (!only_hits_segment)
        {
            intersectionPoint = startPoint + rdir * t;
            return true;
        }

        float u = (a * e - b * d) / denom;
        if (u < 0.0f || u > 1.0f) return false;

        intersectionPoint = startPoint + rdir * t;
        return true;
    }

    bool Ray::IntersectsPlane(const std::vector<vec3>& vertex_array, vec3& intersectionPoint, bool only_hits_polygon)const
    {
        vec3 rdir = endPoint - startPoint;

        vec3 normal = normalize(cross(vertex_array[1] - vertex_array[0], vertex_array[2] - vertex_array[0]));

        float denom = dot(normal, rdir);
        if (fabs(denom) < Math::EPSILON)
            return false;

        float t = dot(normal, vertex_array[0] - startPoint) / denom;
        if (t < 0.0f)
            return false;
        if (!only_hits_polygon)
        {
            intersectionPoint = startPoint + t * rdir;
            return true;
        }
        vec3 P = startPoint + t * rdir;
        for (int i = 0; i < vertex_array.size(); i++)
        {
            const vec3& A = vertex_array[i];
            const vec3& B = vertex_array[(i + 1) % vertex_array.size()];

            vec3 edge = B - A;
            vec3 toPoint = P - A;

            vec3 c = glm::cross(edge, toPoint);
            if (glm::dot(c, normal) < 0.0f)
                return false;
        }

        intersectionPoint = P;
        return true;
    }
}