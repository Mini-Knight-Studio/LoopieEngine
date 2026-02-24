using System;
using System.Collections.Generic;

namespace Loopie
{
    public static class Mathf
    {
        // Constants
        public const float PI = (float)Math.PI;
        public const float Deg2Rad = PI / 180f;
        public const float Rad2Deg = 180f / PI;
        public const float Epsilon = 1e-6f;
        public const float Infinity = float.PositiveInfinity;
        public const float NegativeInfinity = float.NegativeInfinity;
        public const float E = (float)Math.E;
        public const float Tau = 2f * PI; // Full circle in radians

        // Basic math operations
        public static float Abs(float value) => value < 0 ? -value : value;
        public static int Abs(int value) => value < 0 ? -value : value;

        public static float Sign(float value) => value > 0 ? 1 : (value < 0 ? -1 : 0);
        public static int Sign(int value) => value > 0 ? 1 : (value < 0 ? -1 : 0);

        public static float Min(float a, float b) => a < b ? a : b;
        public static float Min(params float[] values)
        {
            if (values == null || values.Length == 0) return 0;
            float min = values[0];
            for (int i = 1; i < values.Length; i++)
                if (values[i] < min) min = values[i];
            return min;
        }

        public static float Max(float a, float b) => a > b ? a : b;
        public static float Max(params float[] values)
        {
            if (values == null || values.Length == 0) return 0;
            float max = values[0];
            for (int i = 1; i < values.Length; i++)
                if (values[i] > max) max = values[i];
            return max;
        }

        // Clamping
        public static float Clamp(float value, float min, float max)
        {
            if (value < min) return min;
            if (value > max) return max;
            return value;
        }

        public static int Clamp(int value, int min, int max)
        {
            if (value < min) return min;
            if (value > max) return max;
            return value;
        }

        public static float Clamp01(float value) => Clamp(value, 0f, 1f);

        // Rounding
        public static float Round(float value) => (float)Math.Round(value);
        public static float Round(float value, int decimals) => (float)Math.Round(value, decimals);
        public static float Floor(float value) => (float)Math.Floor(value);
        public static float Ceil(float value) => (float)Math.Ceiling(value);
        public static int FloorToInt(float value) => (int)Math.Floor(value);
        public static int CeilToInt(float value) => (int)Math.Ceiling(value);
        public static int RoundToInt(float value) => (int)Math.Round(value);

        // Interpolation
        public static float Lerp(float a, float b, float t) => a + (b - a) * Clamp01(t);
        public static float LerpUnclamped(float a, float b, float t) => a + (b - a) * t;
        public static float InverseLerp(float a, float b, float value)
        {
            if (a != b)
                return Clamp01((value - a) / (b - a));
            return 0f;
        }

        // Movement
        public static float MoveTowards(float current, float target, float maxDelta)
        {
            if (Abs(target - current) <= maxDelta)
                return target;
            return current + Sign(target - current) * maxDelta;
        }

        // PingPong - oscillates between 0 and length
        public static float PingPong(float t, float length)
        {
            t = Abs(t);
            float mod = t % (2f * length);
            if (mod <= length)
                return mod;
            return 2f * length - mod;
        }

        // Repeat - loops value between 0 and length
        public static float Repeat(float t, float length) => t - Floor(t / length) * length;

        // Smooth step
        public static float SmoothStep(float from, float to, float t)
        {
            t = Clamp01(t);
            t = t * t * (3f - 2f * t); // Smooth interpolation
            return Lerp(from, to, t);
        }

        // Delta angle
        public static float DeltaAngle(float current, float target)
        {
            float delta = (target - current) % 360f;
            if (delta > 180f) delta -= 360f;
            if (delta < -180f) delta += 360f;
            return delta;
        }

        public static float MoveTowardsAngle(float current, float target, float maxDelta)
        {
            float delta = DeltaAngle(current, target);
            if (-maxDelta < delta && delta < maxDelta)
                return target;
            target = current + delta;
            return MoveTowards(current, target, maxDelta);
        }

        // Trigonometric
        public static float Sin(float value) => (float)Math.Sin(value);
        public static float Cos(float value) => (float)Math.Cos(value);
        public static float Tan(float value) => (float)Math.Tan(value);
        public static float Asin(float value) => (float)Math.Asin(value);
        public static float Acos(float value) => (float)Math.Acos(value);
        public static float Atan(float value) => (float)Math.Atan(value);
        public static float Atan2(float y, float x) => (float)Math.Atan2(y, x);

        // Power and roots
        public static float Sqrt(float value) => (float)Math.Sqrt(value);
        public static float Pow(float x, float y) => (float)Math.Pow(x, y);
        public static float Exp(float power) => (float)Math.Exp(power);
        public static float Log(float value) => (float)Math.Log(value);
        public static float Log10(float value) => (float)Math.Log10(value);
        public static float Log(float value, float newBase) => (float)Math.Log(value, newBase);

        // Angle conversion
        public static float DegToRad(float degrees) => degrees * Deg2Rad;
        public static float RadToDeg(float radians) => radians * Rad2Deg;

