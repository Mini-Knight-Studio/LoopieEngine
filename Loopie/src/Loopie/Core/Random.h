#pragma once
#include <cstdint>
#include <random>

namespace Loopie {

    class Random {
    private:
        inline static uint64_t state = 0x853c49e6748fea9bULL;
        inline static uint64_t inc = 0xda3e39cb94b95bdbULL;

        static uint32_t Next() {
            uint64_t oldstate = state;
            state = oldstate * 6364136223846793005ULL + (inc | 1);

            uint32_t xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
            uint32_t rot = oldstate >> 59u;

            return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
        }

        struct AutoSeeder {
            AutoSeeder() {
                std::random_device rd;
                uint64_t pcgSeed = (static_cast<uint64_t>(rd()) << 32) | rd();
                Random::Seed(pcgSeed);
            }
        };
        inline static AutoSeeder s_Seeder;

    public:
        Random() = delete;

        static void Seed(uint64_t seed) {
            state = 0;
            inc = (seed << 1u) | 1u;
            Next();
            state += seed;
            Next();
        }

        static int Get(int min, int max) {
            uint32_t r = Next();
            return min + (r % (max - min + 1));
        }

        static float Get(float min, float max) {
            float t = Next() * (1.0f / 4294967295.0f);
            return min + t * (max - min);
        }

        static double Get(double min, double max) {
            double t = Next() * (1.0 / 4294967295.0);
            return min + t * (max - min);
        }
    };


    class FRandom {
    private:
        inline static uint32_t state = 0xA3C59AC3;

        static uint32_t Next() {
            state ^= state << 13;
            state ^= state >> 17;
            state ^= state << 5;
            return state;
        }

        struct AutoSeeder {
            AutoSeeder() {
                std::random_device rd;
                FRandom::Seed(rd());
            }
        };
        inline static AutoSeeder s_Seeder;

    public:
        FRandom() = delete;

        static void Seed(uint32_t seed) {
            state = seed ? seed : 0xA3C59AC3;
        }

        static int Get(int min, int max) {
            uint32_t r = Next();
            return min + (r % (max - min + 1));
        }

        static float Get(float min, float max) {
            float t = Next() * (1.0f / 4294967295.0f);
            return min + t * (max - min);
        }

        static double Get(double min, double max) {
            double t = Next() * (1.0 / 4294967295.0);
            return min + t * (max - min);
        }
    };
}