layout (location = 0)in  vec3 positionIn;


#include "height_map_lib.glsl"

uniform mat4 mvp;

uniform sampler2D heightMap;

uniform float xzScale;
uniform vec3 offset;
uniform float yScale;
uniform float resolution;

uniform vec2 cursorPos;
uniform vec3 cameraPos;

out float isRender;
out float d;



void main()
{

    vec3 pos = computePos(
	vec2(cursorPos.x / resolution, cursorPos.y / resolution),
	heightMap,  xzScale, offset, yScale);

    float scale = 2.0;

    gl_Position = mvp * vec4(  ( scale * (positionIn+vec3(0,0.5,0)) + pos ),1);
}
