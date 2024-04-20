#pragma once

#include "havok/hkReferencedObject.h"
#include "havok/hkArray.h"

class hkGeometry : public hkReferencedObject {
    public:

    static void (*hkGeometryCtor)(hkGeometry *_this);
    hkGeometry(){
        hkGeometryCtor(this);
    }

    struct Triangle{
        int m_a;
        int m_b;
        int m_c;
        int m_material;
    };
    static_assert(sizeof(Triangle) == 0x10L);

    hkArray<hkVector4> m_vertices;
    hkArray<Triangle> m_triangles;
    
};
static_assert(sizeof(hkGeometry) == 0x38L);