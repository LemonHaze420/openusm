#pragma once

namespace nglRenderList {

template <typename T>
void nglOpaqueCompare(T *node, int count, int a3);

}  // namespace nglRenderList

//0x00401A20
extern void *nglListAlloc(int size, int align);

extern void nglRenderList_patch();
