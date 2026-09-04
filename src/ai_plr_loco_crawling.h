#pragma once

struct line_info;
struct actor;

namespace ai {
struct crawl_params_record;
struct als_inode;
}  // namespace ai

extern line_info *check_interior_transition(actor *a1, ai::crawl_params_record &arg4, ai::als_inode *a3, bool argC,
                                            bool a5, bool a6);

extern line_info *check_exterior_transition(actor *a1, ai::crawl_params_record &a2, ai::als_inode *a3, bool a4, bool a5,
                                            bool a6);
