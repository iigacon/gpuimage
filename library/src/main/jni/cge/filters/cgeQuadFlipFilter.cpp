/*
* cgeBrightnessAdjust.cpp
*
*  Created on: 2013-12-26
*      Author: Wang Yang
*/

#include "cgeQuadFlipFilter.h"
#include <cmath>

const static char* const s_fshSquare = CGE_SHADER_STRING_PRECISION_H
(
precision highp float;
uniform sampler2D inputImageTexture;
varying vec2 textureCoordinate;

void main(){
   float left = step(textureCoordinate.x, 0.5);
   float top = step(textureCoordinate.y, 0.5);
   vec2 dstPos = vec2(left * (1.0 - textureCoordinate.x * 2.0) + (1.0 - left) * ((textureCoordinate.x - 0.5) * 2.0), top * textureCoordinate.y * 2.0 + (1.0 - top) * (1.0 - (textureCoordinate.y - 0.5) * 2.0));
   dstPos.x = dstPos.x * 1.0/1.0 + 0.0;
   gl_FragColor = texture2D(inputImageTexture, dstPos);
}


);

namespace CGE
{
	CGEConstString CGEQuadFlipFilter::paramName = "intensity";

	bool CGEQuadFlipFilter::init()
	{
		if(initShadersFromString(g_vshDefaultWithoutTexCoord, s_fshSquare))
		{
			return true;
		}
		return false;
	}

	void CGEQuadFlipFilter::setIntensity(float value)
	{
		m_program.bind();
        m_program.sendUniformf(paramName, m_intensity);
	}
    
    void CGEQuadFlipFilter::render2Texture(CGEImageHandlerInterface* handler, GLuint srcTexture, GLuint vertexBufferID)
    {
        CGEImageFilterInterface::render2Texture(handler, srcTexture, vertexBufferID);
    }

	//////////////////////////////////////////////////////////////////////////


}