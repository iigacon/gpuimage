/*
* cgeBrightnessAdjust.cpp
*
*  Created on: 2013-12-26
*      Author: Wang Yang
*/

#include "cgeHeartFilter.h"
#include <cmath>
#include <string>

void append_format(std::string& str, const char *fmt, ...)
    {
        va_list ap;
        char *ret;

        va_start(ap, fmt);
        vasprintf(&ret, fmt, ap);
        va_end(ap);

        str.append(ret);
        free(ret);
    }

static unsigned short const stackblur_mul[255] =
{
		512,512,456,512,328,456,335,512,405,328,271,456,388,335,292,512,
		454,405,364,328,298,271,496,456,420,388,360,335,312,292,273,512,
		482,454,428,405,383,364,345,328,312,298,284,271,259,496,475,456,
		437,420,404,388,374,360,347,335,323,312,302,292,282,273,265,512,
		497,482,468,454,441,428,417,405,394,383,373,364,354,345,337,328,
		320,312,305,298,291,284,278,271,265,259,507,496,485,475,465,456,
		446,437,428,420,412,404,396,388,381,374,367,360,354,347,341,335,
		329,323,318,312,307,302,297,292,287,282,278,273,269,265,261,512,
		505,497,489,482,475,468,461,454,447,441,435,428,422,417,411,405,
		399,394,389,383,378,373,368,364,359,354,350,345,341,337,332,328,
		324,320,316,312,309,305,301,298,294,291,287,284,281,278,274,271,
		268,265,262,259,257,507,501,496,491,485,480,475,470,465,460,456,
		451,446,442,437,433,428,424,420,416,412,408,404,400,396,392,388,
		385,381,377,374,370,367,363,360,357,354,350,347,344,341,338,335,
		332,329,326,323,320,318,315,312,310,307,304,302,299,297,294,292,
		289,287,285,282,280,278,275,273,271,269,267,265,263,261,259
};

static unsigned char const stackblur_shr[255] =
{
		9, 11, 12, 13, 13, 14, 14, 15, 15, 15, 15, 16, 16, 16, 16, 17,
		17, 17, 17, 17, 17, 17, 18, 18, 18, 18, 18, 18, 18, 18, 18, 19,
		19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 20, 20, 20,
		20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 21,
		21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21,
		21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 22, 22, 22, 22, 22, 22,
		22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
		22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 23,
		23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
		23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
		23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
		23, 23, 23, 23, 23, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
		24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
		24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
		24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
		24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24
};

