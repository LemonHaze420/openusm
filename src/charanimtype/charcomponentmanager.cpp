#include "charcomponentmanager.h"

#include "charcomponentbase.h"
#include "log.h"
#include "trace.h"
#include "variable.h"
#include "variables.h"

#include "arbitrarypocharcomp.h"
#include "armikentcompdecomp.h"
#include "armikposedesc.h"
#include "armstdposedesc.h"
#include "fakerootentcompdecomp.h"
#include "fakerootposedesc.h"
#include "fing52knuckcurlentcompdecomp.h"
#include "fing52knuckcurlposedesc.h"
#include "flexiblecharcomp.h"
#include "floatsentcompdecomp.h"
#include "genericcharcomp.h"
#include "legsikentcompdecomp.h"
#include "legsikposedesc.h"
#include "legsstdposedesc.h"
#include "quatsentcompdecomp.h"
#include "tentaclesposedesc.h"
#include "torsoheadentcompdecomp.h"
#include "torsoheadstdposedesc.h"

#ifndef STANDALONE_SYSTEM
#error "Not defined marco STANDALONE_SYSTEM"
#endif

#if !STANDALONE_SYSTEM

int &CharComponentManager::iCurrNumComponents = var<int>(0x0096A558);

BaseComponent **&CharComponentManager::pCompArray = var<BaseComponent **>(0x0096A55C);

#else

int &CharComponentManager::iCurrNumComponents = []() -> auto & {
    static int g_iCurrNumComponents{};
    return g_iCurrNumComponents;
}();

BaseComponent **&CharComponentManager::pCompArray = []() -> auto & {
    static BaseComponent **g_pCompArray{};
    return g_pCompArray;
}();

static ArbitraryPOCharComp g_ArbitraryPOCharComp{};

static GenericCharComp g_my_generic_character_component{};

static FlexibleCharComp<FakerootPoseDesc, FakerootEntCompDecomp<FakerootPoseDesc>> g_FakerootStdEntComp{
    0x40000000, "Fakeroot Entropy Compressed"};

static FlexibleCharComp<TorsoHeadStdPoseDesc, TorsoHeadEntCompDecomp<TorsoHeadStdPoseDesc>> g_TorsoHeadEntComp{
    0x40000000, "TorsoHead OneNeck Entropy Compressed"};

static FlexibleCharComp<TorsoHeadStdPoseDesc, TorsoHeadEntCompDecomp<TorsoHeadStdPoseDesc>> g_TorsoHead2NeckEntComp{
    0x40800000, "TorsoHead TwoNeck Entropy Compressed"};

static FlexibleCharComp<LegsStdPoseDesc, QuatsEntCompDecomp<LegsStdPoseDesc>> g_LegsEntCharComp{
    0x40000000, "Legs&Feet Entropy Compressed"};

static FlexibleCharComp<LegsIKPoseDesc, LegsIKEntCompDecomp<LegsIKPoseDesc>> g_LegsIKCharEntComp{
    0x40800000, "Legs&Feet IK Entropy Compressed"};

static FlexibleCharComp<ArmStdPoseDesc, QuatsEntCompDecomp<ArmStdPoseDesc>> g_ArmsEntCharComp{
    0x40000000, "Arms&Hands Entropy Compressed"};

static FlexibleCharComp<ArmIKPoseDesc, ArmIKEntCompDecomp<ArmIKPoseDesc>> g_ArmIKCharEntComp{
    0x40800000, "Arm&Hands IK Entropy Compressed"};

static FlexibleCharComp<TentaclesPoseDesc, FloatsEntCompDecomp<TentaclesPoseDesc>> g_TentacleEntComp{
    0x40000000, "Tentacles Compressed"};

static FlexibleCharComp<Fing52KnuckCurlPoseDesc, Fing52KnuckCurlEntCompDecomp<Fing52KnuckCurlPoseDesc>>
    g_Fing52KnuckEntComp{0x41200000, "Five Finger Top 2 Knuckle Curl Entropy Compressed"};

#endif

void CharComponentManager::RegisterComponent(CharComponentBase *theComp)
{
    TRACE("CharComponentManager::RegisterComponent");

    if (pCompArray == nullptr) {
        InitComponentArray();
    }

    assert(GetComponentByType(theComp->GetType()) == nullptr && "Cannot register a type twice.");

    pCompArray[iCurrNumComponents++] = theComp;

    assert(iCurrNumComponents < iMaxNumComponents &&
           "Too many character components. Add more slots or cull some components.");
}

void CharComponentManager::InitComponentArray()
{
    pCompArray = (BaseComponent **)operator new(4 * iMaxNumComponents);
    memset(pCompArray, 0, 4 * iMaxNumComponents);
}

BaseComponent *CharComponentManager::GetComponentByType(uint32_t a1)
{
    for (auto i = 0; i < iCurrNumComponents; ++i) {
        if (pCompArray[i]->GetType() == a1) {
            return pCompArray[i];
        }
    }

    return nullptr;
}
