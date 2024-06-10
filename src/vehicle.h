#pragma once

#include "vehicle_sounds.h"

#include "color32.h"
#include "entity.h"
#include "entity_base_vhandle.h"
#include "mstring.h"
#include "oldmath_po.h"
#include "variable.h"

struct vehicle_model {
    int field_0;
    mString field_4;
    int refcount;

    void sub_6B9F30(vhandle_type<entity> a2);
};

struct actor;

struct vehicle : vehicle_sounds {
    vhandle_type<entity> field_50;
    actor *field_54;
    uint32_t bodytype;
	int field_5C;
	int field_60;
	color32 field_64;
	int field_68;
	int field_6C;
	po field_70;
	vector3d field_B0;
	vector3d field_BC;
    float field_C8;
    float field_CC;
    float field_D0;
	int field_D4;
	int field_D8;
	int field_DC;
	float field_E0;
	int field_E4;
	float field_E8;
	float field_EC;
	float field_F0;
	float field_F4;
	int field_F8;
	int field_FC;
	int field_100;
	int field_104;
	int field_108;
	entity *field_10C;
	int field_110;
	int field_114;
	int field_118;
	int field_11C;
	int field_120;
	int field_124;
	int field_128;
	bool field_12C;
	bool field_12D;

    vehicle(vhandle_type<entity> a1);

	void reset();

	void update_part_cache();

	void sub_6D7EA0();

	void set_damage_level(int a2, int a3);

	int get_vehicle_body_type(vhandle_type<entity> a1);

    color32 get_part_color(string_hash a2);

	void set_collidable(bool a2);

	void set_visible(bool a2);

	//virtual
	actor *get_my_actor();

    static bool terminate_vehicles();

    static inline Var<vehicle_model *[1]> models {0x0096C97C};

	static inline Var<int> cur_vehicle_type {0x00937FCC};
};
