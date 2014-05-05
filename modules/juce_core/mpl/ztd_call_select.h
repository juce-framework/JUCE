#ifndef call_select_h__
#define call_select_h__

ZTD_NAMESPACE_START;

#ifndef TINY_MAX_SIZE
#  define TINY_MAX_SIZE 50
#endif

template<int i>
class call_select;

template<>
class call_select<0>
{
public:
// 	template <BOOST_PP_ENUM_PARAMS(TINY_MAX_SIZE,typename Func)>
// 	static forcedinline auto call(BOOST_PP_ENUM_PARAMS(TINY_MAX_SIZE,const Func?&& func??))
// 	{
// 		return func1();
// 	}
	template<typename Func1,typename Func2,typename Func3,typename Func4>
	static forcedinline auto call(const Func1&& func1,const Func2&&,const Func3&&,const Func4&&,const Func2&&,const Func3&&,const Func4&&,const Func2&&,const Func3&&,const Func4&&,const Func2&&,const Func3&&,const Func4&&) -> decltype( func1() )
	{
		return func1();
	}
	template<typename Func1,typename Func2,typename Func3,typename Func4>
	static forcedinline auto call(const Func1&& func1,const Func2&&,const Func3&&,const Func4&&) -> decltype(func1())
	{
		return func1();
	}
	template<typename Func1,typename Func2,typename Func3>
	static forcedinline auto call(const Func1&& func1,const Func2&&,const Func3&&) -> decltype( func1() )
	{
		return func1();
	}
	template<typename Func1,typename Func2>
	static forcedinline auto call(const Func1&& func1,const Func2&&) -> decltype( func1() )
	{
		return func1();
	}
	template<typename Func1>
	static forcedinline auto call(const Func1&& func1) -> decltype( func1() )
	{
		return func1();
	}
};

template<>
class call_select<1>
{
public:
	template<typename Func1,typename Func2,typename Func3,typename Func4>
	static forcedinline auto call(const Func1&&,const Func2&& func2,const Func3&&,const Func4&&) -> decltype( func2() )
	{
		return func2();
	}
	template<typename Func1,typename Func2,typename Func3>
	static forcedinline auto call(const Func1&&,const Func2&& func2,const Func3&&) -> decltype( func2() )
	{
		return func2();
	}
	template<typename Func1,typename Func2>
	static forcedinline auto call(const Func1&&,const Func2&& func2) -> decltype( func2() )
	{
		return func2();
	}
};

template<>
class call_select<2>
{
public:
	template<typename Func1,typename Func2,typename Func3,typename Func4>
	static forcedinline auto call(const Func1&&,const Func2&&,const Func3&& func3,const Func4&&) -> decltype( func3() )
	{
		return func3();
	}
	template<typename Func1,typename Func2,typename Func3>
	static forcedinline auto call(const Func1&&,const Func2&& func2,const Func3&& func3) -> decltype( func3() )
	{
		return func3();
	}
};

template<>
class call_select<3>
{
public:
	template<typename Func1,typename Func2,typename Func3,typename Func4>
	static forcedinline auto call(const Func1&&,const Func2&&,const Func3&&,const Func4&& func4) -> decltype( func4() )
	{
		return func4();
	}
};

ZTD_NAMESPACE_END;

#endif // call_select_h__
