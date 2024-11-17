#ifndef N64_CONTROLLER_H
#define N64_CONTROLLER_H
#include <cstdint>

typedef struct N64C_D
{
    struct
    {
        unsigned A : 1;
        unsigned B : 1;
        unsigned Z : 1;
        unsigned start : 1;
        unsigned up : 1;
        unsigned down : 1;
        unsigned left : 1;
        unsigned right : 1;
        unsigned : 2;
        unsigned L : 1;
        unsigned R : 1;
        unsigned C_up : 1;
        unsigned C_down : 1;
        unsigned C_left : 1;
        unsigned C_right : 1;
        signed x : 8;
        signed y : 8;
    };

    bool operator==(const N64C_D& other) const {
        return A == other.A &&
               B == other.B &&
               Z == other.Z &&
               start == other.start &&
               up == other.up &&
               down == other.down &&
               left == other.left &&
               right == other.right &&
               L == other.L &&
               R == other.R &&
               C_up == other.C_up &&
               C_down == other.C_down &&
               C_left == other.C_left &&
               C_right == other.C_right &&
               x == other.x &&
               y == other.y;
    }
} N64C_D;

typedef void (*N64CCallbackFunction)(N64C_D actualState, N64C_D changed);

class N64C{
public:
    static void init();
    static void updateInputBuffer();
    static void update();
    static void setControllerCallback(N64CCallbackFunction callback);
    static void checkForFactoryStart();
};

#endif