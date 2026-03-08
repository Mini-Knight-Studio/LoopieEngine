using System;

namespace Loopie
{
    public struct Vector4
    {
        public float x, y, z, w;

        public static Vector4 Zero => new Vector4(0.0f);
        public static Vector4 One => new Vector4(1.0f);
        public static Vector4 Right => new Vector4(1.0f, 0.0f, 0.0f, 0.0f);
        public static Vector4 Up => new Vector4(0.0f, 1.0f, 0.0f, 0.0f);
        public static Vector4 Forward => new Vector4(0.0f, 0.0f, 1.0f, 0.0f);
        public double magnitude => Math.Sqrt(x * x + y * y + z * z + w * w);

        public Vector4(float scalar)
        {
            x = scalar;
            y = scalar;
            z = scalar;
            w = scalar;
        }

        public Vector4(float x, float y, float z, float w)
        {
            this.x = x;
            this.y = y;
            this.z = z;
            this.w = w;
        }

        public static Vector4 operator +(Vector4 a, Vector4 b)
        {
            return new Vector4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
        }

        public static Vector4 operator -(Vector4 a, Vector4 b)
        {
            return new Vector4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
        }

        public static Vector4 operator *(Vector4 vector, float scalar)
        {
            return new Vector4(vector.x * scalar, vector.y * scalar, vector.z * scalar, vector.w * scalar);
        }

        public static Vector4 operator /(Vector4 v, float scalar)
        {
            return new Vector4(v.x / scalar, v.y / scalar, v.z / scalar, v.w / scalar);
        }

        public static bool operator == (Vector4 a, Vector4 b)
        {
            return Math.Abs(a.x - b.x) < 1e-10f && Math.Abs(a.y - b.y) < 1e-10f && Math.Abs(a.z - b.z) < 1e-10f && Math.Abs(a.w - b.w) < 1e-10f;
        }

        public static bool operator !=(Vector4 a, Vector4 b)
        {
            return !(a == b);
        }

        public override bool Equals(object obj)
        {
            return obj is Vector4 v && this == v;
        }

        public override int GetHashCode()
        {
            return x.GetHashCode() ^ y.GetHashCode() ^ z.GetHashCode() ^ w.GetHashCode();
        }

        public static double Distance(Vector4 a, Vector4 b)
        {
            return (a - b).magnitude;
        }

        public void Normalize()
        {
            float mag = (float)magnitude;
            if(mag > Mathf.Epsilon)
                this /= mag;
            else
                this = Zero;
        }

        public static float Dot(Vector4 a, Vector4 b)
        {
            return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
        }

        public Vector4 normalized
        {
            get
            {
                float mag = (float)magnitude;
                return mag > Mathf.Epsilon ? this / mag : Zero;
            }
        }


        public static Vector4 Lerp(Vector4 a, Vector4 b, float t)
        {
            t = Mathf.Clamp01(t);
            return new Vector4(
                a.x + (b.x - a.x) * t,
                a.y + (b.y - a.y) * t,
                a.z + (b.z - a.z) * t,
                a.w + (b.w - a.w) * t
            );
        }

        public static Vector4 LerpUnclamped(Vector4 a, Vector4 b, float t)
        {
            return new Vector4(
                a.x + (b.x - a.x) * t,
                a.y + (b.y - a.y) * t,
                a.z + (b.z - a.z) * t,
                a.w + (b.w - a.w) * t
            );
        }

        public static float Angle(Vector4 a, Vector4 b)
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