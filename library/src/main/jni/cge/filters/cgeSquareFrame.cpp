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

namespace CGE
{
    float x_seconds=0.0,x_milliseconds;
    float count_down_time_in_secs=0.0,time_left=0.0;
    clock_t x_startTime,x_countTime;

	CGEConstString CGESquareFilter::paramName = "intensity";

	bool CGESquareFilter::init()
	{
		if(initShadersFromString(g_vshDefaultWithoutTexCoord, s_fshSquare))
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
        x_countTime=clock();
        x_milliseconds=x_countTime-x_startTime;
        x_seconds=(float)((int)(5.0*(x_milliseconds/(CLOCKS_PER_SEC))));
        if(time_left!=count_down_time_in_secs+x_seconds)
        {
           m_program.sendUniformf("iTime", clock() / (CLOCKS_PER_SEC * 1.0f));
           time_left=count_down_time_in_secs+x_seconds/5.0;
        }
        CGEImageFilterInterface::render2Texture(handler, srcTexture, vertexBufferID);
    }

	//////////////////////////////////////////////////////////////////////////


}