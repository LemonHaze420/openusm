#include "vehicle.h"

#include "actor.h"
#include "ai_voice_box_inode.h"
#include "base_ai_core.h"
#include "common.h"
#include "conglom.h"
#include "func_wrapper.h"
#include "region.h"
#include "resource_manager.h"
#include "terrain.h"
#include "wds.h"
#include "wds_entity_manager.h"

#include <numeric>

VALIDATE_SIZE(vehicle, 0x130);

VALIDATE_OFFSET(vehicle_model, refcount, 0x14);
VALIDATE_SIZE(vehicle_model, 0x1C);

static auto &car_colors = var<color32[]>(0x00938190);

static std::pair<string_hash, string_hash> (&s_tail_parts)[5] = var<std::pair<string_hash, string_hash>[5]>(0x0096D208);

static string_hash (&s_car_nose_parts)[5][6] = var<string_hash[5][6]>(0x0096CAB8);

static string_hash (&s_suv_nose_parts)[5][2] = var<string_hash[5][2]>(0x0096CB30);

Var<vehicle_model *[VEHICLE_MODEL_MAX]> vehicle::models {
    0x0096C97C
};

int &vehicle::cur_vehicle_type = var<int>(0x00937FCC);

vehicle::vehicle(vhandle_type<entity> a1)
{
    this->field_50 = a1;
    this->bodytype = this->get_vehicle_body_type(a1);
    if (this->field_50.get_volatile_ptr() != nullptr) {
        auto *c = this->field_50.get_volatile_ptr();
        assert(c->is_an_actor());

        this->field_54 = CAST(field_54, c);

        if ((this->field_54->field_90.field_6 & 0x3FFF) != 0x3FFF) {
            this->field_54->field_90.field_6 |= 0xC000u;
        } else {
            this->field_54->ifl_lock(0);
        }
    } else {
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

        if (this->get_my_actor() != nullptr) {
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
    TRACE("vehicle::update_part_cache");

    auto func = [](vehicle *self, string_hash a3) -> actor * {
        auto *v3 = bit_cast<conglomerate *>(self->get_my_actor());
        auto result = bit_cast<actor *>(v3->get_member(a3, true));
        return result;
    };

    if constexpr (0) {
        if (this->get_my_actor() != nullptr) {
            static string_hash df_id{int(to_hash("DF"))};

            static string_hash pf_id{int(to_hash("PF"))};

            static string_hash dr_id{int(to_hash("DR"))};

            static string_hash pr_id{int(to_hash("PR"))};

            static string_hash body_id{int(to_hash("BODY"))};

            static string_hash police_gear_id{int(to_hash("POLICE_GEAR"))};

            static string_hash shadow_id{int(to_hash("SHADOW"))};

            static string_hash taxi_id{int(to_hash("TAXI"))};

            static string_hash handlbars_id{int(to_hash("HANDLEBARS"))};

            static string_hash taxi_light_cones{int(to_hash("TAXI_LIGHTCONES"))};

            this->dftire = func(this, df_id);

            this->pftire = func(this, pf_id);

            this->drtire = func(this, dr_id);

            this->prtire = func(this, pr_id);

            this->body = func(this, body_id);

            this->field_110 = func(this, police_gear_id);

            this->field_114 = func(this, shadow_id);

            this->field_118 = func(this, taxi_id);

            this->field_11C = func(this, handlbars_id);

            auto bodytype = this->bodytype;
            if (bodytype == 1) {
                this->field_120 = this->body;
            } else if (bodytype) {
                this->field_120 = nullptr;
            } else {
                this->field_120 = func(this, taxi_light_cones);
                auto *v21 = this->body;
                this->field_124 = v21;
                this->field_128 = v21;
            }
        } else {
            this->dftire = 0;
            this->pftire = nullptr;
            this->drtire = nullptr;
            this->prtire = nullptr;
            this->body = nullptr;
            this->field_110 = nullptr;
            this->field_114 = nullptr;
            this->field_118 = nullptr;
            this->field_11C = nullptr;
            this->field_120 = nullptr;
            this->field_124 = nullptr;
            this->field_128 = nullptr;
        }
    } else {
        THISCALL(0x006C1E00, this);
    }

    {
        const string_hash police_gear_id{int(to_hash("POLICE_GEAR"))};
        this->field_120 = func(this, police_gear_id);
        auto *v21 = this->body;
        this->field_124 = v21;
        this->field_128 = v21;
    }
}

static const char *(&off_937FD4)[6] = var<const char *[6]>(0x00937FD4);

void vehicle::sub_6D7EA0()
{
    if constexpr (0) {
        if (this->get_my_actor() != nullptr) {
            auto *v2 = this->get_my_actor();
            if (v2->get_ai_core() != nullptr) {
                auto *v4 = this->get_my_actor();
                auto *v6 = v4->get_ai_core();
                auto *v11 = (ai::voice_box_inode *)v6->get_info_node(ai::voice_box_inode::default_id, false);
                if (v11 != nullptr) {
                    auto idx = [](int a1) -> int {
                        if (a1 <= 0) {
                            return 0;
                        } else {
                            return ((a1 * rand()) / 32768.0);
                        }
                    }(6);

                    v11->sub_6D7E10(off_937FD4[idx]);
                }
            }
        }
    } else {
        THISCALL(0x006D7EA0, this);
    }
}

vector3d vehicle::sub_6DA250()
{
    auto *v2 = this->get_my_actor();
    auto result = v2->get_abs_position();
    return result;
}

void vehicle::set_damage_level(int a2, int a3)
{
    THISCALL(0x006C1330, this, a2, a3);
}

int vehicle::get_vehicle_body_type(vhandle_type<entity> a1)
{
    static string_hash vehicle_body_type_hash{int(to_hash("vehicle_body_type"))};

    auto *e = a1.get_volatile_ptr();
    assert(e != nullptr && e->is_an_actor());

    if (e != nullptr) {
        auto *the_ai_core = e->get_ai_core();
        if (the_ai_core != nullptr) {
            int v6 = -1;
            auto opt_pb_int = the_ai_core->field_50.get_optional_pb_int(vehicle_body_type_hash, v6, nullptr);
            if (opt_pb_int != -1) {
                return opt_pb_int;
            }
        }
    }

    assert(0 && "No body type set for car");

    if (vehicle::cur_vehicle_type == -1) {
        return 9;
    } else {
        return vehicle::cur_vehicle_type;
    }
}

color32 vehicle::get_part_color(string_hash a2)
{
    auto *v3 = this->get_my_actor();
    if (v3->is_a_conglomerate()) {
        auto *v4 = bit_cast<conglomerate *>(v3);

        auto *body_member = bit_cast<entity *>(v4->get_member(a2, true));
        assert(body_member != nullptr && "Can't find the body member of the conglomerate");

        if (body_member != nullptr) {
            auto col = body_member->get_render_color();
            return col;
        }
    }

    auto result = car_colors[0];
    return result;
}

void vehicle::set_collidable(bool a2)
{
    if (this->get_my_actor() != nullptr) {
        auto *v5 = this->get_my_actor();
        v5->set_collisions_active(a2, true);
        if (a2) {
            assert(get_my_actor()->are_collisions_active());

            assert(get_my_actor()->is_walkable());
        }
    }
}

void vehicle::set_visible(bool a2)
{
    if (this->get_my_actor() != nullptr) {
        if (a2) {
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
    return (bool)CDECL_CALL(0x006C12B0);
}

vehicle_model::vehicle_model(int a1, mString a2) : field_0(a1), field_4(a2)
{
    this->refcount = 0;
    this->field_18 = 0.0;
}

void vehicle_model::sub_6B9F30(vhandle_type<entity> a2)
{
    --this->refcount;
    auto *e = a2.get_volatile_ptr();
    g_world_ptr->ent_mgr.release_entity(e);
}

entity_base_vhandle vehicle_model::create(int a3)
{
    entity_base_vhandle result;
    THISCALL(0x006C6680, this, &result, a3);
    return result;
}

void vehicle::sub_6BAED0(const vector3d &pos)
{
    auto *v3 = this->get_my_actor();
    v3->set_allow_tunnelling_into_next_frame(true);
    auto *v4 = this->get_my_actor();
    entity_set_abs_position(v4, pos);
}

int vehicle::pick_random_model()
{
    TRACE("vehicle::pick_random_model");

    if constexpr (0) {
    } else {
        [[maybe_unused]] auto result = CDECL_CALL(0x006BA1B0);
        return 0;
    }
}

int vehicle::pick_model(int a1)
{
    TRACE("vehicle::pick_model");

    if constexpr (0) {
        auto v1 = std::accumulate(std::begin(models()), std::end(models()), 0, [](uint32_t partialSum, auto &model) {
            return (model != nullptr ? model->field_18 : 0) + partialSum;
        });

        if (v1 >= EPSILON) {
            auto v3 = v1 * (a1 / 30.0f);
            for (int i = 0; i < 8; ++i) {
                auto *v4 = models()[i];
                if (v4 != nullptr) {
                    if (v4->field_18 > v3) {
                        return i;
                    }

                    v3 = v3 - v4->field_18;
                }
            }

            return 0;
        } else {
            assert(0 && "Bad usage rates on cars");
            return 0;
        }
    } else {
        [[maybe_unused]] auto result = CDECL_CALL(0x006BA100, a1);
        return result;
    }
}

void sub_6BB1E0(conglomerate *the_conglom, string_hash a2, color32 a3, bool a4, bool a5, char a6)
{
    if (the_conglom->is_a_conglomerate()) {
        auto *body_member = the_conglom->get_member(a2, true);
        assert(body_member != nullptr && "Can't find the body member of the conglomerate");

        auto *v7 = bit_cast<conglomerate *>(body_member);
        if (body_member != nullptr) {
            body_member->set_visible(a4, true);
            if (a5) {
                v7->set_collisions_active(a4, false);
            }

            if (a4) {
                v7->set_render_color(a3);
            }

            auto func = [](actor::mesh_buffers *self, char a2) -> void {
                self->field_6 = (self->field_6 & 0x3FFF) | ((3 - (a2 & 3)) << 14);
            };

            func(&v7->field_90, a6);
        }
    }
}

void vehicle::determine_tire_radius()
{
    if (this->field_118) {
        this->field_EC = 0.47797784;
    } else {
        this->field_EC = 0.42068103;
    }
}

void vehicle::determine_wheel_base()
{
    this->field_F8 = 2 * (this->prtire != nullptr) + 2;

    auto v8 = this->dftire->get_abs_position();

    auto v5 = v8 - this->drtire->get_abs_position();
    auto v7 = v5.xz_norm();

    this->field_E8 = v7;
    if (bodytype == 5 || bodytype == 6) {
        this->field_E8 *= 0.85000002f;
    }
}

void vehicle::pick_body_and_color()
{
    this->update_part_cache();
    if (this->field_E8 <= EPSILON) {
        this->determine_wheel_base();
    }

    if (this->field_EC <= EPSILON) {
        this->determine_tire_radius();
    }

    if (this->field_F8 == 2) {
        static string_hash vcl_wolvcycle_base_id{int(to_hash("VCL_WOLVCYCLE_BASE"))};

        auto *v3 = (conglomerate *)this->get_my_actor();
        this->body = (actor *)v3->get_member(vcl_wolvcycle_base_id, true);
        return;
    } else {
        auto *dftire = this->dftire;
        if (dftire != nullptr) {
            dftire->ifl_lock(0);
        }

        auto *pftire = this->pftire;
        if (pftire != nullptr) {
            pftire->ifl_lock(0);
        }

        auto *drtire = this->drtire;
        if (drtire != nullptr) {
            drtire->ifl_lock(0);
        }

        auto *prtire = this->prtire;
        if (prtire != nullptr) {
            prtire->ifl_lock(0);
        }

        auto func = [](int a1) -> int {
            if (a1 <= 0) {
                return 0;
            } else {
                return ((a1 * rand()) / 32768.0);
            }
        };

        this->field_64 = car_colors[0];
        this->field_68 = 0;
        this->field_6C = 0;

        assert(body != nullptr);
        assert(dftire != nullptr);
        assert(pftire != nullptr);
        assert(drtire != nullptr);
        assert(prtire != nullptr);

        auto *body = this->body;
        if (body != nullptr && this->bodytype == 1) {
            auto *v9 = this->field_110;
            if (v9 != nullptr) {
                v9->set_visible(false, false);
            }

            auto *v10 = this->field_110;
            if (v10 != nullptr) {
                v10->set_member_hidden(true);
            }

            assert(this->body != nullptr && this->bodytype >= VEHICLE_MODEL_CAR);

            this->field_64 = car_colors[func(1)];
            this->field_5C = func(3);
            this->field_60 = func(3);

            auto v11 = this->field_5C;
            switch (v11) {
            case 0: {
                this->set_nose_visible(0, this->field_64, 1, this->field_68);
                this->set_nose_visible(1, this->field_64, 0, this->field_68);
                this->set_nose_visible(4, this->field_64, 0, this->field_68);
                break;
            }
            case 1: {
                this->set_nose_visible(1, this->field_64, 1, this->field_68);
                this->set_nose_visible(4, this->field_64, 0, this->field_68);
                this->set_nose_visible(0, this->field_64, 0, this->field_68);
                break;
            }
            case 2: {
                this->set_nose_visible(4, this->field_64, 1, this->field_68);
                this->set_nose_visible(0, this->field_64, 0, this->field_68);
                this->set_nose_visible(1, this->field_64, 0, this->field_68);
                break;
            }
            }

            auto v13 = this->field_60;
            switch (v13) {
            case 0: {
                this->set_tail_visible(0, this->field_64, 1, this->field_6C);
                this->set_tail_visible(1, this->field_64, 0, this->field_6C);
                this->set_tail_visible(2, this->field_64, 0, this->field_6C);
                break;
            }
            case 1: {
                this->set_tail_visible(1, this->field_64, 1, this->field_6C);
                this->set_tail_visible(2, this->field_64, 0, this->field_6C);
                this->set_tail_visible(0, this->field_64, 0, this->field_6C);
                break;
            }
            case 2: {
                this->set_tail_visible(2, this->field_64, 1, this->field_6C);
                this->set_tail_visible(0, this->field_64, 0, this->field_6C);
                this->set_tail_visible(1, this->field_64, 0, this->field_6C);
                break;
            }
            }

            this->body->set_render_color(car_colors[0]);
            this->body->ifl_play();
        } else if (this->body != nullptr && this->bodytype == 3) {
            assert(this->body != nullptr && this->bodytype >= VEHICLE_MODEL_CAR);

            this->field_64 = car_colors[func(1)];
            this->field_5C = func(3);
            this->field_60 = func(3);

            auto v15 = this->field_5C;
            if (v15 != 0) {
                if (v15 == 1) {
                    this->set_nose_visible(2, this->field_64, 1, this->field_68);
                    this->set_nose_visible(1, this->field_64, 0, this->field_68);
                    this->set_nose_visible(4, this->field_64, 0, this->field_68);
                } else if (v15 == 2) {
                    this->set_nose_visible(4, this->field_64, 1, this->field_68);
                    this->set_nose_visible(1, this->field_64, 0, this->field_68);
                    this->set_nose_visible(2, this->field_64, 0, this->field_68);
                }
            } else {
                this->set_nose_visible(1, this->field_64, 1, this->field_68);
                this->set_nose_visible(2, this->field_64, 0, this->field_68);
                this->set_nose_visible(4, this->field_64, 0, this->field_68);
            }

            auto v17 = this->field_60;
            if (v17 != 0) {
                if (v17 == 1) {
                    this->set_tail_visible(3, this->field_64, 1, this->field_6C);
                    this->set_tail_visible(1, this->field_64, 0, this->field_6C);
                    this->set_tail_visible(4, this->field_64, 0, this->field_6C);
                } else if (v17 == 2) {
                    this->set_tail_visible(4, this->field_64, 1, this->field_6C);
                    this->set_tail_visible(1, this->field_64, 0, this->field_6C);
                    this->set_tail_visible(3, this->field_64, 0, this->field_6C);
                }
            } else {
                this->set_tail_visible(1, this->field_64, 1, this->field_6C);
                this->set_tail_visible(3, this->field_64, 0, this->field_6C);
                this->set_tail_visible(4, this->field_64, 0, this->field_6C);
            }

            this->body->set_render_color(car_colors[0]);
        } else if (this->body == nullptr || this->bodytype != 0) {
            if (this->body != nullptr) {
                this->body->set_render_color(this->field_64);
            }
        } else {
            this->body->set_render_color(car_colors[0]);
            this->body->ifl_play();

            static string_hash taxi_light_cones_id{to_hash("TAXI_LIGHTCONES")};

            auto *v20 = (conglomerate *)this->get_my_actor();
            auto *body_member = (actor *)v20->get_member(taxi_light_cones_id, true);
            if (body_member != nullptr) {
                body_member->ifl_play();
            }
        }

        auto *v22 = (actor *)this->body;
        if (v22 != nullptr) {
            v22->field_90.field_6 = (v22->field_90.field_6 & 0x3FFF) | ((-1 - (this->field_6C & 3)) << 14);
        }

        this->sub_6BA920(-1);
        this->sub_6D7EA0();
    }
}

vector3d vehicle::get_abs_position()
{
    auto *v2 = this->get_my_actor();
    return v2->get_abs_position();
}

void vehicle::sub_6BA920(int a1)
{
    auto *v3 = this->get_my_actor();
    auto *primary_region = v3->get_primary_region();
    if (primary_region == nullptr) {
        auto abs_pos = this->get_abs_position();
        primary_region = g_world_ptr->the_terrain->find_region(abs_pos, nullptr);
    }

    auto func = [](conglomerate *self, bool a2) -> void {
        self->field_110 = (a2 ? (self->field_110 | 0x4000) : (self->field_110 & 0xFFFFBFFF));
    };

    if (this->field_120) {
        if (a1 == -1) {
            auto *v14 = (actor *)this->field_124;
            if (v14 != nullptr) {
                v14->ifl_lock(0);
            }

            auto *v15 = (actor *)this->field_128;
            if (v15 != nullptr) {
                v15->ifl_lock(0);
            }

            bit_cast<actor *>(this->field_120)->ifl_play();
            auto *v16 = (conglomerate *)this->get_my_actor();
            func(v16, false);
        } else if (a1 != 0) {
            if (a1 == 1) {
                auto *v6 = (actor *)this->field_124;
                if (v6 != nullptr) {
                    v6->ifl_lock(0);
                }

                auto *v7 = (actor *)this->field_128;
                if (v7 != nullptr) {
                    v7->ifl_lock(0);
                }

                bit_cast<actor *>(this->field_120)->ifl_lock(1);
                auto *v8 = (conglomerate *)this->get_my_actor();
                func(v8, false);

                if (primary_region != nullptr) {
                    if (primary_region->is_loaded()) {
                        auto *v9 = (conglomerate *)this->get_my_actor();
                        v9->add_member_lights_to_region(primary_region);
                    }
                }
            }
        } else {
            auto *v10 = (actor *)this->field_124;
            if (v10 != nullptr) {
                v10->ifl_lock(1);
            }

            auto *v11 = (actor *)this->field_128;
            if (v11 != nullptr) {
                v11->ifl_lock(1);
            }

            bit_cast<actor *>(this->field_120)->ifl_lock(0);
            auto *v12 = (conglomerate *)this->get_my_actor();
            func(v12, true);

            if (primary_region != nullptr && primary_region->is_loaded()) {
                auto *v13 = (conglomerate *)this->get_my_actor();
                v13->remove_member_lights_from_region(primary_region);
            }
        }
    }
}

void vehicle::pick_body_and_color(actor *a1)
{
    auto func = [](int a1) -> int {
        if (a1 <= 0) {
            return 0;
        } else {
            return (a1 * rand()) / 32768.0;
        }
    };

    auto idx = func(1);

    auto v1 = car_colors[idx];
    auto v2 = func(3);
    auto v3 = func(3);
    switch (v2) {
    case 0: {
        vehicle::set_nose_visible(a1, 0, v1, 1, 0);
        vehicle::set_nose_visible(a1, 1, v1, 0, 0);
        vehicle::set_nose_visible(a1, 4, v1, 0, 0);
        break;
    }
    case 1: {
        vehicle::set_nose_visible(a1, 1, v1, 1, 0);
        vehicle::set_nose_visible(a1, 0, v1, 0, 0);
        vehicle::set_nose_visible(a1, 4, v1, 0, 0);
        break;
    }
    case 2: {
        vehicle::set_nose_visible(a1, 4, v1, 1, 0);
        vehicle::set_nose_visible(a1, 0, v1, 0, 0);
        vehicle::set_nose_visible(a1, 1, v1, 0, 0);
        break;
    }
    }

    switch (v3) {
    case 0: {
        vehicle::set_tail_visible(a1, 0, v1, 1, 0);
        vehicle::set_tail_visible(a1, 1, v1, 0, 0);
        vehicle::set_tail_visible(a1, 2, v1, 0, 0);
        break;
    }
    case 1: {
        vehicle::set_tail_visible(a1, 1, v1, 1, 0);
        vehicle::set_tail_visible(a1, 0, v1, 0, 0);
        vehicle::set_tail_visible(a1, 2, v1, 0, 0);
        break;
    }
    case 2: {
        vehicle::set_tail_visible(a1, 2, v1, 1, 0);
        vehicle::set_tail_visible(a1, 0, v1, 0, 0);
        vehicle::set_tail_visible(a1, 1, v1, 0, 0);
        break;
    }
    }

    static const string_hash body_id{to_hash("BODY")};

    static const string_hash police_gear_id{int(to_hash("POLICE_GEAR"))};

    sub_6BB1E0(bit_cast<conglomerate *>(a1), body_id, v1, true, true, false);
    sub_6BB1E0(bit_cast<conglomerate *>(a1), police_gear_id, car_colors[0], false, true, false);
}

void vehicle::set_part_visible(string_hash a2, color32 a4, bool a5, bool a6, char a7)
{
    if (this->get_my_actor()->is_a_conglomerate()) {
        auto *v7 = (conglomerate *)this->get_my_actor();
        auto *body_member = (actor *)v7->get_member(a2, true);
        assert(body_member != nullptr && "Can't find the body member of the conglomerate");

        if (body_member != nullptr) {
            body_member->set_visible(a5, true);

            body_member->set_member_hidden(!a5);

            body_member->set_active(a5);

            if (a6) {
                body_member->set_collisions_active(a5, false);
            }

            if (a5) {
                body_member->set_render_alpha_mod(1.0f);
                body_member->set_render_color(a4);
            }

            body_member->field_90.field_6 = (body_member->field_90.field_6 & 0x3FFF) | ((-1 - (a7 & 3)) << 14);
            body_member->ifl_lock(0);
        }
    }
}

void vehicle::set_nose_visible(int idx, color32 a2, bool a4, int a5)
{
    auto bodytype = this->bodytype;
    if (bodytype == 1) {
        auto v7 = a5;
        auto v8 = a4;
        auto v24 = a5;
        this->set_part_visible(s_car_nose_parts[idx][1], a2, a4, true, v24);

        this->set_part_visible(s_car_nose_parts[idx][2], car_colors[0], v8, true, v7);

        this->set_part_visible(s_car_nose_parts[idx][3], a2, v8, true, v7);

        this->set_part_visible(s_car_nose_parts[idx][4], car_colors[0], v8, true, v7);

        this->set_part_visible(s_car_nose_parts[idx][5], car_colors[0], v8, true, v7);

        this->set_part_visible(s_car_nose_parts[idx][0], a2, v8, true, v7);

        if (v8) {
            auto *v16 = (conglomerate *)this->get_my_actor();
            this->field_124 = (actor *)v16->get_member(s_car_nose_parts[idx][5], true);
        }
    } else if (bodytype == 3) {
        auto v17 = a5;
        auto v18 = a4;
        auto v24 = a5;
        this->set_part_visible(s_suv_nose_parts[idx][1], car_colors[0], a4, true, v24);

        this->set_part_visible(s_suv_nose_parts[idx][0], a2, v18, true, v17);
    }
}

void vehicle::set_tail_visible(int a2, color32 a3, bool a4, int a5)
{
    this->set_part_visible(s_tail_parts[a2].second, car_colors[0], a4, true, a5);

    this->set_part_visible(s_tail_parts[a2].first, a3, a4, true, a5);

    if (a4) {
        auto v11 = s_tail_parts[a2].second;
        auto *v13 = (conglomerate *)this->get_my_actor();
        this->field_128 = (actor *)v13->get_member(v11, true);
    }
}

void vehicle::set_tail_visible(actor *a1, int a2, color32 a3, bool a4, int a5)
{
    sub_6BB1E0((conglomerate *)a1, s_tail_parts[a2].second, car_colors[0], a4, true, a5);
    sub_6BB1E0((conglomerate *)a1, s_tail_parts[a2].first, a3, a4, true, a5);
}

void vehicle::set_nose_visible(actor *a1, int a2, color32 a3, bool a4, int a5)
{
    sub_6BB1E0((conglomerate *)a1, s_car_nose_parts[a2][1], a3, a4, true, a5);
    sub_6BB1E0((conglomerate *)a1, s_car_nose_parts[a2][2], car_colors[0], a4, true, a5);
    sub_6BB1E0((conglomerate *)a1, s_car_nose_parts[a2][3], a3, a4, true, a5);
    sub_6BB1E0((conglomerate *)a1, s_car_nose_parts[a2][4], car_colors[0], a4, true, a5);
    sub_6BB1E0((conglomerate *)a1, s_car_nose_parts[a2][5], car_colors[0], a4, true, a5);
    sub_6BB1E0((conglomerate *)a1, s_car_nose_parts[a2][0], a3, a4, true, a5);
}

bool sub_6B9E50(const mString &a3)
{
    auto resource_id = create_resource_key_from_path(a3.c_str(), RESOURCE_KEY_TYPE_ENTITY);

    resource_pack_slot *a4 = nullptr;
    int a2;
    return resource_manager::get_resource(resource_id, &a2, &a4) != nullptr;
}

bool vehicle::add_model(int id, mString a2, Float a3)
{
    if constexpr (0) {
        if (sub_6B9E50(a2)) {
            vehicle::models()[id] = new vehicle_model{id, mString{a2}};
            vehicle::models()[id]->field_18 = a3;
            return true;
        } else {
            return false;
        }
    } else {
        return (bool)CDECL_CALL(0x006B9F60, id, a2, a3);
    }
}

void vehicle::manage_vehicle_height(bool a1)
{
    THISCALL(0x006BAF50, this, a1);
}

void vehicle_patch()
{
    {
        auto *address = &vehicle::pick_random_model;
        REDIRECT(0x006CCE8A, address);
    }

    {
        auto *address = &vehicle::pick_model;
        REDIRECT(0x006CCE92, address);
    }

    {
        FUNC_ADDRESS(address, &vehicle::update_part_cache);
        REDIRECT(0x006C8669, address);
        REDIRECT(0x006D6503, address);
        REDIRECT(0x006D87CA, address);
        REDIRECT(0x006D890B, address);
    }
}
