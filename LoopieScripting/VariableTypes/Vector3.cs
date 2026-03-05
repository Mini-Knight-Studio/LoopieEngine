using System;

namespace Loopie
{
    public struct Vector3
    {
        public float x, y, z;

        public static Vector3 Zero => new Vector3(0.0f);
        public static Vector3 Right => new Vector3(1.0f, 0.0f, 0.0f);
        public static Vector3 Up => new Vector3(0.0f, 1.0f, 0.0f);
        public static Vector3 Forward => new Vector3(0.0f, 0.0f, 1.0f);
        public double magnitude => Math.Sqrt(x * x + y * y + z * z);

        public Vector3(float scalar)
        {
            x = scalar;
            y = scalar;
            z = scalar;
        }

        public Vector3(float x, float y, float z)
        {
            this.x = x;
            this.y = y;
            this.z = z;
        }

        public static Vector3 operator +(Vector3 a, Vector3 b)
        {
            return new Vector3(a.x + b.x, a.y + b.y, a.z + b.z);
        }

        public static Vector3 operator -(Vector3 a, Vector3 b)
        {
            return new Vector3(a.x - b.x, a.y - b.y, a.z - b.z);
        }

        public static Vector3 operator *(Vector3 vector, float scalar)
        {
            return new Vector3(vector.x * scalar, vector.y * scalar, vector.z * scalar);
        }

        public static Vector3 operator /(Vector3 v, float scalar)
        {
            return new Vector3(v.x / scalar, v.y / scalar, v.z / scalar);
        }

        public static bool operator == (Vector3 a, Vector3 b)
        {
            return Math.Abs(a.x - b.x) < 1e-10f && Math.Abs(a.y - b.y) < 1e-10f && Math.Abs(a.z - b.z) < 1e-10f;
        }
        public static bool operator !=(Vector3 a, Vector3 b)
        {
            return !(a == b);
        }

        public override bool Equals(object obj)
        {
            return obj is Vector3 v && this == v;
        }

        public override int GetHashCode()
        {
            return x.GetHashCode() ^ y.GetHashCode() ^ z.GetHashCode();
        }

        public static double Distance(Vector3 a, Vector3 b)
        {
            return (a - b).magnitude;
        }

        public void Normalize()
        {
            float mag = (float)magnitude;
            if (mag > 1E-05f)
            {
                this /= mag;
            }
            else
                this = Zero;
        }

        public static float Dot(Vector3 a, Vector3 b)
        {
            return a.x * b.x + a.y * b.y + a.z * b.z;
        }

        public Vector3 normalized
        {
            get
            {
                float mag = (float)magnitude;
                return mag > 1E-05f? this / mag : Zero;
            }
        }
    }
}