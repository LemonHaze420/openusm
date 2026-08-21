#include "string_hash_dictionary.h"

#include "config.h"
#include "func_wrapper.h"
#include "ghetto_mash_file_header.h"
#include "log.h"
#include "trace.h"
#include "mAvlTree.h"
#include "mash_info_struct.h"
#include "memory.h"
#include "mstring.h"
#include "osassert.h"
#include "os_developer_options.h"
#include "variables.h"

#include <cassert>

#if !STANDALONE_SYSTEM
mAvlTree<string_hash_entry> *&string_hash_dictionary::entries = var<mAvlTree<string_hash_entry> *>(0x0095C7E0);
mAvlTree<string_hash_entry> *&string_hash_dictionary::prereg_entries = var<mAvlTree<string_hash_entry> *>(0x0095C7E4);

bool &string_hash_dictionary::is_setup = var<bool>(0x0095C7EC);

const char *&string_hash_dictionary::dictionary_extension = var<const char *>(0x00921CA4);

const char *&string_hash_dictionary::default_dictionary_filename = var<const char *>(0x00921CA0);

const char *&string_hash_dictionary::file_header_identifier_string = var<const char *>(0x00921CAC);

ghetto_mash_file_header *&string_hash_dictionary::header = var<ghetto_mash_file_header *>(0x0095C7DC);

const char *&string_hash_dictionary::textfile_extension = var<const char *>(0x00921CA8);

const char *&string_hash_dictionary::hard_log_filename = var<const char *>(0x00921C9C);

uint8_t *&string_hash_dictionary::mash_image_buffer = var<uint8_t *>(0x0095C7E8);

os_file &string_hash_dictionary::_hard_log = var<os_file>(0x00960448);

#else

static mAvlTree<string_hash_entry> *g_entries{};
mAvlTree<string_hash_entry> *&string_hash_dictionary::entries{g_entries};

static mAvlTree<string_hash_entry> *g_prereg_entries{};
mAvlTree<string_hash_entry> *&string_hash_dictionary::prereg_entries{g_prereg_entries};

static bool g_is_setup{};
bool &string_hash_dictionary::is_setup = g_is_setup;

const char *&string_hash_dictionary::dictionary_extension = []() -> auto & {
    static const char *g_dictionary_extension = ".bin";
    return g_dictionary_extension;
}();

const char *&string_hash_dictionary::default_dictionary_filename = []() -> auto & {
    static const char *g_default_dictionary_filename = "string_hash_dictionary";
    return g_default_dictionary_filename;
}();

const char *&string_hash_dictionary::file_header_identifier_string = []() -> auto & {
    static const char *g_file_header_identifier_string = "shd";
    return g_file_header_identifier_string;
}();

ghetto_mash_file_header *&string_hash_dictionary::header = []() -> auto & {
    static ghetto_mash_file_header *g_header{};
    return g_header;
}();

const char *&string_hash_dictionary::textfile_extension = []() -> auto & {
    static const char *g_textfile_extension = ".txt";
    return g_textfile_extension;
}();

const char *&string_hash_dictionary::hard_log_filename = []() -> auto & {
    static const char *g_hard_log_filename = "dynamic_string_log";
    return g_hard_log_filename;
}();

uint8_t *&string_hash_dictionary::mash_image_buffer = []() -> auto & {
    static uint8_t *g_mash_image_buffer{};
    return g_mash_image_buffer;
}();

os_file &string_hash_dictionary::_hard_log = []() -> auto & {
    static os_file g_hard_log{};
    return g_hard_log;
}();

#endif

static constexpr int RESOURCE_VERSION_MASH_DEP = 0x13;

static_assert('A' == 65);
static_assert('a' == 97);
static_assert('z' == 122);

void string_hash_dictionary::create_inst()
{
    TRACE("string_hash_dictionary::create_inst");

    if constexpr (1) {
        sp_log("g_is_the_packer = %d, LOAD_STRING_HASH_DICTIONARY = %d",
               g_is_the_packer,
               os_developer_options::instance->get_flag(mString{"LOAD_STRING_HASH_DICTIONARY"}));

        if (g_is_the_packer || os_developer_options::instance->get_flag(mString{"LOAD_STRING_HASH_DICTIONARY"})) {
            load_dictionary(nullptr);
        }

        sp_log("string_hash_dictionary::prereg_entries = %s", (prereg_entries == nullptr) ? "null" : "valid");

        if (prereg_entries != nullptr) {
            if (entries != nullptr) {
                assert(prereg_entries->get_destruct_contents());

                entries->insert_tree(prereg_entries);
                prereg_entries->set_destruct_contents(false);
            }

            if (prereg_entries != nullptr) {
                delete prereg_entries;
                prereg_entries = nullptr;
            }
        }

        is_setup = true;
    } else {
        CDECL_CALL(0x005588B0);
    }
}

