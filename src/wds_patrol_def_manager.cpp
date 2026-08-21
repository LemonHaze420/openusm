#include "wds_patrol_def_manager.h"

#include "common.h"
#include "patrol_def.h"
#include "patrol_def_set.h"

#include <cassert>

VALIDATE_SIZE(wds_patrol_def_manager, 0x10u);

patrol_def *wds_patrol_def_manager::get_patrol_def(const string_hash &a2) const
{
    for (auto &def : this->field_0) {
        auto v4 = def->get_id_hash();
        if (v4 == a2) {
            return def;
        }
    }

    return nullptr;
}

void wds_patrol_def_manager::add_patrol_defs_from_set(patrol_def_set *the_set)
{
    for (int i = 0; i < the_set->field_0.size(); ++i) {
        auto *def = the_set->field_0.at(i);
        assert(def != nullptr);

        assert(this->get_patrol_def(def->get_id_hash()) == nullptr && "patrol def name collision");

        this->field_0.push_back(def);
    }
}

void wds_patrol_def_manager::remove_patrol_defs_from_set(patrol_def_set *the_set)
{
    for (int i = 0; i < the_set->field_0.size(); ++i) {
        auto *def = the_set->field_0.at(i);
        assert(def != nullptr);

        auto it = std::find(this->field_0.begin(), this->field_0.end(), def);
        if (it != this->field_0.end()) {
            this->field_0.erase(it);
        }
    }
}
