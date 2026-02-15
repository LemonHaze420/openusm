#pragma once

#include "nal_skeleton.h"

namespace nalCam {

    struct nalCamSkeleton;

    struct nalCamPose {
        nalCamSkeleton *field_0;
        char field_4[0x2C];
    };

    struct nalCamSkeleton : nalBaseSkeleton {
        int field_5C;
        nalCamPose field_60;

        nalCamSkeleton();

        //virtual
        void _Process();

        //virtual
        void _Release();

        //virtual
        bool _CheckVersion();

        static int & vtbl_ptr;
    };

} // namespace nalCam