void stackblurJob(unsigned char* src,				///< input image data
                      unsigned int w,					///< image width
                      unsigned int h,					///< image height
                      unsigned int radius,				///< blur intensity (should be in 2..254 range)
                      int cores,						///< total number of working threads
                      int core,							///< current thread number
                      int step,							///< step of processing (1,2)
                      unsigned char* stack				///< stack buffer
                      )
    {
        unsigned int x, y, xp, yp, i;
        unsigned int sp;
        unsigned int stack_start;
        unsigned char* stack_ptr;

        unsigned char* src_ptr;
        unsigned char* dst_ptr;

        unsigned long sum_r;
        unsigned long sum_g;
        unsigned long sum_b;
        unsigned long sum_a;
        unsigned long sum_in_r;
        unsigned long sum_in_g;
        unsigned long sum_in_b;
        unsigned long sum_in_a;
        unsigned long sum_out_r;
        unsigned long sum_out_g;
        unsigned long sum_out_b;
        unsigned long sum_out_a;

        unsigned int wm = w - 1;
        unsigned int hm = h - 1;
        unsigned int w4 = w * 4;
        unsigned int div = (radius * 2) + 1;
        unsigned int mul_sum = stackblur_mul[radius];
        unsigned char shr_sum = stackblur_shr[radius];


        if (step == 1)
        {
            int minY = core * h / cores;
            int maxY = (core + 1) * h / cores;

            for(y = minY; y < maxY; y++)
            {
                sum_r = sum_g = sum_b = sum_a =
                sum_in_r = sum_in_g = sum_in_b = sum_in_a =
                sum_out_r = sum_out_g = sum_out_b = sum_out_a = 0;

                src_ptr = src + w4 * y; // start of line (0,y)

                for(i = 0; i <= radius; i++)
                {
                    stack_ptr    = &stack[ 4 * i ];
                    stack_ptr[0] = src_ptr[0];
                    stack_ptr[1] = src_ptr[1];
                    stack_ptr[2] = src_ptr[2];
                    stack_ptr[3] = src_ptr[3];
                    sum_r += src_ptr[0] * (i + 1);
                    sum_g += src_ptr[1] * (i + 1);
                    sum_b += src_ptr[2] * (i + 1);
                    sum_a += src_ptr[3] * (i + 1);
                    sum_out_r += src_ptr[0];
                    sum_out_g += src_ptr[1];
                    sum_out_b += src_ptr[2];
                    sum_out_a += src_ptr[3];
                }


                for(i = 1; i <= radius; i++)
                {
                    if (i <= wm) src_ptr += 4;
                    stack_ptr = &stack[ 4 * (i + radius) ];
                    stack_ptr[0] = src_ptr[0];
                    stack_ptr[1] = src_ptr[1];
                    stack_ptr[2] = src_ptr[2];
                    stack_ptr[3] = src_ptr[3];
                    sum_r += src_ptr[0] * (radius + 1 - i);
                    sum_g += src_ptr[1] * (radius + 1 - i);
                    sum_b += src_ptr[2] * (radius + 1 - i);
                    sum_a += src_ptr[3] * (radius + 1 - i);
                    sum_in_r += src_ptr[0];
                    sum_in_g += src_ptr[1];
                    sum_in_b += src_ptr[2];
                    sum_in_a += src_ptr[3];
                }


                sp = radius;
                xp = radius;
                if (xp > wm) xp = wm;
                src_ptr = src + 4 * (xp + y * w); //   img.pix_ptr(xp, y);
                dst_ptr = src + y * w4; // img.pix_ptr(0, y);
                for(x = 0; x < w; x++)
                {
                    dst_ptr[0] = (sum_r * mul_sum) >> shr_sum;
                    dst_ptr[1] = (sum_g * mul_sum) >> shr_sum;
                    dst_ptr[2] = (sum_b * mul_sum) >> shr_sum;
                    dst_ptr[3] = (sum_a * mul_sum) >> shr_sum;
                    dst_ptr += 4;

                    sum_r -= sum_out_r;
                    sum_g -= sum_out_g;
                    sum_b -= sum_out_b;
                    sum_a -= sum_out_a;

                    stack_start = sp + div - radius;
                    if (stack_start >= div) stack_start -= div;
                    stack_ptr = &stack[4 * stack_start];

                    sum_out_r -= stack_ptr[0];
                    sum_out_g -= stack_ptr[1];
                    sum_out_b -= stack_ptr[2];
                    sum_out_a -= stack_ptr[3];

                    if(xp < wm)
                    {
                        src_ptr += 4;
                        ++xp;
                    }

                    stack_ptr[0] = src_ptr[0];
                    stack_ptr[1] = src_ptr[1];
                    stack_ptr[2] = src_ptr[2];
                    stack_ptr[3] = src_ptr[3];

                    sum_in_r += src_ptr[0];
                    sum_in_g += src_ptr[1];
                    sum_in_b += src_ptr[2];
                    sum_in_a += src_ptr[3];
                    sum_r    += sum_in_r;
                    sum_g    += sum_in_g;
                    sum_b    += sum_in_b;
                    sum_a    += sum_in_a;

                    ++sp;
                    if (sp >= div) sp = 0;
                    stack_ptr = &stack[sp*4];

                    sum_out_r += stack_ptr[0];
                    sum_out_g += stack_ptr[1];
                    sum_out_b += stack_ptr[2];
                    sum_out_a += stack_ptr[3];
                    sum_in_r  -= stack_ptr[0];
                    sum_in_g  -= stack_ptr[1];
                    sum_in_b  -= stack_ptr[2];
                    sum_in_a  -= stack_ptr[3];


                }

            }
        }

        // step 2
        if (step == 2)
        {
            int minX = core * w / cores;
            int maxX = (core + 1) * w / cores;

            for(x = minX; x < maxX; x++)
            {
                sum_r =	sum_g =	sum_b =	sum_a =
                sum_in_r = sum_in_g = sum_in_b = sum_in_a =
                sum_out_r = sum_out_g = sum_out_b = sum_out_a = 0;

                src_ptr = src + 4 * x; // x,0
                for(i = 0; i <= radius; i++)
                {
                    stack_ptr    = &stack[i * 4];
                    stack_ptr[0] = src_ptr[0];
                    stack_ptr[1] = src_ptr[1];
                    stack_ptr[2] = src_ptr[2];
                    stack_ptr[3] = src_ptr[3];
                    sum_r           += src_ptr[0] * (i + 1);
                    sum_g           += src_ptr[1] * (i + 1);
                    sum_b           += src_ptr[2] * (i + 1);
                    sum_a           += src_ptr[3] * (i + 1);
                    sum_out_r       += src_ptr[0];
                    sum_out_g       += src_ptr[1];
                    sum_out_b       += src_ptr[2];
                    sum_out_a       += src_ptr[3];
                }
                for(i = 1; i <= radius; i++)
                {
                    if(i <= hm) src_ptr += w4; // +stride

                    stack_ptr = &stack[4 * (i + radius)];
                    stack_ptr[0] = src_ptr[0];
                    stack_ptr[1] = src_ptr[1];
                    stack_ptr[2] = src_ptr[2];
                    stack_ptr[3] = src_ptr[3];
                    sum_r += src_ptr[0] * (radius + 1 - i);
                    sum_g += src_ptr[1] * (radius + 1 - i);
                    sum_b += src_ptr[2] * (radius + 1 - i);
                    sum_a += src_ptr[3] * (radius + 1 - i);
                    sum_in_r += src_ptr[0];
                    sum_in_g += src_ptr[1];
                    sum_in_b += src_ptr[2];
                    sum_in_a += src_ptr[3];
                }

                sp = radius;
                yp = radius;
                if (yp > hm) yp = hm;
                src_ptr = src + 4 * (x + yp * w); // img.pix_ptr(x, yp);
                dst_ptr = src + 4 * x; 			  // img.pix_ptr(x, 0);
                for(y = 0; y < h; y++)
                {
                    dst_ptr[0] = (sum_r * mul_sum) >> shr_sum;
                    dst_ptr[1] = (sum_g * mul_sum) >> shr_sum;
                    dst_ptr[2] = (sum_b * mul_sum) >> shr_sum;
                    dst_ptr[3] = (sum_a * mul_sum) >> shr_sum;
                    dst_ptr += w4;

                    sum_r -= sum_out_r;
                    sum_g -= sum_out_g;
                    sum_b -= sum_out_b;
                    sum_a -= sum_out_a;

                    stack_start = sp + div - radius;
                    if(stack_start >= div) stack_start -= div;
                    stack_ptr = &stack[4 * stack_start];

                    sum_out_r -= stack_ptr[0];
                    sum_out_g -= stack_ptr[1];
                    sum_out_b -= stack_ptr[2];
                    sum_out_a -= stack_ptr[3];

                    if(yp < hm)
                    {
                        src_ptr += w4; // stride
                        ++yp;
                    }

                    stack_ptr[0] = src_ptr[0];
                    stack_ptr[1] = src_ptr[1];
                    stack_ptr[2] = src_ptr[2];
                    stack_ptr[3] = src_ptr[3];

                    sum_in_r += src_ptr[0];
                    sum_in_g += src_ptr[1];
                    sum_in_b += src_ptr[2];
                    sum_in_a += src_ptr[3];
                    sum_r    += sum_in_r;
                    sum_g    += sum_in_g;
                    sum_b    += sum_in_b;
                    sum_a    += sum_in_a;

                    ++sp;
                    if (sp >= div) sp = 0;
                    stack_ptr = &stack[sp*4];

                    sum_out_r += stack_ptr[0];
                    sum_out_g += stack_ptr[1];
                    sum_out_b += stack_ptr[2];
                    sum_out_a += stack_ptr[3];
                    sum_in_r  -= stack_ptr[0];
                    sum_in_g  -= stack_ptr[1];
                    sum_in_b  -= stack_ptr[2];
                    sum_in_a  -= stack_ptr[3];
                }
            }
        }

    }



