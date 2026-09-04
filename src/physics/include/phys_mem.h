#pragma once

struct phys_memory_heap {
    char *m_buffer_start{nullptr};
    char *m_buffer_end{nullptr};
    char *m_buffer_cur{nullptr};

    void init(void *start, int size, int alignment);

    void *allocate(int size, int align);

    void *allocate_internal(int size, int align);
};
