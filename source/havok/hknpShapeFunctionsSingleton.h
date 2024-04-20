#pragma once

#include "havok/hknpShape.h"
#include "havok/hkGeometry.h"
#include "havok/hkResult.h"

class hknpShapeFunctionsSingleton{
    public:
    static hknpShapeFunctionsSingleton **sInstancePtr;
    struct hknpShapeFunctionList{
        struct BuildGeometryArg{
            int _0 = 2;
            int _4 = 0;
            u64 _8 = 0;
            bool _10 = 1;
            u8 _11 = 0;
            u8 _12 = 0;
            u8 _13 = 0;
            u8 _14 = 0;
            u8 _15 = 0;
            u8 _16 = 0;
            u8 _17 = 0;
            u64 _18 = 0;
            float _20 = 1.0f;
            float _24 = 1.0f;
            float _28 = 1.0f;
            float _2C = 0.0f;
            int _30 = 1;
            int _34 = 0;
            u64 _38 = 0;
            int _40 = 0;
            int _44 = 0;
            int _48 = 0;
            int _4c = 0;
            float _50 = -1.0f;
        };
        _BYTE _0[0x100];
        hkResult (*buildGeometry)(hknpShape *shape, BuildGeometryArg &buildGeometryArg, hkGeometry *geometry, u64 unk0);
        _BYTE _108[0x200 - 0x108];
    };
    _BYTE _0[0x18];
    hknpShapeFunctionList mLists[32];
};