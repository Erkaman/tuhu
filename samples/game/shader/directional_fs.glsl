#include "shader/lighting_lib.glsl"

in vec2 texCoord;

out vec4 fragmentColor;

uniform sampler2D colorTexture;
uniform sampler2D depthTexture;
uniform sampler2D normalTexture;
uniform sampler2D specularTexture;
uniform sampler2DShadow shadowMap;

uniform vec2 screenSize;

uniform vec3 ambientLight;
uniform vec3 sceneLight;
uniform vec3 eyePos;
uniform vec3 viewSpaceLightDirection;
uniform mat4 lightVp;
uniform mat4 inverseViewMatrix;
uniform mat4 invProj;
uniform vec4 lightDirection;
uniform mat4 proj;
uniform mat4 inverseViewNormalMatrix;
uniform mat4 invViewMatrix;
uniform mat4 projectionMatrix;

uniform sampler2D refractionMap;
uniform sampler2D reflectionMap;


uniform samplerCube envMap;


void main() {

    vec3 specColor;
    float specShiny;

    readSpecularTexture(specularTexture, texCoord, specColor, specShiny);

    vec3 viewSpacePosition = getViewSpacePosition(invProj, depthTexture, texCoord);

    vec3 v = -normalize(viewSpacePosition);
    vec3 l= -viewSpaceLightDirection;
    vec3 n;
    float id;
    readNormalTexture(normalTexture, texCoord, n, id);

    float diff=  calcDiff(l,n);
    float spec= calcSpec(l,n,v);

    vec3 diffColor;
    float ao;

    readColorTexture(colorTexture, texCoord, diffColor, ao);

    vec3 specMat = specColor; // + (vec3(1.0) - specColor)  * pow(clamp(1.0 + dot(-v, n), 0.0, 1.0), 5.0);
/*    vec3 specMat =
      specColor +

*/
    float aoOnly =0.0;

    vec4 shadowCoord = (lightVp * (inverseViewMatrix * vec4(viewSpacePosition.xyz,1)));

    float visibility = calcVisibility(shadowMap, diff, shadowCoord);
/*
  if(id == 1.0) {
  fragmentColor = vec4(vec3(1,0,0), 1.0);
  }
*/

    vec3 envMapSample = vec3(0);


    if(id == 1.0) {

	// add fresnel.
	specMat += (vec3(1.0) - specColor)  * pow(clamp(1.0 + dot(-v, n), 0.0, 1.0), 5.0);

	vec3 reflectionVector = (inverseViewNormalMatrix *
				 vec4(
				     reflect(-v, n.xyz ), 0.0)).xyz;

	envMapSample = texture(envMap, reflectionVector).rgb;


    }

    fragmentColor =vec4(vec3(1.0-ao), 1.0) * aoOnly +
	(1.0 - aoOnly)*calcLighting(
	    ambientLight,
	    sceneLight,
	    specShiny,
	    diffColor,
	    specMat,
	    diff,
	    spec,
	    visibility,
	    envMapSample );

//	fragmentColor = vec4(vec3(1,0,0), 1.0);

/*
    if(id == 1.0) {


	fragmentColor = vec4(envMapSample, 1.0);
    }
    */

    if(id == 2.0) {

//	fragmentColor =vec4(vec3(n), 1.0);


	vec4 clipSpace = proj * vec4( viewSpacePosition, 1.0);



	vec2 ndc = clipSpace.xy / clipSpace.w;
	ndc = ndc * 0.5 + 0.5;


	vec2 refractionTexcoord = ndc;
	vec2 reflectionTexcoord = vec2(ndc.x, 1 -ndc.y);


	vec2 distort = specColor.xy;

	refractionTexcoord += distort;
	reflectionTexcoord += distort;

	refractionTexcoord = clamp(refractionTexcoord, 0.001, 1.0 - 0.001);

	vec3 refraction = texture(refractionMap, refractionTexcoord).xyz;
	vec3 reflection = texture(reflectionMap, reflectionTexcoord).xyz;

//    refraction = clamp(refraction, 0.001, 1.0 - 0.001);
/*
	vec3 n = texture(textureArray, vec3(distortedTexCoords, normalMap) ).xyz;
	n = vec3(2*n.r - 1.0, n.b, 2*n.g - 1.0);
	n = normalize(n);
*/

	vec3 worldPosition = (invViewMatrix * vec4(viewSpacePosition, 1)).xyz;

	vec3 toCameraVector = normalize(eyePos - worldPosition.xyz);

	vec3 v = toCameraVector;
	vec3 l = -lightDirection.xyz;

	float spec= calcSpec(l,n,v);

	vec3 color;

	float fresnel = dot(
	    toCameraVector, vec3(0,1,0));

	color = mix(refraction, 0.4 * reflection, 1.0 - fresnel);

	color += sceneLight * pow(spec,20.0) * 0.6;

/*
	float waterDistance = toLinearDepth(gl_FragCoord.z);
	float floorDistance = toLinearDepth(  texture(depthMap, ndc).r  );

	float waterDepth = floorDistance - waterDistance;
*/
	fragmentColor = vec4(color, 1.0);

//	fragmentColor = vec4(vec3(1,0,0), 1.0);

//    color = vec3(waterDepth / 70 );

//	float a = clamp(waterDepth / 3.0, 0.0, 1.0);
//    a = 1.0;

//    geoData[0] = vec4(color,a);


    }

//    fragmentColor = vec4( vec3(specMat), 1.0 );


}
