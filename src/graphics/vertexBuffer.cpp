#include "vertexBuffer.hpp"
#include "oGL.hpp"

template <typename T>
void BaseVertexBuffer<T>::reserve(size_t s)
{
	if (s == 0)
		return;

	if (size >= s)
		return;

	generate();

    #ifndef COPPERY_HEADLESS
	glBindBuffer(target, name);
	glBufferData(target, s * sizeof(T), nullptr, usage);
	glBindBuffer(target, 0);
	#endif

}
template <typename T>
void BaseVertexBuffer<T>::load(const T* data, size_t s, size_t index)
{
	if (s == 0)
		return;


	#ifndef COPPERY_HEADLESS


	if (size >= s + index)
	{
		glBindBuffer(target, name);
		glBufferSubData(target, index * sizeof(T), s * sizeof(T), (void*) data);
		glBindBuffer(target, 0);
		return;
	}
	else
	{
		//Not enough size in the buffer: complete reallocation must be
		//performed

		//If the buffer isn't even generated, generate it now
		generate();
		glBindBuffer(target, name);
		if (index == 0)
		{
			//If the index is zero, simply replace whole buffer
			//with input data
			glBufferData(target, s * sizeof(T), data, usage);
			size = s;
		}
		else
		{
			//Otherwise an empty hole is left in the beginning
			glBufferData(target, (s + index) * sizeof(T), nullptr, usage);
			glBufferSubData(target, index * sizeof(T), s * sizeof(T), data);
			size = s + index;
		}
		glBindBuffer(target, 0);		
		
		return;
	}
	#endif
		
}
template <typename T>
void BaseVertexBuffer<T>::setUsage(int t)
{
	usage = t;
}
template <typename T>
void BaseVertexBuffer<T>::setTarget(int t)
{
	target = t;
}



template <typename T>
void BaseVertexBuffer<T>::generate()
{

    #ifndef COPPERY_HEADLESS
	if (name == 0)
		glGenBuffers(1, &name);
	#endif
}

template <typename T>
void BaseVertexBuffer<T>::free()
{

    #ifndef COPPERY_HEADLESS
	if (name != 0)
		glDeleteBuffers(1, &name);
	#endif
	name = 0;
}

template <typename T>
BaseVertexBuffer<T>::BaseVertexBuffer()
{

    #ifndef COPPERY_HEADLESS
	usage = GL_STATIC_DRAW;
	target = GL_ARRAY_BUFFER;
	#else
	usage = 0;
	target = 0;
	#endif
	name = 0;
	size = 0;
}

template <typename T>
BaseVertexBuffer<T>::~BaseVertexBuffer()
{
	free();
}



template class BaseVertexBuffer<float>;

template class BaseVertexBuffer<int>;

template class BaseVertexBuffer<char>;