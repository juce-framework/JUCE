#ifndef call_if_h__
#define call_if_h__

ZTD_NAMESPACE_START;

template<bool UsingType1>
class call_if;

template<>
class call_if<true>
{
public:
    FUNCTION_CLASS(call_if);
public:
    template<typename Func1,typename Func2>
    static forcedinline auto call(const Func1&& func1,const Func2&&) ->decltype(func1()) { return func1(); }
};

template<>
class call_if<false>
{
public:
     FUNCTION_CLASS(call_if);
public:
    template<typename Func1,typename Func2>
    static forcedinline auto call(const Func1&&,const Func2&& func2) ->decltype(func2()) { return func2(); }
};

ZTD_NAMESPACE_END;

#endif // call_if_h__
