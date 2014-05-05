#ifndef fetch_h__
#define fetch_h__

/************************************************************************
* 向全部缓存刷新内存地址
************************************************************************/
forcedinline void memfetch(void*const restrict ptr)
{
	_mm_prefetch((char*)ptr,_MM_HINT_NTA);
}

/************************************************************************
* 向T2缓存刷新内存地址
************************************************************************/
forcedinline void memfetchT2(void*const restrict ptr)
{
	_mm_prefetch((char*)ptr,_MM_HINT_T2);
}

/************************************************************************
* 向T1缓存刷新内存地址
************************************************************************/
forcedinline void memfetchT1(void*const restrict tr)
{
	_mm_prefetch((char*)ptr,_MM_HINT_T1);
}

/************************************************************************
* 向T0缓存刷新内存地址
************************************************************************/
forcedinline void memfetchT0(void*const restrict ptr)
{
	_mm_prefetch((char*)ptr,_MM_HINT_T0);
}

#endif // fetch_h__
