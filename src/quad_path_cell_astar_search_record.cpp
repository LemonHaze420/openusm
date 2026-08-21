#include "quad_path_cell_astar_search_record.h"

quad_path_cell_astar_search_record::quad_path_cell_astar_search_record()
{
    this->m_vtbl = 0x0087F114;
}

void quad_path_cell_astar_search_record::setup(void *a2, void *a3)
{
    this->field_4 = a3;
    this->m_node_pool = &astar_search_record::default_node_pool();

    this->field_8.clear();

    this->path_goal_to_start = &this->field_24;
    this->field_1C = false;
    this->goal_found = false;
    this->create_or_update_astar_node(a2, 0, 0.0f);
}
