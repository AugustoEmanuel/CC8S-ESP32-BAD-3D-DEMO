#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <cstdint>
#include <string>
#include <map>

typedef void (*GFXCallbackFunction)();

class GFX_Image
{
public:
    GFX_Image(const uint8_t *imgData, uint16_t w, uint16_t h)
        : image(imgData), width(w), height(h) {}

    // Getters for encapsulation
    const uint8_t* getImageData() const { return image; }
    uint16_t getWidth() const { return width; }
    uint16_t getHeight() const { return height; }

private:
    const uint8_t *image;
    uint16_t width;
    uint16_t height;
};

class GFX_Image_Info
{
public:
    GFX_Image_Info(const GFX_Image * imagePtr, uint16_t posX, u_int16_t posY)
        : image(imagePtr), x(x), y(posY) {}

    // Getters for encapsulation
    const GFX_Image * getImage() const { return image; }
    uint16_t getX() const { return x;}
    uint16_t getY() const { return y;}

private:
    const GFX_Image * image;
    uint16_t x;
    uint16_t y;
};

class GFX_Font
{
public:
    GFX_Font(const uint8_t** imgData)
        : images(imgData) {}


    const uint8_t** getImageData() const { return images; }
private:
    const uint8_t** images;
};

class GFX
{
public:
    // Initialize the graphics
    static void init();

    static void fillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t color);

    // Write text at (x, y) position
    static void writeText(const std::string &text, int x, int y);

    // Clear the framebuffer
    static void clearFrameBuffer();

    // Draw an image at (x, y) position
    static void drawImage(const GFX_Image &image, uint16_t x, uint16_t y);

    //Execute DrawRotines
    static void executeRoutines();

    //Update screen with framebuffer
    static void updateScreen();

    //Set drawing callback
    static void setDrawCallback(GFXCallbackFunction callback);

    //Set font palette index
    static void setFontColor(u_int8_t cIndex);

    static void drawImageTile(const GFX_Image &image, uint16_t x, uint16_t y, uint16_t ts, uint16_t tile);

    static void setScreenX(int x);
    static void setScreenY(int y);
    static void setPixel(uint16_t x, uint16_t y, uint8_t color);
    static void setFont(const GFX_Font * _fontPtr);
    static uint8_t* getFramebufferPtr();
};

#endif // GRAPHICS_H