#include "mission_manager_script_data.h"

#include "common.h"
#include "func_wrapper.h"
#include "oldmath_po.h"

VALIDATE_SIZE(mission_marker_base, 0xC);
VALIDATE_SIZE(mission_camera_marker, 0x18);
VALIDATE_SIZE(mission_transform_marker, 0x30);
VALIDATE_SIZE(mission_camera_transform_marker, 0x30);
VALIDATE_OFFSET(mission_camera_marker, camera_position, 0xC);
VALIDATE_OFFSET(mission_transform_marker, forward, 0x18);
VALIDATE_OFFSET(mission_transform_marker, up, 0x24);
VALIDATE_OFFSET(mission_camera_transform_marker, camera_position, 0xC);
VALIDATE_OFFSET(mission_camera_transform_marker, forward, 0x18);
VALIDATE_OFFSET(mission_camera_transform_marker, up, 0x24);
VALIDATE_OFFSET(mission_manager_script_data, markers, 0x44);
VALIDATE_OFFSET(mission_manager_script_data, camera_markers, 0x54);
VALIDATE_OFFSET(mission_manager_script_data, transform_markers, 0x64);
VALIDATE_OFFSET(mission_manager_script_data, camera_transform_markers, 0x74);
VALIDATE_OFFSET(mission_manager_script_data, field_94, 0x94);
VALIDATE_OFFSET(mission_manager_script_data, field_A4, 0xA4);
VALIDATE_SIZE(mission_manager_script_data, 0xE8);

mission_manager_script_data::mission_manager_script_data()
{
    if constexpr (STANDALONE_SYSTEM) {
        field_94 = new po{};
        field_A4.m_hash.source_hash_code = 0;
        field_A4.m_type = RESOURCE_KEY_TYPE_NONE;
        uses_script_stack = false;
        field_B0 = -1;
        const auto unset = bit_cast<float>(0xFFFFFFFFu);
        field_98 = vector3d{unset, unset, unset};
        field_B4 = false;
        field_B5 = false;
    } else {
        THISCALL(0x005E9760, this);
    }
}

mission_manager_script_data::mission_manager_script_data(const mission_manager_script_data &other)
    : mission_manager_script_data()
{
    copy(other);
}

mission_manager_script_data &mission_manager_script_data::operator=(const mission_manager_script_data &other)
{
    if (this != &other)
        copy(other);
    return *this;
}

mission_manager_script_data::~mission_manager_script_data()
{
    if constexpr (STANDALONE_SYSTEM)
        delete field_94;
    else
        THISCALL(0x005E8F70, this);
}

void mission_manager_script_data::clear()
{
    pos._Tidy();
    strings._Tidy();
    nums._Tidy();
    markers._Tidy();
    camera_markers._Tidy();
    transform_markers._Tidy();
    field_A4.m_hash.source_hash_code = 0;
    field_A4.m_type = RESOURCE_KEY_TYPE_NONE;
    uses_script_stack = false;
    field_B0 = -1;
    *field_94 = po_identity_matrix;
    const auto unset = bit_cast<float>(0xFFFFFFFFu);
    field_98 = vector3d{unset, unset, unset};
    field_B4 = false;
    field_B5 = false;


    const auto truncate = [](mString &value) {
        if (!value.empty()) {
            value.data()[0] = '\0';
            value.set_size(0);
        }
    };
    truncate(field_B8);
    truncate(field_C8);
    truncate(field_D8);
}

void mission_manager_script_data::copy(const mission_manager_script_data &a2)
{
    field_0 = a2.field_0;
    field_10 = a2.field_10;


    const auto copy_vector = [](auto &destination, const auto &source) {
        if (&destination != &source) {
            if (source.empty())
                destination._Tidy();
            else
                destination = source;
        }
    };
    copy_vector(pos, a2.pos);
    copy_vector(strings, a2.strings);
    copy_vector(nums, a2.nums);
    copy_vector(markers, a2.markers);
    copy_vector(camera_markers, a2.camera_markers);
    copy_vector(transform_markers, a2.transform_markers);
    copy_vector(camera_transform_markers, a2.camera_transform_markers);
    field_84 = a2.field_84;
    field_98 = a2.field_98;
    field_A4 = a2.field_A4;
    uses_script_stack = a2.uses_script_stack;
    field_B0 = a2.field_B0;
    field_B4 = a2.field_B4;
    *field_94 = *a2.field_94;
    field_B5 = a2.field_B5;
    field_B8 = a2.field_B8;
    field_C8 = a2.field_C8;
    field_D8 = a2.field_D8;
}
