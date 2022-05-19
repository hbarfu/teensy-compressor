#ifndef effect_compressor_h_
#define effect_compressor_h_

#include "Arduino.h"
#include "AudioStream.h"

/******************************************************************/

//                A u d i o E f f e c t C o m p r e s s o r
// Written by Hannes Barfuss Apr 2022

class AudioEffectCompressor :
public AudioStream
{
public:
    enum Param
    {
        P_ATTACK, //0
        P_RELEASE, //1
        P_THRESHOLD, //2
        P_RATIO, //3
        P_KNEE, //4
        P_MAKEUP, //5
        P_LOOKAHEAD, //6
        P_LIMIT, //7
        P_BYPASS, //8
        P_SIDECHAIN, //9
        P_N
    };
  AudioEffectCompressor(void):
  AudioStream(1,inputQueueArray)
  {
      params[Param::P_ATTACK] = 0;
      params[Param::P_RELEASE] = 0;
      params[Param::P_THRESHOLD] = 0;
      params[Param::P_RATIO] = 0;
      params[Param::P_KNEE] = 0;
      params[Param::P_MAKEUP] = 0;
      params[Param::P_LOOKAHEAD] = 0;
      params[Param::P_LIMIT] = 0;
      params[Param::P_BYPASS] = 0;
      
      y1_prev = 0;
      yL_prev = 0;
      aA = 0;
      aR = 0;
      yL = 0;
      
      Serial.println("New comrpessor instance created");
  }
  virtual void update(void);
    void setParam(Param param, float value);
    float getAttenuation();
  
private:
  audio_block_t *inputQueueArray[1];
    float params[10];
    float aA;
    float aR;
    float y1_prev;
    float yL_prev;
    float yL; //yL is the attenuation in dB
    float attenuation; //the largest attenuation over the past buffer

    //convenience vars
    float ONE_OVER_RATIO_MINUS_ONE;
    float RATIO_RECIPROCAL;
    float KNEE_HALVED;
    float KNEE_DOUBLED_RECIPROCAL;

    //DC removal
    float w0 = 0;
    float w1 = 1;
    
    float coeff(float time);
    float lin2db(float lin);
    float db2lin(float db);
    float int16_to_float32(int16_t val);
    int16_t float32_to_int16(float val);
};

#endif
