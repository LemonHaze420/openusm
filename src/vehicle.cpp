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
#include "physical_interface.h"
#include "vtbl.h"

#include <numeric>
#include <cmath>
#include <cstdio>
#include <algorithm>

VALIDATE_SIZE(vehicle, 0x130);

VALIDATE_OFFSET(vehicle_model, refcount, 0x14);
VALIDATE_SIZE(vehicle_model, 0x1C);


static const color32 car_colors[] = {
    0xFFFFFFFFu,
    0xFF323232u,
    0xFF4B7BA8u,
    0xFFD23C00u,
    0xFF719B56u,
    0xFFDCB469u,
    0xFF73AF64u,
    0xFFC8FFBEu,
    0xFF3250A0u,
    0xFF64503Cu,
    0xFFA00000u,
    0xFF9B7D5Au,
    0xFF874646u,
};
static const std::pair<string_hash, string_hash> s_tail_parts[5] = {
    {int(to_hash("T1")), int(to_hash("T1_XTRAS_NOTINT"))},
    {int(to_hash("T2")), int(to_hash("T2_XTRAS_NOTINT"))},
    {int(to_hash("T3")), int(to_hash("T3_XTRAS_NOTINT"))},
    {int(to_hash("T4")), int(to_hash("T4_XTRAS_NOTINT"))},
    {int(to_hash("T5")), int(to_hash("T5_XTRAS_NOTINT"))},
};
static const string_hash s_car_nose_parts[5][6] = {
    {int(to_hash("N1")),
     int(to_hash("N1_D")),
     int(to_hash("N1_D_WINDOW_NOTINT")),
     int(to_hash("N1_P")),
     int(to_hash("N1_P_WINDOW_NOTINT")),
     int(to_hash("N1_XTRAS_NOTINT"))},
    {int(to_hash("N2")),
     int(to_hash("N2_D")),
     int(to_hash("N2_D_WINDOW_NOTINT")),
     int(to_hash("N2_P")),
     int(to_hash("N2_P_WINDOW_NOTINT")),
     int(to_hash("N2_XTRAS_NOTINT"))},
    {int(to_hash("N3")),
     int(to_hash("N3_D")),
     int(to_hash("N3_D_WINDOW_NOTINT")),
     int(to_hash("N3_P")),
     int(to_hash("N3_P_WINDOW_NOTINT")),
     int(to_hash("N3_XTRAS_NOTINT"))},
    {int(to_hash("N4")),
     int(to_hash("N4_D")),
     int(to_hash("N4_D_WINDOW_NOTINT")),
     int(to_hash("N4_P")),
     int(to_hash("N4_P_WINDOW_NOTINT")),
     int(to_hash("N4_XTRAS_NOTINT"))},
    {int(to_hash("N5")),
     int(to_hash("N5_D")),
     int(to_hash("N5_D_WINDOW_NOTINT")),
     int(to_hash("N5_P")),
     int(to_hash("N5_P_WINDOW_NOTINT")),
     int(to_hash("N5_XTRAS_NOTINT"))},
};
static const string_hash s_suv_nose_parts[5][2] = {
    {int(to_hash("N1")), int(to_hash("N1_XTRAS_NOTINT"))},
    {int(to_hash("N2")), int(to_hash("N2_XTRAS_NOTINT"))},
    {int(to_hash("N3")), int(to_hash("N3_XTRAS_NOTINT"))},
    {int(to_hash("N4")), int(to_hash("N4_XTRAS_NOTINT"))},
    {int(to_hash("N5")), int(to_hash("N3_XTRAS_NOTINT"))},
};

Var<vehicle_model *[VEHICLE_MODEL_MAX]> vehicle::models {
    0x0096C97C
};

int &vehicle::cur_vehicle_type = var<int>(0x00937FCC);

#if STANDALONE_SYSTEM
static const bool native_vehicle_defaults = [] {
    vehicle::cur_vehicle_type = -1;
    return true;
}();
#endif

namespace {
actor *__fastcall native_vehicle_actor(vehicle *self, void *)
{
    return self->get_my_actor();
}
void __fastcall native_vehicle_reset(vehicle *self, void *)
{
    self->reset();
}
void __fastcall native_vehicle_set_actor(vehicle *self, void *, vhandle_type<entity> handle)
{
    self->set_actor(handle);
}

void __fastcall native_vehicle_out_of_world(vehicle *, void *) {}
}  // namespace