void string_hash_dictionary::clear()
{
    TRACE("string_hash_dictionary::clear");

    if constexpr (1) {
        if (mash_image_buffer != nullptr) {
            if (header != nullptr) {
                header->destruct_mashed_class();
            }

            if (entries != nullptr) {
                entries->destruct_mashed_class();
            }

            mem_freealign(mash_image_buffer);
        } else {
            if (header != nullptr) {
                delete header;
            }

            if (entries != nullptr) {
                delete entries;
            }
        }

        mash_image_buffer = nullptr;
        header = nullptr;
        entries = nullptr;
        if (_hard_log.opened) {
            _hard_log.close();
        }
    } else {
        CDECL_CALL(0x00547A30);
    }
}

bool string_hash_dictionary::read(const char *a1)
{
    TRACE("string_hash_dictionary::read", a1);

    if constexpr (1) {
        assert(mash_image_buffer == nullptr && "we assume the dictionary is not already open");
        assert(header == nullptr && "we assume the dictionary is not already open");
        assert(entries == nullptr && "we assume the dictionary is not already open");

        bool result;

        os_file v10{{a1}, 1u};

        if (v10.opened) {
            std::uint32_t size = v10.get_size();
            //sp_log("size_file = %u", size); //8096228

            mash_image_buffer = static_cast<uint8_t *>(arch_memalign(16u, size));

#ifndef TARGET_XBOX
            mash_info_struct v5{mash_image_buffer, static_cast<int>(size)};
#else
            mash_info_struct v5{mash::UNMASH_MODE, mash_image_buffer, static_cast<int>(size), true};
#endif

            v10.read(mash_image_buffer, size);

            v5.unmash_class(header,
                            nullptr
#ifdef TARGET_XBOX
                            ,
                            mash::NORMAL_BUFFER
#endif
            );
            assert(header != nullptr);

            if (header->validate(file_header_identifier_string, RESOURCE_VERSION_MASH_DEP)) {
                sp_log("string_hash_dictionary::header is valid");

                v5.unmash_class(entries,
                                nullptr
#ifdef TARGET_XBOX
                                ,
                                mash::NORMAL_BUFFER
#endif
                );

                v5.construct_class(entries);

                result = true;
            } else {
                sp_log("string_hash_dictionary::header is non valid");

                clear();

                result = false;
            }
        } else {
            result = false;
        }

        return result;
    } else {
        return (bool)CDECL_CALL(0x00550DC0, a1);
    }
}

void string_hash_dictionary::save_dictionary(const char *a1)
{
    TRACE("string_hash_dictionary::save_dictionary");

    if constexpr (1) {
        mString v2 = figure_out_filename(a1, default_dictionary_filename, textfile_extension);

        write_text(v2.c_str());
        mString v3 = figure_out_filename(a1, default_dictionary_filename, dictionary_extension);

        v2 = v3;
    } else {
        CDECL_CALL(0x00531A60, a1);
    }
}

void string_hash_dictionary::write_text(const char *a1)
{
    TRACE("string_hash_dictionary::write_text", a1);

    if constexpr (1) {
        mString v6{a1};
        os_file file{v6, os_file::FILE_WRITE};

        if (file.opened) {
            mString v4{"hashcode\tstring\r\n"};
            file.write(v4.c_str(), v4.size());

            v4 = "----------------------------------------------------------------------------"
                 "-----------\r\n";

            file.write(v4.c_str(), v4.size());

#if 0
            for ( auto &node : (*entries) )
            {
                mString v5 = node.m_key->generate_text("\r\n");
                file.write(v5.c_str(), v5.size());
            }
#endif

            file.close();
        }
    } else {
        CDECL_CALL(0x0052A6A0, a1);
    }
}

void string_hash_dictionary::delete_inst()
{
    TRACE("string_hash_dictionary::delete_inst");

    if constexpr (1) {
        is_setup = false;
        if (g_is_the_packer || os_developer_options::instance->get_flag(mString{"LOAD_STRING_HASH_DICTIONARY"})) {
            save_dictionary(nullptr);
        }

        clear();
    } else {
        CDECL_CALL(0x0054C1F0);
    }
}

mString string_hash_dictionary::figure_out_filename(const char *a2, const char *dict_filename, const char *dict_ext)
{
    TRACE("string_hash_dictionary::figure_out_filename");

    mString v6{a2};

    if (a2 == nullptr) {
        mString v4{0, "%s%s", packfile_dir[g_platform], dict_filename};

        v6 = v4;
    }

    v6 += dict_ext;

    return v6;
}

void string_hash_dictionary::load_dictionary(const char *a1)
{
    TRACE("string_hash_dictionary::load_dictionary");

    if constexpr (1) {
        mString v1 = figure_out_filename(a1, default_dictionary_filename, dictionary_extension);

        if (!read(v1.c_str())) {
            create_new_dictionary();
        }
    } else {
        void (*func)(const char *) = CAST(func, 0x00556000);
        func(a1);
    }
}

void string_hash_dictionary::create_new_dictionary()
{
    TRACE("string_hash_dictionary::create_new_dictionary");

    if constexpr (1) {
        assert(header == nullptr && "dictionary already loaded");

        assert(entries == nullptr && "dictionary already loaded");

        assert(mash_image_buffer == nullptr && "dictionary already loaded");

        header = new ghetto_mash_file_header{mash::ALLOCATED, file_header_identifier_string, RESOURCE_VERSION_MASH_DEP};

        entries = new mAvlTree<string_hash_entry>{};
    } else {
        CDECL_CALL(0x0052A5E0);
    }
}

