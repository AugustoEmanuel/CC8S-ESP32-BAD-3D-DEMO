#ifndef AMIGA_H
#define AMIGA_H

#include <cstdint>

class AMIGA{
public:
    static void init();
    static void playMusic(const int8_t *modPtr, uint32_t len);
    static void stopMusic();
    static void update();
};

#endif