void *vehicle::native_vtable()
{
    static void *table[] = {
        reinterpret_cast<void *>(&native_vehicle_actor),
        reinterpret_cast<void *>(&native_vehicle_reset),
        reinterpret_cast<void *>(&native_vehicle_set_actor),
        reinterpret_cast<void *>(&native_vehicle_out_of_world),
    };
    return table;
}

void vehicle::set_actor(vhandle_type<entity> handle)
{
    field_50 = handle;
    field_54 = static_cast<actor *>(field_50.get_volatile_ptr());
}

vehicle::~vehicle()
{
    m_vtbl = reinterpret_cast<std::intptr_t>(native_vtable());
    use_model(-1, 0);
}

entity *vehicle::use_model(int model, int instance_id)
{
    const auto assign = [this](vhandle_type<entity> handle) {
        auto function =
            reinterpret_cast<void(__fastcall *)(vehicle *, void *, vhandle_type<entity>)>(get_vfunc(m_vtbl, 8));
        function(this, nullptr, handle);
    };
    if (field_50.get_volatile_ptr()) {
        auto *old_model = models()[bodytype];
        if (old_model) {
            --old_model->refcount;
            g_world_ptr->ent_mgr.release_entity(field_50.get_volatile_ptr());
        }
        assign(vhandle_type<entity>{0});
        bodytype = static_cast<uint32_t>(-1);
    }
    vhandle_type<entity> handle{0};
    if (static_cast<unsigned>(model) < VEHICLE_MODEL_MAX && models()[model]) {
        cur_vehicle_type = model;
        handle = vhandle_type<entity>{models()[model]->create(instance_id)};
        cur_vehicle_type = -1;
    }
    auto *result = handle.get_volatile_ptr();
    if (result) {
        assign(handle);
        bodytype = model;
    }
    return result;
}

vehicle::vehicle(vhandle_type<entity> a1)
{
    m_vtbl = reinterpret_cast<std::intptr_t>(native_vtable());
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
    field_BC = YVEC;
    field_C8 = field_CC = field_D0 = field_F0 = field_E0 = field_E8 = field_EC = 0.0f;
    field_DC = 0;
    field_F8 = 4;
    field_B0 = ZEROVEC;
    field_F4 = 1.25f;
    set_collidable(false);
    if (auto *owner = get_my_actor())
        owner->set_visible(false, false);
    field_12C = field_12D = false;
    field_5C = field_60 = 0;
    field_64 = car_colors[0];
    update_part_cache();
    set_damage_level(0, 0);
    set_damage_level(0, 1);
    sub_6D7EA0();
}

void vehicle::update_part_cache()
{
    auto *owner = static_cast<conglomerate *>(get_my_actor());
    if (!owner) {
        dftire = pftire = drtire = prtire = body = nullptr;
        field_110 = field_114 = field_118 = field_11C = nullptr;
        field_120 = field_124 = field_128 = nullptr;
        return;
    }
    const auto member = [owner](const char *name) {
        return static_cast<actor *>(owner->get_member(string_hash(name), true));
    };
    dftire = member("DF");
    pftire = member("PF");
    drtire = member("DR");
    prtire = member("PR");
    body = member("BODY");
    field_110 = member("POLICE_GEAR");
    field_114 = member("SHADOW");
    field_118 = member("TAXI");
    field_11C = member("HANDLEBARS");
    if (bodytype == 1)
        field_120 = body;
    else if (bodytype != 0)
        field_120 = nullptr;
    else {
        field_120 = member("TAXI_LIGHTCONES");
        field_124 = field_128 = body;
    }
}

static const char *const off_937FD4[6] = {"FW1", "FB1", "FW2", "MB1", "MW1", "MW2"};

void vehicle::sub_6D7EA0()
{
    auto *owner = get_my_actor();
    if (owner && owner->get_ai_core()) {
        auto *voice = static_cast<ai::voice_box_inode *>(
            owner->get_ai_core()->get_info_node(ai::voice_box_inode::default_id, false));
        if (voice) {
            const unsigned index = static_cast<unsigned>(rand() * (6.0 / 32768.0));
            voice->sub_6D7E10(off_937FD4[index]);
        }
    }
}

vector3d vehicle::sub_6DA250()
{
    auto *v2 = this->get_my_actor();
    auto result = v2->get_abs_position();
    return result;
}

