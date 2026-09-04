#pragma once

#include "variable.h"

struct FileUSM {
    char *field_0;
    char **field_4;
    int field_8;
    int field_C;

    //0x0081C610
    FileUSM(const char *a2, char *a3);

    char *sub_81C4C0(const char *a2);
};

//0x0081C580
extern char *get_msg(FileUSM *a1, const char *a2);

//0x0081C7C0
extern FileUSM *create_usm_file(const char *a1, char *a2);

extern FileUSM *&g_fileUSM;

extern void FileUSM_patch();
