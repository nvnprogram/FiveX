#pragma once

#include "havok/hkTypes.h"

class hkBaseObject {
    public:
    struct vtable{
        void *(*getExactType)(hkBaseObject *_this);
        void (*dtor0)(hkBaseObject *_this);
        void (*dtor2)(hkBaseObject *_this);
        void (*destroy)(hkBaseObject *_this);
    };
    ~hkBaseObject(){
        vftable->dtor0(this);
    }
    vtable *vftable;
};
static_assert(sizeof(hkBaseObject) == 0x8L);
static_assert(alignof(hkBaseObject) == 0x8L);
