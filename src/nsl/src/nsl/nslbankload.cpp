#include "nslbankload.h"

#include "utility.h"
#include "func_wrapper.h"
#include "tl_system.h"
#include "trace.h"
#include "nslbank.h"
#include "common.h"

Var<nslBankLoad> nsl_bankLoad{0x0097DA80};

VALIDATE_OFFSET(nslBankLoad::internal, field_44, 0x44);

int nslBankLoad_Fixup(nslBankLoad *a1)
{
    return CDECL_CALL(0x0079AEA0, a1);
}

void *nsl_Alloc(size_t Size)
{
    return tlMemAlloc(Size, 64, 0);
}

unsigned int nslAramAlloc(unsigned int a1, int a2)
{
    return CDECL_CALL(0x0079B5E0, a1, a2);
}

int nslBankLoad_Update(nslBankLoad *bl)
{
    TRACE("nslBankLoad_Update");

    //sp_log("begin: %d", nslGetBankState((nslBankID) 2162687));

    if constexpr (0) {
        if (bl->field_0 == 256) {
            if (bl->rid != NFL_REQUEST_ID_INVALID) {
                if (nflGetRequestState(bl->rid) != -1) {
                    return bl->field_0;
                }

                bl->rid = NFL_REQUEST_ID_INVALID;
            }

            bl->field_0 = -1;
        }

        if (bl->field_0 == 0) {
            bl->field_0 = 2;
        }

        if (bl->field_0 == 2) {
            assert(bl->rid == NFL_REQUEST_ID_INVALID);

            bl->rid = nflReadFileAsync(bl->field_10, bl->field_14, bl->field_18, 4096u);
            if (bl->rid != NFL_REQUEST_ID_INVALID) {
                bl->field_0 = 3;
            }
        }


        if (bl->field_0 == 3) {
            assert(bl->rid != NFL_REQUEST_ID_INVALID);

            if (nflGetRequestState(bl->rid) == -1) {
                bl->rid = NFL_REQUEST_ID_INVALID;
                auto *v4 = bl->field_18;

                if (memcmp(bl->field_18, "WAVEBK", 6) == 0 && v4->field_6 == '1' && v4->field_7 == '1') {
                    auto v9 = v4->field_10;
                    bl->field_1C = v9;
                    bl->field_24 = v4->field_14;
                    bl->field_18 = static_cast<decltype(bl->field_18)>(nsl_Alloc(v9));
                    if (bl->field_24 != 0) {
                        bl->field_20 = nslAramAlloc(bl->field_4, bl->field_24);
                    }

                    if (bl->field_18 != nullptr && (!bl->field_24 || bl->field_20)) {
                        bl->field_0 = 4;
                    } else {
                        nslAramFree(bl->field_20);
                        tlMemFree(bl->field_18);
                        bl->field_18 = nullptr;
                        bl->field_20 = 0;
                        bl->field_0 = -4;
                    }
                } else {
                    sp_log("Invalid wave bank: %s (%-6.6s:%d.%d)\n",
                           bl->field_18->field_20,
                           (const char *)bl->field_18,
                           bl->field_18->field_6,
                           bl->field_18->field_7);

                    bl->field_0 = -5;
                }
            }
        }

        if (bl->field_0 == 4) {
            assert(bl->rid == NFL_REQUEST_ID_INVALID);

            bl->rid = nflReadFileAsync(bl->field_10, bl->field_14, bl->field_18, bl->field_1C);
            if (bl->rid != NFL_REQUEST_ID_INVALID) {
                bl->field_0 = 5;
            }
        }

        if (bl->field_0 == 5) {
            assert(bl->rid != NFL_REQUEST_ID_INVALID);

            if (nflGetRequestState(bl->rid) == -1) {
                auto v11 = bl->field_24;
                bl->rid = NFL_REQUEST_ID_INVALID;
                bl->field_0 = 4 * (v11 == 0) + 7;
            }
        }

        if (bl->field_0 == 7) {
            assert(bl->rid == NFL_REQUEST_ID_INVALID);
            bl->rid = nflReadFileAsync(bl->field_10, bl->field_14 + bl->field_1C, (void *)bl->field_20, bl->field_24);
            if (bl->rid != NFL_REQUEST_ID_INVALID) {
                bl->field_0 = 8;
            }
        }

        if (bl->field_0 == 8) {
            assert(bl->rid != NFL_REQUEST_ID_INVALID);

            if (nflGetRequestState(bl->rid) == -1) {
                bl->field_0 = 11;
                bl->rid = NFL_REQUEST_ID_INVALID;
            }
        }

        if (bl->field_0 == 11) {
            bl->field_0 = 1;
            nslBankLoad_Fixup(bl);
        }

        if (bl->field_0 == -4) {
            sp_log("No memory\n");
        }

        return bl->field_0;
    } else {
        auto result = CDECL_CALL(0x0079AFD0, bl);
        //sp_log("end: %d", nslGetBankState((nslBankID) 2162687));
        //sp_log("result = %d", result);
        //assert(result != -5);
        return result;
    }
}

void nslBankLoad_Init(nslBankLoad *a1, unsigned int a2, nflFileID a3, unsigned int a4)
{
    if constexpr (0) {
        a1->rid.field_0 = -1;
        a1->field_C = -1;
        a1->field_10 = a3;
        a1->field_14 = a4;
        a1->field_4 = a2;

        static Var<char[4096]> byte_97DB40{0x0097DB40};
        a1->field_18 = bit_cast<decltype(a1->field_18)>(&byte_97DB40());
        a1->field_20 = 0;
        memset(byte_97DB40(), 0xBFu, sizeof(byte_97DB40()));
        a1->field_0 = 0;
    } else {
        CDECL_CALL(0x0079AE50, a1, a2, a3, a4);
    }
}
