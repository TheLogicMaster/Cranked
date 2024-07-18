#pragma once

// Based on: https://gist.github.com/Flafla2/f0260a861be0ebdeef76

#include "Utils.hpp"

namespace cranked {
    class Perlin {
    public:

        static float noise(int repeat, float x, float y, float z, int octaves, float persistence) {
            float total = 0;
            float frequency = 1;
            float amplitude = 1;
            float maxValue = 0;
            if (octaves <= 0)
                octaves = 1;
            for (int i = 0; i < octaves; i++) {
                total += noise(repeat, x * frequency, y * frequency, z * frequency) * amplitude;
                maxValue += amplitude;
                amplitude *= persistence;
                frequency *= 2;
            }
            return total / maxValue;
        }

    private:
        static constexpr uint8 PERMUTATION[] = {
            151, 160, 137, 91, 90, 15,
            190, 6, 148, 247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32, 57, 177, 33,
            88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175, 74, 165, 71, 134, 139, 48, 27, 166,
            77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244,
            102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169, 200, 196,
            135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64, 52, 217, 226, 250, 124, 123,
            5, 202, 38, 147, 118, 126, 255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42,
            223, 183, 170, 213, 119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43, 172, 9,
            129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104, 218, 246, 97, 228,
            251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241, 81, 51, 145, 235, 249, 14, 239, 107,
            49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254,
            138, 236, 205, 93, 222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180,
            151, 160, 137, 91, 90, 15,
            190, 6, 148, 247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32, 57, 177, 33,
            88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175, 74, 165, 71, 134, 139, 48, 27, 166,
            77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244,
            102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169, 200, 196,
            135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64, 52, 217, 226, 250, 124, 123,
            5, 202, 38, 147, 118, 126, 255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42,
            223, 183, 170, 213, 119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43, 172, 9,
            129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104, 218, 246, 97, 228,
            251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241, 81, 51, 145, 235, 249, 14, 239, 107,
            49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254,
            138, 236, 205, 93, 222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180,
        };

        static float noise(int repeat, float x, float y, float z) {
            if (repeat > 0) {
                x = fmodf(x, (float) repeat);
                y = fmodf(y, (float) repeat);
                z = fmodf(z, (float) repeat);
            }

            int xi = (int) x & 255;
            int yi = (int) y & 255;
            int zi = (int) z & 255;
            float xf = x - (float) (int) x;
            float yf = y - (float) (int) y;
            float zf = z - (float) (int) z;
            float u = fade(xf);
            float v = fade(yf);
            float w = fade(zf);

            int aaa = PERMUTATION[PERMUTATION[PERMUTATION[xi] + yi] + zi];
            int aba = PERMUTATION[PERMUTATION[PERMUTATION[xi] + inc(repeat, yi)] + zi];
            int aab = PERMUTATION[PERMUTATION[PERMUTATION[xi] + yi] + inc(repeat, zi)];
            int abb = PERMUTATION[PERMUTATION[PERMUTATION[xi] + inc(repeat, yi)] + inc(repeat, zi)];
            int baa = PERMUTATION[PERMUTATION[PERMUTATION[inc(repeat, xi)] + yi] + zi];
            int bba = PERMUTATION[PERMUTATION[PERMUTATION[inc(repeat, xi)] + inc(repeat, yi)] + zi];
            int bab = PERMUTATION[PERMUTATION[PERMUTATION[inc(repeat, xi)] + yi] + inc(repeat, zi)];
            int bbb = PERMUTATION[PERMUTATION[PERMUTATION[inc(repeat, xi)] + inc(repeat, yi)] + inc(repeat, zi)];

            float x1 = lerp(grad(aaa, xf, yf, zf), grad(baa, xf - 1, yf, zf), u);
            float x2 = lerp(grad(aba, xf, yf - 1, zf), grad(bba, xf - 1, yf - 1, zf), u);
            float y1 = lerp(x1, x2, v);

            x1 = lerp(grad(aab, xf, yf, zf - 1), grad(bab, xf - 1, yf, zf - 1), u);
            x2 = lerp(grad(abb, xf, yf - 1, zf - 1), grad(bbb, xf - 1, yf - 1, zf - 1), u);
            float y2 = lerp(x1, x2, v);

            return (lerp(y1, y2, w) + 1) / 2;
        }

        static int inc(int repeat, int num) {
            num++;
            if (repeat > 0)
                num %= repeat;
            return num;
        }

        static float grad(int hash, float x, float y, float z) {
            int h = hash & 0b1111;
            float u = h < 0b1000 ? x : y;
            float v;
            if (h < 0b0100)
                v = y;
            else if (h == 0b1100 or h == 0b1110)
                v = x;
            else
                v = z;
            return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
        }

        static float fade(float t) {
            return t * t * t * (t * (t * 6 - 15) + 10);
        }

        static float lerp(float a, float b, float x) {
            return a + x * (b - a);
        }
    };
}
