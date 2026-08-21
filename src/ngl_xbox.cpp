#include "ngl.h"

#ifdef TARGET_XBOX

nglMaterialBase *nglGetMaterialInFile(const tlHashString &a1, nglMeshFile *a2)
{
    TRACE("nglGetMaterialInFile", a1.c_str());

    for (auto *i = a2->field_13C; i != nullptr; i = i->field_C) {
        if (i->Name == a1) {
            return i;
        }
    }

    return nullptr;
}

nglMesh *nglGetMeshInFile(const tlHashString &a1, nglMeshFile *a2)
{
    TRACE("nglGetMeshInFile", a1.c_str());

    for (auto *i = a2->FirstMesh; i != nullptr; i = i->NextMesh) {
        if (i->Name == a1) {
            return i;
        }
    }

    return nglGetMesh(a1, true);
}


bool nglLoadMeshFileInternal(const tlFixedString &FileName, nglMeshFile *MeshFile, const char *ext)
{
    TRACE("nglLoadMeshFileInternal", (std::string{FileName.to_string()} + ext).c_str());

    if constexpr (1) {
        nglMeshFileHeader *Header = CAST(Header, MeshFile->field_124.Buf);

        MeshFile->field_134 = (int)Header;
        MeshFile->field_144 = -1;
        if (strncmp(Header->Tag, "XBXM", 4u) != 0) {
            sp_log("Corrupted mesh file: %s%s%s.\n", nglMeshPath(), FileName.to_string(), ext);

            return false;
        }

        constexpr auto version = 0x1601;

        if (Header->Version != version) {
            auto *v6 = FileName.to_string();
            sp_log("Unsupported mesh file version: %s%s%s (version %x, current version is %x).\n",
                   nglMeshPath(),
                   v6,
                   ext,
                   Header->Version,
                   version);

            return false;
        }

        if (Header->NDirectoryEntries == 0) {
            auto *v7 = FileName.to_string();
            sp_log("Mesh file hasn't any directory entries: %s%s%s.\n", nglMeshPath(), v7, ext);

            return false;
        }

        auto Base = bit_cast<uint32_t>(&MeshFile->field_124.Buf[-Header->field_10]);

        nglRebaseHeader(Base, Header);

        MeshFile->FirstMesh = nullptr;
        MeshFile->field_13C = nullptr;
        MeshFile->field_140 = nullptr;
        uint32_t num_dir_entries = Header->NDirectoryEntries;
        sp_log("num_dir_entries = %d", num_dir_entries);

        nglMesh *LastMesh = nullptr;
        nglMaterialBase *LastMaterial = nullptr;
        nglMorphSet *prevMorph = nullptr;

        auto *dir_entries = Header->DirectoryEntries;

        std::for_each(dir_entries, dir_entries + num_dir_entries, [&](auto &dir_entry) {
            PTR_OFFSET(Base, dir_entry.field_4);

            auto type_dir_entry = dir_entry.field_3;
            sp_log("%s", to_string(type_dir_entry));
            switch (type_dir_entry) {
            case TypeDirectoryEntry::MATERIAL: {
                nglMaterialBase *Material = CAST(Material, dir_entry.field_4);

                Material->File = MeshFile;
                if (MeshFile->field_13C == nullptr) {
                    MeshFile->field_13C = Material;
                }

                if (LastMaterial != nullptr) {
                    LastMaterial->field_C = Material;
                }

                LastMaterial = Material;
                if (Header->field_10 == 0) {
                    uint32_t v17 = CAST(v17, Material->field_4);
                    const tlHashString a2{v17};

                    auto *v18 = nglShaderBank().Search(a2);

                    if (v18 != nullptr) {
                        auto *shader = static_cast<nglShader *>(v18->field_20);

                        sp_log("%s", a2.c_str());
                        sp_log("%s 0x%08X", Material->Name.c_str(), Material->field_10);

                        if (shader->CheckMaterialVersion(Material)) {
                            Material->field_4 = shader;
                        } else {
                            auto *v27 = a2.c_str();
                            auto v26 = Material->field_10;

                            auto *v8 = Material->Name.c_str();
                            sp_log("Material %s binary version (%d) is not compatible with shader "
                                   "%s.\n",
                                   v8,
                                   v26,
                                   v27);
                            Material->field_4 = &gEmptyShader();
                        }

                    } else {
                        auto *v28 = Material->Name.c_str();
                        auto *v9 = a2.c_str();
                        sp_log("NGL: Unable to find shader %s, used by material %s.\n", v9, v28);

                        Material->field_4 = &gEmptyShader();
                    }
                }

                Material->field_4->RebaseMaterial(Material, Base);

                Material->field_4->BindMaterial(Material);

            } break;
            case TypeDirectoryEntry::MESH: {
                nglMesh *Mesh = CAST(Mesh, dir_entry.field_4);

                sp_log("%s", Mesh->Name.c_str());

                {
                    void(__fastcall * Add)(void *, void *edx, nglMesh *) =
                        CAST(Add, get_vfunc(nglMeshDirectory()->m_vtbl, 0x10));
                    Add(nglMeshDirectory(), nullptr, Mesh);
                }

                Mesh->File = MeshFile;
                if (MeshFile->FirstMesh == nullptr) {
                    MeshFile->FirstMesh = Mesh;
                }

                if (LastMesh != nullptr) {
                    LastMesh->NextMesh = Mesh;
                }

                LastMesh = Mesh;
                if ((Mesh->Flags & NGLMESH_PROCESSED) == 0) {
                    nglRebaseMesh(Base, 0, Mesh);
                }

                sp_log("Mesh->NSections = %d", Mesh->NSections);
                for (auto idx_Section = 0u; idx_Section < Mesh->NSections; ++idx_Section) {
                    Mesh->Sections[idx_Section].field_0 = 1;

                    nglMeshSection *MeshSection = CAST(MeshSection, Mesh->Sections[idx_Section].Section);
                    tlHashString a1{(uint32_t)MeshSection->Name};

                    MeshSection->Material = nglGetMaterialInFile(a1, MeshFile);

                    if (!MeshSection->Material->field_4->CheckVertexDefVersion(MeshSection)) {
                        tlFixedString v111 = MeshSection->Material->field_4->GetName();

                        auto *v12 = v111.to_string();
                        sp_log("Section VertexDef Binary version (%d) is incompatible with "
                               "shader %s\n.",
                               MeshSection->field_50,
                               v12);
                        MeshSection->Material->field_4 = &gEmptyShader();
                    }

                    if (MeshSection->NIndices != 0) {
                        sp_log("NIndices = %d", MeshSection->NIndices);

                        {
                            auto *arr = bit_cast<uint16_t *>(MeshSection->m_indices);
                            sp_log("indices = %u %u %u", arr[0], arr[1], arr[2]);
                        }

                        sp_log("NVertices = %d", MeshSection->field_40);

                        {
                            auto *arr = bit_cast<float *>(MeshSection->m_vertices);
                            sp_log("vertices = %f %f %f %f", arr[0], arr[1], arr[2], arr[3]);
                        }

                        sp_log("stride = %d", MeshSection->m_stride);
                    }

                    if (auto *v27 = MeshSection->m_indices; v27 != nullptr) {
                        bit_cast<nglVertexBuffer *>(&MeshSection->m_indices)
                            ->createIndexBufferAndWriteData(v27, 2 * MeshSection->NIndices);
                    }

                    auto *v28 = MeshSection->Material;
                    MeshSection->field_58 = 0;

                    tlFixedString v112 = v28->field_4->GetName();

                    auto *v29 = v112.to_string();
                    sp_log(v29);

                    [&v29](nglMeshSection *MeshSection) -> void {
                        auto func = [](nglMeshSection *MeshSection) {
                            auto v31 = static_cast<uint32_t>(MeshSection->field_40 >> 6);
                            auto *v32 = (float *)(static_cast<char *>(MeshSection->m_vertices) + 32);
                            MeshSection->field_5C = 2;
                            if (v31 > 0) {
                                for (; v31 != 0; --v31) {
                                    if (equal(v32[7], 0.0f)) {
                                        if (not_equal(v32[6], 0.0f) && MeshSection->field_5C < 3u) {
                                            MeshSection->field_5C = 3;
                                        }
                                    } else {
                                        MeshSection->field_5C = 4;
                                    }

                                    *(uint32_t *)v32 = v32[0];

                                    *((uint32_t *)v32 + 1) = v32[1];

                                    *((uint32_t *)v32 + 2) = v32[2];
                                    *((uint32_t *)v32 + 3) = v32[3];

                                    v32 += 16;
                                }
                            }

                            ((nglVertexBuffer *)&MeshSection->m_vertices)
                                ->createVertexBufferAndWriteData(
                                    (const void *)MeshSection->m_vertices, MeshSection->field_40, 1028);

                            static Var<int> dword_973BC8{0x00973BC8};

                            if (dword_973BC8() < (int)(24 * (MeshSection->field_40 >> 6))) {
                                dword_973BC8() = 24 * (MeshSection->field_40 >> 6);
                            }

                            MeshSection->m_stride = 24;
                        };

                        if (!EnableShader()) {
                            if (strncmp(v29, "uslod", 5u) == 0) {
                                nglVertexBuffer::createIndexOrVertexBuffer((nglVertexBuffer *)&MeshSection->m_vertices,
                                                                           ResourceType::VertexBuffer,
                                                                           16 * (MeshSection->field_40 / 12),
                                                                           520,
                                                                           0,
                                                                           D3DPOOL_DEFAULT);
                                MeshSection->m_stride = 16;
                                MeshSection->field_5C = 0;
                                return;
                            }

                            if (!EnableShader()) {
                                if (ChromeEffect()) {
                                    if (strncmp(v29, "smshiny", 7u) == 0) {
                                        int v30 = 48 * (MeshSection->field_40 / 60u);
                                        ((nglVertexBuffer *)&MeshSection->m_vertices)->createVertexBuffer(v30, 520u);
                                        MeshSection->m_stride = 48;

                                        static Var<int> dword_972960{0x00972960};

                                        if (dword_972960() < v30) {
                                            dword_972960() = v30;
                                        }

                                        return;
                                    }
                                } else {
                                    if (!EnableShader()) {
                                        if (strncmp(v29, "usperson", 8u) == 0) {
                                            func(bit_cast<nglMeshSection *>(MeshSection));
                                            return;
                                        }
                                    }
                                }
                            }
                        }

                        if (strncmp(v29, "us_character", 12u) == 0) {
                            func(bit_cast<nglMeshSection *>(MeshSection));
                            return;
                        }

                        ((nglVertexBuffer *)&MeshSection->m_vertices)
                            ->createVertexBufferAndWriteData(
                                (const void *)MeshSection->m_vertices, MeshSection->field_40, 1028);
                    }(MeshSection);

                    if (auto *v39 = MeshSection->VertexDef; v39 != nullptr) {
                        tlHashString a1{v39->m_vtbl};

                        auto *v40 = nglVertexDefBank().Search(a1);
                        if (v40 != nullptr) {
                            MeshSection->VertexDef->field_4 = MeshSection;

                            void (*func)(void *) = CAST(func, v40->field_20);
                            func(MeshSection->VertexDef);
                        } else {
                            MeshSection->VertexDef = nullptr;
                        }
                    }

                    auto *v41 = MeshSection->Material;
                    if (v41 != nullptr) {
                        auto *v42 = v41->field_4;
                        if (v42 != nullptr) {
                            v42->BindSection(MeshSection);
                        }
                    }
                }

            } break;
            case TypeDirectoryEntry::MORPH: {
                nglMorphSet *new_morph = CAST(new_morph, dir_entry.field_4);
                nglProcessMorph(MeshFile, &dir_entry, (int)Header);
                if (prevMorph != nullptr) {
                    prevMorph->field_10 = new_morph;
                }

                prevMorph = new_morph;
            } break;
            default: {
                auto *v14 = FileName.to_string();

                sp_log("nglLoadMeshFile: file \"%s%s%s\" has an unknown directory entry ( %u ), "
                       "skipping.\n",
                       nglMeshPath(),
                       v14,
                       ext,
                       type_dir_entry);

                break;
            }
            }
        });

        if (LastMesh != nullptr) {
            LastMesh->NextMesh = nullptr;
        }

        if (LastMaterial != nullptr) {
            LastMaterial->field_C = nullptr;
        }

        vector4d a3a;
        a3a[0] = 1.0e32;
        a3a[1] = 1.0e32;
        a3a[2] = 1.0e32;

        vector4d v103;

        v103[0] = -1.0e32;
        v103[1] = -1.0e32;
        v103[2] = -1.0e32;
        v103[3] = -a3a[3];

        bool v46 = false;

        for (auto *Mesh = MeshFile->FirstMesh; Mesh != nullptr; Mesh = Mesh->NextMesh) {
            if ((Mesh->Flags & NGLMESH_PROCESSED) == 0) {
                if (Mesh->NBones != 0) {
                    for (int i = 0; i < Mesh->NBones; ++i) {
                        Mesh->Bones[i] = sub_4150E0(Mesh->Bones[i]);
                    }

                    auto v89 = Mesh->field_20.field_0[0];
                    auto v90 = Mesh->field_20.field_0[1];
                    auto v91 = Mesh->field_20.field_0[2];
                    auto v93 = Mesh->field_20.field_0[3];
                    auto v73 = Mesh->SphereRadius;

                    vector4d v96;
                    v96[0] = v89 - v73;
                    v96[1] = v90 - v73;
                    v96[2] = v91 - v73;
                    v96[3] = v93 - v73;

                    a3a = sub_401270(v96, a3a);

                    vector4d v110;
                    v110[0] = v89 + v73;
                    v110[1] = v90 + v73;
                    v110[2] = v91 + v73;
                    v110[3] = v93 + v73;

                    v103 = sub_4012F0(v110, v103);

                    v46 = true;
                } else {
                    Mesh->Flags |= NGLMESH_PROCESSED;
                }

                auto *Lods = Mesh->LODs;
                for (int i = 0; i < Mesh->NLODs; ++i) {
                    tlHashString v1{bit_cast<uint32_t>(Lods[i].field_0)};

                    Mesh->LODs[i].field_0 = nglGetMeshInFile(v1, MeshFile);
                    Lods = Mesh->LODs;
                    if (Lods[i].field_0 == nullptr) {
                        --i;
                        --Mesh->NLODs;
                    }
                }
            }
        }

        if (v46) {
            auto v60 = sub_411750(a3a, v103);
            auto v78 = v60[0] * 0.5f;
            auto v61 = v78;
            auto v62 = v60[1];

            vector4d v96;
            v96[0] = v78;
            auto v81 = v62 * 0.5f;

            v96[1] = v81;
            auto v84 = v60[2] * 0.5f;

            auto *v67 = MeshFile->FirstMesh;

            auto v87 = v60[3] * 0.5f;
            auto v68 = v87;
            auto v69 = 0.0f;
            v96[2] = v84;
            v96[3] = v87;
            for (; v67 != nullptr; v67 = v67->NextMesh) {
                if ((v67->Flags & NGLMESH_PROCESSED) == 0) {
                    a3a[0] = v96[0] - v67->field_20.field_0[0];
                    a3a[1] = v96[1] - v67->field_20.field_0[1];
                    a3a[2] = v96[2] - v67->field_20.field_0[2];
                    a3a[3] = v96[3] - v67->field_20.field_0[3];
                    auto v76 = vector3d{a3a[0], a3a[1], a3a[3]}.length() + v67->SphereRadius;
                    if (v69 <= v76) {
                        v69 = v76;
                    }
                }
            }

            for (auto *Mesh = v67; Mesh != nullptr; Mesh = Mesh->NextMesh) {
                if ((Mesh->Flags & NGLMESH_PROCESSED) == 0) {
                    Mesh->SphereRadius = v69;
                    Mesh->field_20.field_0[0] = v61;
                    Mesh->field_20.field_0[1] = v81;
                    Mesh->field_20.field_0[2] = v84;
                    Mesh->field_20.field_0[3] = v68;
                    Mesh->Flags |= NGLMESH_PROCESSED;
                }
            }
        }

        Header->field_10 = (int)MeshFile->field_124.Buf;
        return true;
    } else {
        auto result = static_cast<bool>(CDECL_CALL(0x0076F500, &FileName, MeshFile, ext));

        return result;
    }
}
#endif