const static char* const s_fshSquare = CGE_SHADER_STRING_PRECISION_H
(
precision highp float;
varying vec2 textureCoordinate;
uniform sampler2D inputImageTexture;
float texelWidthOffset=1.0/720.0;
float texelHeightOffset = 1.0/720.0;

float inHeart (vec2 pos, vec2 center, float size) {
  vec2 p = pos;

  if (size == 0.0) return 0.0;
  vec2 o = (p-center)/(1.6*size);
  float tmp = o.x*o.x+o.y*o.y-0.3;
  return step(tmp*tmp*tmp, o.x*o.x*o.y*o.y*o.y);
}
void main() {
  float m = inHeart(1.0-textureCoordinate.xy, vec2(0.5, 0.42), 0.44);
  vec2 dstPos = textureCoordinate;
  dstPos.x = dstPos.x * 1.0/1.0 + 0.0 ;
  vec4 texel;
  if (m > 0.5) {
      texel = texture2D(inputImageTexture, dstPos);
      gl_FragColor = texel;
  }else{
    vec4 sum = vec4(0.0);
     vec2 singleStepOffset = vec2(texelWidthOffset, texelHeightOffset);
 sum += texture2D(inputImageTexture, textureCoordinate.xy) * 0.015958;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 1.499400) * 0.031852;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 1.499400) * 0.031852;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 3.498600) * 0.031598;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 3.498600) * 0.031598;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 5.497800) * 0.031146;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 5.497800) * 0.031146;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 7.497000) * 0.030505;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 7.497000) * 0.030505;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 9.496200) * 0.029687;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 9.496200) * 0.029687;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 11.495400) * 0.028707;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 11.495400) * 0.028707;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 13.494600) * 0.027582;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 13.494600) * 0.027582;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 15.493801) * 0.026331;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 15.493801) * 0.026331;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 17.493000) * 0.024978;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 17.493000) * 0.024978;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 19.492199) * 0.023543;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 19.492199) * 0.023543;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 21.491400) * 0.022048;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 21.491400) * 0.022048;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 23.490601) * 0.020517;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 23.490601) * 0.020517;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 25.489801) * 0.018971;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 25.489801) * 0.018971;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 27.489000) * 0.017429;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 27.489000) * 0.017429;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 29.488203) * 0.015910;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 29.488203) * 0.015910;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 31.487404) * 0.014431;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 31.487404) * 0.014431;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 33.486603) * 0.013006;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 33.486603) * 0.013006;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 35.485802) * 0.011648;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 35.485802) * 0.011648;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 37.485001) * 0.010364;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 37.485001) * 0.010364;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 39.484203) * 0.009163;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 39.484203) * 0.009163;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 41.483410) * 0.008050;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 41.483410) * 0.008050;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 43.482605) * 0.007027;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 43.482605) * 0.007027;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 45.481808) * 0.006094;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 45.481808) * 0.006094;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 47.481007) * 0.005252;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 47.481007) * 0.005252;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 49.480213) * 0.004497;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 49.480213) * 0.004497;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 51.479412) * 0.003826;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 51.479412) * 0.003826;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 53.478615) * 0.003235;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 53.478615) * 0.003235;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 55.477814) * 0.002717;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 55.477814) * 0.002717;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 57.477020) * 0.002268;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 57.477020) * 0.002268;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 59.476219) * 0.001881;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 59.476219) * 0.001881;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 61.475422) * 0.001550;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 61.475422) * 0.001550;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 63.474621) * 0.001269;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 63.474621) * 0.001269;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 65.473824) * 0.001033;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 65.473824) * 0.001033;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 67.473022) * 0.000835;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 67.473022) * 0.000835;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 69.472229) * 0.000670;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 69.472229) * 0.000670;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 71.471428) * 0.000535;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 71.471428) * 0.000535;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 73.470634) * 0.000424;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 73.470634) * 0.000424;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 75.469841) * 0.000334;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 75.469841) * 0.000334;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 77.469040) * 0.000262;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 77.469040) * 0.000262;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 79.468246) * 0.000204;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 79.468246) * 0.000204;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 81.467445) * 0.000157;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 81.467445) * 0.000157;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 83.466644) * 0.000121;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 83.466644) * 0.000121;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 85.465858) * 0.000092;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 85.465858) * 0.000092;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 87.465057) * 0.000070;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 87.465057) * 0.000070;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 89.464264) * 0.000053;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 89.464264) * 0.000053;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 91.463463) * 0.000039;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 91.463463) * 0.000039;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 93.462669) * 0.000029;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 93.462669) * 0.000029;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 95.461876) * 0.000022;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 95.461876) * 0.000022;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 97.461067) * 0.000016;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 97.461067) * 0.000016;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 99.460289) * 0.000012;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 99.460289) * 0.000012;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 101.459488) * 0.000008;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 101.459488) * 0.000008;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 103.458687) * 0.000006;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 103.458687) * 0.000006;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 105.457901) * 0.000004;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 105.457901) * 0.000004;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 107.457100) * 0.000003;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 107.457100) * 0.000003;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 109.456314) * 0.000002;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 109.456314) * 0.000002;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 111.455521) * 0.000002;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 111.455521) * 0.000002;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 113.454720) * 0.000001;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 113.454720) * 0.000001;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 115.453926) * 0.000001;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 115.453926) * 0.000001;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 117.453140) * 0.000001;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 117.453140) * 0.000001;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 119.452347) * 0.000000;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 119.452347) * 0.000000;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 121.451561) * 0.000000;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 121.451561) * 0.000000;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 123.450752) * 0.000000;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 123.450752) * 0.000000;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 125.449966) * 0.000000;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 125.449966) * 0.000000;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 127.449173) * 0.000000;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 127.449173) * 0.000000;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 129.448395) * 0.000000;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 129.448395) * 0.000000;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 131.447586) * 0.000000;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 131.447586) * 0.000000;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 133.446808) * 0.000000;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 133.446808) * 0.000000;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 135.446014) * 0.000000;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 135.446014) * 0.000000;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 137.445206) * 0.000000;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 137.445206) * 0.000000;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 139.444427) * 0.000000;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 139.444427) * 0.000000;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 141.443634) * 0.000000;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 141.443634) * 0.000000;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 143.442856) * 0.000000;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 143.442856) * 0.000000;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 145.442062) * 0.000000;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 145.442062) * 0.000000;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 147.441269) * 0.000000;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 147.441269) * 0.000000;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 149.440475) * 0.000000;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 149.440475) * 0.000000;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 151.439697) * 0.000000;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 151.439697) * 0.000000;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 153.438904) * 0.000000;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 153.438904) * 0.000000;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 155.438126) * 0.000000;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 155.438126) * 0.000000;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 157.437332) * 0.000000;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 157.437332) * 0.000000;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 159.436539) * 0.000000;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 159.436539) * 0.000000;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 161.435760) * 0.000000;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 161.435760) * 0.000000;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 163.434967) * 0.000000;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 163.434967) * 0.000000;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 165.434189) * 0.000000;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 165.434189) * 0.000000;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 167.433395) * 0.000000;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 167.433395) * 0.000000;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 169.432617) * 0.000000;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 169.432617) * 0.000000;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 171.431839) * 0.000000;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 171.431839) * 0.000000;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 173.431030) * 0.000000;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 173.431030) * 0.000000;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 175.430267) * 0.000000;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 175.430267) * 0.000000;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 177.429474) * 0.000000;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 177.429474) * 0.000000;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 179.428696) * 0.000000;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 179.428696) * 0.000000;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 181.427902) * 0.000000;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 181.427902) * 0.000000;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 183.427109) * 0.000000;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 183.427109) * 0.000000;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 185.426331) * 0.000000;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 185.426331) * 0.000000;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 187.425568) * 0.000000;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 187.425568) * 0.000000;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 189.424789) * 0.000000;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 189.424789) * 0.000000;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 191.424011) * 0.000000;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 191.424011) * 0.000000;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 193.423203) * 0.000000;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 193.423203) * 0.000000;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 195.422440) * 0.000000;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 195.422440) * 0.000000;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 197.421661) * 0.000000;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 197.421661) * 0.000000;
 sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * 199.420868) * 0.000000;
 sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * 199.420868) * 0.000000;
    gl_FragColor = sum;

  }
}

);


   std::string fragmentShaderForOptimizedBlurOfRadius(int blurRadius, double sigma)
   {
       if (blurRadius < 1)
       {
           return "";
       }

       // First, generate the normal Gaussian weights for a given sigma
       float *standardGaussianWeights = (float *)calloc(blurRadius + 1, sizeof(float));
       float sumOfWeights = 0.0;
       for (int currentGaussianWeightIndex = 0; currentGaussianWeightIndex < blurRadius + 1; currentGaussianWeightIndex++)
       {
           standardGaussianWeights[currentGaussianWeightIndex] = (1.0 / sqrt(2.0 * M_PI * pow(sigma, 2.0))) * exp(-pow(currentGaussianWeightIndex, 2.0) / (2.0 * pow(sigma, 2.0)));

           if (currentGaussianWeightIndex == 0)
           {
               sumOfWeights += standardGaussianWeights[currentGaussianWeightIndex];
           }
           else
           {
               sumOfWeights += 2.0 * standardGaussianWeights[currentGaussianWeightIndex];
           }
       }

       // Next, normalize these weights to prevent the clipping of the Gaussian curve at the end of the discrete samples from reducing luminance
       for (int currentGaussianWeightIndex = 0; currentGaussianWeightIndex < blurRadius + 1; currentGaussianWeightIndex++)
       {
           standardGaussianWeights[currentGaussianWeightIndex] = standardGaussianWeights[currentGaussianWeightIndex] / sumOfWeights;
       }

       // From these weights we calculate the offsets to read interpolated values from
       int numberOfOptimizedOffsets = blurRadius / 2 + (blurRadius % 2)< 20? blurRadius / 2 + (blurRadius % 2):20;
       int trueNumberOfOptimizedOffsets = blurRadius / 2 + (blurRadius % 2);

       std::string shaderString;

       // Header
       append_format(shaderString, "uniform sampler2D inputImageTexture;\n varying vec2 textureCoordinate;\n float texelWidthOffset=1.0/240.0;\n float texelHeightOffset = 1.0/240.0;\n void main()\n {\n vec4 sum = vec4(0.0);\n vec2 singleStepOffset = vec2(texelWidthOffset, texelHeightOffset);\n");

       // Inner texture loop
       append_format(shaderString,"sum += texture2D(inputImageTexture, textureCoordinate.xy) * %f;\n", standardGaussianWeights[0]);

       // From these weights we calculate the offsets to read interpolated values from
       float *optimizedGaussianOffsets = (float *) calloc(numberOfOptimizedOffsets, sizeof(float));

       for (int currentOptimizedOffset = 0; currentOptimizedOffset < numberOfOptimizedOffsets; currentOptimizedOffset++)
       {
           float firstWeight = standardGaussianWeights[currentOptimizedOffset*2 + 1];
           float secondWeight = standardGaussianWeights[currentOptimizedOffset*2 + 2];

           float optimizedWeight = firstWeight + secondWeight;

           optimizedGaussianOffsets[currentOptimizedOffset] = (firstWeight * (currentOptimizedOffset*2 + 1) + secondWeight * (currentOptimizedOffset*2 + 2)) / optimizedWeight;
       }


       for (int currentBlurCoordinateIndex = 0; currentBlurCoordinateIndex < numberOfOptimizedOffsets; currentBlurCoordinateIndex++)
       {
           float firstWeight = standardGaussianWeights[currentBlurCoordinateIndex * 2 + 1];
           float secondWeight = standardGaussianWeights[currentBlurCoordinateIndex * 2 + 2];
           float optimizedWeight = firstWeight + secondWeight;

           append_format(shaderString, "sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * %f) * %f;\n", (unsigned long)((currentBlurCoordinateIndex * 2) + 1), optimizedGaussianOffsets[currentBlurCoordinateIndex], optimizedWeight);
           append_format(shaderString, "sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * %f) * %f;\n", (unsigned long)((currentBlurCoordinateIndex * 2) + 2), optimizedGaussianOffsets[currentBlurCoordinateIndex], optimizedWeight);
       }

       // If the number of required samples exceeds the amount we can pass in via varyings, we have to do dependent texture reads in the fragment shader
       if (trueNumberOfOptimizedOffsets > numberOfOptimizedOffsets)
       {

           for (int currentOverlowTextureRead = numberOfOptimizedOffsets; currentOverlowTextureRead < trueNumberOfOptimizedOffsets; currentOverlowTextureRead++)
           {
               float firstWeight = standardGaussianWeights[currentOverlowTextureRead * 2 + 1];
               float secondWeight = standardGaussianWeights[currentOverlowTextureRead * 2 + 2];

               float optimizedWeight = firstWeight + secondWeight;
               float optimizedOffset = (firstWeight * (currentOverlowTextureRead * 2 + 1) + secondWeight * (currentOverlowTextureRead * 2 + 2)) / optimizedWeight;

               append_format(shaderString,"sum += texture2D(inputImageTexture, textureCoordinate.xy + singleStepOffset * %f) * %f;\n", optimizedOffset, optimizedWeight);
               append_format(shaderString,"sum += texture2D(inputImageTexture, textureCoordinate.xy - singleStepOffset * %f) * %f;\n", optimizedOffset, optimizedWeight);
           }
       }

       // Footer
       append_format(shaderString, "gl_FragColor = sum;\n}\n");

       free(standardGaussianWeights);
       return shaderString;
   }


namespace CGE
{
	CGEConstString CGEHeartFilter::paramName = "intensity";

	bool CGEHeartFilter::init()
	{
		if(initShadersFromString(g_vshDefaultWithoutTexCoord, s_fshSquare))
		{
			return true;
		}
		return false;
	}

	void CGEHeartFilter::setIntensity(float value)
	{
		m_program.bind();
        m_program.sendUniformf(paramName, m_intensity);
	}
    
    void CGEHeartFilter::render2Texture(CGEImageHandlerInterface* handler, GLuint srcTexture, GLuint vertexBufferID)
    {
        CGEImageFilterInterface::render2Texture(handler, srcTexture, vertexBufferID);
    }

	//////////////////////////////////////////////////////////////////////////


}