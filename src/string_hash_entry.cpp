#include "string_hash_entry.h"

#include "string_hash.h"

#include "common.h"
#include "func_wrapper.h"
#include "mash_info_struct.h"

VALIDATE_SIZE(string_hash_entry, 20u);

string_hash_entry::string_hash_entry(const char *a2, const string_hash *a3)
{
    this->field_0.source_hash_code = 0;

    this->field_4 = mString{};

    if (a2 != nullptr) {
        this->field_4 = a2;
    }

    if (a3 != nullptr) {
        this->field_0 = *a3;
    }
}

mString string_hash_entry::sub_50DBC0(const char *a3) {
    if constexpr (1) {
        mString a1{0, "0x%08x\t%s%s", this->field_0.source_hash_code, this->field_4.c_str(), a3};

        return a1;
    } else {
        mString result;
        THISCALL(0x0050DBC0, this, &result, a3);

        return result;
    }
}

void string_hash_entry::unmash(mash_info_struct *a1, void *a2)
{
    this->custom_unmash(a1, a2);
}

void string_hash_entry::custom_unmash(mash_info_struct *a2, void *)
{
    a2->unmash_class_in_place(this->field_0, this);
    a2->unmash_class_in_place(this->field_4, this);
}
