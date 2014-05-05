#ifndef value_if_h__
#define value_if_h__

ZTD_NAMESPACE_START;

template<bool UsingType1,int V1,int V2>
class value_if;

template<int V1,int V2>
class value_if<true,V1,V2>
{
public:
	FUNCTION_CLASS(value_if);
	enum { value=V1 };
};

template<int V1,int V2>
class value_if<false,V1,V2>
{
public:
	FUNCTION_CLASS(value_if);
	enum { value=V2 };
};

ZTD_NAMESPACE_END;

#endif // value_if_h__
