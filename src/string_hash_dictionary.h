#pragma once

#include "string_hash_entry.h"

#include "os_file.h"

template<typename T>
struct mAvlTree;

struct ghetto_mash_file_header;

namespace string_hash_dictionary {

    extern mAvlTree<string_hash_entry> *& entries;
    extern mAvlTree<string_hash_entry> *& prereg_entries;

    extern char *& default_dictionary_filename;
    extern char *& dictionary_extension;

    extern char *& file_header_identifier_string;

    extern ghetto_mash_file_header *& header;

    extern uint8_t *& mash_image_buffer;

    extern char *& textfile_extension;

    extern os_file & _hard_log;

    extern char *& hard_log_filename;

    extern bool & is_setup;

    //0x0053DE30
#ifdef TEST_CASE
    [[nodiscard]] extern string_hash register_string(const char *str);
#else
    extern string_hash register_string(const char *str);
#endif

    extern bool is_loaded();

    extern bool exists(uint32_t a1);

    //0x00531990
    extern const char *lookup_string(string_hash a1);

    //0x0053DD00
    extern bool register_in_tree(mAvlTree<string_hash_entry> *a1,
                                 const char *str,
                                 const string_hash &a3);

    //0x005588B0
    extern void create_inst();

    //0x0054C1F0
    extern void delete_inst();

    //0x00547A30
    extern void clear();

    //0x00531A60
    extern void save_dictionary(const char *a1);

    //0x00550DC0
    extern bool read(const char *a1);

    //0x00556000
    extern void load_dictionary(const char *a1);

    //0x0051EB70
    extern void hard_log_string(const char *a1, const string_hash &a2);

    //0x0050DC80
    extern mString figure_out_filename(const char *a2,
                                       const char *dict_filename,
                                       const char *dict_ext);

    //0x0052A5E0
    extern void create_new_dictionary();

    //0x0052A6A0
    extern void write_text(const char *a1);
}

void string_hash_dictionary_patch();
