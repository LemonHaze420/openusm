#include "vector2di.h"

#include "vector2d.h"

vector2di::vector2di(int a1, int a2)
{
    this->x = a1;
    this->y = a2;
}

vector2di::vector2di(const vector2d &a2)
{
    this->x = (int)a2[0];
    this->y = (int)a2[1];
}
