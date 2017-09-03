/*
 * cgeBrightnessAdjust.h
 *
 *  Created on: 2013-12-26
 *      Author: Wang Yang
 */


#include "cgeAdvancedEffectsCommon.h"

namespace CGE
{
	class CGETripleFilter : public CGEImageFilterInterface
	{
	public:
		~CGETripleFilter();

		enum { MAX_LERP_BLUR_INTENSITY = 12 };
		
		bool init();

		struct TextureCache
		{
			GLuint texID;
			CGESizei size;
		};

		void setBlurLevel(int value);

		void setIntensity(float value);

		void render2Texture(CGEImageHandlerInterface* handler, GLuint srcTexture, GLuint vertexBufferID);

		//图像渐进缩小基数( > 1.1)
		void setMipmapBase(float value);

	protected:
		void _genMipmaps(int width, int height);
		void _clearMipmaps();
		int _calcLevel(int len, int level);

	private:
				
		TextureCache m_texCache[MAX_LERP_BLUR_INTENSITY];
		CGESizei m_cacheTargetSize;
		int m_intensity;
		float m_mipmapBase;
		bool m_isBaseChanged;

		FrameBuffer m_framebuffer;

	};


}