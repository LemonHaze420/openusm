#include "district_graph_container.h"

#include "common.h"
#include "func_wrapper.h"
#include "memory.h"
#include "parse_generic_mash.h"
#include "terrain.h"
#include "region.h"
#include "subdivision_obb.h"

#include <limits>
#include <new>

#include "trace.h"

VALIDATE_SIZE(dsg_box_container, 0x34);
VALIDATE_SIZE(dsg_region_container, 0x54);

VALIDATE_SIZE(district_graph_container, 0x14);

void district_graph_container::setup_terrain(terrain *the_terrain)
{
    the_terrain->total_regions = field_0.size();
    the_terrain->regions = new region *[the_terrain->total_regions];

    for (int index = 0; index < the_terrain->total_regions; ++index) {
        auto &source = field_0[index];
        auto *reg = new region{mString{source.field_0}};
        reg->field_A4 = *reinterpret_cast<vector3d *>(&source.field_4[1]);
        if (source.field_50 & 2)
            reg->flags |= 0x101;

        const auto &center = *reinterpret_cast<const vector3d *>(&source.field_4[4]);
        const auto &x_axis = *reinterpret_cast<const vector3d *>(&source.field_4[7]);
        const auto &y_axis = *reinterpret_cast<const vector3d *>(&source.field_4[10]);
        const auto &z_axis = *reinterpret_cast<const vector3d *>(&source.field_4[13]);
        if (source.field_4[16] & 0x20000) {
            auto *obb = new subdivision_node_large_aabb;
            const vector3d half_size{x_axis.x, y_axis.y, z_axis.z};
            obb->init(0, 0, center, half_size);
            reg->obb = obb;
        } else {
            auto *obb = new subdivision_node_large_obb;
            obb->init(0, 0, center, x_axis, y_axis, z_axis);
            reg->obb = obb;
        }

        reg->neighbors.reserve(source.field_48.size());
        for (int neighbor = 0; neighbor < source.field_48.size(); ++neighbor)
            reg->neighbors.push_back(static_cast<unsigned short>(source.field_48[neighbor]));
        reg->field_78 = source.field_0;
        reg->field_88 = reg->field_78;
        vector3d vertices[8];
        reg->obb->get_vertices(vertices);
        reg->field_B0 = vector3d{};
        reg->field_BC = std::numeric_limits<float>::max();
        for (const auto &vertex : vertices) {
            reg->field_B0.x += vertex.x;
            reg->field_B0.y += vertex.y;
            reg->field_B0.z += vertex.z;
            if (vertex.y < reg->field_BC)
                reg->field_BC = vertex.y;
        }
        reg->field_B0 *= 0.125f;
        if (!(source.field_50 & 4))
            reg->field_A4 = reg->field_B0;
        the_terrain->regions[index] = reg;
        auto *entry =
            ::new (mem_alloc(sizeof(region_lookup_entry))) region_lookup_entry{string_hash{source.field_0}, index};
        the_terrain->field_5C.add(entry);
    }

    the_terrain->init_region_proximity_map();
}

void dsg_region_container::un_mash(generic_mash_header *header, [[maybe_unused]] void *a3, generic_mash_data_ptrs *a4)
{
    if constexpr (1) {
        a4->rebase(4u);

        auto v5 = *a4->get<int>();
        this->field_0 = a4->get<char>(v5);

        assert(((int)header) % 4 == 0);

        this->field_48.custom_un_mash(header, &this->field_48, a4, nullptr);
    } else {
        assert(0);
    }
}

void district_graph_container::un_mash_start(generic_mash_header *a2, void *a3, generic_mash_data_ptrs *a4,
                                             [[maybe_unused]] void *a5)
{
    this->un_mash(a2, a3, a4);
}

void district_graph_container::un_mash(generic_mash_header *a2, [[maybe_unused]] void *a3, generic_mash_data_ptrs *a4)
{
    TRACE("district_graph_container::un_mash");

    if constexpr (1) {
        this->field_0.custom_un_mash(a2, &this->field_0, a4, nullptr);
        this->field_8.custom_un_mash(a2, &this->field_8, a4, nullptr);
    } else {
        THISCALL(0x00520C90, this, a2, a3, a4);
    }
}

void dsg_box_container::un_mash([[maybe_unused]] generic_mash_header *a2, [[maybe_unused]] void *a3,
                                [[maybe_unused]] generic_mash_data_ptrs *a4)
{
    ;
}
