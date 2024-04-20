#include "fivex/PhiveWriter.hpp"
#include "fivex/PhiveBinData.inc"

namespace FiveX
{
PhiveWriter::PhiveWriter(){
    mOffset = 0;
    mData.reserve(cReserveChunkSize);
}
void PhiveWriter::ensureSize(u64 size){
    if(size > mData.size()){
        if(size > mData.capacity()) mData.reserve(size + cReserveChunkSize);
        mData.resize(size);
    }
}
void PhiveWriter::write(const void *data, u64 size){
    ensureSize(mOffset + size);
    memcpy(mData.data() + mOffset, data, size);
    mOffset+=size;
}
void PhiveWriter::writeZeroes(u64 size){
    ensureSize(mOffset + size);
    memset(mData.data() + mOffset, 0, size);
    mOffset+=size;
}
void PhiveWriter::writeByte(u8 byte){
    write(&byte, sizeof(byte));
}
void PhiveWriter::writeBytes(const char *bytes){
    write(bytes, strlen(bytes));
}
void PhiveWriter::writeInteger(u64 data, u64 size){
    write(&data, size);
}
void PhiveWriter::seek(u64 off, SeekType seekType){
    mOffset = (seekType == SeekType::Current ? mOffset : 0) + off;
    ensureSize(mOffset);
}
void PhiveWriter::align(u64 alignment){
    seek(ALIGN_UP(mOffset, alignment), SeekType::Begin);
}
u64 PhiveWriter::getOffset(){
    return mOffset;
}
void PhiveWriter::writeBaseObject(void *obj)
{
    writeInteger(0, sizeof(uint64_t)); // writeInteger(*reinterpret_cast<u64*>(obj), sizeof(uint64_t));
}
void PhiveWriter::writeReferencedObject(hkReferencedObject *obj)
{
	writeBaseObject(obj);
	writeInteger(obj->m_sizeAndFlags, sizeof(uint64_t));
	writeInteger(obj->m_refCount, sizeof(uint64_t));
}
void PhiveWriter::writeShapePropertyBase(hknpShapePropertyBase *obj)
{
	writeReferencedObject(obj);
	writeInteger(obj->m_propertyKey, sizeof(uint16_t));
}
void PhiveWriter::writeShapePropertiesEntry(hknpShapeProperties::Entry *obj)
{
    // ???, i think this would fail if any were present but there are none so...
    writeRelPtrHeader(&obj->m_object, 8);
    writeShapePropertyBase(obj->m_object.get());
}
void PhiveWriter::writeShapeProperties(hknpShapeProperties *obj)
{
    writeRelArrayHeader(&obj->m_properties, getOffset() + 16);
    for(int i = 0; i < obj->m_properties.m_size; i++) writeShapePropertiesEntry(&obj->m_properties[i]); // ???
}
void PhiveWriter::writeShape(hknpShape *shape)
{
	writeReferencedObject(shape);
	writeInteger((uint8_t)shape->mType, sizeof(uint8_t));
	writeInteger((uint8_t)shape->mDispatchType, sizeof(uint8_t));
	writeInteger((uint16_t)shape->mFlags, sizeof(uint16_t));

	writeInteger(shape->m_numShapeKeyBits, sizeof(uint8_t));
	writeZeroes(3); // Padding
	write(reinterpret_cast<const char*>(&shape->m_convexRadius), sizeof(float));
	writeZeroes(sizeof(uint32_t)); // Padding
	writeInteger(0, sizeof(uint64_t)); //writeInteger(shape->m_userData, sizeof(uint64_t));
	writeShapeProperties(&shape->m_properties);
}
void PhiveWriter::writeCompositeShape(hknpCompositeShape *shape)
{
	writeShape(shape);
	writeInteger(shape->m_shapeTagCodecInfo, sizeof(uint32_t));
	writeZeroes(sizeof(uint32_t) + sizeof(uint64_t)); // Padding
}
void PhiveWriter::writeMeshShapeVertexConversionUtil(hknpMeshShapeVertexConversionUtil *obj)
{
	write(obj->m_bitScale16, 4 * 4);
	write(obj->m_bitScale16Inv, 4 * 4);
}
void PhiveWriter::writeMeshShapeTagTableEntry(hknpMeshShape::ShapeTagTableEntry *obj)
{
	writeInteger(obj->m_meshPrimitiveKey, sizeof(uint32_t));
	writeInteger(obj->m_shapeTag, sizeof(uint16_t));
	writeZeroes(sizeof(uint16_t)); // Padding
}
void PhiveWriter::writeHkcdFourAabb(hkcdFourAabb *obj)
{
	write(&obj->m_lx[0], 4 * sizeof(float));
	write(&obj->m_hx[0], 4 * sizeof(float));
	write(&obj->m_ly[0], 4 * sizeof(float));
	write(&obj->m_hy[0], 4 * sizeof(float));
	write(&obj->m_lz[0], 4 * sizeof(float));
	write(&obj->m_hz[0], 4 * sizeof(float));
}
void PhiveWriter::writeHkcdSimdTreeNode(hkcdSimdTree::Node *obj)
{
    writeHkcdFourAabb(obj);
	write(obj->m_data, 4 * 4);
	writeInteger(obj->m_isLeaf, sizeof(uint8_t));
	writeInteger(obj->m_isActive, sizeof(uint8_t));
	writeZeroes(sizeof(uint16_t) + sizeof(uint32_t) + sizeof(uint64_t)); // Padding
}
u64 PhiveWriter::writeHkcdSimdTree(hkcdSimdTree *obj)
{
    u64 headerOff = skipRelArrayHeader(&obj->m_nodes);
	writeInteger(obj->m_isCompact, sizeof(uint8_t));
	writeZeroes(3 + sizeof(uint32_t)); // Padding
    return headerOff;
}
void PhiveWriter::writeTransposedFourAabbs8(hknpTransposedFourAabbs8 *obj)
{
	writeInteger(obj->m_lx, sizeof(uint32_t));
	writeInteger(obj->m_hx, sizeof(uint32_t));
	writeInteger(obj->m_ly, sizeof(uint32_t));
	writeInteger(obj->m_hy, sizeof(uint32_t));
	writeInteger(obj->m_lz, sizeof(uint32_t));
	writeInteger(obj->m_hz, sizeof(uint32_t));
}
void PhiveWriter::writeAabb8TreeNode(hknpAabb8TreeNode *obj)
{
	writeTransposedFourAabbs8(obj);
	write(obj->m_data, 4);
}
void PhiveWriter::writeMeshShapeGeometrySectionPrimitive(hknpMeshShape::GeometrySection::Primitive *obj)
{
	writeInteger(obj->m_aId, sizeof(uint8_t));
	writeInteger(obj->m_bId, sizeof(uint8_t));
	writeInteger(obj->m_cId, sizeof(uint8_t));
	writeInteger(obj->m_dId, sizeof(uint8_t));
}

void PhiveWriter::writeMeshShapeGeometrySectionHeader(hknpMeshShape::GeometrySection *obj, int id)
{
    mSectionBvhHeaderOffsets[id] = skipRelArrayViewHeader(&obj->m_sectionBvh);
    mPrimitivesHeaderOffsets[id] = skipRelArrayViewHeader(&obj->m_primitives);
    mVertexBufferHeaderOffsets[id] = skipRelArrayViewHeader(&obj->m_vertexBuffer);
    mInteriorPrimitiveBitFieldHeaderOffsets[id] = skipRelArrayViewHeader(&obj->m_interiorPrimitiveBitField);

	write(&obj->m_sectionOffset[0], 3 * 4);
	write(&obj->m_bitScale8Inv.m_x, 3 * 4);
	write(&obj->m_bitOffset[0], 3 * 2);
	writeZeroes(sizeof(uint16_t)); // Padding
}
void PhiveWriter::writeMeshShapeGeometrySectionSectionBvh(hknpMeshShape::GeometrySection *obj, int id)
{
	mSectionBvhOffsets[id] = getOffset();
    writeRelArrayViewHeader(&obj->m_sectionBvh, mSectionBvhOffsets[id], mSectionBvhHeaderOffsets[id]);
    for(int i = 0; i < obj->m_sectionBvh.m_size; i++) writeAabb8TreeNode(&obj->m_sectionBvh[i]);
	align(16);
}

void PhiveWriter::writeMeshShapeGeometrySectionPrimitives(hknpMeshShape::GeometrySection *obj, int id)
{
	mPrimitivesOffsets[id] = getOffset();
	writeRelArrayViewHeader(&obj->m_primitives, mPrimitivesOffsets[id], mPrimitivesHeaderOffsets[id]);
	for(int i = 0; i < obj->m_primitives.m_size; i++) writeMeshShapeGeometrySectionPrimitive(&obj->m_primitives[i]);
	align(16);
}

void PhiveWriter::writeMeshShapeGeometrySectionVertexBuffer(hknpMeshShape::GeometrySection *obj, int id)
{
	mVertexBufferOffsets[id] = getOffset();
    //logF("%i %llx\n", id, mVertexBufferOffsets[id]);
    writeRelArrayViewHeader(&obj->m_vertexBuffer, mVertexBufferOffsets[id], mVertexBufferHeaderOffsets[id]);
    write(obj->m_vertexBuffer.begin(), obj->m_vertexBuffer.m_size  * sizeof(hknpMeshShape::GeometrySection::Vertex16_3));
    align(16);
}

void PhiveWriter::writeMeshShapeGeometrySectionInteriorPrimitiveBitField(hknpMeshShape::GeometrySection *obj, int id)
{
	mInteriorPrimitiveBitFieldOffsets[id] = getOffset();
    writeRelArrayViewHeader(&obj->m_interiorPrimitiveBitField, mInteriorPrimitiveBitFieldOffsets[id], mInteriorPrimitiveBitFieldHeaderOffsets[id]);
    write(obj->m_interiorPrimitiveBitField.begin(), obj->m_interiorPrimitiveBitField.m_size);
    align(16);
}

void PhiveWriter::writeMeshShapePrimitiveMapping(hknpMeshShapePrimitiveMapping *obj)
{
    writeReferencedObject(obj);

    u64 sectionStartHeaderOff = skipRelArrayHeader(&obj->m_sectionStart);
    u64 bitStringHeaderOff = skipRelArrayHeader(&obj->m_bitString);

	writeInteger(obj->m_bitsPerEntry, sizeof(uint32_t));
	writeInteger(obj->m_triangleIndexBitMask, sizeof(uint32_t));
    
    writeRelArrayHeader(&obj->m_sectionStart, getOffset(), sectionStartHeaderOff);
    write(obj->m_sectionStart.begin(), obj->m_sectionStart.m_size * sizeof(uint32_t));

	writeRelArrayHeader(&obj->m_bitString, getOffset(), bitStringHeaderOff);
    write(obj->m_bitString.begin(), obj->m_bitString.m_size * sizeof(uint32_t));
}
void PhiveWriter::writeMeshShape(hknpMeshShape *shape)
{
    writeCompositeShape(shape);
    writeMeshShapeVertexConversionUtil(&shape->m_vertexConversionUtil);
    u64 shapeTagTableHeaderOff = skipRelArrayViewHeader(&shape->m_shapeTagTable);
    u64 hkcdSimdTreeHeaderOff = writeHkcdSimdTree(&shape->m_topLevelTree);
    u64 geometrySectionsHeaderOff = skipRelArrayViewHeader(&shape->m_geometrySections);
    u64 primitiveMappingHeaderOff = skipRelPtrHeader(&shape->m_primitiveMapping);

	mShapeTagTableOffset = getOffset();
    writeRelArrayViewHeader(&shape->m_shapeTagTable, mShapeTagTableOffset, shapeTagTableHeaderOff);
    for(int i = 0; i < shape->m_shapeTagTable.m_size; i++) writeMeshShapeTagTableEntry(&shape->m_shapeTagTable[i]);
	align(16);

    mTopLevelTreeOffset = getOffset();
    writeRelArrayHeader(&shape->m_topLevelTree.m_nodes, mTopLevelTreeOffset, hkcdSimdTreeHeaderOff);
    for(int i = 0; i < shape->m_topLevelTree.m_nodes.m_size; i++) writeHkcdSimdTreeNode(&shape->m_topLevelTree.m_nodes[i]);
	align(16);

    mGeometrySectionsOffset = getOffset();
    writeRelArrayViewHeader(&shape->m_geometrySections, mGeometrySectionsOffset, geometrySectionsHeaderOff);

    auto &gs = shape->m_geometrySections;

    mSectionBvhHeaderOffsets.resize(gs.m_size);
    mPrimitivesHeaderOffsets.resize(gs.m_size);
    mVertexBufferHeaderOffsets.resize(gs.m_size);
    mInteriorPrimitiveBitFieldHeaderOffsets.resize(gs.m_size);

    mSectionBvhOffsets.resize(gs.m_size);
    mPrimitivesOffsets.resize(gs.m_size);
    mVertexBufferOffsets.resize(gs.m_size);
    mInteriorPrimitiveBitFieldOffsets.resize(gs.m_size);

    for(int i = 0; i < gs.m_size; i++) writeMeshShapeGeometrySectionHeader(&gs[i], i);
    for(int i = 0; i < gs.m_size; i++) writeMeshShapeGeometrySectionSectionBvh(&gs[i], i);
    for(int i = 0; i < gs.m_size; i++) writeMeshShapeGeometrySectionPrimitives(&gs[i], i);
    for(int i = 0; i < gs.m_size; i++) writeMeshShapeGeometrySectionVertexBuffer(&gs[i], i);
    for(int i = 0; i < gs.m_size; i++) writeMeshShapeGeometrySectionInteriorPrimitiveBitField(&gs[i], i);

	align(16);

    //logF("OFFS: %llx %llx %llx %llx\n", mShapeTagTableOffset, mTopLevelTreeOffset, mGeometrySectionsOffset, getOffset());

    writeRelPtrHeader(&shape->m_primitiveMapping, getOffset(), primitiveMappingHeaderOff);
    if(shape->m_primitiveMapping.get()) writeMeshShapePrimitiveMapping(shape->m_primitiveMapping.get());
	align(16);
}
void PhiveWriter::writeTypeSection(){
    write(dummyTYPEdata, sizeof(dummyTYPEdata));
    align(16);
}
u64 PhiveWriter::writeItemSection(hknpMeshShape *shape)
{
    u64 indxOff = getOffset();
	seek(sizeof(uint32_t), SeekType::Current);
	writeBytes("INDX");

    u64 itemOff = getOffset();
	seek(sizeof(uint32_t), SeekType::Current);
	writeBytes("ITEM");

	writeZeroes(12); //Default section

	//hknpMeshShape
	writeByte(0x01);
	writeByte(0x00);
	writeByte(0x00);
	writeByte(0x10);
	writeInteger(0, sizeof(uint32_t));
	writeInteger(1, sizeof(uint32_t));

	//ShapeTagTable
	writeByte(0x05);
	writeByte(0x00);
	writeByte(0x00);
	writeByte(0x20);
	writeInteger(mShapeTagTableOffset - 0x50, sizeof(uint32_t));
	writeInteger(shape->m_shapeTagTable.m_size, sizeof(uint32_t));

	//NodeTree
	writeByte(0xE);
	writeByte(0x00);
	writeByte(0x00);
	writeByte(0x20);
	writeInteger(mTopLevelTreeOffset - 0x50, sizeof(uint32_t));
	writeInteger(shape->m_topLevelTree.m_nodes.m_size, sizeof(uint32_t));

	//GeometrySection
	writeByte(0x09);
	writeByte(0x00);
	writeByte(0x00);
	writeByte(0x20);
	writeInteger(mGeometrySectionsOffset - 0x50, sizeof(uint32_t));
	writeInteger(shape->m_geometrySections.m_size, sizeof(uint32_t));

    auto &gs = shape->m_geometrySections;

    // Aabb8TreeNode
    for(int i = 0; i < gs.m_size; i++){
        writeByte(0x2B);
        writeByte(0x00);
        writeByte(0x00);
        writeByte(0x20);
        writeInteger(mSectionBvhOffsets[i] - 0x50, sizeof(uint32_t));
        writeInteger(gs[i].m_sectionBvh.m_size, sizeof(uint32_t));
    }

    // Primitives
    for(int i = 0; i < gs.m_size; i++){
        writeByte(0x2D);
        writeByte(0x00);
        writeByte(0x00);
        writeByte(0x20);
        writeInteger(mPrimitivesOffsets[i] - 0x50, sizeof(uint32_t));
        writeInteger(gs[i].m_primitives.m_size, sizeof(uint32_t));
    }

    // VertexBuffer (Vertex16_3)
    for(int i = 0; i < gs.m_size; i++){
        writeByte(0x2F);
        writeByte(0x00);
        writeByte(0x00);
        writeByte(0x20);
        writeInteger(mVertexBufferOffsets[i] - 0x50, sizeof(uint32_t));
        writeInteger(gs[i].m_vertexBuffer.m_size, sizeof(uint32_t));
    }

    // InteriorPrimitiveBitField (hkUint8)
    for(int i = 0; i < gs.m_size; i++){
        writeByte(0x13);
        writeByte(0x00);
        writeByte(0x00);
        writeByte(0x20);
        writeInteger(mInteriorPrimitiveBitFieldOffsets[i] - 0x50, sizeof(uint32_t));
        writeInteger(gs[i].m_interiorPrimitiveBitField.m_size, sizeof(uint32_t));
    }

	u64 end = getOffset();

    seek(indxOff, SeekType::Begin);
    writeInteger(std::byteswap(u32(end - indxOff)), sizeof(uint32_t));

    seek(itemOff, SeekType::Begin);
    writeInteger(std::byteswap(u32((end - itemOff) | 0x40000000)), sizeof(uint32_t));

    seek(end, SeekType::Begin);

	align(16);

	return end;
}
void PhiveWriter::serialize(Phive::PhiveMeshShape *shape){

    mData.resize(0);
    mOffset = 0;

    seek(0x50, SeekType::Begin);

    // DATA
    writeMeshShape(shape->mShape);
	uint32_t DataEnd = getOffset();

    // Other sections
    writeTypeSection();
	u64 HktEnd = writeItemSection(shape->mShape);

    Phive::PhiveHeader header;
    memcpy(header.Magic, "Phive", sizeof(header.Magic));
    header.Reserve1 = 1;
    header.BOM = 0xFEFF;
    header.MajorVersion = 0;
    header.MinorVersion = 4;
    header.HktOffset = 0x30;
    header.HktSize = ALIGN_UP(HktEnd - header.HktOffset, 16);

    header.TableOffset0 = getOffset();
    write(shape->mMaterialArray, shape->mMaterialNum * sizeof(Phive::PhiveShapeMaterialData));
    align(16);
    header.TableSize0 = getOffset() - header.TableOffset0;

    header.TableOffset1 = getOffset();
    write(shape->mMatColFlags, shape->mMatColFlagsNum * sizeof(u64));
    align(16);
    header.TableSize1 = getOffset() - header.TableOffset1;
    header.FileSize = getOffset();

	seek(0x0, SeekType::Begin);

    write(&header, sizeof(header));

	writeInteger(std::byteswap(u32(HktEnd - header.HktOffset)), sizeof(uint32_t));
	writeBytes("TAG0");
	writeByte(0x40);
	writeByte(0x00);
	writeByte(0x00);
	writeByte(0x10);
	writeBytes("SDKV");
	writeBytes("20210100");

	uint32_t DataSize = DataEnd - (header.HktOffset + 0x18);
	writeInteger(std::byteswap(DataSize), sizeof(uint32_t));
	seek(-4, SeekType::Current);
	writeByte(0x40);
	seek(3, SeekType::Current);
	writeBytes("DATA");
    
}

}