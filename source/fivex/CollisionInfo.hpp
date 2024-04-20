#pragma once

#include "types.h"
#include "havok/hkGeometry.h"
#include "havok/hknpShapeFunctionsSingleton.h"
#include "havok/hknpMeshShapeBuild.h"
#include "phive/phive.h"
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <cstring>
#include "lib/Json.hpp"

namespace FiveX{
    class CollisionInfo{
        public:

        struct MaterialInfo{
            int mMatId;
            u64 mMatFlags;
            u64 mMatColFlags;
        };

        static void setConfig(json &config);

        CollisionInfo();
        ~CollisionInfo();
        void destroy();
        bool initialize(u8 *objData, u64 objSize, json &matConfig);
        bool initialize(Phive::PhiveMeshShape *meshShape);
        void serializeToObj(std::vector<u8> &outObj, std::vector<u8> &outMatInfos);
        Phive::PhiveMeshShape *buildShape();

        std::vector<MaterialInfo> mMaterials;
        hkGeometry mGeometry;
        bool mInitialized;
        bool mOwnsGeometry;

    };
};