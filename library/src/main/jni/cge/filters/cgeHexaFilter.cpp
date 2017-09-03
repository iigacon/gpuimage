/*
* cgeBrightnessAdjust.cpp
*
*  Created on: 2013-12-26
*      Author: Wang Yang
*/

#include "cgeHexaFilter.h"
#include <cmath>

const static char* const s_fshSquare = CGE_SHADER_STRING_PRECISION_H
(
precision highp float;
varying vec2 textureCoordinate;
uniform sampler2D inputImageTexture;
const float lineWidth = 0.005;
const float lowMainAreaBound = 1.0/6.0;
const float highMainAreaBound = 5.0/6.0;
const float lowBorderBound = lowMainAreaBound + lineWidth;
const float highBorderBound = highMainAreaBound - lineWidth;
void main() {
  float checkInMainArea = step(textureCoordinate.x, highMainAreaBound) * step(lowMainAreaBound, textureCoordinate.x) * step(textureCoordinate.y, highMainAreaBound) * step(lowMainAreaBound, textureCoordinate.y);
  vec2 dstPos1 = (textureCoordinate + vec2(-lowMainAreaBound, -lowMainAreaBound)) * 1.5;
  vec2 dstPos2 = vec2(mod(textureCoordinate.x, lowMainAreaBound) * 6.0, mod(textureCoordinate.y, lowMainAreaBound) * 6.0);
  vec2 dstPos = checkInMainArea * dstPos1 + (1.0 - checkInMainArea) * dstPos2;
  dstPos.x = dstPos.x * 1.0/1.0 + 0.0 ;
  vec4 texel = texture2D(inputImageTexture, dstPos);
  float tmpCheck = step(lowBorderBound, textureCoordinate.x) * step(textureCoordinate.x, highBorderBound) * step(lowBorderBound, textureCoordinate.y) * step(textureCoordinate.y, highBorderBound);
  float checkInBorder = checkInMainArea * (1.0 - tmpCheck);
  gl_FragColor = mix(texel, vec4(1.0, 1.0, 1.0, 1.0), checkInBorder);
}


);

namespace CGE
{
	CGEConstString CGEHexaFilter::paramName = "intensity";

	bool CGEHexaFilter::init()
	{
		if(initShadersFromString(g_vshDefaultWithoutTexCoord, s_fshSquare))
		{
			return true;
		}
		return false;
	}

	void CGEHexaFilter::setIntensity(float value)
	{
		m_program.bind();
        m_program.sendUniformf(paramName, m_intensity);
	}
    
    void CGEHexaFilter::render2Texture(CGEImageHandlerInterface* handler, GLuint srcTexture, GLuint vertexBufferID)
    {
        CGEImageFilterInterface::render2Texture(handler, srcTexture, vertexBufferID);
    }

	//////////////////////////////////////////////////////////////////////////


}