#ifndef SPRITE_H
#define SPRITE_H

#include <cstdint>
#include <esp_timer.h>

#define MS_TO_US(ms) ((ms) * 1000)

class SPR_Definition
{
public:
    SPR_Definition(const uint8_t *imgData, uint16_t w, uint16_t h, uint16_t f, uint8_t a)
        : image(imgData), width(w), height(h), framecount(f), animcount(a) {}

    // Getters for encapsulation
    const uint8_t* getImageData() const { return image; }
    uint16_t getWidth() const { return width; }
    uint16_t getHeight() const { return height; }
    uint16_t getFrameCount() const { return framecount; }
    uint8_t getAnimCount() const { return animcount; }

private:
    const uint8_t *image;
    uint16_t width;
    uint16_t height;
    uint16_t framecount;
    uint16_t animcount;
};

class SPR_Alloc
{
public:
    SPR_Alloc() {
        timer = 0;
        animation = 0;
        frame = 0;
        posX = 0;
        posY = 0;
        frameLimit = 0;
        animDuration = 0;
        active = false;
        visible = false;
    }
    SPR_Alloc(const SPR_Definition *def, uint16_t x, uint16_t y, uint16_t f, uint16_t a)
        : definition(def), posX(x), posY(y), frame(f), animDuration(a) {}

    friend class SPR;

private:
    const SPR_Definition *definition;
    uint64_t timer;
    uint16_t animation;
    uint16_t frame;
    uint16_t posX;
    uint16_t posY;
    uint16_t frameLimit;
    uint16_t animDuration;
    bool active;
    bool highPriority = false;
    bool visible = true;
};

class SPR
{
public:
    static void init();
    static void update();
    static int addSprite(const SPR_Definition *def, uint16_t posX, uint16_t posY);
    static void removeSprite(int& spritePtr);
    static void setAnimation(int& sprite, uint16_t _anim, uint16_t _duration);
    static void setAnimation(int& sprite, uint16_t _anim, uint16_t _duration, uint16_t _frame);
    static void setAnimation(int& sprite, uint16_t _anim, uint16_t _duration, uint16_t _frame, uint16_t _frameLimit);
    static void setVisible(int& sprite, bool visible);
    static void setPosX(int& sprite, uint16_t x);
    static void setPosY(int& sprite, uint16_t y);
    static void setPriority(int& sprite, bool highPriority);
    static void drawOnBuffer(uint8_t *buffer);
    static void enableSprites();
    static void disableSprites();
};

#endif