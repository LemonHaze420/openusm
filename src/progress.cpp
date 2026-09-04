#include "progress.h"

#include <cassert>

void progress::start()
{
    assert(m_state == START);

    this->m_state = WORK;
}
