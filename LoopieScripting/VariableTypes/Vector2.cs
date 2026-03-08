using System;

namespace Loopie
{
    public struct Vector2
    {
        public float x, y;

        public static Vector2 Zero => new Vector2(0.0f);
        public static Vector2 One => new Vector2(1.0f);
        public static Vector2 Right => new Vector2(1.0f, 0.0f);
        public static Vector2 Up => new Vector2(0.0f, 1.0f);
        public double magnitude => Math.Sqrt(x * x + y * y);

        public Vector2(float scalar)
        {
            x = scalar;
            y = scalar;
        }

        public Vector2(float x, float y)
        {
            this.x = x;
            this.y = y;
        }

        public static Vector2 operator +(Vector2 a, Vector2 b)
        {
            return new Vector2(a.x + b.x, a.y + b.y);
        }

        public static Vector2 operator -(Vector2 a, Vector2 b)
        {
            return new Vector2(a.x - b.x, a.y - b.y);
        }

        public static Vector2 operator *(Vector2 vector, float scalar)
        {
            return new Vector2(vector.x * scalar, vector.y * scalar);
        }

        public static Vector2 operator /(Vector2 v, float scalar)
        {
            return new Vector2(v.x / scalar, v.y / scalar);
        }

        public static bool operator ==(Vector2 a, Vector2 b)
        {
            return Math.Abs(a.x - b.x) < 1e-10f && Math.Abs(a.y - b.y) < 1e-10f;
        }

        public static bool operator !=(Vector2 a, Vector2 b)
        {
            return !(a == b);
        }

        public override bool Equals(object obj)
        {
            return obj is Vector2 v && this == v;
        }

        public override int GetHashCode()
        {
            return x.GetHashCode() ^ y.GetHashCode();
        }

        public static double Distance(Vector2 a, Vector2 b)
        {
            return (a - b).magnitude;
        }

        public void Normalize()
        {
            float mag = (float)magnitude;
            if (mag > Mathf.Epsilon)
            {
                this /= mag;
            }
            else
                this = Zero;
        }

        public static float Dot(Vector2 a, Vector2 b)
        {
            return a.x * b.x + a.y * b.y;
        }

        public Vector2 normalized
        {
            get
            {
                float mag = (float)magnitude;
                return mag > Mathf.Epsilon ? this / mag : Zero;
            }
        }

        public static Vector2 Lerp(Vector2 a, Vector2 b, float t)
        {
            t = Mathf.Clamp01(t);
            return new Vector2(
                a.x + (b.x - a.x) * t,
                a.y + (b.y - a.y) * t
            );
        }

        public static Vector2 LerpUnclamped(Vector2 a, Vector2 b, float t)
        {
            return new Vector2(
                a.x + (b.x - a.x) * t,
                a.y + (b.y - a.y) * t
            );
        }

        public static float Angle(Vector2 a, Vector2 b)
        {
            float dot = Dot(a, b);
            float mag = (float)(a.magnitude * b.magnitude);

            if (mag == Mathf.Epsilon)
                return 0.0f;

            float cosTheta = dot / mag;
            cosTheta = Mathf.Clamp(cosTheta, -1.0f, 1.0f);

            return Mathf.Acos(cosTheta) * Mathf.Rad2Deg;
        }
    }
}