#pragma once

class progress {
    enum state_t {
        START = 0,
        WORK = 1,
        END = 2,
    };

    state_t m_state;

public:
    progress() : m_state(START) {}

    void clear()
    {
        this->m_state = START;
    }

    bool is_started()
    {
        return this->m_state != START;
    }

    bool is_done()
    {
        return this->m_state == END;
    }

    void done()
    {
        this->m_state = END;
    }

    void start();
};
