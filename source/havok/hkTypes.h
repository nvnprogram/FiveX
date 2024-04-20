#pragma once

typedef unsigned char hkUint8;
typedef unsigned short hkUint16;
typedef unsigned int hkUint32;
typedef unsigned long long hkUlong;
typedef unsigned long long hkUint64;
typedef char hkInt8;
typedef short hkInt16;
typedef short hkHalf16;
typedef int hkInt32;
typedef long long hkLong;
typedef long long hkInt64;
typedef float hkReal;
typedef bool hkBool;

typedef hkUint32 hknpLevelOfDetail;

using hkVector4f = float[0x4L];
typedef hkVector4f hkVector4;

struct hkFloat3 {
    float m_x;
    float m_y;
    float m_z;
};
static_assert(sizeof(hkFloat3) == 0xcL);