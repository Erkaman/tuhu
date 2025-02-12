float f(sampler2D heightMap, vec2 texCoord) {
    return -1 + texture(heightMap, texCoord).r*2;
}

float f(sampler2D heightMap, float x, float z) {
    return f(heightMap, vec2(x,z));
}

vec3 getNormal(sampler2D hm, vec2 texCoord)
{

    float eps = 1.0 / resolution;
    vec3 p = vec3(texCoord.x, 0, texCoord.y);

    //eps on x axis.
    vec3 va = vec3(2*eps, f(hm,p.x+eps,p.z) - f(hm,p.x-eps,p.z), 0 );

    vec3 vb = vec3(0, f(hm,p.x,p.z+eps) - f(hm,p.x,p.z-eps), 2*eps );

    // is there not some more optimal way of doing this?
    // http://stackoverflow.com/questions/5281261/generating-a-normal-map-from-a-height-map
    vec3 n = normalize(cross(normalize(vb), normalize(va) ));

    return n;


//    return normalize(va);
}

vec3 computePos(
    vec2 pos,
    sampler2D heightMap) {

    return offset + vec3(pos.x, f(heightMap,pos.xy), pos.y) *  vec3(xzScale,yScale,xzScale);
}
