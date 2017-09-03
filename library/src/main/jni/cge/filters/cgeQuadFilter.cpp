/*
* cgeBrightnessAdjust.cpp
*
*  Created on: 2013-12-26
*      Author: Wang Yang
*/

#include "cgeQuadFilter.h"
#include <cmath>

const static char* const s_fshSquare = CGE_SHADER_STRING_PRECISION_H
(
precision highp float;
uniform sampler2D inputImageTexture;
varying vec2 textureCoordinate;

void main(){
  vec2 dstPos = vec2(mod(textureCoordinate.x, 0.5) * 2.0, mod(textureCoordinate.y, 0.5) * 2.0);
  dstPos.x = dstPos.x * 1.0/1.0 + 0.0;
  gl_FragColor = texture2D(inputImageTexture, dstPos);

}


);

namespace CGE
{
	CGEConstString CGEQuadFilter::paramName = "intensity";

	bool CGEQuadFilter::init()
	{
		if(initShadersFromString(g_vshDefaultWithoutTexCoord, s_fshSquare))
		{
			return true;
		}
		return false;
	}

	void CGEQuadFilter::setIntensity(float value)
	{
		m_program.bind();
        m_program.sendUniformf(paramName, m_intensity);
	}
    
    void CGEQuadFilter::render2Texture(CGEImageHandlerInterface* handler, GLuint srcTexture, GLuint vertexBufferID)
    {
        CGEImageFilterInterface::render2Texture(handler, srcTexture, vertexBufferID);
    }

	//////////////////////////////////////////////////////////////////////////


}