#pragma once

#include "havok/hkTypes.h"

template<class T>
struct hkRelPtr {
    T *get(){
        if(m_offset == 0) return NULL;
        return reinterpret_cast<T*>(reinterpret_cast<hkUint8*>(this) + m_offset);
    };
    hkInt64 m_offset;
};