        // Geometry
        public static float Distance(float x1, float y1, float x2, float y2) =>
            Sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));

        public static float Distance(float x1, float y1, float z1, float x2, float y2, float z2) =>
            Sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1) + (z2 - z1) * (z2 - z1));

        public static float Hypotenuse(float a, float b) => Sqrt(a * a + b * b);

        // Random (useful for games)
        private static readonly Random _random = new Random();

        public static float RandomValue() => (float)_random.NextDouble();
        public static float RandomRange(float min, float max) => min + (float)_random.NextDouble() * (max - min);
        public static int RandomRange(int min, int max) => _random.Next(min, max);
        public static bool RandomBool() => _random.Next(2) == 0;

        // Missing from .NET 4.7.2
        public static float CopySign(float x, float y) => Math.Abs(x) * (y >= 0 ? 1 : -1);
        public static int CopySign(int x, int y) => Math.Abs(x) * (y >= 0 ? 1 : -1);

        public static float Ieee754Remainder(float x, float y) => x - (y * (float)Math.Round(x / y));

        public static float Scale(float value, float fromMin, float fromMax, float toMin, float toMax)
        {
            if (Math.Abs(fromMax - fromMin) < Epsilon) return toMin;
            float t = (value - fromMin) / (fromMax - fromMin);
            return LerpUnclamped(toMin, toMax, t);
        }

        // Advanced math
        public static float Map(float value, float from1, float to1, float from2, float to2, bool clamp = false)
        {
            float val = (value - from1) / (to1 - from1) * (to2 - from2) + from2;
            return clamp ? Clamp(val, Min(from2, to2), Max(from2, to2)) : val;
        }

        public static float Fract(float value) => value - Floor(value);

        public static float Mod(float a, float b) => a - b * Floor(a / b);

        public static bool Approximately(float a, float b, float tolerance = Epsilon) => Abs(b - a) < tolerance;

        // Vector math helpers
        public static float Dot(float x1, float y1, float x2, float y2) => x1 * x2 + y1 * y2;
        public static float Cross(float x1, float y1, float x2, float y2) => x1 * y2 - y1 * x2;

        public static float Angle(float x1, float y1, float x2, float y2)
        {
            float dot = Dot(x1, y1, x2, y2);
            float mag = Sqrt((x1 * x1 + y1 * y1) * (x2 * x2 + y2 * y2));
            if (mag == 0) return 0;
            return RadToDeg((float)Math.Acos(dot / mag));
        }

        // Number theory
        public static bool IsPrime(int number)
        {
            if (number <= 1) return false;
            if (number == 2) return true;
            if (number % 2 == 0) return false;

            int boundary = (int)Floor(Sqrt(number));
            for (int i = 3; i <= boundary; i += 2)
                if (number % i == 0)
                    return false;
            return true;
        }

        public static int GreatestCommonDivisor(int a, int b)
        {
            while (b != 0)
            {
                int temp = b;
                b = a % b;
                a = temp;
            }
            return Abs(a);
        }

        public static int LeastCommonMultiple(int a, int b) => Abs(a * b) / GreatestCommonDivisor(a, b);

        public static float[] QuadraticFormula(float a, float b, float c)
        {
            if (Approximately(a, 0))
                return new float[] { -c / b }; // Linear equation

            float discriminant = b * b - 4 * a * c;
            if (discriminant < 0)
                return new float[0]; // No real roots

            if (Approximately(discriminant, 0))
                return new float[] { -b / (2 * a) }; // One real root

            float sqrtDisc = Sqrt(discriminant);
            return new float[]
            {
                (-b + sqrtDisc) / (2 * a),
                (-b - sqrtDisc) / (2 * a)
            };
        }

        // Statistical
        public static float Mean(params float[] values)
        {
            if (values == null || values.Length == 0) return 0;
            float sum = 0;
            foreach (float val in values) sum += val;
            return sum / values.Length;
        }

        public static float Median(params float[] values)
        {
            if (values == null || values.Length == 0) return 0;
            Array.Sort(values);
            int mid = values.Length / 2;
            if (values.Length % 2 == 0)
                return (values[mid - 1] + values[mid]) / 2;
            return values[mid];
        }

        public static float Variance(params float[] values)
        {
            if (values == null || values.Length <= 1) return 0;
            float mean = Mean(values);
            float sumSqDiff = 0;
            foreach (float val in values)
                sumSqDiff += (val - mean) * (val - mean);
            return sumSqDiff / values.Length;
        }

        public static float StandardDeviation(params float[] values) => Sqrt(Variance(values));

        // Extensions (as static methods since we can't have extension methods in static class without 'this')
        public static float ToRadians(float degrees) => DegToRad(degrees);
        public static float ToDegrees(float radians) => RadToDeg(radians);
        public static bool IsPowerOfTwo(int value) => value > 0 && (value & (value - 1)) == 0;
        public static int NextPowerOfTwo(int value)
        {
            value--;
            value |= value >> 1;
            value |= value >> 2;
            value |= value >> 4;
            value |= value >> 8;
            value |= value >> 16;
            return value + 1;
        }
    }
}