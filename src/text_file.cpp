#include "text_file.h"

#include "filespec.h"
#include "func_wrapper.h"
#include "memory.h"
#include "trace.h"
#include "variables.h"

text_file::text_file() : io(), field_34()
{
    this->buffer = (char *)arch_memalign(32u, 2048u);
    this->bufpos = 0;
    this->bufamt = 0;
    this->opened = false;
    this->field_54 = -1;
}

void text_file::sub_5BC320()
{
    assert(io.is_open());

    this->bufamt = io.read(this->buffer, 2048);
    this->bufpos = 0;
}

void text_file::open(const mString &a1, int mode)
{
    TRACE("text_file::open");

    if constexpr (1) {
        filespec v9{a1};
        //if ( g_is_the_packer()
        //       //|| sub_C01680(a1)
        //        )
        {
            auto v8 = os_file::system_locked;
            os_file::system_locked = false;
            if (mode == os_file::FILE_READ || mode == os_file::FILE_APPEND) {
                auto v4 = v9.fullname();
                io.open(v4, mode);
                this->opened = false;
            } else if (mode == os_file::FILE_WRITE) {
                auto v5 = v9.fullname();
                io.open(v5, mode);
            }

            if (io.is_open() && (mode == 1 || mode == 4)) {
                this->sub_5BC320();
            }


            if (v8) {
                os_file::system_locked = true;
            }
        }

        this->field_34 = {a1.c_str()};
    } else {
        THISCALL(0x005C6AF0, this, &a1, mode);
    }
}

bool text_file::is_open() const
{
    return io.is_open() || this->opened;
}

bool text_file::at_eof() const
{
    return this->bufpos >= this->bufamt && (io.field_15 || opened);
}

char text_file::peek_char() const
{
    if (this->field_54 >= 0) {
        return this->field_54;
    }

    assert(bufpos < bufamt);
    return this->buffer[this->bufpos];
}

char text_file::read_char()
{
    char v2;
    if (this->field_54 < 0) {
        assert(bufpos < bufamt);

        v2 = this->buffer[this->bufpos++];
        if (this->bufpos >= this->bufamt && !this->at_eof()) {
            auto sub_6707CA = [](text_file *file) -> void {
                auto &io = file->io;
                assert(io.is_open());

                file->bufamt = io.read(file->buffer, 2048);
                file->bufpos = 0;
            };
            sub_6707CA(this);
        }
    } else {
        v2 = this->field_54;
        this->field_54 = -1;
    }

    return v2;
}

void text_file::eat_whitespace()
{
    THISCALL(0x005CBD10, this);
}

void text_file::read(char *a2, int maxlen)
{
    if constexpr (1) {
        int v26 = 0;
        auto *cp = a2;
        this->eat_whitespace();
        if (this->at_eof() || this->peek_char() != '\"') {
            while (!this->at_eof()) {
                auto v3 = this->peek_char();
                if (isspace(v3)) {
                    break;
                }

                if (v26 >= maxlen) {
                    *cp = 0;
                    mString v5 = this->get_name() + ": string too long for buffer: " + a2;
                    sp_log(v5.c_str());
                    assert(0);
                }

                *cp++ = this->read_char();
                ++v26;
            }
        } else {
            this->read_char();  // eat first quote
            bool v22 = false;
            while (!this->at_eof() && !v22) {
                auto v24 = this->read_char();
                if (v24 == '\"') {
                    v22 = true;
                } else if (v24 == '\\') {
                    auto v11 = this->peek_char();
                    if (v11 == '\"') {
                        *cp++ = '\"';
                        this->read_char();
                    } else if (v11 == '\\') {
                        *cp++ = '\\';
                        this->read_char();
                    } else {
                        *cp++ = v24;
                    }
                } else {
                    *cp++ = v24;
                }
            }
        }

        *cp = 0;
#if 0
        int v15 = 0;
        v4 = (_WORD *)sub_69745B(v17, &v16, &v15);
        v21 = (char *)sub_66B2E8(a3, v25, *v4);
        if ( v21 != v25 )
        {
            auto v6 = this->get_name() + ": garbage string: " + a3;
            sp_log(v6.c_str());
            assert(0);
        }
#endif

    } else {
        THISCALL(0x005D2DD0, this, a2, maxlen);
    }
}

void text_file::read(int *a2)
{
    THISCALL(0x005D26C0, this, a2);
}

void text_file::read(float *a3)
{
    THISCALL(0x005D2990, this, a3);
}

void text_file::read(mString *a1)
{
    THISCALL(0x005D5970, this, a1);
}

void text_file::write(const mString &str)
{
    this->io.write(str.c_str(), str.size());
}
