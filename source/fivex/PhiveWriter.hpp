#pragma once

#include "types.h"
#include <vector>
#include <cstring>
#include "phive/phive.h"
#include <bit>

namespace FiveX{
    class PhiveWriter{
        
        public:

        PhiveWriter();

        void serialize(Phive::PhiveMeshShape *shape);

        std::vector<u8> mData;

        private:

        void writeTypeSection();
        u64 writeItemSection(hknpMeshShape *shape);

        template<typename T>
        u64 skipRelArrayViewHeader(hkRelArrayView<T, int> *obj){
            u64 at = getOffset();
            seek(8, SeekType::Current);
            return at;
        };
        template<typename T>
        void writeRelArrayViewHeader(hkRelArrayView<T, int> *obj, u64 dataOff, u64 headerOff = 0){
            if(headerOff == 0) headerOff = getOffset();
            seek(headerOff, SeekType::Begin);
            writeInteger(dataOff - getOffset(), sizeof(uint32_t));
            writeInteger(obj->m_size, sizeof(uint32_t));
            seek(dataOff, SeekType::Begin);
        };
        template<typename T>
        u64 skipRelArrayHeader(hkRelArray<T> *obj){
            u64 at = getOffset();
            seek(16, SeekType::Current);
            return at;
        };
        template<typename T>
        void writeRelArrayHeader(hkRelArray<T> *obj, u64 dataOff, u64 headerOff = 0){
            if(headerOff == 0) headerOff = getOffset();
            seek(headerOff, SeekType::Begin);
            writeInteger(obj->m_size ? (uint64_t)(dataOff - getOffset()) : 0, sizeof(uint64_t));
            writeInteger((int32_t)obj->m_size, sizeof(int32_t));
            writeInteger((int32_t)obj->m_capacityAndFlags, sizeof(int32_t));
            seek(dataOff, SeekType::Begin);
        };
        template<typename T>
        u64 skipRelPtrHeader(hkRelPtr<T> *obj){
            u64 at = getOffset();
            seek(8, SeekType::Current);
            return at;
        };
        template<typename T>
        void writeRelPtrHeader(hkRelPtr<T> *obj, u64 dataOff, u64 headerOff = 0){
            if(headerOff == 0) headerOff = getOffset();
            seek(headerOff, SeekType::Begin);
            writeInteger(obj->m_offset ? (uint64_t)(dataOff - getOffset()) : 0, sizeof(uint32_t));
            seek(dataOff, SeekType::Begin);
        };
        void writeMeshShape(hknpMeshShape *shape);
        void writeMeshShapeVertexConversionUtil(hknpMeshShapeVertexConversionUtil *obj);
        void writeCompositeShape(hknpCompositeShape *shape);
        void writeShape(hknpShape *shape);
        void writeReferencedObject(hkReferencedObject *obj);
        void writeBaseObject(void *object);
        void writeHkcdFourAabb(hkcdFourAabb *obj);
        void writeHkcdSimdTreeNode(hkcdSimdTree::Node *obj);
        void writeShapePropertyBase(hknpShapePropertyBase *obj);
        void writeShapePropertiesEntry(hknpShapeProperties::Entry *obj);
        void writeShapeProperties(hknpShapeProperties *obj);
        void writeTransposedFourAabbs8(hknpTransposedFourAabbs8 *obj);
        void writeMeshShapeGeometrySectionHeader(hknpMeshShape::GeometrySection *obj, int id);
        void writeMeshShapeGeometrySectionSectionBvh(hknpMeshShape::GeometrySection *obj, int id);
        void writeMeshShapeGeometrySectionPrimitives(hknpMeshShape::GeometrySection *obj, int id);
        void writeMeshShapeGeometrySectionVertexBuffer(hknpMeshShape::GeometrySection *obj, int id);
        void writeMeshShapeGeometrySectionInteriorPrimitiveBitField(hknpMeshShape::GeometrySection *obj, int id);
        void writeMeshShapeTagTableEntry(hknpMeshShape::ShapeTagTableEntry *obj);
        u64 writeHkcdSimdTree(hkcdSimdTree *obj);
        void writeAabb8TreeNode(hknpAabb8TreeNode *obj);
        void writeMeshShapeGeometrySectionPrimitive(hknpMeshShape::GeometrySection::Primitive *obj);
        void writeMeshShapePrimitiveMapping(hknpMeshShapePrimitiveMapping *obj);

        enum class SeekType{
            Begin, Current
        };

        static const u64 cReserveChunkSize = 1024 * 1024; // 1mb

        void ensureSize(u64 size);
        
        void write(const void *data, u64 size);
        void writeByte(u8 byte);
        void writeBytes(const char *bytes);
        void writeInteger(u64 data, u64 size);
        void writeZeroes(u64 size);

        void seek(u64 off, SeekType seekType);

        void align(u64 alignment);

        u64 getOffset();
        
        u64 mOffset;
        
        u64 mShapeTagTableOffset;
        u64 mTopLevelTreeOffset;
        u64 mGeometrySectionsOffset;
        std::vector<u64> mSectionBvhHeaderOffsets;
        std::vector<u64> mPrimitivesHeaderOffsets;
        std::vector<u64> mVertexBufferHeaderOffsets;
        std::vector<u64> mInteriorPrimitiveBitFieldHeaderOffsets;
        std::vector<u64> mSectionBvhOffsets;
        std::vector<u64> mPrimitivesOffsets;
        std::vector<u64> mVertexBufferOffsets;
        std::vector<u64> mInteriorPrimitiveBitFieldOffsets;
        
    };
};