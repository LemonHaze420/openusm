#include "femenuentry.h"

#include "common.h"
#include "femultilinetext.h"
#include "func_wrapper.h"
#include "utility.h"

VALIDATE_SIZE(FEMenuEntry, 0x18);

FEMenuEntry::FEMenuEntry(FEMenu *a1, FEText *a2, bool a3) {
    this->m_vtbl = 0x00894F60;
    this->field_15 = false;
    this->field_16 = false;
    this->field_17 = false;
    this->CommonConstructor(a2, a1);

    this->field_17 = a3;
}

FEMenuEntry::FEMenuEntry(global_text_enum a3, FEMenu *arg4, bool a11, font_index a2, int a6) {
    void (__fastcall *func)(FEMenuEntry *, void *edx, global_text_enum a3, FEMenu *arg4, bool a11, font_index a2, int a6) = CAST(func, 0x00629650);

    func(this, nullptr, a3, arg4, a11, a2, a6);

}

void FEMenuEntry::Update(Float a2) {
    this->field_10->Update(a2);
}

bool FEMenuEntry::GetDisable() {
    return this->field_16;
}

void FEMenuEntry::SetPos(Float a2, Float a3) {
    this->field_10->SetPos(a2, a3);
}

void FEMenuEntry::CommonConstructor(FEText *a2, FEMenu *a3) {
    THISCALL(0x0060A840, this, a2, a3);
}

void FEMenuEntry::SetText(global_text_enum a2) {
    this->field_10->SetText(a2);
}

void FEMenuEntry::SetTextNoLocalize(FEMenuEntry::string a2) {
    mString v3{a2.guts};
    this->field_10->SetTextNoLocalize(*bit_cast<FEText::string *>(&v3));
}

void FEMenuEntry::Highlight(bool a2, bool a3) {
    THISCALL(0x0060A8A0, this, a2, a3);
}

void FEMenuEntry_patch() {
    {
        FUNC_ADDRESS(address, &FEMenuEntry::SetText);
        SET_JUMP(0x00649800, address);
    }

    {
        FUNC_ADDRESS(address, &FEMenuEntry::SetTextNoLocalize);
        SET_JUMP(0x00649810, address);
    }
}
