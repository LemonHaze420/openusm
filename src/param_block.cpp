#include "param_block.h"
#include "mash_config.h"

#include "common.h"
#include "func_wrapper.h"
#include "log.h"
#include "mash_info_struct.h"
#include "memory.h"
#include "osassert.h"
#include "trace.h"
#include "utility.h"

#include <cassert>

namespace ai {

VALIDATE_SIZE(param_block, 0xC);
VALIDATE_SIZE(param_block::param_data_array, 0x18);

param_block::param_block() {}

void param_block::unmash(mash_info_struct *a1, void *a3)
{
    TRACE("param_block::unmash");

    if constexpr (1)
    {
#if OPENUSM_XBOX_MASH_FORMAT && !defined(OPENUSM_XBPACK_V10)
        {
            uint8_t class_mashed = -1;
            class_mashed = *a1->read_from_buffer(mash::SHARED_BUFFER, 1, 1);
            assert(class_mashed == 0xAF || class_mashed == 0);
        }
#endif

        if ( this->param_array != nullptr )
        {
            a1->unmash_class(this->param_array, a3
#if OPENUSM_XBOX_MASH_FORMAT
                    , mash::NORMAL_BUFFER
#endif
                    );
        }
    }
    else
    {
        THISCALL(0x006D56B0, this, a1, a3);
    }
}

void param_block::add_param(
        string_hash a2,
        param_types a3,
        const void *a4,
        string_hash a5)
{
    if constexpr (0)
    {
    }
    else
    {
        THISCALL(0x006D6710, this, a2, a3, a4, a5);
    }
}

int param_block::get_parameter_data_type(string_hash a2) const
{
    assert(param_array != nullptr);

    auto *the_param = this->param_array->common_find_data(a2);
    assert(the_param != nullptr);

    return the_param->get_data_type();
}

const char *param_block::get_pb_fixedstring(string_hash a2) const {
    auto *v2 = this->param_array;
    if (v2 == nullptr) {
        return nullptr;
    }

    auto *curr_data = v2->common_find_data(a2);
    if (curr_data == nullptr) {
        auto *v3 = a2.to_string();
        error("Could not find the parameter %s.", v3);
    }

    assert((curr_data->get_data_type() == PT_FIXED_STRING) && "Parameter is of the wrong type.");

    return curr_data->get_data_fixedstring();
}

void param_block::set_pb_float(string_hash a2, Float a3, bool a4)
{
    if ( a4 )
    {
        this->add_param(a2, PT_FLOAT, &a3, string_hash {0});
    }
    else
    {
        auto *data = this->param_array->common_find_data(a2);
        if ( data != nullptr ) {
            this->add_param(a2, PT_FLOAT, &a3, string_hash {0});
        }
    }
}

float param_block::get_pb_float(string_hash a2)
{
    auto *v2 = this->param_array;

    if (v2 == nullptr) {
        return 0.0f;
    }

    param_data *curr_data = v2->common_find_data(a2);
    if (curr_data == nullptr) {
        const char *str = a2.to_string();
        error("Could not find the parameter %s.", str);
    }

    assert((curr_data->get_data_type() == PT_FLOAT) && "Parameter is of the wrong type.");

    return curr_data->get_data_float();
}

void param_block::param_data::finalize(mash::allocation_scope a2)
{
    if (a2 == 0 && this->m_union.i) {
        if (this->my_type == PT_FIXED_STRING) {
            ::operator delete[](this->m_union.str);
        } else if (this->my_type == PT_VECTOR_3D || this->my_type == PT_FLOAT_VARIANCE) {
            ::operator delete(this->m_union.vec3);
            this->m_union.vec3 = nullptr;
            return;
        }

        this->m_union.i = 0;
    }
}

param_block::param_data::param_data() {
    this->m_name.initialize(mash::FROM_MASH, nullptr, 0);
    this->initialize(mash::FROM_MASH);
}

param_block::param_data::~param_data() {
    this->finalize(mash::ALLOCATED);
}

void param_block::param_data::initialize(mash::allocation_scope a2)
{
    if ( a2 == mash::ALLOCATED )
    {
        this->my_type = static_cast<param_types>(9);
        this->m_union.i = 0;
    }
}

void param_block::param_data::operator delete(void *ptr, size_t size) {
    mem_dealloc(ptr, size);
}

void param_block::param_data::destruct_mashed_class() {
    this->m_name.destruct_mashed_class();
}

void param_block::param_data::set_data_float(float a2)
{
    assert(my_type == PT_FLOAT);
    this->m_union.f = a2;
}

void param_block::param_data::set_data_float_variance(const variance_variable<float> &a2)
{
    assert(my_type == PT_FLOAT_VARIANCE);

    if (this->get_data_float_variance() == nullptr) {
        this->m_union.float_variance = new variance_variable<float>{};
    }

    *this->m_union.float_variance = a2;
}

void param_block::param_data::set_data_vector3d(const vector3d &a2)
{
    assert(my_type == PT_VECTOR_3D);

    if (this->get_data_vector3d() == nullptr) {
        this->m_union.vec3 = new vector3d{};
    }

    *this->m_union.vec3 = a2;
}

void param_block::param_data::set_data_entity(entity_base_vhandle &a2)
{
    assert(my_type == PT_ENTITY);
    this->m_union.ent = &a2;
}

void param_block::param_data::set_data_pointer(void *a2)
{
    assert(my_type == PT_POINTER);
    this->m_union.ptr = a2;
}

void param_block::param_data::set_data_fixedstring(char *a2)
{
    assert(my_type == PT_FIXED_STRING);

    if ( this->m_union.str == nullptr ) {
        this->m_union.str = new char[32] {};
    }

    for ( int i = 0; i < 32; ++i )
    {
        if ( a2[i] != '\0' ) {
            this->m_union.str[i] = a2[i];
        } else {
            this->m_union.str[i] = '\0';
        }
    }

    this->m_union.str[31] = '\0';
}

string_hash param_block::get_pb_hash(string_hash a3) const
{
    ai::param_block v7{};
    auto *v3 = this->param_array;
    if (v3 != nullptr) {
        ai::param_block::param_data *v5 = v3->common_find_data(a3);
        if (v5 == nullptr) {
            const char *str = a3.to_string();
            sp_log("Could not find the parameter %s.", str);
            assert(0);
        }

        auto a2 = v5->get_data_hash();
        return a2;
    }

    return {};
}

variance_variable<float> *param_block::get_pb_float_variance(string_hash a2) {
    return (variance_variable<float> *) THISCALL(0x006CDDF0, this, a2);
}

vector3d *param_block::get_pb_vector3d(string_hash a2) {
    return (vector3d *) THISCALL(0x006CDD80, this, a2);
}

int param_block::get_pb_int(string_hash a2) {
    auto *v2 = this->param_array;
    if (v2 == nullptr) {
        return 0;
    }

    param_data *v4 = v2->common_find_data(a2);
    if (v4 == nullptr) {
        const char *v7 = a2.to_string();
        sp_log("Could not find the parameter %s.", v7);
        assert(0);
    }

    return v4->get_data_int();
}

float param_block::param_data::get_data_float() const
{
    assert(my_type == PT_FLOAT);
    return m_union.f;
}

string_hash param_block::param_data::get_data_hash() const
{
    assert(my_type == PT_STRING_HASH);
    return this->m_union.hash;
}

const char * param_block::param_data::get_data_fixedstring() const
{
    assert(my_type == PT_FIXED_STRING);
    return this->m_union.str;
}

vector3d * param_block::param_data::get_data_vector3d() const {
    return m_union.vec3;
}

variance_variable<float> * param_block::param_data::get_data_float_variance() const
{
    assert(my_type == PT_FLOAT_VARIANCE);
    return this->m_union.float_variance;
}

int param_block::param_data::get_data_int() {
    assert(my_type == PT_INTEGER);
    return m_union.i;
}

void * param_block::param_data::get_data_pointer() const
{
    assert(my_type == PT_POINTER);
    return this->m_union.ptr;
}

int param_block::get_optional_pb_int(string_hash a2, const int &a3, bool *a4) const {
    if (a4 != nullptr) {
        *a4 = false;
    }

    auto *v4 = this->param_array;
    if (v4 == nullptr) {
        return a3;
    }

    float result;

    auto *curr_data = v4->common_find_data(a2);
    if (curr_data != nullptr) {
        assert((curr_data->get_data_type() == PT_INTEGER) && "Parameter is of the wrong type.");

        if (a4 != nullptr) {
            *a4 = true;
        }

        result = curr_data->get_data_int();
    } else {
        result = a3;
    }

    return result;
}

float param_block::get_optional_pb_float(string_hash a2, const float &a3, bool *a4) const
{
    if (a4 != nullptr) {
        *a4 = false;
    }

    auto *v4 = this->param_array;

    if (v4 == nullptr) {
        return a3;
    }

    float result;

    auto *curr_data = v4->common_find_data(a2);
    if (curr_data != nullptr) {
        assert((curr_data->get_data_type() == PT_FLOAT) && "Parameter is of the wrong type.");

        if (a4 != nullptr) {
            *a4 = true;
        }

        result = curr_data->get_data_float();
    } else {
        result = a3;
    }

    return result;
}

param_block::param_data_array::~param_data_array()
{
    this->field_14 = 0;
    this->field_0.clear();
}

void param_block::param_data_array::unmash(mash_info_struct *a1, void *)
{
    a1->unmash_class_in_place(this->field_0, this); 
}

void param_block::param_data_array::destruct_mashed_class()
{
    this->finalize(mash::FROM_MASH);
    this->field_0.destruct_mashed_class();
}

param_block::~param_block()
{
    this->finalize(mash::ALLOCATED);
}

void param_block::finalize(mash::allocation_scope )
{
    if ( this->param_array != nullptr && this->field_8 )
    {
        if ( this->param_array != nullptr ) {
            this->param_array->~param_data_array();
            mem_dealloc(this->param_array, sizeof(param_data_array));
        }

        this->param_array = nullptr;
    }
}

param_block::param_data *param_block::param_data_array::common_find_data(string_hash a2) {
    return (param_block::param_data *) THISCALL(0x006CD450, this, a2);
}

bool param_block::does_parameter_exist(string_hash a2) const {
    auto *v2 = this->param_array;
    bool result = false;
    if (v2 != nullptr) {
        if (v2->common_find_data(a2) != nullptr) {
            result = true;
        }
    }

    return result;
}

bool param_block::does_parameter_match(const param_block::param_data *a2) const
{
    if ( this->param_array == nullptr ) {
        return false;
    }

    auto v5 = a2->get_name();
    auto *other = this->param_array->common_find_data(v5);
    assert(other != nullptr && "attempting to match a parameter that doesn't exist");
    return a2->deep_compare(other);
}

void param_block::destruct_mashed_class()
{
    this->finalize(mash::FROM_MASH);

    auto *v3 = this->param_array;
    if (v3 != nullptr) {
        v3->destruct_mashed_class();
        this->param_array = nullptr;
    }
}

void param_block::param_data::custom_unmash(mash_info_struct *a2, void *a3)
{
    if constexpr (OPENUSM_XBOX_MASH_FORMAT)
    {
        switch ( this->get_data_type() )
        {
        case PT_FIXED_STRING:
            this->m_union.str = (char *) a2->read_from_buffer(0x20u, 4);
            break;
        case PT_VECTOR_3D:
            this->m_union.vec3 = (vector3d *) a2->read_from_buffer(sizeof(vector3d), 4);
            break;
        case PT_FLOAT_VARIANCE:
            this->m_union.float_variance = (variance_variable<float> *) a2->read_from_buffer(8, 4);
            break;
        case PT_ENTITY:
        case PT_POINTER:
            this->m_union.ptr = nullptr;
            break;
        default:
            return;
        }
    }
    else
    {
        THISCALL(0x006CC070, this, a2, a3);
    }
}

bool param_block::param_data::deep_compare(const param_block::param_data *a2) const
{
    return (bool) THISCALL(0x006C4AE0, this, a2);
}

void param_block::param_data::unmash(mash_info_struct *a1, void *a3)
{
    a1->unmash_class_in_place(this->m_name, this);
    this->custom_unmash(a1, a3);
}

} // namespace ai

void param_block_patch() {
    {
        FUNC_ADDRESS(address, &ai::param_block::get_pb_float);

        REDIRECT(0x00694C33, address);
        REDIRECT(0x00694C56, address);
        REDIRECT(0x00694C79, address);
        REDIRECT(0x00694C9C, address);
        REDIRECT(0x00694CBA, address);
    }

    {
        FUNC_ADDRESS(address, &ai::param_block::get_optional_pb_float);

        REDIRECT(0x00694BBB, address);
        REDIRECT(0x00694BE0, address);
        REDIRECT(0x00694C05, address);
        REDIRECT(0x00694D48, address);
    }
}
