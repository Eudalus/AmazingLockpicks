#pragma once

// #define SKYRIM_VR = TRUE
// #define SKYRIM_SE = TRUE
#define SKYRIM_AE = TRUE

#ifdef SKYRIM_VR
    //#define ENABLE_SKYRIM_VR = TRUE
    #define OFFSET(se, ae) se
    #define OFFSET_3(se, ae, vr) vr
#endif

#ifdef SKYRIM_SE
    //#define ENABLE_SKYRIM_SE = TRUE
    #define OFFSET(se, ae) se
    #define OFFSET_3(se, ae, vr) se
    
#endif

#ifdef SKYRIM_AE
    //#define ENABLE_SKYRIM_AE = TRUE
    #define OFFSET(se, ae) ae
    #define OFFSET_3(se, ae, vr) ae
#endif

#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"

using namespace std::literals;