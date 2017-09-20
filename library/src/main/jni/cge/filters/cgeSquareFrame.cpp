/*
* cgeBrightnessAdjust.cpp
*
*  Created on: 2013-12-26
*      Author: Wang Yang
*/

#include "cgeSquareFrame.h"
#include <cmath>
#include <time.h>

const static char* const s_fshSquare = CGE_SHADER_STRING_PRECISION_H
(

precision highp float;
uniform sampler2D inputImageTexture;
varying vec2 textureCoordinate;
vec3 iResolution;
uniform float iTime;
float PI = 3.1415926535;

// beat value, 0->1, 1->0
float beat(float x) {
	float temp = x-1.0;
	temp *= temp;
	temp *= temp;
	return temp*(cos(30.0*x)*.5+.5);
}

// point to background color value
vec3 backgroundColor(vec2 p) {
	vec3 color = vec3(.3, .05, .2);

	// add a star
	float t = atan(p.y, p.x) / PI;
	t *= 5.0;
	t += iTime*0.5;
	t = abs(fract(t)*2.0-1.0);
	float star = smoothstep(0.5, 0.6, t);
	color = mix(color, vec3(0.5, 0.2, 0.4), star);
	return color;
}

float inHeart (vec2 pos, vec2 center, float size) {
  vec2 p = pos;
  if (size == 0.0) return 0.0;
  vec2 o = (p-center)/(1.6*size);
  float tmp = o.x*o.x+o.y*o.y-0.3;
  return step(tmp*tmp*tmp, o.x*o.x*o.y*o.y*o.y);
}

void main(){

    float m = inHeart(1.0-textureCoordinate.xy, vec2(0.5, 0.42), 0.44);
  vec2 dstPos = textureCoordinate;
  dstPos.x = dstPos.x * 1.0/1.0 + 0.0 ;
  vec4 texel;
  if (m > 0.5) {
      texel = texture2D(inputImageTexture, dstPos);
      gl_FragColor = texel;
  }else{

       vec3 iResolution= vec3(1.0, 1.0, 0.0);
       vec2 uv = dstPos.xy / iResolution.xy * 2.0 - 1.0;
       uv.x *= iResolution.x / iResolution.y;
       float mult = 3.0+4.0*beat(min(1.0, 0.09*iTime));
       uv *= mult;

       // get background
       vec3 color = backgroundColor(uv);
       // heart formula and color
       gl_FragColor = vec4(color, 1.0);

  }

}


);

const static char* const frag_1 = CGE_SHADER_STRING_PRECISION_H
(

precision highp float;
uniform sampler2D inputImageTexture;
varying vec2 textureCoordinate;
vec3 iResolution;
uniform float iTime;

void main(){
        vec3 iResolution= vec3(1.0, 1.0, 0.0);
        vec2 uv = textureCoordinate.xy / iResolution.xy;
        float s = sin(iTime * 12.);
        float l = .01;
        float r = texture2D(inputImageTexture, uv).x;
        float g = texture2D(inputImageTexture, uv + vec2(l*s,0.)).y;
        float b = texture2D(inputImageTexture, uv - vec2(0.,l*s)).z;
        gl_FragColor = vec4(r,g,b,1.);
}

);

const static char* const frag_2 = CGE_SHADER_STRING_PRECISION_H
(

precision highp float;
uniform sampler2D inputImageTexture;
varying vec2 textureCoordinate;
vec3 iResolution;
uniform float iTime;

vec3 hsv(float h,float s,float v) { return mix(vec3(1.),clamp((abs(fract(h+vec3(3.,2.,1.)/3.)*6.-3.)-1.),0.,1.),s)*v; }

float heart2D(vec2 P) {
	// Center in a unit circle
	P *= 1.8;
	P.y -= 1.25;

	float r = length(P);
	P = normalize(P);
	float pulse = 0.05 * sin(3.0 * iTime);

	return r -
		((P.y * pow(abs(P.x), 0.75 + pulse)) /
		 (P.y + 1.6) -
		 (pulse + 1.5) * P.y +
		  1.5);
}

float pow4(float x) { x *= x; return x * x; }

void main(){
        vec3 iResolution= vec3(1.0, 1.0, 0.0);

        vec2 p = textureCoordinate.xy / iResolution.xy;
          	const float maxValue = 1.5;
      	vec2 P = 2.0 * (textureCoordinate.xy - iResolution.xy / 2.0) / min(iResolution.x, iResolution.y);
      	P *= maxValue;

          float d = heart2D(P);

         const float speed 				= 0.4;
      		const float outlineThickness 	= 0.06;
      		const vec3  outlineColor 		= vec3(1.0);
      		const float bandThickness 		= 2.0;
      		const vec3  foreground 			= vec3(1.0, 0.6, 0.6);

      		// Rainbow
      		vec3 background = hsv(speed * iTime - d * 0.25 / bandThickness, 0.1, 1.0);

      		// Heart
      		vec3 color = mix(foreground, background, clamp(d / (outlineThickness / 2.0), 0.0, 1.0));

      		// Outline
      		color = mix(outlineColor, color, clamp(pow4(abs(d / outlineThickness)), 0.0, 1.0));

          // add Jean-Claude Van Damme
          vec3 fg = texture2D( inputImageTexture, p ).xyz;
          float maxrb = max( fg.r, fg.b );
          float k = clamp( (fg.g-maxrb)*3.0, 0.0, 1.0 );
          float dg = fg.g;
          fg.g = min( fg.g, maxrb*0.8 );
          fg += dg - fg.g;
          color = mix(fg, color, k);


          gl_FragColor = vec4( color, 1.0 );
}

);

