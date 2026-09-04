#include "panelanim.h"

#include "common.h"
#include "panelfile.h"
#include "panelanimkeyframe.h"
#include "trace.h"

VALIDATE_SIZE(PanelAnim, 0x1C);

PanelAnim::PanelAnim() {}

void PanelAnim::PostUnmashFixup(PanelFile *pf)
{
    TRACE("PanelAnim::PostUnmashFixup");

    auto the_index = (int)this->field_14;
    assert(the_index >= 0);

    if (this->field_18) {
        assert(the_index < pf->pquads.size());
        this->field_14 = pf->pquads.m_data[the_index];
    } else {
        assert(the_index < pf->ptext.size());
        this->field_14 = CAST(this->field_14, pf->ptext.m_data[the_index]);
    }
}
