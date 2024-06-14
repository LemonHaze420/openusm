#pragma once

#include "text_file.h"

inline constexpr auto CHUNK_FLAVOR_SIZE = 16u;

struct script_object;
struct script_var_container;

struct chunk_flavor {
    char field_0[CHUNK_FLAVOR_SIZE];

    chunk_flavor() = default;

    chunk_flavor(const char *s);

    bool operator==(const chunk_flavor &a2) const;

    bool operator!=(const chunk_flavor &a2) const {
        return (!((*this) == a2));
    }

    const char *c_str() const {
        return field_0;
    }
};

inline const chunk_flavor SCRIPT_MANAGER_VAR_HEADER_CHUNK {"vhdr"};

inline const chunk_flavor CHUNK_END {"chunkend"};

inline const chunk_flavor CHUNK_SCRIPT_OBJECT {"scrobj"};

inline const chunk_flavor CHUNK_SCRIPT_OBJECTS {"scrobjs"};

inline const chunk_flavor CHUNK_VM_EXECUTABLE {"xecutabl"};

inline const chunk_flavor CHUNK_PARMS_NAME {"parmname"};

inline const chunk_flavor CHUNK_PARMS_STACKSIZE {"parmsize"};

inline const chunk_flavor CHUNK_PARMS {"parms"};

inline const chunk_flavor CHUNK_CODESIZE {"codesiz"};

inline const chunk_flavor CHUNK_CODE {"code"};

inline const chunk_flavor CHUNK_FUNCS {"Nfuncs"};

inline const chunk_flavor CHUNK_DATA_BLOCKSIZE {"datasize"};

inline const chunk_flavor CHUNK_EXTERNAL {"external"};

inline const chunk_flavor CHUNK_GLOBAL {"global"};

inline const chunk_flavor CHUNK_STANDARD {"standard"};

inline const chunk_flavor CHUNK_PARENT {"parent"};

inline const chunk_flavor CHUNK_NSTATIC {"Nstatic"};

inline const chunk_flavor CHUNK_STAT_INIT {"statinit"};

inline const chunk_flavor CHUNK_GAME_INIT {"gameinit"};

inline const chunk_flavor CHUNK_STATIC_BLOCKSIZE {"statsize"};

inline const chunk_flavor CHUNK_NDATA {"Ndata"};

inline const chunk_flavor CHUNK_NULL {"null"};

struct chunk_file : text_file {

    //0x005CBB90
    void open(const mString &a1, int a3);

    void set_fp(int lDistanceToMove, os_file::filepos_t a3);

    void close();

    void read(script_object *so);

    void read(script_var_container *a1);

    template<typename T>
    T read();

    template<typename T>
    void write(const T &arg);
};

