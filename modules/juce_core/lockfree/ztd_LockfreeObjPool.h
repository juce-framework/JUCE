#ifndef LockfreeObjPool_h__
#define LockfreeObjPool_h__

ZTD_NAMESPACE_START;

template<typename T,int allocPreSize=4*1024*16/sizeof(T)>
using LockfreeObjPool=ObjPool<T,true,allocPreSize>;

ZTD_NAMESPACE_END;

#endif // LockfreeObjPool_h__