const char *string_hash_dictionary::lookup_string(string_hash a1)
{
    const char *res = nullptr;

    if constexpr (1) {
        string_hash_entry a2{};
        a2.field_0 = a1;

        auto *v6 = entries->find(&a2);
        if (v6 != nullptr) {
            res = v6->field_4.c_str();
        }
    } else {
        res = bit_cast<char *>(CDECL_CALL(0x00531990, a1));
    }

    return res;
}

void string_hash_dictionary::hard_log_string(const char *a1, const string_hash &a2)
{
    if (!_hard_log.is_open()) {
        mString v4 = figure_out_filename(nullptr, hard_log_filename, textfile_extension);

        _hard_log.open({v4.c_str()}, 2);
    }

    if (_hard_log.is_open()) {
        string_hash_entry v5{a1, a2};

        mString v2 = v5.generate_text("\r\n");

        _hard_log.write(v2.c_str(), v2.size());
    }
}

bool string_hash_dictionary::is_loaded()
{
    return entries != nullptr;
}

#if 1

string_hash string_hash_dictionary::register_string(const char *str)
{
    static const char *s_debug_string = "ai_arena.dsg";
    _strcmpi(str, s_debug_string);

    const int v2 = to_hash(str);
    string_hash a3{};
    a3.source_hash_code = v2;

    string_hash a1;
    if (entries != nullptr) {
        register_in_tree(entries, str, a3);
        a1.source_hash_code = a3.source_hash_code;
    }
#if 1
    else {
        a1.source_hash_code = v2;
    }
#else
    else if (!string_hash_dictionary::is_setup) {
        if (string_hash_dictionary::prereg_entries == nullptr) {
            string_hash_dictionary::prereg_entries = new mAvlTree<string_hash_entry>{};
        }

        string_hash_dictionary::register_in_tree(string_hash_dictionary::prereg_entries, str, &a3);
    }

    a1 = a3;

    if (!g_is_the_packer()) {
        if (string_hash_dictionary::is_setup && os_developer_options::instance->get_flag(14)) {
            string_hash_dictionary::hard_log_string(str, a3);
        }
        a1 = a3;
    }

#endif

    return a1;
}

#else

string_hash *string_hash_dictionary::register_string(string_hash *out, const char *str)
{
    //sp_log(str);

    string_hash *result = (string_hash *)CDECL_CALL(0x0053DE30, out, str);

    return result;
}
#endif

bool string_hash_dictionary::register_in_tree(mAvlTree<string_hash_entry> *a1, const char *str, const string_hash &a3)
{
    TRACE("string_hash_dictionary::register_in_tree");

    if constexpr (1) {
        string_hash_entry a2{nullptr, a3};

        auto *v4 = a1->find(&a2);
        if (v4 != nullptr) {
            if (_strcmpi(str, v4->field_4.c_str()) != 0) {
                auto *v6 = v4->field_4.c_str();
                auto v5 = a3.source_hash_code;
                error("String hash collision (0x%08x) old=%s, new=%s", v5, v6, str);
            }

            return false;
        }

        string_hash_entry *v11 = new string_hash_entry{str, a3};

        auto did_insert = a1->insert(v11);
        assert(did_insert && "duplicate string_hash entry");

        return true;
    } else {
        bool (*func)(mAvlTree<string_hash_entry> *a1, const char *str, const string_hash *a3) = CAST(func, 0x0053DD00);
        return func(a1, str, &a3);
    }
}

bool string_hash_dictionary::exists(uint32_t a1)
{
    if constexpr (0) {
        string_hash a4{static_cast<int>(a1)};
        string_hash_entry v6{nullptr, a4};

        string_hash_entry *v5 = nullptr;
        if (entries != nullptr) {
            v5 = entries->find(&v6);
        } else if (prereg_entries != nullptr) {
            v5 = prereg_entries->find(&v6);
        }

        bool v4 = (v5 != nullptr);
        return v4;
    } else {
        bool(__cdecl * func)(uint32_t) = CAST(func, 0x0054C220);
        return func(a1);
    }
}


void string_hash_dictionary_patch()
{
    REDIRECT(0x005E1113, string_hash_dictionary::create_inst);

    return;

    REDIRECT(0x00556041, string_hash_dictionary::read);

    REDIRECT(0x005588CC, string_hash_dictionary::load_dictionary);

    REDIRECT(0x0055604D, string_hash_dictionary::create_new_dictionary);

    REDIRECT(0x0053DE68, string_hash_dictionary::register_in_tree);

    REDIRECT(0x005D9495, string_hash_dictionary::delete_inst);

    REDIRECT(0x005374C8, string_hash_dictionary::lookup_string);

    {
        [[maybe_unused]] auto address = &string_hash_dictionary::register_string;

        //REDIRECT(0x00542730, address);
        //REDIRECT(0x0054F1BE, address);
        //REDIRECT(0x0054F3B8, address);
    }
}
