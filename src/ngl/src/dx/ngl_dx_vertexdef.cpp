#include "ngl_dx_vertexdef.h"

#include "func_wrapper.h"
#include "ngl.h"
#include "trace.h"
#include "variable.h"

static Var<nglMeshSection *> dword_972A10{0x00972A10};

void sub_40FD00(int16_t *a1, unsigned int NVerts, int16_t a3)
{
    assert((nglScratchBuffer().GetIndexCount() + NVerts) < (nglScratchMeshWorkSize() / sizeof(uint16_t)) &&
           "Scratch mesh index count overflow !");

    for (auto i = 0u; i < NVerts; ++i) {
        a1[nglScratchBuffer().GetIndexCount()] = a3 + i;
        nglScratchBuffer().IncreaseIndexCount(1);
    }
}


void BeginStrip(int nVerts, unsigned int stride)
{
    TRACE("BeginStrip");

    assert(nVerts > 2 && "BeginStrip: a strip must have at least 3 vertices !");

    if constexpr (1) {
        auto IndexCount = nVerts;
        auto v4 = [](const nglScratchBuffer_t &self, uint32_t stride, int a3) -> uint32_t {
            return a3 + self.field_2C / stride;
        }(nglScratchBuffer(), stride, dword_972A10()->NVertices);

        auto *v2 = nglScratchBuffer().GetIndexBuffer();
        if (dword_972A10()->NIndices != 0) {
            bool v5 = ((nglScratchBuffer().m_numVertices & 1) != 0);
            IndexCount = (v5 ? nVerts + 3 : nVerts + 2);

            assert((nglScratchBuffer().GetIndexCount() + IndexCount) <
                       static_cast<int>(nglScratchMeshWorkSize() / sizeof(uint16_t)) &&
                   "Scratch mesh index count overflow !");

            auto v3 = nglScratchBuffer().GetIndexCount();
            v2[v3] = v2[v3 - 1];
            nglScratchBuffer().IncreaseIndexCount(1);

            v2[nglScratchBuffer().GetIndexCount()] = v4;
            nglScratchBuffer().IncreaseIndexCount(1);
            if (v5) {
                v2[nglScratchBuffer().GetIndexCount()] = v4;
                nglScratchBuffer().IncreaseIndexCount(1);
            }
        }

        sub_40FD00(v2, nVerts, v4);
        nglScratchBuffer().SetCountVerts(nVerts);
        dword_972A10()->NIndices += IndexCount;
        dword_972A10()->NVertices += nVerts;
    } else {
        CDECL_CALL(0x0040FD50, nVerts, stride);
    }
}
