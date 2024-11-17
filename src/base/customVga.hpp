#ifndef CUSTOM_VGA_H
#define CUSTOM_VGA_H

#define FRAMEBUFFER_COUNT   2

#include <ESP32Video.h>
#include "soc/i2s_reg.h"
#include <math.h>

class CustomVga : public VGA14BitI
{   
public:
	//override this routine that is called during init to link the custom interrupt
	void propagateResolution(const int xres, const int yres);
    void setPixel(u_int16_t x, u_int16_t y, uint8_t colorIndex);
    void updateScreenBuffer();
    void clearFrameBuffer();
	static void setOffsetX(int _offsetX);
    static void setOffsetY(int _offsetY);
    static uint8_t * getFramebufferPtr();

    void waitVsync();

protected:
    //override frame buffer allocation
	virtual Color **allocateFrameBuffer()
	{
		return 0;
	}
    static void custominterrupt(void *arg);
    static void custominterruptPixelLine(int y, uint8_t *pixels, void *arg);
};

#endif