#ifndef ztd_align_type_h__
#define ztd_align_type_h__

#include "../../ztd_core.h"

using juce::ScopedPointer;

template <class ObjectType>
class ScopedPointer;

template<typename T,size_t alignByte>
class aligned_type;

template<typename T>
class aligned_type<T,2>
{
public:
	FUNCTION_CLASS(aligned_type);
	typedef JUCE_ALIGN(2) T type;
	typedef JUCE_ALIGN(2) T* pointer;
	typedef ScopedPointer<type> ScopedPtr;
};

template<typename T>
class aligned_type<T,4>
{
public:
	FUNCTION_CLASS(aligned_type);
	typedef JUCE_ALIGN(4) T type;
	typedef JUCE_ALIGN(4) T* pointer;
	typedef ScopedPointer<type> ScopedPtr;
};

template<typename T>
class aligned_type<T,8>
{
public:
	FUNCTION_CLASS(aligned_type);
	typedef JUCE_ALIGN(8) T type;
	typedef JUCE_ALIGN(8) T* pointer;
	typedef ScopedPointer<type> ScopedPtr;
};

template<typename T>
class aligned_type<T,16>
{
public:
	FUNCTION_CLASS(aligned_type);
	typedef JUCE_ALIGN(16) T type;
	typedef JUCE_ALIGN(16) T* pointer;
	typedef ScopedPointer<type> ScopedPtr;
};

template<typename T>
class aligned_type<T,32>
{
public:
	FUNCTION_CLASS(aligned_type);
	typedef JUCE_ALIGN(32) T type;
	typedef JUCE_ALIGN(32) T* pointer;
	typedef ScopedPointer<type> ScopedPtr;
};

template<typename T>
class aligned_type<T,64>
{
public:
	FUNCTION_CLASS(aligned_type);
	typedef JUCE_ALIGN(64) T type;
	typedef JUCE_ALIGN(64) T* pointer;
	typedef ScopedPointer<type> ScopedPtr;
};

template<typename T>
class aligned_type<T,128>
{
public:
	FUNCTION_CLASS(aligned_type);
	typedef JUCE_ALIGN(128) T type;
	typedef JUCE_ALIGN(128) T* pointer;
	typedef ScopedPointer<type> ScopedPtr;
};



#endif // ztd_align_type_h__
