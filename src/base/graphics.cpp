#include "graphics.hpp"
#include <vector>
#include "customVga.hpp"
#include "res/systemFont.hpp"

const int redPin[] = {2,4,5,12,13};
const int greenPin[] = {14,15,16,17,18};
const int bluePin[] = {19,23,27,32};
const int hsyncPin = 3;
const int vsyncPin = 33;

const char textOrder[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '=', '%', '&', '*', '_', '>', '?', '!', 'A', 'B',
							'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W',
							'X', 'Y', 'Z', '<', '-', '.', ':', '!', '\'', ',', ' ', '@', '#'};

GFXCallbackFunction drawCallback;
CustomVga vga;

uint8_t fontColor;
const GFX_Font * fontPtr;

void GFX::init() {
    fontColor = 0x1;
    fontPtr = &sys_font;
    drawCallback = nullptr;
    bool vgab = vga.init(vga.MODE320x240, redPin, greenPin, bluePin, hsyncPin, vsyncPin);
    vga.clearFrameBuffer();
}



static void drawChar(char chr, uint16_t x, uint16_t y) {
	uint16_t offset = 0;
	for (uint16_t c = 0; c < sizeof(textOrder); c++) {
		if (textOrder[c] == chr) {
			offset = c;
			break;
		}
	}

    const uint8_t * image = fontPtr->getImageData()[offset];
	
    for(uint16_t i = 0; i < 8; i++){
        for(uint16_t j = 0; j < 8; j++){
            uint8_t color = image[j * 8 + i];
            if(!color) continue;
            vga.setPixel(i + x, j + y, fontColor); //GO HORSE MEGA SKILLS TRANSPARENCIAS
        }
    }
}

void GFX::fillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t color) {
    for(uint16_t i = 0; i < w; i++){
        for(uint16_t j = 0; j < h; j++){
            vga.setPixel(i + x, j + y, color);
        }
    }
}

void GFX::writeText(const std::string &text, int x, int y) {
    for(int i = 0; i < text.size(); i++){
        drawChar(text[i], x, y);
        x += 8;
    }
}

void GFX::clearFrameBuffer() {
    for(uint16_t i = 0; i < vga.xres; i++){
        for(uint16_t j = 0; j < vga.yres; j++){
            vga.setPixel(i, j, 0);
        }
    }
}

void GFX::drawImage(const GFX_Image &image, uint16_t x, uint16_t y){
    for(int i = 0; i < image.getWidth(); i++){
         for(int j = 0; j < image.getHeight(); j++){
            vga.setPixel(x+i, y+j, image.getImageData()[j * image.getWidth() + i]);
        }
    }
}

void GFX::setDrawCallback(GFXCallbackFunction callback){
    drawCallback = callback;
}

void GFX::setFontColor(u_int8_t cIndex)
{
    fontColor = cIndex;
}

void GFX::drawImageTile(const GFX_Image &image, uint16_t x, uint16_t y, uint16_t ts, uint16_t tile)
{

    int tilesPerRow = image.getWidth() / ts;
    int size = image.getWidth();
    
    int row = tile / tilesPerRow;
    int column = tile % tilesPerRow;

    int startIndex = (row * ts * size) + (column * ts);

    for(int i = 0; i < ts; i++){
         for(int j = 0; j < ts; j++){
            vga.setPixel(x+i, y+j, image.getImageData()[startIndex + (j * size) + i]);
        }
    }
}

void GFX::setScreenX(int x)
{
    vga.setOffsetX(x);
}

void GFX::setScreenY(int y)
{
    vga.setOffsetY(y);
}

void GFX::setPixel(uint16_t x, uint16_t y, uint8_t color)
{
    vga.setPixel(x, y, color);
}

void GFX::setFont(const GFX_Font *_fontPtr)
{
    fontPtr = _fontPtr;
}

uint8_t *GFX::getFramebufferPtr()
{
    return vga.getFramebufferPtr();
}

void GFX::executeRoutines(){
    if(drawCallback != nullptr){
        drawCallback();
    }
}

void GFX::updateScreen(){
    vga.updateScreenBuffer();
}