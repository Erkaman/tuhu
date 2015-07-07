
#include "lib.glsl"

#include "noise_lib.glsl"
#include "noise_3d.glsl"

out vec4 fragmentColor;

uniform vec3 viewSpaceLightPosition;

uniform sampler2D tex;

in vec3 vertexColor;
in vec2 texCoord;

in vec3 viewSpaceNormal;
in vec3 viewSpacePosition;

vec3 sampleDiffuseTexture() {
    return vec3( snoise(vec3(texCoord.xy,0)));
//    return texture(tex, texCoord).xyz;
}

void main()
{
    vec3 shading = phongVertex(sampleDiffuseTexture(), viewSpaceNormal, viewSpaceLightPosition, viewSpacePosition);

    // this is our height map color.

    fragmentColor = vec4(shading, 1.0);
}
