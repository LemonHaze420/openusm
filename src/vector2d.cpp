#include "vector2d.h"

#ifdef USE_GLM
#include <glm/gtx/norm.hpp>
#endif

#include <cmath>
#include <cstdio>

const char *vector2d::to_string() const
{
    static char str[40];

    sprintf(str, "vector2d{%.2f, %.2f}", this->x, this->y);
    return str;
}

float vector2d::length2() const
{
#ifndef USE_GLM
    return (this->x * this->x + this->y * this->y);
#else
    return glm::length2(glm::vec2{this->x, this->y});
#endif
}

float vector2d::length() const
{
    return std::sqrt(this->length2());
}

vector2d vector2d::operator+(const vector2d &a1) const
{
    return vector2d{this->x + a1[0], this->y + a1[1]};
}

vector2d vector2d::operator-(const vector2d &a3) const
{
    return {this->x - a3[0], this->y - a3[1]};
}

vector2d vector2d::operator*(float a1) const
{
    return vector2d{this->x * a1, this->y * a1};
}

vector2d &vector2d::operator*=(float a2)
{
    this->x *= a2;
    this->y *= a2;
    return (*this);
}

vector2d &vector2d::operator/=(float a2)
{
    this->x *= (1.0 / a2);
    this->y *= (1.0 / a2);
    return (*this);
}
