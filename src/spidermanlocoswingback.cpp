#include "spidermanlocoswingback.h"

#include "common.h"
#include "dangler.h"
#include "func_wrapper.h"
#include "polytube.h"

namespace ai {

VALIDATE_SIZE(SpidermanLocoSwingBack, 0x10);

SpidermanLocoSwingBack::SpidermanLocoSwingBack() {
    void (__fastcall *func)(SpidermanLocoSwingBack *) = CAST(func, 0x0045D2C0);
    func(this);
}

SpidermanLocoSwingBack::~SpidermanLocoSwingBack() {
    THISCALL(0x00438EC0, this);
}

void SpidermanLocoSwingBack::init(
        polytube *web,
        actor *own,
        entity_base *a4)
{
    assert(web != nullptr && "No web passed to swingback");

    assert(web->get_num_control_pts() >= 2);

    assert(own != nullptr);

    assert(this->web_dangler != nullptr);

    if constexpr (0)
    {}
    else
    {
        THISCALL(0x00481650, this, web, own, a4);
    }
}

} // namespace ai
