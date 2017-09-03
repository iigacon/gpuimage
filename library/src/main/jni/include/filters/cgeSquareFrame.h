/*
 * cgeBrightnessAdjust.h
 *
 *  Created on: 2013-12-26
 *      Author: Wang Yang
 */

#ifndef _CGE_SQUAREFILTER_H_
#define _CGE_SQUAREFILTER_H_

#include "cgeGLFunctions.h"

namespace CGE
{
	class CGESquareFilter : public CGEImageFilterInterface
	{
	public:

		void setIntensity(float value); // range: [-1, 1]

		bool init();
        
        void render2Texture(CGEImageHandlerInterface* handler, GLuint srcTexture, GLuint vertexBufferID);

	protected:
		static CGEConstString paramName;
        
    private:
        float m_intensity;
	};
}

#endif