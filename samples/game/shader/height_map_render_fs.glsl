#include "shader/lighting_lib.glsl"

in vec3 position;

out vec4 geoData[2];

uniform vec3 viewSpaceLightDirection;
uniform float aoOnly;

in vec2 texCoord;

in vec3 viewSpaceNormal;
in vec3 viewSpacePosition;

uniform sampler2D grass;
uniform sampler2D dirt;
uniform sampler2D rock;
uniform sampler2D road;

uniform sampler2DShadow shadowMap;

uniform sampler2D splatMap;
uniform sampler2D aoMap;

in vec3 outn;


uniform vec3 ambientLight;
uniform vec3 sceneLight;

in vec2 scaledTexcoord;


void main()
{

    // TODO OPTIMIZE.

    vec4 splat =texture(splatMap, texCoord);

    vec3 diffColor = splat.r * texture(grass, scaledTexcoord).xyz;
    diffColor += splat.g * texture(dirt, scaledTexcoord).xyz;
    diffColor +=  splat.b * texture(rock, scaledTexcoord).xyz;

    // TODO, use mix function here!
    diffColor = mix(diffColor, texture(road, scaledTexcoord).xyz, splat.a);

   // shadowing is done in screenspace, so comment out.


/*
#ifdef SHADOW_MAPPING
    float visibility = calcVisibility(shadowMap, diff, shadowCoordOut);
#else
    float visibility = 1.0;
#endif
*/
       float visibility = 1.0;

    float ao = texture(aoMap, texCoord).r;


#ifdef DEFERRED
/*
    vec3 n = normalize(vec4(viewSpaceNormal,0.0)).xyz; // lol3
    n = normalize(n);
    */
    //  viewSpaceNormal.xyz =  n;


/*
    vec2 n2 = viewSpaceNormal.xy;

    float nx = n2.x;
    float ny = n2.y;
    float nz = sqrt(1.0 - dot(n2.xy, n2.xy));

    vec3 n3 = vec3(nx,ny,  nz *  sign(viewSpaceNormal.z) );

*/

//    vec3 n3 = viewSpaceNormal;


/*
    vec2 enc = encode(viewSpaceNormal);
    vec3 n3 = decode(vec4(enc, 0,0 ) );
*/


//    vec3 n3 = n;

    geoData[0] = packColorTexture(diffColor, vec3(0,0,0), ao);
    geoData[1] =  packNormalTexture(viewSpaceNormal, 0, 0);

#else

    vec3 v = -normalize(viewSpacePosition);
    vec3 l= -viewSpaceLightDirection;
    vec3 n = viewSpaceNormal;


    // TODO: we really need all this?
    float specShiny = 0;
    vec3 specColor = vec3(0);
    float diff=  calcDiff(l,n);

   float spec= calcSpec(l,n,v);

    geoData[0] =vec4(vec3(1.0-ao), 1.0) * aoOnly +
	(1.0 - aoOnly)*calcLighting(
	ambientLight* (1.0 -ao),
	sceneLight,
	specShiny,
	diffColor.xyz,
	specColor.xyz,
	diff,
	spec,
	visibility,
	vec3(0) );
#endif

//    geoData[0] = vec4(vec4(vec3(1,0,0), 1));

}
