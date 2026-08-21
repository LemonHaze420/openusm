#include "capsule.h"

capsule::capsule(const vector3d &a2, const vector3d &a3, Float a4) : base(a2), end(a3), radius(std::abs(a4)) {}
