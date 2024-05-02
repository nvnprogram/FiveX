#include "FiveX/CollisionInfo.hpp"

namespace FiveX
{

template <typename T>
static bool jsonSafeGet(json &jason, const char *at, T *res){
    if (jason.contains(at)) {
        auto obj = jason.at(at);
        if (!obj.is_null()){
            *res = obj.get<T>();
            return 1;
        }
    }
    return 0;
};

static std::vector<std::string> sMatNames;
static std::unordered_map<std::string, int> sMatNameMap;
int getMatId(const char *matName){
    auto ita = sMatNameMap.find(matName);
    return (ita != sMatNameMap.end() ? ita->second : 0);
};
static std::vector<std::string> sMatFlagNames;
static std::unordered_map<std::string, int> sMatFlagNameMap;
int getMatFlagNameId(const char *flagName){
    auto ita = sMatFlagNameMap.find(flagName);
    return (ita != sMatFlagNameMap.end() ? ita->second : -1);
};
static std::vector<std::string> sMatColFlagNames;
static std::unordered_map<std::string, int> sMatColFlagNameMap;
int getMatColFlagNameId(const char *flagName){
    auto ita = sMatColFlagNameMap.find(flagName);
    return (ita != sMatColFlagNameMap.end() ? ita->second : -1);
};

void CollisionInfo::setConfig(json &config){

    sMatNames.clear();
    sMatNameMap.clear();
    sMatFlagNames.clear();
    sMatFlagNameMap.clear();
    sMatColFlagNames.clear();
    sMatColFlagNameMap.clear();

    json matNames = {};
    jsonSafeGet(config, "mat_names", &matNames);
    for(auto &ita: matNames.items()){
        sMatNames.push_back(ita.value().get<std::string>());
        sMatNameMap[sMatNames[sMatNames.size() - 1]] = sMatNames.size() - 1;
    }

    json matFlagNames = {};
    jsonSafeGet(config, "mat_flag_names", &matFlagNames);
    for(auto &ita: matFlagNames.items()){
        sMatFlagNames.push_back(ita.value().get<std::string>());
        sMatFlagNameMap[sMatFlagNames[sMatFlagNames.size() - 1]] = sMatFlagNames.size() - 1;
    }

    json matColFlagNames = {};
    jsonSafeGet(config, "col_disable_flag_names", &matColFlagNames);
    for(auto &ita: matColFlagNames.items()){
        sMatColFlagNames.push_back(ita.value().get<std::string>());
        sMatColFlagNameMap[sMatColFlagNames[sMatColFlagNames.size() - 1]] = sMatColFlagNames.size() - 1;
    }

}
struct Vector3{
    Vector3(){
        X = 0.0f;
        Y = 0.0f;
        Z = 0.0f;
    }
    Vector3(float X_, float Y_, float Z_){
        X = X_;
        Y = Y_;
        Z = Z_;
    }
    bool operator==(const Vector3& other) const{
        return (this->X == other.X && this->Y == other.Y && this->Z == other.Z);
    }
    bool operator!=(const Vector3& other) const{
        return !(this->X == other.X && this->Y == other.Y && this->Z == other.Z);
    }
    Vector3 operator+(const Vector3& right) const{
        return Vector3(this->X + right.X, this->Y + right.Y, this->Z + right.Z);
    }
    Vector3 operator-(const Vector3& right) const{
        return Vector3(this->X - right.X, this->Y - right.Y, this->Z - right.Z);
    }
    Vector3 operator*(const float& other) const{
        return Vector3(this->X * other, this->Y * other, this->Z * other);
    }
    Vector3 operator/(const float& other) const{
        return Vector3(this->X / other, this->Y / other, this->Z / other);
    }
    float X;
    float Y;
    float Z;
};
namespace math{
    Vector3 CrossV3(const Vector3 a, const Vector3 b){
        return Vector3(a.Y * b.Z - a.Z * b.Y,
            a.Z * b.X - a.X * b.Z,
            a.X * b.Y - a.Y * b.X);
    }
    float MagnitudeV3(const Vector3 in){
        return (sqrtf(powf(in.X, 2) + powf(in.Y, 2) + powf(in.Z, 2)));
    }
    float DotV3(const Vector3 a, const Vector3 b){
        return (a.X * b.X) + (a.Y * b.Y) + (a.Z * b.Z);
    }
    float AngleBetweenV3(const Vector3 a, const Vector3 b){
        float angle = DotV3(a, b);
        angle /= (MagnitudeV3(a) * MagnitudeV3(b));
        return angle = acosf(angle);
    }
    Vector3 ProjV3(const Vector3 a, const Vector3 b){
        Vector3 bn = b / MagnitudeV3(b);
        return bn * DotV3(a, bn);
    }
}

namespace algorithm{
    inline void split(const std::string &in, std::vector<std::string> &out, std::string token){
        out.clear();
        std::string temp;
        for (int i = 0; i < int(in.size()); i++){
            std::string test = in.substr(i, token.size());
            if (test == token){
                if (!temp.empty()){
                    out.push_back(temp);
                    temp.clear();
                    i += (int)token.size() - 1;
                }
                else out.push_back("");
            }
            else if (i + token.size() >= in.size()){
                temp += in.substr(i, token.size());
                out.push_back(temp);
                break;
            }
            else temp += in[i];
        }
    }
    inline std::string tail(const std::string &in)
    {
        size_t token_start = in.find_first_not_of(" \t");
        size_t space_start = in.find_first_of(" \t", token_start);
        size_t tail_start = in.find_first_not_of(" \t", space_start);
        size_t tail_end = in.find_last_not_of(" \t");
        if (tail_start != std::string::npos && tail_end != std::string::npos) return in.substr(tail_start, tail_end - tail_start + 1);
        else if (tail_start != std::string::npos) return in.substr(tail_start);
        return "";
    }
    inline std::string firstToken(const std::string &in){
        if (in.empty()) return "";
        size_t token_start = in.find_first_not_of(" \t");
        size_t token_end = in.find_first_of(" \t", token_start);
        if (token_start != std::string::npos && token_end != std::string::npos) return in.substr(token_start, token_end - token_start);
        else if (token_start != std::string::npos) return in.substr(token_start);
    }
    bool SameSide(Vector3 p1, Vector3 p2, Vector3 a, Vector3 b){
        Vector3 cp1 = math::CrossV3(b - a, p1 - a);
        Vector3 cp2 = math::CrossV3(b - a, p2 - a);
        if (math::DotV3(cp1, cp2) >= 0) return true;
        else return false;
    }
    Vector3 GenTriNormal(Vector3 t1, Vector3 t2, Vector3 t3){
        Vector3 u = t2 - t1;
        Vector3 v = t3 - t1;
        Vector3 normal = math::CrossV3(u,v);
        return normal;
    }
    bool inTriangle(Vector3 point, Vector3 tri1, Vector3 tri2, Vector3 tri3){
        if (!SameSide(point, tri1, tri2, tri3) || !SameSide(point, tri2, tri1, tri3) || !SameSide(point, tri3, tri1, tri2)) return false;
        Vector3 n = GenTriNormal(tri1, tri2, tri3);
        Vector3 proj = math::ProjV3(point, n);
        if (math::MagnitudeV3(proj) == 0)  return true;
        else return false;
    }
    void VertexTriangluation(std::vector<unsigned int>& oIndices, const std::vector<Vector3>& iVerts){
        if (iVerts.size() < 3) return;
        
        if (iVerts.size() == 3){
            oIndices.push_back(0);
            oIndices.push_back(1);
            oIndices.push_back(2);
            return;
        }

        // does this even work? never tested

        std::vector<Vector3> tVerts = iVerts;

        while (true)
        {
            for (int i = 0; i < int(tVerts.size()); i++)
            {
                Vector3 pPrev;
                if (i == 0) pPrev = tVerts[tVerts.size() - 1];
                else pPrev = tVerts[i - 1];

                Vector3 pCur = tVerts[i];

                Vector3 pNext;
                if (i == tVerts.size() - 1) pNext = tVerts[0];
                else pNext = tVerts[i + 1];

                if (tVerts.size() == 3) {
                    for (int j = 0; j < int(tVerts.size()); j++) {
                        if (iVerts[j] == pCur) oIndices.push_back(j);
                        if (iVerts[j] == pPrev) oIndices.push_back(j);
                        if (iVerts[j] == pNext) oIndices.push_back(j);
                    }

                    tVerts.clear();
                    break;
                }

                if (tVerts.size() == 4) {
                    for (int j = 0; j < int(iVerts.size()); j++){
                        if (iVerts[j] == pCur) oIndices.push_back(j);
                        if (iVerts[j] == pPrev) oIndices.push_back(j);
                        if (iVerts[j] == pNext) oIndices.push_back(j);
                    }

                    Vector3 tempVec;
                    for (int j = 0; j < int(tVerts.size()); j++) {
                        if (tVerts[j] != pCur
                            && tVerts[j] != pPrev
                            && tVerts[j] != pNext) {
                            tempVec = tVerts[j];
                            break;
                        }
                    }

                    for (int j = 0; j < int(iVerts.size()); j++) {
                        if (iVerts[j] == pPrev) oIndices.push_back(j);
                        if (iVerts[j] == pNext) oIndices.push_back(j);
                        if (iVerts[j] == tempVec) oIndices.push_back(j);
                    }

                    tVerts.clear();
                    break;
                }

                float angle = math::AngleBetweenV3(pPrev - pCur, pNext - pCur) * (180 / 3.14159265359);
                if (angle <= 0 && angle >= 180)
                    continue;

                bool inTri = false;
                for (int j = 0; j < int(iVerts.size()); j++) {
                    if (algorithm::inTriangle(iVerts[j], pPrev, pCur, pNext)
                        && iVerts[j] != pPrev
                        && iVerts[j] != pCur
                        && iVerts[j] != pNext)
                    {
                        inTri = true;
                        break;
                    }
                }
                if (inTri) continue;

                for (int j = 0; j < int(iVerts.size()); j++) {
                    if (iVerts[j] == pCur) oIndices.push_back(j);
                    if (iVerts[j] == pPrev) oIndices.push_back(j);
                    if (iVerts[j] == pNext) oIndices.push_back(j);
                }

                for (int j = 0; j < int(tVerts.size()); j++) {
                    if (tVerts[j] == pCur) {
                        tVerts.erase(tVerts.begin() + j);
                        break;
                    }
                }
                
                i = -1;
            }

            if (oIndices.size() == 0) break;
            if (tVerts.size() == 0) break;
        }
    }
};

CollisionInfo::CollisionInfo(){
    mOwnsGeometry = 0;
    mInitialized = 0;
};
CollisionInfo::~CollisionInfo(){

    destroy();

}
void CollisionInfo::destroy(){

    if(!mInitialized) return;

    mInitialized = 0;

    if(mOwnsGeometry){

        auto &tri = mGeometry.m_triangles;
        auto &vert = mGeometry.m_vertices;

        if(tri.m_data){
            delete tri.m_data;
            tri.m_data = NULL;
        }
        tri.m_size = 0;
        tri.m_capacityAndFlags = 0;

        if(vert.m_data){
            delete vert.m_data;
            vert.m_data = NULL;
        }
        vert.m_size = 0;
        vert.m_capacityAndFlags = 0;

        mOwnsGeometry = 0;

    }
    
    mGeometry.vftable->dtor0(&mGeometry);
    mGeometry.hkGeometryCtor(&mGeometry);

    mMaterials.clear();
    

}
bool CollisionInfo::initialize(u8 *objData, u64 objSize, json &matConfig){

    destroy();

    std::vector<const char*> lines;
    char *plainText = reinterpret_cast<char*>(objData);

    lines.push_back(plainText);
    for(char *line = strchr(plainText, '\n'); line != NULL; line = strchr(line, '\n')){
        if(line > plainText && line[-1] == 0xD) line[-1] = '\0';
        *line = '\0';
        lines.push_back(++line);
    }

    std::map<std::string, int> mats;
    int curMatId = 0;
    std::vector<Vector3> vertices;
    std::vector<hkGeometry::Triangle> triangles;

    for(auto &ita: lines){

        std::string line = ita;

        std::string first = algorithm::firstToken(line);

        if(first == "usemtl"){

            std::string matName = algorithm::tail(line);
            
            if(mats.find(matName) == mats.end()){

                MaterialInfo info(0, 0, 0xFFFFFFFFFFFFFFFFULL);

                json matInfo;
                if(jsonSafeGet(matConfig, matName.c_str(), &matInfo)){
                    
                    std::string matname;
                    if(jsonSafeGet(matInfo, "mat_name", &matname)) info.mMatId = getMatId(matname.c_str());

                    json matArray;
                    jsonSafeGet(matInfo, "mat_flags", &matArray);
                    for(auto &ita: matArray.items()){
                        int id = getMatFlagNameId(ita.value().get<std::string>().c_str());
                        if(id != -1) info.mMatFlags|=1ULL << u64(id);
                    }

                    json colArray;
                    jsonSafeGet(matInfo, "col_disable_flags", &colArray);
                    for(auto &ita: colArray.items()){
                        int id = getMatColFlagNameId(ita.value().get<std::string>().c_str());
                        if(id != -1) info.mMatColFlags&=~(1ULL << u64(id));
                    }

                }

                mMaterials.push_back(info);

                mats[matName] = mMaterials.size() - 1;
            
            }

            curMatId = mats[matName];
            continue;
        }

        if(first == "v"){
            std::vector<std::string> spos;
            algorithm::split(algorithm::tail(line), spos, " ");
            vertices.push_back({strtof(spos[0].c_str(), NULL), strtof(spos[1].c_str(), NULL), strtof(spos[2].c_str(), NULL)});
            continue;
        }

        if(first == "f"){

            if(mMaterials.size() == 0) mMaterials.push_back(MaterialInfo(0, 0, 0xFFFFFFFFFFFFFFFFULL));

            std::vector<std::string> sface, svert;
			algorithm::split(algorithm::tail(line), sface, " ");

            std::vector<Vector3> verts;
            std::vector<int> idx(sface.size());
            
			for (int i = 0; i < int(sface.size()); i++){
				algorithm::split(sface[i], svert, "/");
                idx[i] = strtoul(svert[0].c_str(), NULL, 10) - 1;
                verts.push_back(vertices[idx[i]]);
            }

            std::vector<unsigned int> indices;
            algorithm::VertexTriangluation(indices, verts);

            for(int i = 0; i < indices.size() / 3; i++){
                triangles.push_back(hkGeometry::Triangle(
                    idx[indices[i * 3]], 
                    idx[indices[i * 3 + 1]], 
                    idx[indices[i * 3 + 2]], 
                    curMatId
                ));
            }

            continue;

        }
    }

    if(vertices.size() == 0 || triangles.size() == 0) return 0;

    auto &gVert = mGeometry.m_vertices;
    auto &gTri = mGeometry.m_triangles;

    gVert.m_data = new hkVector4[vertices.size()];
    for(int i = 0; i < vertices.size(); i++){
        gVert[i][0] = vertices[i].X;
        gVert[i][1] = vertices[i].Y;
        gVert[i][2] = vertices[i].Z;
        gVert[i][3] = 0;
    }
    gVert.m_size = vertices.size();
    vertices = std::vector<Vector3>();

    gTri.m_data = new hkGeometry::Triangle[triangles.size()];
    memcpy(gTri.m_data, triangles.data(), sizeof(hkGeometry::Triangle) * triangles.size());
    gTri.m_size = triangles.size();

    mOwnsGeometry = 1;
    mInitialized = 1;

    return 1;

}   

bool CollisionInfo::initialize(Phive::PhiveMeshShape *shape){

    destroy();

    hknpShapeFunctionsSingleton *f = *hknpShapeFunctionsSingleton::sInstancePtr;
    if(f == NULL) return 0;

    hknpShapeFunctionsSingleton::hknpShapeFunctionList::BuildGeometryArg buildArg;
    if(f->mLists[shape->mShape->mType].buildGeometry(shape->mShape, buildArg, &mGeometry, NULL) != HK_SUCCESS) return 0;

    for(int i = 0; i < shape->mMaterialNum; i++) 
        mMaterials.push_back(MaterialInfo(shape->mMaterialArray[i].mMatId, shape->mMaterialArray[i].mFlags, shape->mMatColFlags[i]));

    mOwnsGeometry = 0;
    mInitialized = 1;

    return 1;

}

void CollisionInfo::serializeToObj(std::vector<u8> &outObj, std::vector<u8> &outMatInfos){

    outObj.reserve(4 * 1024 * 1024);

    auto writeVertex = [&outObj](float x, float y, float z){

        char buf[256];
        buf[0] = '\0';
        int len = snprintf(buf, sizeof(buf), "v %.6f %.6f %.6f\n", x, y, z);

        outObj.resize(outObj.size() + len);
        memcpy(outObj.data() + outObj.size() - len, buf, len);

    };
    auto writeTriangle = [&outObj](int a, int b, int c){

        char buf[256];
        buf[0] = '\0';
        int len = snprintf(buf, sizeof(buf), "f %i %i %i\n", a + 1, b + 1, c + 1);

        outObj.resize(outObj.size() + len);
        memcpy(outObj.data() + outObj.size() - len, buf, len);

    };
    auto writeObject = [&outObj](const char *objName){

        char buf[256];
        buf[0] = '\0';
        int len = snprintf(buf, sizeof(buf), "o %s\n", objName);

        outObj.resize(outObj.size() + len);
        memcpy(outObj.data() + outObj.size() - len, buf, len);

    };
    auto writeMat = [&outObj](const char *matName){

        char buf[256];
        buf[0] = '\0';
        int len = snprintf(buf, sizeof(buf), "usemtl %s\n", matName);

        outObj.resize(outObj.size() + len);
        memcpy(outObj.data() + outObj.size() - len, buf, len);

    };

    auto &gVert = mGeometry.m_vertices;
    auto &gTri = mGeometry.m_triangles;

    json matInfos = {};

    std::map<std::string, int> matNameUsedCnt;

    // Write Vertices to .obj
    for(int i = 0; i < gVert.m_size; i++) writeVertex(gVert[i][0], gVert[i][1], gVert[i][2]);

    for(int m = 0; m < mMaterials.size(); m++){

        auto &mat = mMaterials[m];

        std::string matName = (mat.mMatId != -1 ? sMatNames[mat.mMatId] : "UNDEFINED");

        if(matNameUsedCnt.find(matName) == matNameUsedCnt.end()) matNameUsedCnt[matName] = 0;

        char matId[256];
        matId[0] = '\0';
        snprintf(matId, sizeof(matId), "%s%02i", matName.c_str(), matNameUsedCnt[matName]++);

        // Write Material to .obj
        writeObject(matId);
        writeMat(matId);
        for(int i = 0; i < gTri.m_size; i++){
            if(gTri[i].m_material != m) continue;
            writeTriangle(gTri[i].m_a, gTri[i].m_b, gTri[i].m_c);
        }

        // Write Material info to json
        matInfos[matId] = {
            {"mat_name", matName},
            {"mat_flags", json::array()},
            {"col_disable_flags", json::array()},
        };
        for(u64 i = 0; i < 64; i++){
            if((mat.mMatFlags & (1ULL << i)) != 0 && i < sMatFlagNames.size())
                matInfos[matId]["mat_flags"].push_back(sMatFlagNames[i]);
            if((mat.mMatColFlags & (1ULL << i)) == 0 && i < sMatColFlagNames.size())
                matInfos[matId]["col_disable_flags"].push_back(sMatColFlagNames[i]);
        }

    }

    std::string matDataFile = matInfos.dump(4);
    outMatInfos.resize(matDataFile.length());
    memcpy(outMatInfos.data(), matDataFile.c_str(), matDataFile.length());

}

static Phive::PhiveMeshShape::vtable phiveMeshShapeCstmVtable;
static bool sInitCstmVtable = 0;

void (*phiveMeshShapeDtor0Impl)(Phive::PhiveMeshShape *meshShape);
void phiveMeshShapeDtor0Hook(Phive::PhiveMeshShape *meshShape){
    if(meshShape->mShape){
        meshShape->mShape->vftable->dtor2(meshShape->mShape);
        meshShape->mShape = NULL;
    }
    if(meshShape->mShapeHolder){
        meshShape->mShapeHolder->mShape = NULL;
        delete meshShape->mShapeHolder;
        meshShape->mShapeHolder = NULL;
    }

    if(meshShape->mMaterialArray){
        delete meshShape->mMaterialArray;
        meshShape->mMaterialArray = NULL;
    }
    meshShape->mMaterialNum = 0;
    meshShape->mPhiveShapeUserData.mMaterialArray = NULL;
    meshShape->mPhiveShapeUserData.mMaterialNum = 0;

    if(meshShape->mMatColFlags){
        delete meshShape->mMatColFlags;
        meshShape->mMatColFlags = NULL;
    }
    meshShape->mMatColFlagsNum = 0;
    meshShape->mPhiveShapeUserData.mMatColFlags = NULL;
    meshShape->mPhiveShapeUserData.mMatColFlagsNum = 0;

    phiveMeshShapeDtor0Impl(meshShape);
}

void (*phiveMeshShapeDtor2Impl)(Phive::PhiveMeshShape *meshShape);
void phiveMeshShapeDtor2Hook(Phive::PhiveMeshShape *meshShape){
    phiveMeshShapeDtor0Hook(meshShape);
    delete meshShape;
}

Phive::PhiveMeshShape *CollisionInfo::buildShape(){

    hknpMeshShapeBuildArg arg;
    hknpMeshShapeBuildArgCtor(&arg, &mGeometry, 1);
    arg._74 = 0;
    
    hknpMeshShapeUnkBuildArg2 arg2;

    hknpMeshShapeBuildResult res = hknpMeshShapeBuild(arg, arg2);

    if(res.shape == NULL) return NULL;

    Phive::PhiveMeshShape *shape = new Phive::PhiveMeshShape();

    if(!sInitCstmVtable){
        memcpy(&phiveMeshShapeCstmVtable, shape->vftable, sizeof(phiveMeshShapeCstmVtable));
        phiveMeshShapeDtor0Impl = phiveMeshShapeCstmVtable.dtor0;
        phiveMeshShapeCstmVtable.dtor0 = phiveMeshShapeDtor0Hook;
        phiveMeshShapeDtor2Impl = phiveMeshShapeCstmVtable.dtor2;
        phiveMeshShapeCstmVtable.dtor2 = phiveMeshShapeDtor2Hook;
        sInitCstmVtable = 1;
    }

    shape->vftable = &phiveMeshShapeCstmVtable;
    
    auto &m = mMaterials;

    shape->mMaterialNum = m.size();
    shape->mMaterialArray = new Phive::PhiveShapeMaterialData[m.size()];
    for(int i = 0; i < m.size(); i++) shape->mMaterialArray[i] = Phive::PhiveShapeMaterialData(m[i].mMatId, 0, m[i].mMatFlags);

    shape->mMatColFlagsNum = m.size();
    shape->mMatColFlags = new u64[m.size()];
    for(int i = 0; i < m.size(); i++) shape->mMatColFlags[i] = m[i].mMatColFlags;

    auto *userData = &shape->mPhiveShapeUserData;

    userData->mMaterialNum = shape->mMaterialNum;
    userData->mMaterialArray = shape->mMaterialArray;
    userData->mMatColFlagsNum = shape->mMatColFlagsNum;
    userData->mMatColFlags = shape->mMatColFlags;

    res.shape->m_userData = reinterpret_cast<hkUint64>(userData);
    shape->mShape = res.shape;

    return shape;
    
}

}