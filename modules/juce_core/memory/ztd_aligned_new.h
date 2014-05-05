#ifndef aligned_class_h__
#define aligned_class_h__

/************************************************************************/
/* 在类定义中使用此宏,new和delete,以及其[]均为对齐到x的.                */
/************************************************************************/
#define ALIGNED_OPERATOR_NEW(classname,x) \
	void* operator new( size_t size ,size_t alignedByte= x ) \
	{ \
		void*const k = ztd::aligned_malloc(size,alignedByte); \
		if(k == nullptr) throw std::bad_alloc(); \
		checkPtrSIMD(k,alignedByte);\
		return k; \
	}; \
	void operator delete( void* ptr ,size_t) \
	{ \
		ztd::aligned_free(ptr); \
	}; \
	void* operator new[](size_t size,size_t alignedByte= x ) \
	{ \
		void*const k = ztd::aligned_malloc(size,alignedByte); \
		if(k == nullptr) throw std::bad_alloc(); \
		checkPtrSIMD(k,alignedByte);\
		return k; \
	}; \
	void operator delete[](void* ptr,size_t) \
	{ \
		ztd::aligned_free(ptr); \
	};


#endif // aligned_class_h__
