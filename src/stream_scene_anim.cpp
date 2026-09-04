#include "stream_scene_anim.h"

#include "common.h"
#include "func_wrapper.h"
#include "tl_system.h"
#include "trace.h"

VALIDATE_SIZE(nalStreamInstance, 0x7Cu);
VALIDATE_OFFSET(nalStreamInstance, m_callback, 0x20);

void *nalStreamInstance::operator new(size_t size)
{
    return tlMemAlloc(size, 8u, 0);
}

void nalStreamInstance::operator delete(void *ptr, size_t)
{
    tlMemFree(ptr);
}

bool nalStreamInstance::IsReady() const
{
    //sp_log("nalStreamInstance::IsReady(): %d", this->field_1C);

    return this->field_1C == 5;
}

nalStreamInstance::~nalStreamInstance()
{
    THISCALL(0x004AD4C0, this);
}

void nalStreamInstance::AdvanceStream()
{
    THISCALL(0x00492EF0, this);

    //sp_log("nalStreamInstance::AdvanceStream(): %d", v2);
}

bool nalStreamInstance::Advance(Float dt)
{
#if 1
    sp_log("%d", this->field_1C);
#endif

    bool result = (bool)THISCALL(0x00498580, this, dt);

    //sp_log("nalStreamInstance::Advance(): post %d", this->field_1C);

    return result;
}

nalStreamInstance *create_stream_instance(uint32_t a1, uint32_t a2, uint32_t a3, [[maybe_unused]] int a4,
                                          nalClientSceneAnim *(*p_cb)(const tlFixedString &, void *), void *a6)
{
    TRACE("create_stream_instance()");

    if constexpr (1) {
        nalStreamInstance *result = new nalStreamInstance{};

        result->m_callback = p_cb;
        result->field_24 = a6;
        result->field_28.field_0 = a1;
        result->field_4 = nullptr;
        result->field_8 = nullptr;
        result->field_C = 0;
        result->field_10 = 0.0;
        result->field_14 = 0.0;
        result->field_18 = nullptr;
        result->field_1C = 0;
        result->BufferSize = 0;
        result->field_78 = 0;
        result->field_79 = 0;
        result->m_vtbl = 0x00880A88;
        result->field_5C = a2;
        result->field_70 = a3;

        return result;
    } else {
        return (nalStreamInstance *)CDECL_CALL(0x00492E70, a1, a2, a3, a4, p_cb, a6);
    }
}
