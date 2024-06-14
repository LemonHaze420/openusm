#include "chunk_file.h"

#include <cstring>
#include <cassert>

#include "func_wrapper.h"
#include "script_object.h"
#include "script_var_container.h"
#include "variables.h"

chunk_flavor::chunk_flavor(const char *s)
{
    assert(strlen( s ) < CHUNK_FLAVOR_SIZE);
    strcpy(this->field_0, s);
}

void chunk_file::set_fp(int lDistanceToMove, os_file::filepos_t a3)
{
    if ( g_is_the_packer() ) {
        this->io.set_fp(lDistanceToMove, a3);
    }
}

void chunk_file::close()
{
    if ( !this->opened ) {
        this->io.close();
    }
}

bool chunk_flavor::operator==(const chunk_flavor &a2) const {
    return strcmp(this->field_0, a2.field_0) == 0;
}

void chunk_file::open(const mString &a1, int a3)
{
    if constexpr(1) {
        static_cast<text_file *>(this)->open(a1, a3);
    } else {
        THISCALL(0x005CBB90, this, &a1, a3);
    }
}

void chunk_file::read(script_object *so) {
    script_object::read(this, so);
}

void chunk_file::read(script_var_container *a1) {
    script_var_container::read(this, a1);
}

template<>
chunk_flavor chunk_file::read() {
    char str[16]{};
    static_cast<text_file *>(this)->read(str, 16);
    printf("%s\n", str);

    return chunk_flavor {str};
}

template<>
int chunk_file::read()
{
    int a1;
    static_cast<text_file *>(this)->read(&a1);
    return a1;
}

template<>
float chunk_file::read()
{
    float a1;
    static_cast<text_file *>(this)->read(&a1);
    return a1;
}

template<>
bool chunk_file::read()
{
    auto v3 = this->read<int>();
    return (v3 != 0);
}

template<>
mString chunk_file::read()
{
    mString result;
    auto *v2 = static_cast<text_file *>(this);
    v2->read(&result);
    return result;
}


template<>
uint32_t chunk_file::read()
{
    uint32_t result = this->read<int>();
    return result;
}

template<>
void chunk_file::write(const mString &str) {
    static_cast<text_file *>(this)->write(str + "\n");
}

template<>
void chunk_file::write(const chunk_flavor &d) {
    this->write(mString {d.c_str()});
}

template<>
void chunk_file::write(const int &d) {
    this->write(mString {d});
}

template<>
void chunk_file::write(const uint32_t &d) {
    this->write(mString {static_cast<int>(d)});
}
