#include "als_request_data.h"

#include "common.h"
#include "func_wrapper.h"

namespace als {
VALIDATE_SIZE(request_data, 0x14);

request_data::request_data()
{
    clear();
}

request_data::request_data(const request_data &a2)
{
    this->did_transition_occur = a2.did_transition_occur;
    this->do_post_action = a2.do_post_action;
    this->is_trans_to_category = a2.is_trans_to_category;
    this->ignore_no_transition = a2.ignore_no_transition;
    this->post_req_for_category = a2.post_req_for_category;
    this->field_8 = a2.field_8;
    this->field_C = a2.field_C;
}

void request_data::clear()
{
    this->did_transition_occur = false;
    this->do_post_action = false;
    this->is_trans_to_category = false;
    this->ignore_no_transition = false;
    this->post_req_for_category = false;
    this->field_8 = {0};
}

void request_data::operator=(const request_data &a2)
{
    THISCALL(0x004ADF40, this, &a2);
}

}  // namespace als
