#include "lib.hpp"
#include "nn/oe.hpp"
#include "lib/Json.hpp"
#include "fivex/SocketClient.hpp"
#include "fivex/CollisionInfo.hpp"
#include "fivex/PhiveWriter.hpp"
#include <cstdarg>

void logF(const char *format, ...){
    va_list args;
    va_start(args, format);

    char buff[0x1000];
    buff[0] = '\0';

    int tlen = vsnprintf(buff, sizeof(buff), format, args);
    va_end(args);

    svcOutputDebugString(buff, tlen);
}

namespace FiveX{
    enum class OpType{
        LoadPhiveConfig = 0,
        PhiveToObj = 1,
        ObjToPhive = 2
    };
    enum class RespType{
        Error = 0,
        LoadedPhiveConfig = 1,
        ObjAndMats = 2,
        Phive = 3,
    };
    struct InHeader{
        OpType mOperationType;
        u32 mMessageSize;
    };
    struct OutHeader{
        RespType mResponseType;
        u32 mMessageSize;
    };
};

static FiveX::SocketClient sSocket;

bool waitConnect(){

    if(sSocket.mIsConnectedClient) return 1;

    if(!sSocket.mIsConnected){
        if(!sSocket.bind(8778)){
            logF("Failed to bind!");
            return 0;
        }
    }

    logF("Waiting for commands!");
    
    return sSocket.Accept();

}

bool respondError(const char *text){

    if(!sSocket.mIsConnectedClient) return 0;

    logF(text);

    FiveX::OutHeader resp(FiveX::RespType::Error, strlen(text));

    if(!sSocket.SendMessage(&resp, sizeof(resp))) return 0;
    if(!sSocket.SendMessage(text, resp.mMessageSize)) return 0;

    return 1;

}

void appLoop(){

    if(!sSocket.mIsConnectedClient) waitConnect();

    FiveX::InHeader msg;

    if(!sSocket.ReceiveMessage(&msg, sizeof(msg))) return;

    u8 *msgBuf = reinterpret_cast<u8*>(malloc(msg.mMessageSize + 1));
    msgBuf[msg.mMessageSize] = 0;

    if(!sSocket.ReceiveMessage(msgBuf, msg.mMessageSize)){
        logF("Failed to receive msg %i", msg.mOperationType);
        free(msgBuf);
        return;
    }

    if(msg.mOperationType == FiveX::OpType::LoadPhiveConfig){

        json config = json::parse(reinterpret_cast<const char*>(msgBuf), NULL, 0);
        free(msgBuf);

        logF("Loading phive config...");

        if(config.is_discarded()){
            respondError("Failed to parse phive config json!");
            return;
        }

        FiveX::CollisionInfo::setConfig(config);

        FiveX::OutHeader resp(FiveX::RespType::LoadedPhiveConfig, 0);
        
        if(!sSocket.SendMessage(&resp, sizeof(resp))) return;

    }

    if(msg.mOperationType == FiveX::OpType::ObjToPhive){

        u32 *sizes = reinterpret_cast<u32*>(msgBuf);
        u32 objSize = sizes[0];
        u32 matConfigSize = sizes[1];

        u8 *objData = msgBuf + 8;
        json config = {};
        if(matConfigSize){
            config = json::parse(reinterpret_cast<const char*>(objData + objSize), NULL, 0);
            if(config.is_discarded()) config = {};
        }

        logF("Converting .obj to phive...");

        FiveX::CollisionInfo *info = new FiveX::CollisionInfo();
        if(!info->initialize(objData, objSize, config)){
            respondError("Failed to parse the .obj file!");
            free(msgBuf);
            delete info;
            return;
        }

        free(msgBuf);

        Phive::PhiveMeshShape *shape = info->buildShape();
        
        delete info;

        if(shape == NULL){
            respondError("Failed to build a mesh shape from the .obj file!");
            return;
        }

        FiveX::PhiveWriter writer;
        writer.serialize(shape);

        shape->vftable->dtor0(shape);
        delete shape;

        FiveX::OutHeader resp(FiveX::RespType::Phive, writer.mData.size());

        if(!sSocket.SendMessage(&resp, sizeof(resp))) return;
        if(!sSocket.SendMessage(writer.mData.data(), writer.mData.size())) return;

    }

    if(msg.mOperationType == FiveX::OpType::PhiveToObj){

        Phive::PhiveMeshShape *shape = new Phive::PhiveMeshShape();
        // shape is inplace loaded, so msgBuf has to be freed only after it's deinit
        shape->vftable->load(shape, msgBuf, msg.mMessageSize, NULL); 

        logF("Converting phive to .obj...");
        
        if(shape->mShape == NULL){
            respondError("Failed to load the mesh shape from the .bphsh file!");
            shape->vftable->dtor0(shape);
            delete shape;
            free(msgBuf);
            return;
        }
        
        FiveX::CollisionInfo *info = new FiveX::CollisionInfo();
        if(!info->initialize(shape)){
            respondError("Failed to parse the mesh shape into geometry!");
            shape->vftable->dtor0(shape);
            delete shape;
            free(msgBuf);
            return;
        }
        
        shape->vftable->dtor0(shape);
        delete shape;
        free(msgBuf);

        std::vector<u8> objData;
        std::vector<u8> matData;
        info->serializeToObj(objData, matData);

        delete info;

        u32 sizes[] = {objData.size(), matData.size()};

        FiveX::OutHeader resp(FiveX::RespType::ObjAndMats, sizeof(u32) * 2 + sizes[0] + sizes[1]);

        if(!sSocket.SendMessage(&resp, sizeof(resp))) return;
        if(!sSocket.SendMessage(sizes, sizeof(sizes))) return;
        if(!sSocket.SendMessage(objData.data(), sizes[0])) return;
        if(!sSocket.SendMessage(matData.data(), sizes[1])) return;
        
    }
}

