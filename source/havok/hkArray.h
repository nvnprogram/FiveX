#pragma once 

template<class T>
class hkArray {
    public:
    T &operator[](int id) { return m_data[id]; };
    T *m_data;
    int m_size;
    int m_capacityAndFlags;
};
