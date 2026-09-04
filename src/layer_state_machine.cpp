#include "layer_state_machine.h"

#include "layer_state_machine_shared.h"

namespace als {

layer_state_machine::layer_state_machine()
{
    this->m_vtbl = 0x00881538;
}

void layer_state_machine::init(layer_state_machine_shared *shared_machine)
{
    assert(shared_portion == nullptr);
    assert(shared_machine != nullptr);

    this->shared_portion = shared_machine;
    this->field_14.m_active = false;
}

int layer_state_machine::get_domain_bitmask() const
{
    return this->shared_portion->field_40;
}

}  // namespace als
