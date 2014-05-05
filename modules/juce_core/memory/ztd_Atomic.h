#ifndef ztd_Zatomic_h__dsdaaaaaaaaaaaaa3243222222222222
#define ztd_Zatomic_h__dsdaaaaaaaaaaaaa3243222222222222

ZTD_NAMESPACE_START;

/****************************************************
* 原子类,内部使用了C++11的原子类所以编译器要支持C++11,
* 之所以要在std::atomic外面再包一层,是因为内存模型要修改为memory_order_acq_rel,而不是std::atomic默认的(默认的太慢了!而且未必是lockfree的)
* 要使用原子类,类型T必须是TRIVIAL的,且平台支持sizeof(T)尺寸的lockfree操作(不能lockfree原子类还有毛用),原子类支持的sizeof(T)针对不同平台有所不同
****************************************************************/

template<typename T>
class Zatomic
{
public:
	Zatomic() = default;
	Zatomic(const Zatomic&) = default;
	Zatomic& operator=( const Zatomic& ) = default;
	~Zatomic() = default;
	//* 可以使用一个T的值来初始化原子类,请注意! 这个操作不是原子的
	forcedinline explicit Zatomic(const T initValue) noexcept;
	//* 将原子的值替换为新值,这个操作是原子的
	forcedinline void operator=( T value ) volatile noexcept;
	forcedinline T operator++(int) volatile noexcept;
	forcedinline T operator++( ) volatile noexcept;
	forcedinline T operator--(int) volatile noexcept;
	forcedinline T operator--( ) volatile noexcept;
	//* 先得到原来的值,再相加
	forcedinline T fetch_add(const int k) volatile noexcept;
	//* 先相加,再得到新的值
	forcedinline T add_fetch(const int k) volatile noexcept;
	//* 先得到原来的值,再相减
	forcedinline T fetch_sub(const int k) volatile noexcept;
	//* 先相减,再得到新的值
	forcedinline T sub_fetch(const int k) volatile noexcept;
	//* 原子load
	forcedinline T load() volatile noexcept;
	//* 原子存储
	forcedinline void store(const T k) volatile noexcept;
	//* 等同于原子load
	forcedinline T get() volatile noexcept;
	//* 等同于原子存储
	forcedinline void set(const T k) volatile noexcept;
	//* 用newValue替代原子值,返回老的值
	forcedinline T exchange(const T newValue) volatile noexcept;
	//* 若valueToCmp等于原子当前的值,则将原子值替换为newValue,返回原子的旧值,若不然,不修改原子值,返回原子的旧值
	forcedinline T compareAndSetValue(const T valueToCmp,const T newValue) volatile noexcept;
	//* 若valueToCmp等于原子当前的值,则将原子值替换为newValue,返回true,若不然,不修改原子值,返回false
	forcedinline bool compareAndSetBool(const T valueToCmp,const T newValue) volatile noexcept;
public:
	T m_data;
};

/*********************************************************************************************
* 可以使用一个T的值来初始化原子类,请注意! 这个操作不是原子的
*********************************************************************************************/
template<typename T>
forcedinline Zatomic<T>::Zatomic(const T initValue) noexcept
	:m_data(initValue)
{}

/*********************************************************************************************
* 这个操作是原子的
*********************************************************************************************/
template<typename T>
forcedinline void Zatomic<T>::operator=( T value ) volatile noexcept
{
	set(value);
}

/*********************************************************************************************
* 这个操作是原子的
*********************************************************************************************/
template<typename T>
forcedinline T Zatomic<T>::operator++(int) volatile noexcept
{
	return fetch_add(1);
}

/*********************************************************************************************
* 这个操作是原子的
*********************************************************************************************/
template<typename T>
forcedinline T Zatomic<T>::operator++( ) volatile noexcept
{
	return add_fetch(1);
}

/*********************************************************************************************
* 这个操作是原子的
*********************************************************************************************/
template<typename T>
forcedinline T Zatomic<T>::operator--(int) volatile noexcept
{
	return fetch_sub(1);
}

/*********************************************************************************************
* 这个操作是原子的
*********************************************************************************************/
template<typename T>
forcedinline T Zatomic<T>::operator--( ) volatile noexcept
{
	return sub_fetch(1);
}

/*********************************************************************************************
* 这个操作是原子的
*********************************************************************************************/
template<typename T>
forcedinline T Zatomic<T>::fetch_add(const int k) volatile noexcept
{
	return ZInterlockedFetchAndAdd(m_data,k);
}

/*********************************************************************************************
* 这个操作是原子的
*********************************************************************************************/
template<typename T>
forcedinline T Zatomic<T>::add_fetch(const int k) volatile noexcept
{
	return ZInterlockedAddAndFetch(m_data,k);
}

/*********************************************************************************************
* 这个操作是原子的
*********************************************************************************************/
template<typename T>
forcedinline T Zatomic<T>::fetch_sub(const int k) volatile noexcept
{
	return ZInterlockedFetchAndSub(m_data,k);
}

/*********************************************************************************************
* 这个操作是原子的
*********************************************************************************************/
template<typename T>
forcedinline T Zatomic<T>::sub_fetch(const int k) volatile noexcept
{
	return ZInterlockedSubAndFetch(m_data,k);
}

/*********************************************************************************************
* 这个操作是原子的
*********************************************************************************************/
template<typename T>
forcedinline T Zatomic<T>::load() volatile noexcept
{
	static_assert(IS_TRIVIAL(T),"T must be POD");
	return ZInterlockedLoad(m_data);
}

/*********************************************************************************************
* 这个操作是原子的
*********************************************************************************************/
template<typename T>
forcedinline void Zatomic<T>::store(const T k) volatile noexcept
{
	static_assert(IS_TRIVIAL(T),"T must be POD");
	ZInterlockedStore(m_data,k);
}

/*********************************************************************************************
* 这个操作是原子的
*********************************************************************************************/
template<typename T>
forcedinline T Zatomic<T>::get() volatile noexcept
{
	return load();
}

/*********************************************************************************************
* 这个操作是原子的
*********************************************************************************************/
template<typename T>
forcedinline void Zatomic<T>::set(const T k) volatile noexcept
{
	store(k);
}

/*********************************************************************************************
* 这个操作是原子的
*********************************************************************************************/
template<typename T>
forcedinline T Zatomic<T>::exchange(const T newValue) volatile noexcept
{
	return ZInterlockedExchange(m_data,newValue);
}

/*********************************************************************************************
* 这个操作是原子的
*********************************************************************************************/
template<typename T>
forcedinline T Zatomic<T>::compareAndSetValue(const T valueToCmp,const T newValue) volatile noexcept
{
	return ZInterlockedCompareAndSetValue(m_data,valueToCmp,newValue);
}

/*********************************************************************************************
* 这个操作是原子的
*********************************************************************************************/
template<typename T>
forcedinline bool Zatomic<T>::compareAndSetBool(const T valueToCmp,const T newValue) volatile noexcept
{
	return ZInterlockedCompareAndSetBool(m_data,valueToCmp,newValue);
}

ZTD_NAMESPACE_END;

#endif // ztd_Zatomic_h__
