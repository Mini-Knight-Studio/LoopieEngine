using System;

namespace Loopie
{
    public static class Random
    {
        private static readonly System.Random _random = new System.Random();

        public static float Value()
        {
            return (float)_random.NextDouble();
        }

        public static int Range(int min, int max)
        {
            return _random.Next(min, max);
        }

        public static float Range(float min, float max)
        {
            return min + (float)_random.NextDouble() * (max - min);
        }

        public static bool RandomBool()
        {
            return _random.Next(2) == 0;
        }

        public static bool Chance(float probability)
        {
            return Value() < probability;
        }

        public static T Pick<T>(T[] array)
        {
            if (array == null || array.Length == 0)
                throw new ArgumentException("Array cannot be null or empty.");

            return array[Range(0, array.Length)];
        }
    }
}