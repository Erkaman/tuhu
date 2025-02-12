uniform sampler2D permSampler;
uniform sampler2D randomSampler;

vec3 fade(vec3 t){
    return t * t * t * (t * (t * 6 - 15) + 10); // new curve
}

vec4 perm(vec2 p){
    return texture(permSampler, p);
}

float rand(float x){
    return texture(randomSampler, vec2(x,0.0)).x;
}

// 3D version

float value_noise(vec3 p){
    vec3 P = mod(floor(p), 256.0);
    p -= floor(p);
    vec3 f = fade(p);




    // HASH COORDINATES FOR 6 OF THE 8 CUBE CORNERS

    P = P / 256.0;
    const float one = 1.0 / 256.0;


    // six texture lookups!
/*    float A = perm(P.x) + P.y;
    float AA = perm(A) + P.z;
    float AB = perm(A + 1) + P.z;
    float B =  perm(P.x + 1) + P.y;
    float BA = perm(B) + P.z;
    float BB = perm(B + 1) + P.z;*/

    vec4 AA = perm(P.xy) + P.z;




    // put AA, AB, BA, BB in texture. 256x256 texture.

    // put random integer in range [0,1] in 256x1 texture


/*
    return mix(
	mix(mix(rand(AA), rand(BA), f.x),
	     mix(rand(AB), rand(BB), f.x), f.y),
	mix(mix(rand(AA + one),rand(BA + one), f.x),
	     mix(rand(AB + one),rand(BB + one), f.x), f.y),
	f.z);
*/

    return mix(
	mix(mix(rand(AA.x), rand(AA.z), f.x),
	     mix(rand(AA.y), rand(AA.w), f.x), f.y),
	mix(mix(rand(AA.x + one),rand(AA.z + one), f.x),
	     mix(rand(AA.y + one),rand(AA.w + one), f.x), f.y),
	f.z);
}


float value_noise_turbulence(int octaves, vec3 P, float lacunarity, float gain)
{
  float sum = 0;
  float scale = 1;
  float totalgain = 1;
  for(int i=0;i<octaves;i++){
    sum += totalgain*value_noise(P*scale);
    scale *= lacunarity;
    totalgain *= gain;
  }
  return abs(sum);
}
