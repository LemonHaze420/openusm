#pragma once

#include "color32.h"
#include "variable.h"

#include <ngl_dx_scene.h>

struct nglSceneParamType {
    int field_0;

    operator int() {
        return field_0;
    }
};

struct nglParam {
    uint32_t field_0;
    uint32_t field_4;
};

extern Var<nglParam> EmptyParam;

struct nglShaderParamSet_Pool {
    static Var<int> NextID;
};

struct nglSceneParamSet_Pool {
    static Var<int> NextID;
};

struct USSectionIFLParam {
    struct {
        uint32_t NSections;
        uint32_t CurrentSection;
        char *field_8;
    } * field_0;

    static inline Var<int> ID{0x00956788};
};

struct USDamageFrameParam {
    int field_0;

    static inline Var<int> ID{0x00956784};
};

struct nglTextureFrameParam {
    int field_0;

    static inline Var<int> ID{0x00971EE8};
};

struct nglMaterialBase;

struct USMMaterialListParam {
    nglMaterialBase **field_0;

    static inline Var<int> ID{0x009566D8};
};

struct USMMaterialIndicesParam {
    uint8_t *field_0;

    static inline Var<int> ID{0x00956730};
};

template<typename T>
struct nglParamSet
{
    nglParam *field_0;

    bool IsSet(int id) {
        return ((1ull << id) & *bit_cast<uint64_t *>(&this->field_0->field_0)) == 1ull << id;
    }

public:
    void set_color(color32 a2);

    nglParamSet() = default;

    nglParamSet(int a2) {
        if (a2 != 0) {
            if (a2 == 1) {
                auto v2 = 4 * T::NextID() + 8;
                this->field_0 = static_cast<decltype(field_0)>(nglListAlloc(v2, 8));

                field_0->field_0 = 0;
                field_0->field_4 = 0;
            }
        } else {
            this->field_0 = &EmptyParam();
        }
    }

    void operator=(const nglParamSet &params) {
        memcpy(this->field_0, params.field_0, 4 * T::NextID() + 8);
    }

    template<typename Param>
    bool IsSetParam() {
        return this->IsSet(Param::ID());
    }

    void *Get(int id) {
        struct {
            int field_0;
            int field_4;
            struct {
                int field_0;
            } field_8[1];
        } *temp = CAST(temp, this->field_0);

        return &temp->field_8[id];
    }

    void Set(int id, void *a2) {
        auto *v4 = this->field_0;
        auto v3 = v4->field_4;
        v4->field_0 |= 1 << id;
        v4->field_4 = v3;
        struct {
            void *field_0;
        } *param = static_cast<decltype(param)>(Get(id));
        param->field_0 = a2;
    }

    template<typename Param>
    void SetParam(const Param &param) {
        this->Set(Param::ID(), (void *) param.field_0);
    }

    template<typename Param>
    Param *Get() {
        assert(IsSet(Param::ID()) && "Parameter not set.");

        return static_cast<Param *>(this->Get(Param::ID()));
    }

    template<typename Param>
    Param *GetOrDefault(Param &a1) {
        if (this->IsSet(Param::ID())) {
            return static_cast<Param *>(this->Get(Param::ID()));
        }

        return &a1;
    }
};

extern nglMaterialBase *sub_8EA2E0(nglParamSet<nglShaderParamSet_Pool> *a1,
                                   nglMaterialBase *DefaultMaterial);
