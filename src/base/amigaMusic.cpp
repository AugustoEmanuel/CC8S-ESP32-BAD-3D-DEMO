#include "amigaMusic.hpp"
#include <AudioOutputI2S.h>
#include <AudioFileSourcePROGMEM.h>
#include <AudioGeneratorMOD.h>
static AudioGeneratorMOD *mod;
static AudioFileSourcePROGMEM *file;
static AudioOutputI2S *out;

SemaphoreHandle_t amigaMutex = NULL;

void AMIGA::init(){
    out = new AudioOutputI2S(0, 1);
    mod = new AudioGeneratorMOD();
    mod->SetBufferSize(3*1024);
    mod->SetSampleRate(44100);
    mod->SetStereoSeparation(64);
    amigaMutex = xSemaphoreCreateMutex();
}

void AMIGA::playMusic(const int8_t * modPtr, uint32_t len) {
    if (xSemaphoreTake(amigaMutex, portMAX_DELAY)) {
      AudioFileSourcePROGMEM *file = new AudioFileSourcePROGMEM( modPtr, len );
      mod->begin(file, out);
      xSemaphoreGive(amigaMutex);
    }
}

void AMIGA::stopMusic() {
  if (xSemaphoreTake(amigaMutex, portMAX_DELAY)) {
    if (!mod->isRunning()) return;
    mod->stop();
    xSemaphoreGive(amigaMutex);
  }
}


void AMIGA::update()
{
  if (xSemaphoreTake(amigaMutex, portMAX_DELAY)) {
    if (mod->isRunning()) {
      if (!mod->loop()) mod->stop();
    }
    xSemaphoreGive(amigaMutex);
  }
}