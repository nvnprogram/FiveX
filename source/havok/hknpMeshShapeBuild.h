#pragma once

#include "havok/hkTypes.h"
#include "havok/hknpShape.h"
#include "havok/hkGeometry.h"

struct hknpMeshShapeBuildArg{
    hkUint8 _0[0x74];
    bool _74;
    hkUint8 _75[0x88 - 0x75];
};
struct hknpMeshShapeUnkBuildArg2{
    hkUint64 _0 = 0;
    hkUint64 _8 = 0;
};
extern void (*hknpMeshShapeBuildArgCtor)(hknpMeshShapeBuildArg *arg, hkGeometry *geom, bool unk);
struct hknpMeshShapeBuildResult{
    hknpMeshShape *shape;
    hkUint64 _8;
    hkUint64 _10;
};
extern hknpMeshShapeBuildResult (*hknpMeshShapeBuild)(hknpMeshShapeBuildArg &arg, hknpMeshShapeUnkBuildArg2 &arg2);