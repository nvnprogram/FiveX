#pragma once

#include "havok/hkTypes.h"

template<class T>
class hkRelArray {
    public:
    T *begin(){
        return reinterpret_cast<T*>(reinterpret_cast<hkUint8*>(this) + m_offset);
    };
    T &operator[](int id) { return begin()[id]; };
    hkInt64 m_offset;
    int m_size;
    int m_capacityAndFlags;
};

template<class T, class OFFSET_TYPE>
struct hkRelArrayView {
    T *begin(){
        return reinterpret_cast<T*>(reinterpret_cast<hkUint8*>(this) + ((hkUint64)m_offset));
    };
    T &operator[](int id) { return begin()[id]; };
    int m_offset;
    int m_size;
};