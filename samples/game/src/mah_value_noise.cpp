#include "mah_value_noise.hpp"
#include "util.hpp"
#include "log.hpp"
#include "common.hpp"

#include "math/vector2f.hpp"

#include <math.h>

static inline Vector2f Fade(const float px, const float py) {
    return Vector2f(
	px * px * px * (px * (px * 6 - 15) + 10),
	py * py * py * (py * (py * 6 - 15) + 10));
}

static inline float Lerp(float x, float y, float a) {
    return x*(1-a)+y*a;
}

static inline int Floor(float x) {
    return x>=0 ? (int)x : (int)x-1;
}


ValueNoise::ValueNoise(unsigned long long seed):
  m_perm { 151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,135,130,116,188,159,86,164,100,109,198,173,186,3,64,52,217,226,250,124,123,5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180},
m_rand {
    -0.0981f, -0.40864f, -0.83471f, 0.7629f, -0.88928f, -0.73205f, -0.98301f, -0.8413f, 0.2244f, -0.82447f, -0.69362f, 0.80965f, 0.46538f, -0.87811f, 0.95732f, 0.92346f, -0.49762f, -0.53678f, 0.19646f, -0.25686f, 0.52117f, -0.7873f, 0.41926f, -0.40308f, -0.87757f, 0.90956f, 0.36462f, -0.94381f, -0.40163f, -0.99154f, 0.37259f, -0.56724f, -0.12684f, 0.38608f, -0.58476f, -0.86567f, 0.69384f, -0.73754f, -0.51769f, 0.38838f, -0.65309f, 0.51169f, 0.22205f, 0.69238f, -0.85721f, 0.51418f, -0.7966f, 0.30132f, -0.86968f, 0.92732f, -0.56383f, 0.77343f, -0.07106f, 0.43105f, -0.53437f, 0.49927f, -0.82962f, 0.5713f, -0.53804f, -0.88297f, -0.12902f, 0.34831f, -0.81519f, 0.85324f, 0.12632f, -0.07393f, -0.13143f, 0.2859f, 0.45306f, -0.45066f, -0.48751f, -0.49365f, -0.80652f, -0.8754f, 0.90845f, -0.96366f, -0.9765f, -0.95729f, -0.67994f, 0.49749f, 0.87805f, 0.59416f, 0.44118f, -0.26234f, -0.52395f, 0.61278f, -0.19424f, -0.88291f, -0.69848f, 0.04647f, -0.38343f, -0.72445f, 0.29729f, -0.40358f, -0.36911f, -0.71077f, -0.94623f, 0.48189f, -0.53916f, -0.11935f, -0.28486f, 0.5578f, -0.8789f, -0.56338f, -0.48569f, 0.58067f, -0.39518f, -0.52655f, -0.24218f, 0.37803f, 0.845f, -0.20302f, -0.72601f, 0.79186f, -0.20283f, -0.22299f, 0.13636f, 0.24576f, -0.11363f, -0.84821f, 0.53186f, -0.7551f, 0.80287f, -0.28548f, 0.01644f, 0.98115f, 0.01997f, -0.19818f, -0.99103f, 0.88168f, -0.85841f, -0.1624f, 0.12453f, 0.85897f, 0.35169f, -0.76771f, -0.95124f, -0.62212f, -0.39573f, 0.92737f, -0.2663f, -0.41265f, 0.81337f, 0.22969f, -0.52445f, -0.47623f, 0.05697f, 0.48005f, -0.78686f, -0.81875f, 0.78105f, -0.02604f, 0.84217f, 0.24896f, 0.40528f, 0.3759f, 0.25428f, 0.06534f, 0.80561f, -0.2204f, 0.78085f, 0.06544f, 0.64178f, -0.66991f, -0.44004f, -0.37098f, 0.64988f, 0.969f, -0.51129f, -0.9337f, 0.81645f, -0.64257f, -0.61232f, -0.33543f, -0.60753f, -0.48863f, 0.35619f, -0.64307f, 0.72882f, 0.01224f, 0.81879f, 0.00557f, 0.04305f, 0.85873f, 0.56987f, -0.3359f, 0.59709f, 0.45098f, 0.13796f, 0.8327f, 0.77587f, -0.92782f, 0.68568f, -0.75998f, 0.3272f, -0.49211f, 0.48539f, -0.95106f, -0.33055f, -0.87487f, 0.1561f, -0.78431f, -0.26021f, -0.64203f, 0.2698f, -0.29904f, 0.91483f, -0.50684f, -0.12898f, 0.67784f, -0.74222f, -0.00267f, -0.01865f, -0.25386f, -0.29269f, -0.01665f, -0.50018f, -0.76183f, -0.87196f, 0.44619f, -0.26948f, 0.72047f, -0.18302f, 0.64217f, 0.1612f, -0.5061f, -0.80956f, -0.71651f, 0.65736f, -0.27149f, 0.71933f, 0.83153f, -0.33811f, -0.84073f, -0.26807f, -0.69259f, -0.65434f, -0.72321f, -0.30333f, -0.18411f, -0.7383f, 0.9608f, 0.54196f, 0.19092f, 0.11336f, 0.91377f, 0.70434f, 0.44032f, -0.37831f, 0.39686f, 0.6159f, 0.39759f, 0.09404f, 0.96196f, -0.32873f, -0.88459f
} {

    Random rng(seed);
    Shuffle(rng, m_perm, m_perm+256);
}

float ValueNoise::Sample(const Vector2f& p)const {

    const Vector2f P =
	Vector2f( (float)(Floor(p.x) % 256), (float)(Floor(p.y) % 256) );

    const Vector2f f = Fade(
	p.x - Floor(p.x),
	p.y - Floor(p.y));

    const int A = GetPerm((int)P.x) +(int) P.y;
    const int AA = GetPerm(A);
    const int AB = GetPerm(A + 1);
    const int B =  GetPerm((int)P.x + 1) + (int)P.y;
    const int BA = GetPerm(B);
    const int BB = GetPerm(B + 1);


/*
    return mix(
	mix(mix(rand(AA), rand(BA), f.x),
	     mix(rand(AB), rand(BB), f.x), f.y),
	mix(mix(rand(AA + one),rand(BA + one), f.x),
	     mix(rand(AB + one),rand(BB + one), f.x), f.y),
	f.z);
*/

    return
	Lerp(Lerp(GetRand(AA), GetRand(BA), f.x), Lerp(GetRand(AB), GetRand(BB), f.x), f.y);
}

int ValueNoise::GetPerm(int i)const {
    return m_perm[i % 256];
}

float ValueNoise::GetRand(int i)const {
    return m_rand[i % 256];
}


float ValueNoise::Turbulence(const int octaves, const Vector2f& P, const float lacunarity, const float gain)const {

    float sum = 0;
    float scale = 1;
    float totalgain = 1;
    float max = 0;
    for(int i=0;i<octaves;i++){
	sum += totalgain*Sample(P*scale);
	max += totalgain;

	scale *= lacunarity;
	totalgain *= gain;
    }
    return fabs(sum / max);

}
