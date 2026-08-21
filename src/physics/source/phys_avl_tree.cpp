#include "phys_avl_tree.h"

#include "rbc_def_contact.h"
#include "rigid_body_pair_key.h"
#include "func_wrapper.h"

template <>
int phys_avl_tree<rigid_body_pair_key, rigid_body_constraint_contact>::add_recurse(
    phys_avl_tree<rigid_body_pair_key, rigid_body_constraint_contact>::avl_tree_node *&root)
{
    return THISCALL(0x007A2F40, this, &root);
}
