#include "vehicle.h"

#include "actor.h"
#include "base_ai_core.h"
#include "common.h"
#include "conglom.h"
#include "func_wrapper.h"
#include "wds.h"
#include "wds_entity_manager.h"

VALIDATE_SIZE(vehicle, 0x130);

VALIDATE_OFFSET(vehicle_model, refcount, 0x14);

static auto & car_colors = var<color32[]>(0x00938190);

vehicle::vehicle(vhandle_type<entity> a1)
{
	this->field_50 = a1;
	this->bodytype = this->get_vehicle_body_type(a1);
	if ( this->field_50.get_volatile_ptr() != nullptr )
	{
		auto *c = this->field_50.get_volatile_ptr();
		assert(c->is_an_actor());

		this->field_54 = CAST(field_54, c);

		if ( (this->field_54->field_90.field_6 & 0x3FFF) != 0x3FFF ) {
			this->field_54->field_90.field_6 |= 0xC000u;
		} else {
			this->field_54->ifl_lock(0);
		}
	}
	else
	{
		this->field_54 = nullptr;
	}

	this->reset();
}

void vehicle::reset()
{
	if constexpr (0) {
		this->field_BC = YVEC;
		this->field_C8 = 0.0;
		this->field_CC = 0.0;
		this->field_D0 = 0.0;
		this->field_DC = 0;
		this->field_F0 = 0.0;
		this->field_E0 = 0;
		this->field_E8 = 0.0;
		this->field_F8 = 4;
		this->field_EC = 0.0;
		this->field_B0 = ZEROVEC;
		this->field_F4 = 1.25;
		this->set_collidable(false);
		this->set_visible(false);

		if ( this->get_my_actor() != nullptr )
		{
			auto *v4 = this->get_my_actor();
			v4->set_visible(0, 0);
		}

		this->field_12C = 0;
		this->field_12D = 0;
		this->field_5C = 0;
		this->field_60 = 0;
		this->field_64 = car_colors[0];
		this->update_part_cache();
		this->set_damage_level(0, 0);
		this->set_damage_level(0, 1);
		this->sub_6D7EA0();	
	} else {
		THISCALL(0x006D86F0, this);
	}
}

void vehicle::update_part_cache()
{
	THISCALL(0x006C1E00, this);
}

void vehicle::sub_6D7EA0()
{
	THISCALL(0x006D7EA0, this);
}

void vehicle::set_damage_level(int a2, int a3)
{
	THISCALL(0x006C1330, this, a2, a3);
}

int vehicle::get_vehicle_body_type(vhandle_type<entity> a1)
{
	static string_hash vehicle_body_type_hash {int(to_hash("vehicle_body_type"))};

	auto *e = a1.get_volatile_ptr();
	assert(e != nullptr && e->is_an_actor());

	if ( e != nullptr )
	{ 
		auto *the_ai_core = e->get_ai_core();
		if (the_ai_core != nullptr)
		{
			int v6 = -1;
			auto opt_pb_int = the_ai_core->field_50.get_optional_pb_int(vehicle_body_type_hash, v6, nullptr);
			if (opt_pb_int != -1) {
				return opt_pb_int;
			}

		}
	}

	assert(0 && "No body type set for car");

	if ( vehicle::cur_vehicle_type() == -1 ) {
		return 9;
	} else {
		return vehicle::cur_vehicle_type();
	}
}

color32 vehicle::get_part_color(string_hash a2)
{
    auto *v3 = this->get_my_actor();
    if ( v3->is_a_conglomerate() )
    {
        auto *v4 = bit_cast<conglomerate *>(v3);

        auto *body_member = bit_cast<entity *>(v4->get_member(a2, true));
        assert(body_member != nullptr && "Can't find the body member of the conglomerate");

        if (body_member != nullptr)
        {
            auto col = body_member->get_render_color();
            return col;
        }
    }

    auto result = car_colors[0];
    return result;
}

void vehicle::set_collidable(bool a2)
{
	if ( this->get_my_actor() != nullptr )
	{
		auto *v5 = this->get_my_actor();
		v5->set_collisions_active(a2, true);
		if ( a2 )
		{
			assert(get_my_actor()->are_collisions_active());

			assert(get_my_actor()->is_walkable());
		}
	}
}

void vehicle::set_visible(bool a2)
{
	if ( this->get_my_actor() != nullptr )
	{
		if ( a2 )
		{
			auto *v2 = this->get_my_actor();
			v2->set_fade_distance(90.0);
		}

		auto *v4 = this->get_my_actor();
		v4->set_visible(a2, false);
	}
}

actor *vehicle::get_my_actor()
{
  	return this->field_54;
}

bool vehicle::terminate_vehicles()
{
    return (bool) CDECL_CALL(0x006C12B0);
}

void vehicle_model::sub_6B9F30(vhandle_type<entity> a2)
{
    --this->refcount;
    auto *e = a2.get_volatile_ptr();
    g_world_ptr->ent_mgr.release_entity(e);
}


