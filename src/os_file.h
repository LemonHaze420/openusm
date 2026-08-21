#pragma once

#include "mstring.h"

#include <fstream>

static_assert(sizeof(HANDLE) == sizeof(std::ifstream *));

struct os_file {
    enum mode_flags { FILE_READ = 1, FILE_WRITE = 2, FILE_MODIFY = 3, FILE_APPEND = 4 };

    enum filepos_t { FP_BEGIN, FP_CURRENT, FP_END };

    mString m_path;
    uint32_t flags;
    bool opened;
    bool field_15;

    HANDLE io;

    uint32_t field_1C;
    uint32_t m_fileSize;
    bool field_24;
    uint32_t m_offset;
    uint32_t field_2C;
    uint32_t field_30;

    //0x0058DEA0
    os_file();

    //0x0059EAA0
    explicit os_file(const mString &a2, int dwShareMode);

    //0x0059B6F0
    ~os_file();

    bool is_open() const
    {
        return this->opened;
    }

    void sub_58DEF0();

    size_t sub_58DF50();

    //0x0058DF90
    int get_size();

    //0x0059B740
    void open(const mString &path, int shareMode);

    //0x00598C30
    int write(const void *lpBuffer, int nNumberOfBytesToWrite);

    //0x00598AD0
    int read(LPVOID data, int bytes);

    //0x0058E020
    void set_fp(uint32_t lDistanceToMove, filepos_t a3);

    //0x00598A60
    void close();

    static bool &system_locked;
};

//0x00519F40
extern void add_str(char *Dest, const char *Source, size_t a3);

//0x00519F10
extern void copy_str(char *Dest, const char *Source, size_t Count);

extern void os_file_patch();
