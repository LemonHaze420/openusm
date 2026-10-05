#include "subdivision_static_region_list.h"

#include "common.h"
#include "func_wrapper.h"
#include "proximity_map.h"
#include "region.h"
#include "scratchpad_stack.h"
#include "stack_allocator.h"
#include "subdivision_obb.h"
#include "trace.h"
#include "utility.h"
#include "variables.h"

VALIDATE_SIZE(static_region_list_methods, 0x4);

VALIDATE_SIZE(region_mirror_data, 0x1C);
VALIDATE_SIZE(static_region_list_node, 0x2);

void static_region_list_methods::init()
{
    TRACE("static_region_list_methods::init");

    if constexpr (1) {
        scratchpad() = static_cast<int *>(scratchpad_stack::alloc(0x40));
        for (int i = 0; i < 16; ++i) {
            scratchpad()[i] = 0;
        }
    } else {
        CDECL_CALL(0x00513F90);
    }
}

void static_region_list_methods::term()
{
    if constexpr (1) {
        scratchpad_stack::pop(scratchpad(), 0x40);
        scratchpad() = nullptr;
    } else {
        CDECL_CALL(0x00513FF0);
    }
}

int static_region_list_methods::traverse_sphere(const subdivision_node &a2, const vector3d &arg4, float a4,
                                                subdivision_visitor &a3)
{
    traverse_test a1;

    a1.field_0 = 0;
    a1.field_4 = arg4;
    a1.field_10 = a4;

    return this->traverse_using_test(a1, a2, a3);
}

int static_region_list_methods::traverse_all(const subdivision_node &a2, subdivision_visitor &a3, bool)
{
    traverse_test a1;

    static Var<vector3d> ZEROVEC_5{0x0095C968};

    a1.field_0 = 2;
    a1.field_4 = ZEROVEC_5();
    a1.field_10 = 0.0;
    return this->traverse_using_test(a1, a2, a3);
}

int static_region_list_methods::traverse_line_segment(const subdivision_node &a2, const vector3d &, const vector3d &,
                                                      subdivision_visitor &visitor)
{
    return this->traverse_all(a2, visitor, false);
}

int static_region_list_methods::traverse_point(const subdivision_node &a1, const vector3d &a2, subdivision_visitor &a3)
{
    traverse_test v6;
    v6.field_0 = 1;
    v6.field_4 = a2;
    v6.field_10 = 0.0;
    return this->traverse_using_test(v6, a1, a3);
}

int static_region_list_methods::traverse_using_test(const traverse_test &a1, const subdivision_node &a2,
                                                    subdivision_visitor &a3)
{
    return THISCALL(0x00513E50, this, &a1, &a2, &a3);
}

void static_region_list_builder::build_mirror(stack_allocator &stk, _std::vector<proximity_map_construction_leaf> &a2)
{
    TRACE("static_region_list_builder::build_mirror");

    for (uint32_t k = 0; k < a2.size(); ++k) {
        auto *reg = a2[k].field_0.r;
        auto *v3 = stk.push(sizeof(region_mirror_data));
        auto *mirror = new (v3) region_mirror_data{reg};
        if (k == 0) {
            static_region_list_methods::mirror() = mirror;
        }

        assert(static_region_list_methods::mirror() + k == mirror);
        a2[k].field_0.i = k;
    }
}

region_mirror_data::region_mirror_data(region *reg)
{
    this->field_0 = reg;
    auto *v2 = this->field_0->obb;
    v2->get_extents(&this->field_4, &this->field_10);
}

subdivision_node_builder::subdivision_node_builder()
{
    this->field_4 = 0;
}

#if STANDALONE_SYSTEM
namespace {

subdivision_node *__fastcall build_static_region_list(
    subdivision_node_builder *builder, void *, stack_allocator &stack,
    _std::vector<proximity_map_construction_leaf> &leaves)
{
    return static_cast<static_region_list_builder *>(builder)->build(stack, leaves);
}

void __fastcall build_static_region_mirror(
    subdivision_node_builder *builder, void *, stack_allocator &stack,
    _std::vector<proximity_map_construction_leaf> &leaves)
{
    static_cast<static_region_list_builder *>(builder)->build_mirror(stack, leaves);
}

const subdivision_node_builder_vtable native_static_region_list_builder{
    build_static_region_list, build_static_region_mirror};

}
#endif

subdivision_node *static_region_list_builder::build(
    stack_allocator &stack, _std::vector<proximity_map_construction_leaf> &leaves)
{
#if STANDALONE_SYSTEM
    auto *node = new (stack.push(sizeof(static_region_list_node) +
        leaves.size() * sizeof(uint16_t))) static_region_list_node{};
    node->set_type(subdivision_node::STATIC_REGION_LIST_NODE);
    node->count = static_cast<uint8_t>(leaves.size());
    for (uint32_t i = 0; i < leaves.size(); ++i) {
        node->region_indices()[i] = static_cast<uint16_t>(leaves[i].field_0.i);
    }
    return node;
#else
    return reinterpret_cast<subdivision_node *>(THISCALL(0x00534470, this, &stack, &leaves));
#endif
}


static_region_list_builder::static_region_list_builder()
{
#if STANDALONE_SYSTEM
    this->m_vtbl = &native_static_region_list_builder;
#else
    this->m_vtbl = 0x00888BB8;
#endif
}

void static_region_list_methods_patch()
{
    auto address = int(static_region_list_methods::init);
    REDIRECT(0x0052E065, address);

    {
        FUNC_ADDRESS(address, &static_region_list_builder::build_mirror);
        set_vfunc(0x00888BBC, address);
    }
}
