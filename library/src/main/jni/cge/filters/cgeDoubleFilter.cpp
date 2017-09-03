/*
* cgeBrightnessAdjust.cpp
*
*  Created on: 2013-12-26
*      Author: Wang Yang
*/

#include "cgeDoubleFilter.h"
#include <cmath>

const static char* const s_fshSquare = CGE_SHADER_STRING_PRECISION_H
(
precision highp float;
varying vec2 textureCoordinate;
uniform sampler2D inputImageTexture;

void main() {
  float left = step(0.5, textureCoordinate.x);
  vec2 dstPos = vec2(left * (textureCoordinate.x + 0.25) -0.35+(1.0 - left) * (1.25 - textureCoordinate.x), textureCoordinate.y);
  dstPos.x = dstPos.x * 1.0/1.0 + 0.0;
  gl_FragColor = texture2D(inputImageTexture, dstPos);
}

);

namespace CGE
{
	CGEConstString CGEDoubleFilter::paramName = "intensity";

	bool CGEDoubleFilter::init()
	{
		if(initShadersFromString(g_vshDefaultWithoutTexCoord, s_fshSquare))
		{
			return true;
		}
		return false;
	}

	void CGEDoubleFilter::setIntensity(float value)
	{
		m_program.bind();
        m_program.sendUniformf(paramName, m_intensity);
	}
    
    void CGEDoubleFilter::render2Texture(CGEImageHandlerInterface* handler, GLuint srcTexture, GLuint vertexBufferID)
    {
        CGEImageFilterInterface::render2Texture(handler, srcTexture, vertexBufferID);
    }

	//////////////////////////////////////////////////////////////////////////


}