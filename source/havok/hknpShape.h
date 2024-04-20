#pragma once

#include "havok/hkReferencedObject.h"
#include "havok/hkRelArray.h"
#include "havok/hkRelPtr.h"


struct hknpShapePropertyBase : public hkReferencedObject {
    hkUint16 m_propertyKey;
};
static_assert(sizeof(hknpShapePropertyBase) == 0x20L);

struct hknpShapeProperties {
    struct Entry {
        hkRelPtr<hknpShapePropertyBase> m_object;
    };
    static_assert(sizeof(Entry) == 0x8L);
    hkRelArray<Entry> m_properties;
};
static_assert(sizeof(hknpShapeProperties) == 0x10L);

class hknpShape : public hkReferencedObject {
    public:
    hkUint8 mType;
    hkUint8 mDispatchType;
    hkUint16 mFlags;
    hkUint8 m_numShapeKeyBits;
    hkReal m_convexRadius;
    hkUint64 m_userData;
    hknpShapeProperties m_properties;
};
static_assert(sizeof(hknpShape) == 0x40L);

struct hknpCompositeShape : public hknpShape {
    hkUint32 m_shapeTagCodecInfo;
    hkUint8 align[0xC];
};
static_assert(sizeof(hknpCompositeShape) == 0x50L);

struct hknpMeshShapeVertexConversionUtil {
    hkVector4 m_bitScale16;
    hkVector4 m_bitScale16Inv;
};
static_assert(sizeof(hknpMeshShapeVertexConversionUtil) == 0x20L);

struct hkcdFourAabb {
    hkVector4 m_lx;
    hkVector4 m_hx;
    hkVector4 m_ly;
    hkVector4 m_hy;
    hkVector4 m_lz;
    hkVector4 m_hz;
};
static_assert(sizeof(hkcdFourAabb) == 0x60L);

struct hkcdSimdTree {

    struct Node : public hkcdFourAabb {
        hkUint32 m_data[4];
        bool m_isLeaf;
        bool m_isActive;
        hkUint8 align[0xE];
    };
    static_assert(sizeof(Node) == 0x80L);

    hkRelArray<Node> m_nodes;
    bool m_isCompact;
};
static_assert(sizeof(hkcdSimdTree) == 0x18L);

struct hknpTransposedFourAabbs8 {
    hkUint32 m_lx;
    hkUint32 m_hx;
    hkUint32 m_ly;
    hkUint32 m_hy;
    hkUint32 m_lz;
    hkUint32 m_hz;
};
static_assert(sizeof(hknpTransposedFourAabbs8) == 0x18L);

struct hknpAabb8TreeNode : public hknpTransposedFourAabbs8 {
    hkUint8 m_data[4];
};
static_assert(sizeof(hknpAabb8TreeNode) == 0x1cL);

struct hknpMeshShapePrimitiveMapping : public hkReferencedObject {
    hkRelArray<hkUint32> m_sectionStart;
    hkRelArray<unsigned int> m_bitString;
    hkUint32 m_bitsPerEntry;
    hkUint32 m_triangleIndexBitMask;
};
static_assert(sizeof(hknpMeshShapePrimitiveMapping) == 0x40L);

struct hknpMeshShape : public hknpCompositeShape {

    struct ShapeTagTableEntry {
        hkUint32 m_meshPrimitiveKey;
        hkUint16 m_shapeTag;
    };
    static_assert(sizeof(ShapeTagTableEntry) == 0x8L);

    struct GeometrySection {

        struct Primitive {
            hkUint8 m_aId;
            hkUint8 m_bId;
            hkUint8 m_cId;
            hkUint8 m_dId;
        };
        static_assert(sizeof(Primitive) == 0x4L);

        struct Vertex16_3{
            hkUint16 m_x = 0;
            hkUint16 m_y = 0;
            hkUint16 m_z = 0;
        };
        static_assert(sizeof(Vertex16_3) == 0x6L);

        hkRelArrayView<hknpAabb8TreeNode, int> m_sectionBvh;
        hkRelArrayView<Primitive, int> m_primitives;
        hkRelArrayView<Vertex16_3, int> m_vertexBuffer;
        hkRelArrayView<hkUint8, int> m_interiorPrimitiveBitField;
        hkUint32 m_sectionOffset[3];
        hkFloat3 m_bitScale8Inv;
        hkInt16 m_bitOffset[3];
    };
    static_assert(sizeof(GeometrySection) == 0x40L);

    hknpMeshShapeVertexConversionUtil m_vertexConversionUtil;
    hkRelArrayView<ShapeTagTableEntry, int> m_shapeTagTable;
    hkcdSimdTree m_topLevelTree;
    hkRelArrayView<GeometrySection, int> m_geometrySections;
    hkRelPtr<hknpMeshShapePrimitiveMapping> m_primitiveMapping;
};
static_assert(sizeof(hknpMeshShape) == 0xa0L);