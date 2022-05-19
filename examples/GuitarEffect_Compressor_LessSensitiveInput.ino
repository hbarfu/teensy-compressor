/*
Nice Presets:

Heart attack:
  compressor.setParam(AudioEffectCompressor::Param::P_ATTACK, 15.0f);
  compressor.setParam(AudioEffectCompressor::Param::P_RELEASE, 20.0f);
  compressor.setParam(AudioEffectCompressor::Param::P_THRESHOLD, -34.872808f);
  compressor.setParam(AudioEffectCompressor::Param::P_RATIO, 192.0f);
  compressor.setParam(AudioEffectCompressor::Param::P_KNEE, 12.0f);
  compressor.setParam(AudioEffectCompressor::Param::P_MAKEUP, 12.0f);
  compressor.setParam(AudioEffectCompressor::Param::P_BYPASS, 0.0f);

Flat like holland:
  compressor.setParam(AudioEffectCompressor::Param::P_ATTACK, 0.001f);
  compressor.setParam(AudioEffectCompressor::Param::P_RELEASE, 20.0f);
  compressor.setParam(AudioEffectCompressor::Param::P_THRESHOLD, -45.0f);
  compressor.setParam(AudioEffectCompressor::Param::P_RATIO, INFINITY);
  compressor.setParam(AudioEffectCompressor::Param::P_KNEE, 18.0f);
  compressor.setParam(AudioEffectCompressor::Param::P_MAKEUP, 12.0f);
  compressor.setParam(AudioEffectCompressor::Param::P_BYPASS, 0.0f);
*/

#include <Audio.h>
#include <effect_compressor.h>

AudioInputI2S           i2s_in;
AudioOutputI2S          i2s_out;
AudioControlSGTL5000     sgtl5000_1;

AudioEffectCompressor    compressor;

AudioConnection         patchCord1(i2s_in, 0, compressor, 0);
AudioConnection         patchCord2(compressor, 0, i2s_out, 0);
AudioConnection         patchCord3(compressor, 0, i2s_out, 1);

float bypass = 0;

void setup() {
  Serial.begin(9600);
  while (!Serial) ;
  AudioMemory(8);
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.5);
  sgtl5000_1.inputSelect(AUDIO_INPUT_LINEIN);
  sgtl5000_1.lineInLevel(5);
  //sgtl5000_1.micGain(36);
  compressor.setParam(AudioEffectCompressor::Param::P_ATTACK, 15.0f);
  compressor.setParam(AudioEffectCompressor::Param::P_RELEASE, 20.0f);
  compressor.setParam(AudioEffectCompressor::Param::P_THRESHOLD, -34.872808f);
  compressor.setParam(AudioEffectCompressor::Param::P_RATIO, 192.0f);
  compressor.setParam(AudioEffectCompressor::Param::P_KNEE, 12.0f);
  compressor.setParam(AudioEffectCompressor::Param::P_MAKEUP, 12.0f);
  compressor.setParam(AudioEffectCompressor::Param::P_BYPASS, 0.0f);
  delay(1000);
  Serial.println("Setup completed, starting routine");
}

void loop() {
  Serial.print("AudioMemory Usage = ");
  Serial.println(AudioMemoryUsageMax());
  Serial.print("Max CPU Usage = ");
  //Serial.print(AudioProcessorUsageMax(), 1);
  Serial.print(AudioProcessorUsage(), 1);
  Serial.println("%");
  Serial.print("Compressor attenuation: ");
  Serial.println(compressor.getAttenuation());
  /*bypass = 1.0f - bypass;
  compressor.setParam(AudioEffectCompressor::Param::P_BYPASS, bypass);
  Serial.print("Bypass: ");
  Serial.println(bypass);*/
  delay(30);
}
