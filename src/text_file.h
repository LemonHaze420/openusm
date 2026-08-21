#pragma once

#include "os_file.h"

struct text_file {
    os_file io;
    mString field_34;
    char *buffer;
    int bufpos;
    int bufamt;
    bool opened;
    int field_54;

    //0x005BC240
    text_file();

    auto get_name() const
    {
        return field_34;
    }

    //0x005C6AF0
    void open(const mString &a2, int dwShareMode);

    //0x005BC4B0
    bool is_open() const;

    bool at_eof() const;

    char peek_char() const;

    char read_char();

    //0x005CBD10
    void eat_whitespace();

    void sub_5BC320();

    //0x005D2DD0
    void read(char *a2, int a3);

    //0x005D26C0
    void read(int *a2);

    //0x005D2990
    void read(float *a3);

    //0x005D5970
    void read(mString *a1);

    void write(const mString &str);
};
