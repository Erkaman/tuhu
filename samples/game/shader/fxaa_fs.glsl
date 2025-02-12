in vec2 texCoord;
out vec4 fragmentColor;

uniform sampler2D colorTexture;
uniform float doAA;

#define FxaaSat(a) clamp((a), 0.0, 1.0)
#define FxaaTexLod0(t, p) textureLod(t, p, 0.0)
#define FxaaTexOff(t, p, o, r) textureLodOffset(t, p, 0.0, o)


vec3 FxaaPixelShader(
vec4 posPos,       // Output of FxaaVertexShader interpolated across screen.
sampler2D tex,         // Input texture.
vec2 rcpFrame) {   // Constant {1.0/frameWidth, 1.0/frameHeight}.
/*--------------------------------------------------------------------------*/
    #define FXAA_REDUCE_MIN   (1.0/128.0)
#define FXAA_REDUCE_MUL   0.0 //(1.0/8.0) // 0
    #define FXAA_SPAN_MAX     8.0
/*--------------------------------------------------------------------------*/
    vec3 rgbNW = FxaaTexLod0(tex, posPos.zw).xyz;
    vec3 rgbNE = FxaaTexOff(tex, posPos.zw, ivec2(1,0), rcpFrame.xy).xyz;
    vec3 rgbSW = FxaaTexOff(tex, posPos.zw, ivec2(0,1), rcpFrame.xy).xyz;
    vec3 rgbSE = FxaaTexOff(tex, posPos.zw, ivec2(1,1), rcpFrame.xy).xyz;
    vec3 rgbM  = FxaaTexLod0(tex, posPos.xy).xyz;
/*--------------------------------------------------------------------------*/
    vec3 luma = vec3(0.299, 0.587, 0.114);
    float lumaNW = dot(rgbNW, luma);
    float lumaNE = dot(rgbNE, luma);
    float lumaSW = dot(rgbSW, luma);
    float lumaSE = dot(rgbSE, luma);
    float lumaM  = dot(rgbM,  luma);
/*--------------------------------------------------------------------------*/
    float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
    float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));
/*--------------------------------------------------------------------------*/
    vec2 dir;
    dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
    dir.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));
/*--------------------------------------------------------------------------*/
    float dirReduce = max(
        (lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * FXAA_REDUCE_MUL),
        FXAA_REDUCE_MIN);
    float rcpDirMin = 1.0/(min(abs(dir.x), abs(dir.y)) + dirReduce);
    dir = min(vec2( FXAA_SPAN_MAX,  FXAA_SPAN_MAX),
          max(vec2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX),
          dir * rcpDirMin)) * rcpFrame.xy;
/*--------------------------------------------------------------------------*/
    vec3 rgbA = (1.0/2.0) * (
        FxaaTexLod0(tex, posPos.xy + dir * (1.0/3.0 - 0.5)).xyz +
        FxaaTexLod0(tex, posPos.xy + dir * (2.0/3.0 - 0.5)).xyz);
    vec3 rgbB = rgbA * (1.0/2.0) + (1.0/4.0) * (
        FxaaTexLod0(tex, posPos.xy + dir * (0.0/3.0 - 0.5)).xyz +
        FxaaTexLod0(tex, posPos.xy + dir * (3.0/3.0 - 0.5)).xyz);
    float lumaB = dot(rgbB, luma);
    if((lumaB < lumaMin) || (lumaB > lumaMax)) return rgbA;
    return rgbB;
}

vec4 FxaaVertexShader(
vec2 pos,                 // Both x and y range {-1.0 to 1.0 across screen}.
vec2 rcpFrame) {          // {1.0/frameWidth, 1.0/frameHeight}
/*--------------------------------------------------------------------------*/
#define FXAA_SUBPIX_SHIFT 0.0 //(1.0/4.0) // 0
/*--------------------------------------------------------------------------*/
    vec4 posPos;
    posPos.xy = (pos.xy * 0.5) + 0.5;
    posPos.zw = posPos.xy - (rcpFrame * (0.5 + FXAA_SUBPIX_SHIFT));
    return posPos; }


void main() {

    //  fragmentColor = vec4( vec3(texture(colorTexture, texCoord).r, 0.0, 0.0),1.0);
//    fragmentColor = vec4(vec3(texCoord, 0.0),1.0);


    vec2 rcpFrame = 1.0 / textureSize(colorTexture, 0);

    vec4 posPos = FxaaVertexShader(texCoord * 2.0 - 1.0, rcpFrame);

  fragmentColor =
      doAA > 0.0 ?
      vec4(FxaaPixelShader(posPos, colorTexture, rcpFrame),1.0) :

      vec4( texture(colorTexture, texCoord).rgb,1.0);

/*


    fragmentColor =;
    */
}
