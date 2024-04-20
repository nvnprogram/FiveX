#pragma once

#include "types.h"

#include "havok/hknpShape.h"

namespace Phive{
    struct PhiveHeader
	{
		char Magic[6]; // "Phive"
		uint16_t Reserve1 = 0; // 6
		uint16_t BOM = 0; // 8
		uint8_t MajorVersion = 0; // A
		uint8_t MinorVersion = 0; // B
		uint32_t HktOffset = 0; // C
		uint32_t TableOffset0 = 0; // 10
		uint32_t TableOffset1 = 0; // 14
		uint32_t FileSize = 0; // 18
		uint32_t HktSize = 0; // 1C
		uint32_t TableSize0 = 0; // 20
		uint32_t TableSize1 = 0; // 24
		uint32_t Reserve2 = 0; // 28
		uint32_t Reserve3 = 0; // 2c
	};
    struct PhiveShapeMaterialData{
        int mMatId;
        int _4;
        u64 mFlags;
    };
    struct PhiveShapeUserData {
        u64 vftable = 0; // lazy, but its not needed
        int mMatColFlagsNum = 0;
        u64 *mMatColFlags = NULL;
        int mMaterialNum = 0;
        PhiveShapeMaterialData *mMaterialArray = NULL;
    };
    struct PhiveShapeHolder{
        u64 vtable;
        hknpShape *mShape;
    };
    struct PhiveMeshShape{
        
        PhiveMeshShape(){
            vftable = sVtable;
            mFileData = NULL;
            mFileSize = 0;
            _14 = 0;
            mFlags = 0;
            _1c = 0;
            mShapeHolder = NULL;
            mMaterialNum = 0;
            mMaterialArray = NULL;
            mMatColFlagsNum = 0;
            mMatColFlags = NULL;
            mShape = NULL;
            mDoesntOwnUserDataMatInfo = 0;
        }

        struct vtable{
            u64 _0[2]; // rtti
            void (*dtor0)(PhiveMeshShape *shape);
            void (*dtor2)(PhiveMeshShape *shape);
            int (*unk)(PhiveMeshShape *shape);
            void (*load)(PhiveMeshShape *shape, void *phiveBin, s32 fileSize, void *heap);
        };
        static vtable *sVtable;

        vtable *vftable;
        u8 *mFileData;
        int mFileSize;
        int _14;
        u32 mFlags;
        int _1c;
        PhiveShapeHolder *mShapeHolder;
        int mMaterialNum; // Table 0 Size / sizeof(PhiveShapeMaterialData)
        PhiveShapeMaterialData *mMaterialArray; // Table 0 
        int mMatColFlagsNum; // Table 1 Size / sizeof(u64)
        u64 *mMatColFlags; // Table 1
        PhiveShapeUserData mPhiveShapeUserData;
        hknpMeshShape *mShape;
        bool mDoesntOwnUserDataMatInfo;

    };
};