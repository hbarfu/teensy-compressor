//Optimization diary (Teensy 3.6):
//Without optimizations: 25%
//with optimized pow10f and lin2db: 10%

//https://github.com/PaulStoffregen/Audio/blob/master/utility/dspinst.h

/* The definition of audio_block_t is in AudioStream.h:
 
 typedef struct audio_block_struct {
     uint8_t  ref_count;
     uint8_t  reserved1;
     uint16_t memory_pool_index;
     int16_t  data[AUDIO_BLOCK_SAMPLES];
 } audio_block_t;
 
=> 16bit PCM signed int -> [-32768...32767] or [-2^15...2^15-1] = q15 format
  
In https://github.com/PaulStoffregen/Audio/blob/1726bf554541b602b2323b17e210b57ff6026e7e/filter_biquad.h
 they use the conversion:
    coef[0] = coefficients[0] * 1073741824.0; 
 to convert from double to int?! (1073741824 is 2^30)
 
 */

#include <Arduino.h>
#include <cmath>
#include "effect_compressor.h"

#define one_minus_oneOverE 0.6321205588285576784044762298
#define Q15_MAX_RECIPROCAL 0.00003051757f // 1 / 2^15
#define Q15_MAX 32768 //2^15

// Accelerate the powf(10.0,x) function (from Chip's single slope compressor)
float pow10f(float x) {
    return expf(2.30258509f*x);
 }

//Fast version of 20 * log10f(lin)
/* See https://github.com/Tympan/Tympan_Library/blob/master/src/AudioCalcGainWDRC_F32.h
  * Dr Paul Beckmann
  * https://community.arm.com/tools/f/discussions/4292/cmsis-dsp-new-functionality-proposal/22621#22621*/
float AudioEffectCompressor::lin2db(float lin)
{
    float Y, F;
     int E;
     F = frexpf(lin, &E);
     Y = 1.23149591;   //C[0]
     Y *= F;
     Y += -4.11852516f; //C[1]
     Y *= F;
     Y += 6.02197014f;  //C[2]
     Y *= F;
     Y += -3.13396450f; //C[3]
     Y += E;
     return 6.020599f * Y;
}

float AudioEffectCompressor::db2lin(float db)
{
    return pow10f(db*0.05f); //Strangely, the compiler does not optimize dB/20 to dB*0.05, so do it manually
}

//Needs more optimization
float AudioEffectCompressor::int16_to_float32(int16_t val)
{
    return val * Q15_MAX_RECIPROCAL; //fast version of val / 32768.0f. Strangely, the compiler does not optimize.
}

//Needs optimization
int16_t AudioEffectCompressor::float32_to_int16(float val)
{
    return (int16_t) (val * 32768);
}

float AudioEffectCompressor::coeff(float time)
{
    float smpls = (time * 0.001f) * AUDIO_SAMPLE_RATE_EXACT;
    return powf((1-one_minus_oneOverE), (1.0f/smpls));
}

void AudioEffectCompressor::setParam(Param param, float value)
{
    __disable_irq();
    switch (param) {
        case Param::P_ATTACK:
            if(value != params[param])
                aA = coeff(value);
            break;
        case Param::P_RELEASE:
            if(value != params[param])
                aR = coeff(value);
            break;
        case Param::P_RATIO:
            if(value != params[param])
                {
                    ONE_OVER_RATIO_MINUS_ONE = 1 / value - 1;
                    RATIO_RECIPROCAL = 1 / value;
                }
            break;
        case Param::P_KNEE:
            if(value != params[param])
            {
                KNEE_HALVED = value / 2;
                KNEE_DOUBLED_RECIPROCAL = 1 / (value * 2);
            }
            break;
        default:
            break;
    }
    
    params[param] = value;
    __enable_irq();
}

float AudioEffectCompressor::getAttenuation()
{
    __disable_irq();
    //return -yL;
    return attenuation;
    __enable_irq();
}

void AudioEffectCompressor::update(void)
{
  audio_block_t *block;

  if(params[P_BYPASS] > 0)
  {
    block = receiveReadOnly();
    if (!block) return;
    transmit(block);
    release(block);
    return;
  }

  short *bp;

    float x, xG, xL, y1, yG, tmp, c, threshold = params[P_THRESHOLD], knee = params[P_KNEE], makeup = params[P_MAKEUP];
    
    float att_mom = -192; //Only for analyzing/debugging

  block = receiveWritable(0);
  if(block) {
    bp = block->data;
    for(int i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {
        x = int16_to_float32(*bp);

        //Simple filter to remove DC.
        w0 = x + 0.99f * w1;
        x = w0 - w1;
        w1 = w0;

        //Take magnitude in decibel:
        xG = fabsf(x);
        xG = lin2db(xG);
        att_mom += xG; //Only for analyzing/debugging

        //Apply threshold:
        tmp = 2*(xG - threshold);
        if(tmp < -(knee)) yG = xG; //Signal is below knee range => no attenuation
        else if ((tmp > knee)) yG = threshold + (xG - threshold) * RATIO_RECIPROCAL; //signal is above knee range => full ateenuation
        else yG = xG + ONE_OVER_RATIO_MINUS_ONE * powf(xG - threshold + KNEE_HALVED, 2) * KNEE_DOUBLED_RECIPROCAL; //signal is in knee range => smooth attenuation
        xL = xG - yG;

        //Apply gain smoothing filter:
        y1 = fmaxf(xL, aR * y1_prev + (1 - aR) * xL);
        yL = aA * yL_prev + (1.0f - aA) * y1;
        //if(-yL < att_mom) att_mom = -yL;

        //Compute final attenuation and apply to output signal:
        c = -yL + makeup;
        c = x * db2lin(c);
        *bp++ = float32_to_int16(c);

        //Save filter history:
        y1_prev = y1;
        yL_prev = yL;
    }

    attenuation = att_mom / AUDIO_BLOCK_SAMPLES; //Only for analyzing/debugging

    transmit(block,0);
    release(block);
  }
}
