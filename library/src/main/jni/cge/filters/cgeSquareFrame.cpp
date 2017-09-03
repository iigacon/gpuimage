/*
* cgeBrightnessAdjust.cpp
*
*  Created on: 2013-12-26
*      Author: Wang Yang
*/

#include "cgeSquareFrame.h"
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
	CGEConstString CGESquareFilter::paramName = "intensity";

	bool CGESquareFilter::init()
	{
		if(initShadersFromString(g_vshDefaultWithoutTexCoord, s_fshSquare))
		{
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
        CGEImageFilterInterface::render2Texture(handler, srcTexture, vertexBufferID);
    }

	//////////////////////////////////////////////////////////////////////////


}