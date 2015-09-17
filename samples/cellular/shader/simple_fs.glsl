#include "shader_lib/lib.glsl"

in vec3 position;

out vec4 fragmentColor;

in vec2 texCoord;

uniform sampler2D tex;

#ifdef NORMAL_MAPPING
uniform sampler2D normalMap;
#endif

in vec3 normal;

#ifdef NORMAL_MAPPING
in vec3 tangent;
#endif


uniform vec3 viewSpaceLightPosition;
in vec3 viewSpaceNormal;
in vec3 viewSpacePosition;

void main()
{

    vec4 color = texture(tex, vec2(texCoord.x, 1-texCoord.y  )  );

    vec3 shading = phongVertex(color.rgb, viewSpaceNormal, viewSpaceLightPosition, viewSpacePosition);

#ifdef NORMAL_MAPPING

    vec4 normal = texture(normalMap, vec2(texCoord.x, 1-texCoord.y  )  );

fragmentColor = vec4( /*abs(tangent.rgb)*/ normal.rgb , color.a);
#else


    fragmentColor = vec4( shading , color.a);
#endif
}
