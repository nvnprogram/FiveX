#pragma once

#include "havok/hkBaseObject.h"

class hkReferencedObject : public hkBaseObject {
    public:
    hkUlong m_sizeAndFlags;
    hkUlong m_refCount;
};