#include "spidermanlocoswingback.h"

#include "common.h"
#include "dangler.h"
#include "actor.h"
#include "oldmath_po.h"
#include "polytube.h"
#include "wds.h"

#include <cmath>
#include <functional>

namespace ai {

VALIDATE_SIZE(SpidermanLocoSwingBack, 0x10);

SpidermanLocoSwingBack::SpidermanLocoSwingBack()
{
    field_0 = nullptr;
    web_dangler = new dangler;
    field_8 = nullptr;
    field_C = false;
}

SpidermanLocoSwingBack::~SpidermanLocoSwingBack()
{
    delete web_dangler;
    field_C = false;
}

void SpidermanLocoSwingBack::init(polytube *web, actor *own, entity_base *)
{
    assert(web != nullptr && "No web passed to swingback");

    assert(web->get_num_control_pts() >= 2);

    assert(own != nullptr);

    assert(this->web_dangler != nullptr);



    field_0 = own;
    const vector3d start = web->get_control_pt(web->get_num_control_pts() - 1);
    const vector3d end = web->get_control_pt(0);
    vector3d direction = end - start;
    const float squared_length = dot(direction, direction);
    if (squared_length > 9.99999944e-11f)
        direction *= 1.0f / std::sqrt(squared_length);
    entity_set_abs_position(field_8, start + direction * 0.1f);
    vector3d velocity;
    own->get_velocity(&velocity);
    if (web->get_num_control_pts() >= 3)
        web_dangler->init_polytube(web, velocity, 0);
    else
        web_dangler->init_line(start, end, 4, velocity, 0);
    web_dangler->collision_enabled = true;
    field_8->set_render_color(web->get_render_color());
    if (std::not_equal_to<float>{}(field_8->tube_radius, web->tube_radius)) {
        field_8->tube_radius = web->tube_radius;
        field_8->field_78 = false;
    }
    field_8->tiles_per_meter = web->tiles_per_meter;
    field_8->max_length = -1.0f;
    field_8->clear_simulations();
    field_8->the_spline.reserve_control_pts(5);
    if (field_8->num_sides != web->num_sides) {
        field_8->num_sides = web->num_sides;
        field_8->field_78 = false;
    }
    for (const auto &particle : *web_dangler->particles)
        field_8->add_control_pt(field_8->get_abs_po().inverse_xform(particle.position));
    field_C = true;
    field_8->set_visible(true, false);
    field_8->compute_sector(g_world_ptr->the_terrain, false, nullptr);
}

}  // namespace ai
