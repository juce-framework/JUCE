#ifndef fetch_h__
#define fetch_h__

ZTD_NAMESPACE_START;

/************************************************************************
* 向全部缓存刷新内存地址
************************************************************************/
forcedinline void MemoryFetch(void*const ptr)
{
	_mm_prefetch((char*)ptr,_MM_HINT_NTA);
}

/************************************************************************
* 向T2缓存刷新内存地址
************************************************************************/
forcedinline void MemoryFetchT2(void*const ptr)
{
	_mm_prefetch((char*)ptr,_MM_HINT_T2);
}

/************************************************************************
* 向T1缓存刷新内存地址
************************************************************************/
forcedinline void MemoryFetchT1(void*const ptr)
{
	_mm_prefetch((char*)ptr,_MM_HINT_T1);
}

/************************************************************************
* 向T0缓存刷新内存地址
************************************************************************/
forcedinline void MemoryFetchT0(void*const ptr)
{
	_mm_prefetch((char*)ptr,_MM_HINT_T0);
}

ZTD_NAMESPACE_END;

#endif // fetch_h__
