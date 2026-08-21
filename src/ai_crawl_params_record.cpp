#include "ai_crawl_params_record.h"

#include "als_animation_logic_system.h"
#include "als_inode.h"
#include "common.h"
#include "param_list.h"
#include "trace.h"

namespace ai {

VALIDATE_SIZE(crawl_params_record, 0x2C);

void crawl_params_record::update_crawl_transition_als_params(ai::als_inode *a2)
{
    TRACE("crawl_params_record::update_crawl_transition_als_params");

    als::param_list v10{};
    auto v3 = !this->field_4;

    als::param v5;
    v5.field_0 = 6;
    v5.field_4 = (float)v3;
    v10.add_param(v5);

    als::param v6;
    v6.field_4 = (float)this->field_0;
    v6.field_0 = 7;
    v10.add_param(v6);

    als::param v7;
    v7.field_4 = this->field_8;
    v7.field_0 = 9;
    v10.add_param(v7);

    als::param v8;
    v8.field_4 = this->field_C;
    v8.field_0 = 8;
    v10.add_param(v8);

    v10.add_param(0x2Au, this->field_10);
    v10.add_param(0x27u, this->field_1C);

    als::param v9;
    v9.field_4 = this->field_28;
    v9.field_0 = 51;
    v10.add_param(v9);

    auto *v4 = a2->field_1C->get_als_layer((als::layer_types)0);
    v4->set_desired_params(v10);
}

}  // namespace ai
