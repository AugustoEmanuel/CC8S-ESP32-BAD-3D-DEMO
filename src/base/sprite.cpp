#include "sprite.hpp"
#include <Arduino.h>
#include <bits/algorithmfwd.h>
#include <deque>

#define SPRITE_MAX_COUNT    256
SPR_Alloc * sprites = NULL;

SemaphoreHandle_t spriteMutex = NULL;
bool enabled = false;

void SPR::init(){
    if(!spriteMutex)
    spriteMutex = xSemaphoreCreateMutex();

    sprites = new SPR_Alloc[SPRITE_MAX_COUNT];
    enabled = true;

    // memcpy(sprites, 0, sizeof(SPR_Alloc) * SPRITE_MAX_COUNT);
}

void SPR::update()
{
    uint64_t timeNow = esp_timer_get_time();
    for(int i = 0; i < SPRITE_MAX_COUNT; i++){
        SPR_Alloc * sprite = &sprites[i];
        if(!sprite->visible || !sprite->active || !sprite->animDuration) continue;

        if(timeNow - sprite->animDuration > sprite->timer){
            sprite->frame = (sprite->frame + 1) % sprite->frameLimit;
            sprite->timer = esp_timer_get_time();
        }
    }
}

int SPR::addSprite(const SPR_Definition * def, uint16_t posX, uint16_t posY){
    if (xSemaphoreTake(spriteMutex, portMAX_DELAY)) {
        //Serial.println("LOCKED AT ADD SPRITE");
        for(int i = 0; i < SPRITE_MAX_COUNT; i++){
            if(sprites[i].active) continue;
            SPR_Alloc sprite = SPR_Alloc(def, posX, posY, 1, 0);
            sprite.frame = 0;
            sprite.animation = 0;
            sprite.animDuration = 0;
            sprite.frameLimit = def->getFrameCount();
            sprite.active = true;
            sprites[i] = sprite;

            xSemaphoreGive(spriteMutex);
            return i;
        }
        xSemaphoreGive(spriteMutex);
        return -1;
    }
}

void SPR::removeSprite(int& spriteIndexPtr)
{
    if (xSemaphoreTake(spriteMutex, portMAX_DELAY)) {
        //Serial.println("LOCKED AT REMOVE SPRITE");
        if(spriteIndexPtr == -1){
            xSemaphoreGive(spriteMutex);
            return;
        }

        sprites[spriteIndexPtr].active = false;
        spriteIndexPtr = -1;
        xSemaphoreGive(spriteMutex);
    }
}

void SPR::setAnimation(int& spriteIndex, uint16_t _anim, uint16_t _duration) { 
    setAnimation(spriteIndex, _anim, _duration, 0, (&sprites[spriteIndex])->definition->getFrameCount());
}

void SPR::setAnimation(int& spriteIndex, uint16_t _anim, uint16_t _duration, uint16_t _frame) { 
    if(spriteIndex == -1)return;
    setAnimation(spriteIndex, _anim, _duration, _frame, (&sprites[spriteIndex])->definition->getFrameCount());
}

void SPR::setAnimation(int& spriteIndex, uint16_t _anim, uint16_t _duration, uint16_t _frame, uint16_t _frameLimit) { 
    if (xSemaphoreTake(spriteMutex, portMAX_DELAY)) {
        //Serial.println("LOCKED AT ANIM SPRITE");
        if(spriteIndex == -1){
            xSemaphoreGive(spriteMutex);
            return;
        }

        SPR_Alloc * sprite = &sprites[spriteIndex];

        if(_anim >= sprite->definition->getAnimCount() || _frame >= sprite->definition->getFrameCount()) {
            xSemaphoreGive(spriteMutex);
            return;
        }
        sprite->timer = esp_timer_get_time();
        sprite->animation = _anim;
        sprite->animDuration = _duration;
        sprite->frame = _frame;
        sprite->frameLimit = _frameLimit;
        xSemaphoreGive(spriteMutex);
        //Serial.println("UNLOCKED AT ANIM SPRITE");
    }
}

void SPR::setVisible(int& spriteIndex, bool visible)
{
     if (xSemaphoreTake(spriteMutex, portMAX_DELAY)) {
        //Serial.println("LOCKED AT VIS SPRITE");
        if(spriteIndex == -1){
            xSemaphoreGive(spriteMutex);
            return;
        }
        SPR_Alloc * sprite = &sprites[spriteIndex];
        sprite->visible = visible;
        xSemaphoreGive(spriteMutex);
     }
}

void SPR::setPosX(int& spriteIndex, uint16_t x)
{
     if (xSemaphoreTake(spriteMutex, portMAX_DELAY)) {
        //Serial.println("LOCKED AT X SPRITE");
        if(spriteIndex == -1){
            xSemaphoreGive(spriteMutex);
            return;
        }
        SPR_Alloc * sprite = &sprites[spriteIndex];
        sprite->posX = x;
        xSemaphoreGive(spriteMutex);
     }
}

void SPR::setPosY(int& spriteIndex, uint16_t y)
{
    if (xSemaphoreTake(spriteMutex, portMAX_DELAY)) {
        //Serial.println("LOCKED AT Y SPRITE");
        if(spriteIndex == -1){
            xSemaphoreGive(spriteMutex);
            return;
        }
        SPR_Alloc * sprite = &sprites[spriteIndex];
        sprite->posY = y;
        xSemaphoreGive(spriteMutex);
     }
}

void SPR::setPriority(int &spriteIndex, bool highPriority)
{
    if (xSemaphoreTake(spriteMutex, portMAX_DELAY)) {
        //Serial.println("LOCKED AT Y SPRITE");
        if(spriteIndex == -1){
            xSemaphoreGive(spriteMutex);
            return;
        }
        SPR_Alloc * sprite = &sprites[spriteIndex];
        sprite->highPriority = highPriority;
        xSemaphoreGive(spriteMutex);
    }
}

void SPR::drawOnBuffer(uint8_t * buffer){
    if(!enabled) return;
    
    for(int h = 0; h < 2; h++){
        for(int i = 0; i < SPRITE_MAX_COUNT; i++){
            SPR_Alloc spr = sprites[i];
            if(!spr.visible || !spr.active || spr.highPriority != h) continue;
            uint16_t width = spr.definition->getWidth();
            uint16_t height = spr.definition->getHeight();
            uint16_t frame = spr.frame;
            uint16_t anim = spr.animation;

            const uint8_t * imageData = spr.definition->getImageData() + (width * height * (frame + (spr.definition->getFrameCount() * anim)));

            for(int y = 0; y < height; y++){
                uint16_t posY = spr.posY + y;
                if(posY >= 240) break;
                for(int x = 0; x < width; x++){
                    uint16_t posX = spr.posX + x;
                    if(posX >= 320) break;
                    uint8_t color = imageData[y * width + x];
                    if(color > 0xf) continue;

                    uint32_t bufferPos = posY * 192 + (posX/2);

                    if (posX % 2)
                    {
                        buffer[bufferPos] = (buffer[bufferPos] & 0xF0) | (color & 0x0F);
                    }
                    else
                    {
                        buffer[bufferPos] = (buffer[bufferPos] & 0x0F) | (color << 4);
                    }
                }
            }
        }
    }
}

void SPR::enableSprites()
{
    enabled = true;
}

void SPR::disableSprites()
{
    enabled = false;
}