extern "C" void nnMain();
void mainHook(){

    nn::oe::Initialize();
    nn::oe::FinishStartupLogo();

    nn::nifm::Initialize();
    nn::nifm::SubmitNetworkRequestAndWait();

    static char ALIGNA(0x1000) g_SocketMemoryPoolBuffer[6 * 1024 * 1024];
    nn::socket::Initialize(g_SocketMemoryPoolBuffer, 6 * 1024 * 1024, 0x180000, 14);
    
    logF("Starting socket server...");

    while(true) appLoop();
    
}

Phive::PhiveMeshShape::vtable *Phive::PhiveMeshShape::sVtable = NULL;
hknpShapeFunctionsSingleton **hknpShapeFunctionsSingleton::sInstancePtr = NULL;
void (*hknpMeshShapeBuildArgCtor)(hknpMeshShapeBuildArg *arg, hkGeometry *geom, bool unk);
hknpMeshShapeBuildResult (*hknpMeshShapeBuild)(hknpMeshShapeBuildArg &arg, hknpMeshShapeUnkBuildArg2 &arg2);
void (*hkGeometry::hkGeometryCtor)(hkGeometry *_this);

#define SetFuncPtr(func, addr) (*(uintptr_t*)&func) = (uintptr_t)(addr)

extern "C" void exl_main(void* x0, void* x1) {

    exl::hook::Initialize();

    nn::oe::DisplayVersion gameVer;
    nn::oe::GetDisplayVersion(&gameVer);
    logF("Game Version: %s", gameVer.name);

    if(strcmp(gameVer.name, "1.0.0") != 0){ // TODO: add pattern search for auto port
        logF("This tool is currently only supported on Splatoon 3 v1.0.0!");
        EXL_CALL_ABORT_IMPL("Unsupported version", 0);
        return;
    }

    exl::patch::CodePatcher(0x3C0618C).BranchInst((void*)mainHook);
    u64 targetStart = exl::util::GetMainModuleInfo().m_Total.m_Start;
    Phive::PhiveMeshShape::sVtable = (Phive::PhiveMeshShape::vtable*)(targetStart + 0x5745CE8);
    hknpShapeFunctionsSingleton::sInstancePtr = (hknpShapeFunctionsSingleton**)(targetStart + 0x57DD738);
    SetFuncPtr(hknpMeshShapeBuildArgCtor, targetStart + 0xACFC68);
    SetFuncPtr(hknpMeshShapeBuild, targetStart + 0x99E7F0);
    SetFuncPtr(hkGeometry::hkGeometryCtor, targetStart + 0x900818);

    logF("Hooks installed!");

}

extern "C" NORETURN void exl_exception_entry() {
    /* TODO: exception handling */
    EXL_ABORT(0x420);
}