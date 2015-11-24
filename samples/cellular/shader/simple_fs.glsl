in vec3 viewSpacePixelPositionOut;

in vec3 normalOut;
in vec3 tangentOut;
in vec3 bitangentOut;
in vec2 texcoordOut;

in vec3 lightpos;

uniform sampler2D normalMap;
uniform sampler2D diffMap;
out vec4 fragmentColor;

// #ifdef HEIGHT_MAPPING
float ray_intersect_rm(sampler2D reliefmap, vec2 dp, vec2 ds) {

    const int linear_search_steps=10; // 10
    const int binary_search_steps=5; // 5

    float size=1.0/linear_search_steps; // current size of search window

    float depth=0.0; // current depth position

    // best match found (starts with last position 1.0)
    float best_depth=1.0;

    // linear search for first point within object.
    for ( int i=0; i< linear_search_steps-1;i++){
	depth+=size;
	float height=texture(reliefmap,dp+ds*depth).w;
	if (best_depth>0.996) // if no depth found yet
	    if (depth >= height)
		best_depth=depth; // store best depth
    }
    depth=best_depth;

    // binary search to improve the precision of the result.
    for ( int i=0; i<binary_search_steps;i++) {
	size*=0.5;
	float height=texture(reliefmap,dp+ds*depth).w;
	if (depth >= height) { // found better depth.
	    best_depth = depth;
	    depth -= 2*size;
	}
	depth+=size;
    }
    return best_depth;
}
// #endif

void main(void) {


    vec4 ambient = vec4(vec3(0.3), 1.0);
    vec4 diffuse = vec4(vec3(1.0), 1.0);
    vec4 specular = vec4(vec3(0.5), 1.0);






    float depth = 0.1;
    float tile = 1.0;

    vec3 p = viewSpacePixelPositionOut; // pixel position in eye space
    vec3 v = normalize(p); // view vector in eye space

    // view vector in tangent space
    vec3 s = normalize(vec3(dot(v,tangentOut.xyz),
		       dot(v,bitangentOut.xyz),dot(normalOut,-v)));

    // ray direction.
    vec2 ds = (s.xy*depth)/ ( s.z   );
    // ray origin
    vec2 dp = texcoordOut*tile;
    // get intersection distance
    float d = ray_intersect_rm(normalMap,dp,ds);

    // get normal and color at intersection point
    vec2 uv=dp+ds*d;
    vec4 finalNormal = texture(normalMap,uv);
    vec4 finalDiffuse=texture(diffMap,uv);









    finalNormal.xyz=finalNormal.xyz*2.0-1.0;
     // expand normal to eye space(from tangent space)
    finalNormal.xyz=normalize(finalNormal.x*tangentOut.xyz+
		    finalNormal.y*bitangentOut.xyz+finalNormal.z*normalOut);

    // compute light direction
    p += v*d*s.z;
    vec3 l=normalize(p-lightpos.xyz);

    /*
      Depth correct.
    */
    const float near = 0.1;
    const float far  = 1024.0;
    float planes_x=-far/(far-near);
    float planes_y =-far*near/(far-near);
    gl_FragDepth=((planes_x*p.z+planes_y)/-p.z);


    // compute diffuse and specular terms
    float diff=clamp(dot(-l,finalNormal.xyz),0,1);

    // -l, because the light vector goes from the light TO the point!
    float spec=clamp(dot(normalize(-l-v),finalNormal.xyz),0,1);

    vec4 finalcolor=ambient*finalDiffuse;

    finalcolor.xyz+=0.3*(finalDiffuse.xyz*diffuse.xyz*diff+
			 specular.xyz*pow(spec,specular.w));
    finalcolor.w=1.0;
    fragmentColor=finalcolor;
}