void vehicle::set_damage_level(int level, int end)
{
    level = std::max(0, std::min(level, 3));
    if (end == 0)
        field_68 = level;
    else
        field_6C = level;
    if (!body)
        return;
    if (bodytype == 1 || bodytype == 3) {
        const int variant = end ? field_60 : field_5C;
        if (variant < 0 || variant > 2)
            return;
        if (end) {
            static const int car_parts[] = {0, 1, 2};
            static const int suv_parts[] = {1, 3, 4};
            set_tail_visible((bodytype == 1 ? car_parts : suv_parts)[variant], field_64, true, field_6C);
        } else {
            static const int car_parts[] = {0, 1, 4};
            static const int suv_parts[] = {1, 2, 4};
            set_nose_visible((bodytype == 1 ? car_parts : suv_parts)[variant], field_64, true, field_68);
        }
        return;
    }
    const auto lock_damage_frame = [this](actor *part) {
        part->field_90.field_6 = (part->field_90.field_6 & 0x3FFF) | ((-1 - (field_6C & 3)) << 14);
    };
    lock_damage_frame(body);
    if (bodytype == 6) {
        auto *owner = static_cast<conglomerate *>(get_my_actor());
        lock_damage_frame(static_cast<actor *>(owner->get_member(string_hash("DL_DOOR"), true)));
        lock_damage_frame(static_cast<actor *>(owner->get_member(string_hash("PF_DOOR"), true)));
    }
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
    for (auto &model : models()) {
        delete model;
        model = nullptr;
    }
    return true;
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

entity_base_vhandle vehicle_model::create(int instance_id)
{
    char name[32];
    std::snprintf(name, sizeof(name), "VEHICLE_%u", static_cast<unsigned>(instance_id));
    const auto resource = create_resource_key_from_path(field_4.c_str(), RESOURCE_KEY_TYPE_NONE);
    auto *owner = g_world_ptr->ent_mgr.acquire_entity(resource.m_hash, string_hash(name), 129);
    if (!owner || !owner->is_an_entity())
        return entity_base_vhandle{0};
    po placement;
    placement.set_po(ZVEC, YVEC, ZEROVEC);
    entity_set_abs_po(owner, placement);
    if (owner->has_physical_ifc()) {
        owner->physical_ifc()->set_allow_manage_standing(false);
        owner->physical_ifc()->enable(false);
    }
    ++refcount;
    return owner->my_handle;
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
    float total = 0.0f;
    for (const auto *model : models())
        if (model)
            total += model->field_18;
    if (total < EPSILON)
        return 0;
    double position = std::rand() * static_cast<double>(1.0f / RAND_MAX) * total;
    for (int index = 0; index < VEHICLE_MODEL_MAX; ++index) {
        if (auto *model = models()[index]) {
            if (position < model->field_18)
                return index;
            position -= model->field_18;
        }
    }
    return 0;
}

int vehicle::pick_model(int instance_id)
{
    double total = 0.0;
    for (const auto *model : models())
        if (model)
            total += model->field_18;
    if (total < EPSILON)
        return 0;
    double position = total * (static_cast<double>(instance_id) * (1.0f / 30.0f));
    for (int index = 0; index < VEHICLE_MODEL_MAX; ++index) {
        if (auto *model = models()[index]) {
            if (position < model->field_18)
                return index;
            position -= model->field_18;
        }
    }
    return 0;
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

bool vehicle::add_model(int id, mString path, Float usage)
{
    if (!sub_6B9E50(path))
        return false;
    models()[id] = new vehicle_model{id, path};
    models()[id]->field_18 = usage;
    return true;
}

void vehicle::manage_vehicle_height(bool)
{
    auto *owner = get_my_actor();
    float floor = owner->get_floor_offset();
    if (floor < EPSILON)
        floor = 1.0f;
    vector3d probe = owner->get_rel_position();
    const vector3d forward = owner->get_abs_po().get_z_facing();
    vector3d normal = field_BC;
    entity *hit_entity = nullptr;
    subdivision_node_obb_base *hit_obb = nullptr;
    auto *ground = g_world_ptr->the_terrain;
    float elevation;
    const auto out_of_world = [this] {
        auto callback = reinterpret_cast<void(__fastcall *)(vehicle *, void *)>(get_vfunc(m_vtbl, 0xC));
        callback(this, nullptr);
    };
    if (ground && ground->find_region(probe, nullptr)) {
        elevation = ground->get_elevation(probe, normal, owner, &hit_entity, &hit_obb, -1.0f);
        if (dot(normal, YVEC) > 0.75f && !is_colinear(forward, normal, 0.01f))
            field_BC = normal;
        if (elevation < -100.0f) {
            out_of_world();
            return;
        }
    } else {
        out_of_world();
        elevation = 0.0f;
    }
    vector3d position = get_my_actor()->get_abs_position();
    position.y = elevation + floor;
    po transform;
    transform.set_po(forward, field_BC, position);
    entity_set_abs_po(get_my_actor(), transform);
}

namespace {

float vehicle_steering_sine(float angle)
{
    const float phase = -std::fabs(angle + 4.71238899230957f) * 0.15915493667125702f;
    const float t = std::fabs(std::ceil(phase) - phase - 0.5f) - 0.25f;
    const float t2 = t * t;
    const float t3 = t2 * t;
    const float t4 = t2 * t2;
    const float t5 = t4 * t;
    float result = t5 * t4 * 39.71065902709961f;
    result += t3 * t4 * -76.57495880126953f;
    result += t5 * 81.60222625732422f;
    result += t3 * -41.3416748046875f;
    return result + t * 6.283185005187988f;
}

void rotate_vehicle_pose(po &transform, int first, int second, float angle)
{
    const float sine = std::sin(angle);
    const float cosine = std::cos(angle);
    for (int row = 0; row != 3; ++row) {
        const float old_first = transform[row][first];
        const float old_second = transform[row][second];
        transform[row][first] = old_first * cosine - old_second * sine;
        transform[row][second] = old_second * cosine + old_first * sine;
    }
}


void set_vehicle_tire_pose(actor *part, float steering, float rotation)
{
    po transform;
    transform.set_po(ZVEC, YVEC, part->get_rel_position());
    if (rotation > EPSILON)
        rotate_vehicle_pose(transform, 1, 2, rotation);
    if (std::fabs(steering) > EPSILON)
        rotate_vehicle_pose(transform, 0, 2, steering);
    part->set_abs_po(transform);
}

void set_vehicle_body_pose(actor *part, float pitch, float roll)
{
    po transform;
    transform.set_po(ZVEC, YVEC, part->get_rel_position());
    if (!(pitch <= 0.0f && pitch >= 0.0f))
        rotate_vehicle_pose(transform, 1, 2, pitch);
    if (!(roll <= 0.0f && roll >= 0.0f))
        rotate_vehicle_pose(transform, 0, 1, roll);
    part->set_abs_po(transform);
}

void finish_vehicle_motion(actor *owner, const vector3d &displacement, Float dt)
{
    owner->set_frame_delta_trans(displacement, dt);
    auto radius = reinterpret_cast<float(__fastcall *)(actor *, void *)>(get_vfunc(owner->m_vtbl, 0x28));
    if (radius(owner, nullptr) > 0.0f)
        static_cast<conglomerate *>(owner)->field_110 &= ~1u;
}
}  // namespace

bool vehicle::is_grounded() const
{
    return std::fabs(field_CC) < 0.05f && std::fabs(field_D0) < 0.05f;
}

vector3d vehicle::get_up_direction()
{
    return get_my_actor()->get_abs_po().get_y_facing();
}

vector3d vehicle::get_forward_direction()
{
    return get_my_actor()->get_abs_po().get_z_facing();
}

void vehicle::drive(Float time, float throttle, float steering, bool traction, bool animate_parts, bool dynamics)
{
    const float dt = time;
    if (bodytype == 2)
        traction = false;
    if (field_E8 <= EPSILON)
        determine_wheel_base();
    if (field_EC <= EPSILON)
        determine_tire_radius();
    const float old_speed = field_C8;
    const float acceleration = dt * 30.0f;
    const float braking = dt * 37.5f;
    if (throttle > 0.05f) {
        field_C8 = std::min(field_C8 + (field_C8 > -1.0f ? acceleration : braking) * throttle, 50.0f);
        if (old_speed < 0.0f && field_C8 > 0.0f)
            field_C8 = 0.0f;
    } else if (-throttle > 0.05f) {
        field_C8 = std::max(field_C8 + braking * throttle, -50.0f);
        if (old_speed > 0.0f && field_C8 < 0.0f)
            field_C8 = 0.0f;
    }
    bool body_changed = false;
    if (dynamics) {
        const float pitch =
            static_cast<int>((old_speed - field_C8) / acceleration * 10.0f) * 0.1f * 0.0872664675116539f;
        const float roll = static_cast<int>(field_C8 * 0.02f * 10.0f * steering * 2.0f) * 0.1f * 0.2617993950843811f;
        const auto approach = [](float current, float target, float amount) {
            return target > current ? std::min(current + amount, target) : std::max(current - amount, target);
        };
        const float next_pitch = approach(field_CC, pitch, dt * 0.39269909262657166f);
        const float next_roll = approach(field_D0, roll, dt * 0.6544985175132751f);
        body_changed =
            !(field_CC <= next_pitch && field_CC >= next_pitch) || !(field_D0 <= next_roll && field_D0 >= next_roll);
        field_CC = next_pitch;
        field_D0 = next_roll;
    } else {
        field_CC = field_D0 = 0.0f;
    }
    const float distance = dt * field_C8;
    auto *owner = get_my_actor();
    vector3d forward = owner->get_abs_po().get_z_facing();
    const float wheel_steer = steering * -0.6981317400932312f;
    const float yaw_rate = field_C8 / (field_E8 / vehicle_steering_sine(wheel_steer));
    bool tires_changed = false;
    if (dynamics) {
        tires_changed = !(field_D8 <= wheel_steer && field_D8 >= wheel_steer);
        field_DC += distance / field_EC;
        if (field_DC >= 6.2831854820251465f)
            field_DC -= 6.2831854820251465f;
        tires_changed = tires_changed || !(field_D4 <= field_DC && field_D4 >= field_DC);
    } else {
        field_DC = 0.0f;
    }
    field_D4 = field_DC;
    field_D8 = wheel_steer;
    if (animate_parts) {
        if (tires_changed) {
            const float angle = std::max(-0.6981317400932312f, std::min(wheel_steer, 0.6981317400932312f));
            set_vehicle_tire_pose(dftire, angle, field_DC);
            set_vehicle_tire_pose(pftire, angle, field_DC);
            set_vehicle_tire_pose(drtire, 0.0f, field_DC);
            set_vehicle_tire_pose(prtire, 0.0f, field_DC);
        }
        if (body && body_changed)
            set_vehicle_body_pose(body, field_CC, field_D0);
    }
    const float yaw = yaw_rate * dt;
    const bool yaw_changed = !(field_E4 <= yaw && field_E4 >= yaw);
    field_E4 = yaw;
    const vector3d displacement = forward * distance;
    const float braking_skid = std::fabs(throttle) > 0.75f ? (-throttle - 0.75f) * 4.0f : 0.0f;
    if (traction && std::fabs(field_C8) > 1.0f) {
        const float skid = steering * steering * field_F4 * field_C8 * 0.02f;
        const float blend = std::max(0.0f, std::min(1.0f - skid, 1.0f)) * 0.95f;
        field_B0 = displacement * blend + field_B0 * (1.0f - blend);
        if (field_F4 > 10.0f)
            field_B0 *= 0.9f;
        field_C8 = dot(forward, field_B0) / dt;
        field_18 = std::max(braking_skid, skid);
    } else {
        field_B0 = displacement;
        field_18 = 0.0f;
    }
    const vector3d position = owner->get_abs_position() + field_B0;
    if (yaw_changed) {
        rotate_vehicle_pose(owner->get_rel_po(), 0, 2, yaw);
        owner->dirty_family(false);
    }
    forward = owner->get_abs_po().get_z_facing();
    const vector3d right = vector3d::cross(field_BC, forward);
    forward = vector3d::cross(right, field_BC);
    po transform;
    transform.set_po(forward, field_BC, position);
    if (!(field_70 == transform))
        entity_set_abs_po(owner, transform);
    field_70 = transform;
    finish_vehicle_motion(owner, field_B0, time);
    field_6 = true;
    field_C = field_10;
    field_10 = field_C8 * 0.02f;
    field_14 = std::max(0.0f, std::min(std::fabs(steering), 1.0f));
    audio_advance(time);
    if (old_speed > 0.5f && field_C8 <= 0.5f)
        play_stopped_sound();
}

void vehicle::drive_to(Float dt, float speed, const vector3d &target, bool, bool filtered)
{
    field_C8 = filtered ? (field_C8 * 3.0f + speed) * 0.25f : speed;
    const float distance = static_cast<float>(dt) * field_C8;
    auto *owner = get_my_actor();
    vector3d forward = owner->get_abs_po().get_z_facing();
    const vector3d position = owner->get_abs_position();
    vector3d direction = target - position;
    if (direction.x * direction.x + direction.z * direction.z >= LARGE_EPSILON) {
        direction.normalize();
        forward = filtered ? forward + (direction - forward) * 0.25f : direction;
    }
    po transform;
    transform.set_po(forward, field_BC, position + forward * distance);
    if (!(field_70 == transform))
        entity_set_abs_po(owner, transform);

    finish_vehicle_motion(owner, field_B0, dt);
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
