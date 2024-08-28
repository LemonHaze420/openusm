#include "fileusm.h"

#include "common.h"
#include "func_wrapper.h"
#include "log.h"
#include "utility.h"
#include "variables.h"

#include <cassert>
#include <cstdio>
#include <cstring>

VALIDATE_SIZE(FileUSM, 0x10);

FileUSM *& g_fileUSM = var<FileUSM *>(0x0096190C);

void sub_81C5D0(const char *a1, char *a2)
{
    char v3;
    auto *v2 = a2;
    if (a2[0] != '\0') {
        do {
            if (v2[0] == '.') {
                break;
            }

            v3 = (v2++)[1];
        } while (v3);
    }

    auto *v4 = bit_cast<char *>(a1);
    *v2 = '.';
    int v5 = v2 + 1 - a1;

    char v6;
    do {
        v6 = *v4;
        v4[v5] = *v4;
        ++v4;
    } while (v6 != '\0');
}

void sub_81D0B0(char *a1, char *a2, int a3) {
    char *v3 = a1;
    if (a1 != nullptr) {
        if (a2 != nullptr) {
            char *v4 = a2;

            if (a3) {
                int v5 = a3;
                do {
                    *v3 ^= *v4;
                    char v6 = v4[1];
                    ++v3;
                    ++v4;
                    if (!v6) {
                        v4 = a2;
                    }

                    --v5;
                } while (v5);
            }
        }
    }
}

char *get_msg(FileUSM *a1, const char *a2) {
    char *result = nullptr;
    if constexpr (1) {
        if (a1 != nullptr) {
            result = a1->sub_81C4C0(a2);

            //sp_log("get_msg() = %s", result);
        }
    } else {
        result = (char *) CDECL_CALL(0x0081C580, a1, a2);
    }

    //sp_log("%s %s", result, a2);

    return result;
}

FileUSM::FileUSM(const char *a2, char *a3) {
    this->field_0 = nullptr;
    this->field_4 = nullptr;
    this->field_8 = 0;
    this->field_C = 0;

    char Filename[260];
    strcpy(Filename, a2);
    if (a3 != nullptr) {
        sub_81C5D0("dat", Filename);
    }

    FILE *v4 = fopen(Filename, "rb");
    FILE *v5 = v4;
    if (v4) {
        int v6 = ftell(v4);
        fseek(v5, 0, 2);
        int v7 = ftell(v5);
        fseek(v5, v6, 0);
        this->field_8 = v7;
        auto *v8 = static_cast<char *>(malloc(v7 + 1));
        size_t v9 = this->field_8;
        this->field_0 = v8;
        fread(v8, 1u, v9, v5);
        this->field_0[this->field_8] = 0;
        fclose(v5);
        sub_81D0B0(this->field_0, a3, this->field_8);

        char *v12;
        for (int i{0}; i < this->field_8; ++i) {
            v12 = &this->field_0[i];
            if (*v12 == '\n') {
                ++this->field_C;
                *v12 = '\0';
            }

            if (this->field_0[i] == '\r') {
                this->field_0[i] = '\0';
            }
        }

        ++this->field_C;
        this->field_4 = static_cast<char **>(malloc(4 * this->field_C));

        bool v16 = false;
        int j = 0;
        int v22 = 0;

        if (this->field_8 > 0) {
            int v10 = 0;

            do {
                char *v18 = this->field_0;
                char v19 = this->field_0[j];
                int v20;

                if (v19 && (v20 = this->field_8 - 1, j != v20)) {
                    v16 = false;
                    if (v19 == '\\' && j < v20 && v18[j + 1] == 'n') {
                        v18[j] = '\r';
                        this->field_0[j + 1] = '\n';
                    }

                } else {
                    if (!v16) {
                        this->field_4[v22++] = &v18[v10];
                        v16 = true;
                    }

                    v10 = j + 1;
                }

                ++j;
            } while (j < this->field_8);
        }

        this->field_C = v22;
    }
}

char *FileUSM::sub_81C4C0(const char *a2) {
    int str_len = strlen(a2);
    auto *new_string = static_cast<char *>(malloc(str_len + 2));
    //strcpy_s(new_string, str_len, a2);
    strcpy(new_string, a2);

    new_string[str_len] = '=';
    new_string[str_len + 1] = '\0';

    if (this->field_0 != nullptr && this->field_4 != nullptr && this->field_8 > 0 &&
        this->field_C > 0) {
        for (int i = 0; i < this->field_C; ++i) {
            if (strncmp(this->field_4[i], new_string, str_len + 1) == 0) {
                free(new_string);
                auto *result = &this->field_4[i][str_len + 1];

                return result;
            }
        }
    }

    free(new_string);

    return nullptr;
}

//0x0081C7C0
FileUSM *create_usm_file(const char *a1, char *a2) {
    FileUSM *v3 = (FileUSM *) malloc(0x10u);
    if (v3 != nullptr) {
        *v3 = FileUSM{a1, a2};
    } else {
        assert(0);
    }

    if (v3->field_0 && v3->field_4 && v3->field_8 > 0 && v3->field_C > 0) {
        return v3;
    }

    free(v3->field_0);
    free(v3->field_4);
    free(v3);
    return nullptr;
}

void FileUSM_patch() {
    REDIRECT(0x005AC8A9, get_msg);
}