const static char* const frag_3 = CGE_SHADER_STRING_PRECISION_H
(

precision highp float;
uniform sampler2D inputImageTexture;
varying vec2 textureCoordinate;
vec3 iResolution;
uniform float iTime;

vec2 zoom(vec2 uv, float amount) {
  return 0.5 + ((uv - 0.5) * amount);
}

void main(){
        vec3 iResolution= vec3(1.0, 1.0, 0.0);
        vec2 uv = textureCoordinate.xy / iResolution.xy;
        float s = tan(iTime * 2.0);
        float l = .04;
        if(s<0.8 && s>0.1){
            uv = zoom(uv, 1.0-s/4.0);
            float r = texture2D(inputImageTexture, uv).x;
            float g = texture2D(inputImageTexture, uv + vec2(l*s,0.)).y;
            float b = texture2D(inputImageTexture, uv - vec2(0.,l*s)).z;
            gl_FragColor = vec4(r,g,b,1.);
        }else{

            gl_FragColor = vec4(texture2D(inputImageTexture, uv));
        }
}

);

const static char* const frag_4 = CGE_SHADER_STRING_PRECISION_H
(

precision highp float;
uniform sampler2D inputImageTexture;
varying vec2 textureCoordinate;
vec3 iResolution;
uniform float iTime;

vec2 zoom(vec2 uv, float amount) {
  return 0.5 + ((uv - 0.5) * amount);
}

void main(){
        vec3 iResolution= vec3(1.0, 1.0, 0.0);
        vec2 uv = textureCoordinate.xy / iResolution.xy;
        float s = tan(iTime * 2.0);
        float l = .04;
        if(s>0.1){
            uv = zoom(uv, 1.0-s/8.0);
            float r = texture2D(inputImageTexture, uv).x;
            float g = texture2D(inputImageTexture, uv + vec2(l*s,0.)).y;
            float b = texture2D(inputImageTexture, uv - vec2(0.,l*s)).z;
            gl_FragColor = vec4(r,g,b,1.);
        }else{

            gl_FragColor = vec4(texture2D(inputImageTexture, uv));
        }
}

);

namespace CGE
{
    float x_seconds=0.0,x_milliseconds;
    float count_down_time_in_secs=0.0,time_left=0.0;
    clock_t x_startTime,x_countTime;
    float step = 0.02;
    float iTime = 0;

	CGEConstString CGESquareFilter::paramName = "intensity";

	bool CGESquareFilter::init()
	{
		if(initShadersFromString(g_vshDefaultWithoutTexCoord, frag_3))
		{
            count_down_time_in_secs=0;
            x_startTime=clock();
            time_left=count_down_time_in_secs+x_seconds;
			return true;
		}
		return false;
	}

	void CGESquareFilter::setIntensity(float value)
	{
		m_program.bind();
        m_program.sendUniformf(paramName, m_intensity);
	}
    
    void CGESquareFilter::render2Texture(CGEImageHandlerInterface* handler, GLuint srcTexture, GLuint vertexBufferID)
    {
        m_program.bind();
        iTime += step;
        m_program.sendUniformf("iTime", iTime);
        CGEImageFilterInterface::render2Texture(handler, srcTexture, vertexBufferID);
    }

	//////////////////////////////////////////////////////////////////////////


}