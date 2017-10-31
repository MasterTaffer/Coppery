#pragma once
#include <cstddef>

template<typename T>
class BaseVertexBuffer
{
    size_t size;
    unsigned int name;
    int usage;
    int target;

    void generate();
public:
    size_t getSize() const
    {
        return size;
    }
    unsigned int getName() const
    {
        return name;
    }

    void free();
    void reserve(size_t s);
    void load(const T* data, size_t s, size_t index = 0);
    void setUsage(int t);
    void setTarget(int t);

    BaseVertexBuffer();
    ~BaseVertexBuffer();

};

typedef BaseVertexBuffer<char> ByteBuffer;
typedef BaseVertexBuffer<float> VertexBuffer;
typedef BaseVertexBuffer<int> IndexBuffer;