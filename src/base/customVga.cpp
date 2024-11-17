#include "customVga.hpp"
#include "../base/sprite.hpp"

// NÃO RAM HAHAHAHAHHAHA NÃO TEM RAM SOCORRO
static uint8_t screenbuffer[240][160];
static uint8_t *framebuffer = NULL;
static int offsetX = 0;
static int offsetY = 0;

#include <stdint.h>

void CustomVga::setOffsetX(int _offsetX){
    offsetX = _offsetX;
}

void CustomVga::setOffsetY(int _offsetY){
    offsetY = _offsetY;
}

uint8_t *CustomVga::getFramebufferPtr()
{
    return framebuffer;
}

// custominterrupt just copied from VGA14Bit except the initial and end lines
void IRAM_ATTR CustomVga::custominterrupt(void *arg)
{
    CustomVga *staticthis = (CustomVga *)arg;

    // obtain currently rendered line from the buffer just read, based on the conventioned ordering and buffers per line
    staticthis->currentLine = staticthis->dmaBufferDescriptorActive >> ((staticthis->descriptorsPerLine == 2) ? 1 : 0);

    // in the case of two buffers per line,
    // render only when the sync half of the line ended (longer period until next interrupt)
    // else exit early
    // This might need to be revised, because it might be better to overlap and miss the second interrupt
    if ((staticthis->descriptorsPerLine == 2) && ((staticthis->dmaBufferDescriptorActive & 1) != 0))
        return;

    // TO DO: This should be precalculated outside the interrupt
    int vInactiveLinesCount = staticthis->mode.vFront + staticthis->mode.vSync + staticthis->mode.vBack;

    // render ahead (the lenght of buffered lines)
    int renderLine = (staticthis->currentLine + staticthis->lineBufferCount) % staticthis->totalLines;

    if (renderLine >= vInactiveLinesCount)
    {
        int renderActiveLine = renderLine - vInactiveLinesCount;
        uint8_t *activeRenderingBuffer = ((uint8_t *)
                                              staticthis->dmaBufferDescriptors[staticthis->indexRendererDataBuffer[0] + renderActiveLine * staticthis->descriptorsPerLine + staticthis->descriptorsPerLine - 1]
                                                  .buffer() +
                                          staticthis->dataOffsetInLineInBytes);

        int y = renderActiveLine / staticthis->mode.vDiv;
        if (y >= 0 && y < staticthis->yres)
            staticthis->custominterruptPixelLine(y, activeRenderingBuffer, arg);
    }

    if (renderLine == 0)
        staticthis->vSyncPassed = true;
}

uint16_t colors[] = {
0x0,
0x3fff,
0xd11,
0x32ef,
0x2911,
0x12ad,
0x28c7,
0x1f7a,
0x972,
0x10b,
0x19d7,
0x154a,
0x2210,
0x23b5,
0x35cf,
0x2ab5,
};

// CARALIO A DOC ME TROLLOU, OLHA O GO HORSE
void IRAM_ATTR CustomVga::custominterruptPixelLine(int y, uint8_t *pixels, void *arg)
{
	CustomVga * staticthis = (CustomVga *)arg;
	unsigned long syncBits = (staticthis->hsyncBitI | staticthis->vsyncBitI) * staticthis->rendererStaticReplicate32mask;

    y = y - offsetY;
    if(y < 0 || y >= 240){
        std::fill((uint32_t *)pixels, (uint32_t *)pixels + (staticthis->mode.hRes / 2), syncBits);
        return;
    }

	uint8_t * line = screenbuffer[y];

    int j = 0;
    int indexed = offsetX;
    uint64_t p0, p1, p2, p3, p4, p5, p6, p7;

	for (int i = 0; i < staticthis->mode.hRes / 2; i+=4)
	{
		//SPEED CODE, SIM! ISSO FAZ DIFERENÇA, SE EU QUISSESE PROGRAMAR LENTO EU USUÁRIA PYTHON. AIIIIN MAS É FEIO, P N S C.
        p0 = p1 = p2 = p3 = p4 = p5 = p6 = p7 = 0;

        p0 = colors[(line[j] >> 0) & 0xf];
        p1 = colors[(line[j++] >> 4) & 0xf];
        p2 = colors[(line[j] >> 0) & 0xf];
        p3 = colors[(line[j++] >> 4) & 0xf];
        p4 = colors[(line[j] >> 0) & 0xf];
        p5 = colors[(line[j++] >> 4) & 0xf];
        p6 = colors[(line[j] >> 0) & 0xf];
        p7 = colors[(line[j++] >> 4) & 0xf];

        ((uint32_t *)pixels)[i]     = syncBits | (p0 & 0x3fff) | ((p1 & 0x3fff) << 16);
        ((uint32_t *)pixels)[i + 1] = syncBits | (p2 & 0x3fff) | ((p3 & 0x3fff) << 16);
        ((uint32_t *)pixels)[i + 2] = syncBits | (p4 & 0x3fff) | ((p5 & 0x3fff) << 16);
        ((uint32_t *)pixels)[i + 3] = syncBits | (p6 & 0x3fff) | ((p7 & 0x3fff) << 16);

        indexed += 8;
	}
}

void CustomVga::propagateResolution(const int xres, const int yres)
{
    setResolution(xres, yres);
    interruptStaticChild = &CustomVga::custominterrupt;

    Serial.println("TRYING TO ALLOCATED...");

    if(framebuffer) free(framebuffer);
    framebuffer = (uint8_t *)malloc(160 * 240 * sizeof(uint8_t));;
    if (framebuffer == NULL) {
        Serial.println("FAILED TO ALLOC FRAMEBUFFER");
        return;
    }

    Serial.println("FRAME BUFFER ALLOCATED");
}

void CustomVga::setPixel(uint16_t x, uint16_t y, uint8_t colorIndex)
{
    if (x >= 320 || y >= 240)
        return;


    // Calculate the byte index for the pixel
    int byteOffset = x % 2; // Determine if it's the high or low nibble
    int pos = y * 160 + (x / 2);

    // Set the pixel in the appropriate nibble
    if (byteOffset == 0)
    {
        // Set the high nibble
        framebuffer[pos] = (framebuffer[pos] & 0x0F) | (colorIndex << 4);
    }
    else
    {
        // Set the low nibble
        framebuffer[pos] = (framebuffer[pos] & 0xF0) | (colorIndex & 0x0F);
    }
}

void CustomVga::updateScreenBuffer()
{
    // WAIT FOR END SCREEN DRAWING
    // waitVsync();
    memcpy(screenbuffer, framebuffer, sizeof(screenbuffer));

    //NEED CALL SPRITE ENGINE RIGHT HERE!
    SPR::drawOnBuffer(*screenbuffer);
    //THIS MUST BE FAST AS F0CK!
}

void CustomVga::clearFrameBuffer()
{
    for (int x = 0; x < 160; x++)
    {
        for (int y = 0; y < 240; y++)
        {
            framebuffer[y * 160 + x] = 0;
        }
    }
}

void CustomVga::waitVsync()
{
    vSyncPassed = false;
    while (!vSyncPassed)
    {
    }
}