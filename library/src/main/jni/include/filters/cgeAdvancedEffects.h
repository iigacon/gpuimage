/*
 * cgeAdvancedEffects.h
 *
 *  Created on: 2013-12-13
 *      Author: Wang Yang
 */

#ifndef _CGEADVANCEDEFFECTS_H_
#define _CGEADVANCEDEFFECTS_H_

#include "cgeEmbossFilter.h"
#include "cgeEdgeFilter.h"
#include "cgeRandomBlurFilter.h"
#include "cgeBilateralBlurFilter.h"
#include "cgeMosaicBlurFilter.h"
#include "cgeLiquidationFilter.h"
#include "cgeHalftoneFilter.h"
#include "cgePolarPixellateFilter.h"
#include "cgePolkaDotFilter.h"
#include "cgeCrosshatchFilter.h"
#include "cgeHazeFilter.h"
#include "cgeLerpblurFilter.h"

#include "cgeSketchFilter.h"
#include "cgeBeautifyFilter.h"
#include "cgeSquareFrame.h"
#include "cgeDoubleFilter.h"
#include "cgeQuadFilter.h"
#include "cgeQuadFlipFilter.h"
#include "cgeHeartFilter.h"
#include "cgeZoomFilter.h"
#include "cgeHexaFilter.h"
#include "cgeTripleFilter.h"

namespace CGE
{
	CGESquareFilter* createSquareFilter();
	CGEEmbossFilter* createEmbossFilter();
	CGEEdgeFilter* createEdgeFilter();
	CGEEdgeSobelFilter* createEdgeSobelFilter();
	CGERandomBlurFilter* createRandomBlurFilter();
	CGEBilateralBlurFilter* createBilateralBlurFilter();
    CGEBilateralBlurBetterFilter* createBilateralBlurBetterFilter();
	CGEMosaicBlurFilter* createMosaicBlurFilter();
	CGELiquidationFilter* getLiquidationFilter(float ratio, float stride);
	CGELiquidationFilter* getLiquidationFilter(float width, float height , float stride);

	CGELiquidationNicerFilter* getLiquidationNicerFilter(float ratio, float stride);
	CGELiquidationNicerFilter* getLiquidationNicerFilter(float width, float height , float stride);

	CGEHalftoneFilter* createHalftoneFilter();
	CGEPolarPixellateFilter* createPolarPixellateFilter();
	CGEPolkaDotFilter* createPolkaDotFilter();
	CGECrosshatchFilter* createCrosshatchFilter();
	CGEHazeFilter* createHazeFilter();
	CGELerpblurFilter* createLerpblurFilter();

	CGESketchFilter* createSketchFilter();
    
    CGEBeautifyFilter* createBeautifyFilter();

    CGEDoubleFilter* createDoubleFilter();
	CGEQuadFilter* createQuadFilter();
	CGEQuadFlipFilter* createQuadFlipFilter();
	CGEHexaFilter* createHexaFilter();
	CGEHeartFilter* createHeartFilter();
	CGEZoomFilter* createZoomFilter();
	CGETripleFilter* createTripleFilter();

}

#endif 
