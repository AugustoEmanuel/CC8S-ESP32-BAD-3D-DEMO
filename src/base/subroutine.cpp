#include "subroutine.hpp"
#include <cstddef>

SUBCallbackFunction subCallback;

void SUB::clear()
{
    subCallback = NULL;
}

void SUB::setCallback(SUBCallbackFunction callback)
{
    subCallback = callback;
}

void SUB::run()
{
    if(!subCallback) return;
    subCallback();
}
