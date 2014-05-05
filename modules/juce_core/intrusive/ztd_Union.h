#ifndef ztd_Zunion_h__
#define ztd_Zunion_h__

template<typename... T>
class Zunion
{
private:
	typedef type_queue<T...> TypeQueue; 
	typedef typename aligned_type<int8,TypeQueue::maxAlignof>::type  SmallContainer[TypeQueue::maxSizeof];
	typedef typename aligned_type<m128i,TypeQueue::maxAlignof>::type BigContainer[(TypeQueue::maxSizeof+15)/16];
	typedef typename type_if<(TypeQueue::maxSizeof<16),SmallContainer,BigContainer>::type ContainerToUse;
public:
	template<typename TypeToSearch>
	class ID
	{
	public:
		FUNCTION_CLASS(ID);
		static_assert(contain_type<TypeQueue,TypeToSearch>::result,"the type we get must in Zunion!");
		enum { index = search_index<TypeQueue,TypeToSearch>::result, 
			k =contain_type<TypeQueue,TypeToSearch>::result
		};
	};
public:
	TRIVIAL_CLASS(Zunion);
	ALIGNED_OPERATOR_NEW(Zunion,TypeQueue::maxAlignof);
	forcedinline int getID() const noexcept{ return m_typeId; }

	template<typename TypeToGet>
	forcedinline TypeToGet& get() noexcept
	{
		jassert(getID() == ID<TypeToGet>::index);//如果触发这个断点,说明你想get的类型不是当前类型
		return byte_ref_cast<TypeToGet>( m_data );
	}

	template<typename NewTypeToUse,typename Func>
	forcedinline void SetNewType(const Func&& func) noexcept
	{
		m_typeId = ID<NewTypeToUse>::index;
		func(get<NewTypeToUse>());
	}

	template<typename TypeToUse,typename Func>
	Zunion(const Func&& func) noexcept
	{
		SetNewType<TypeToUse>(std::move(func));
	}
private:
	ContainerToUse m_data;
	int m_typeId;
};



#endif // ztd_Zunion_h__
