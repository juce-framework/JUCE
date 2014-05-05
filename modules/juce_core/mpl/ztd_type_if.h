#ifndef type_if_h__
#define type_if_h__



template<bool UsingType1,typename Type1,typename Type2>
class type_if;

template<typename Type1,typename Type2>
class type_if<true,Type1,Type2>
{
public:
    FUNCTION_CLASS(type_if);
	typedef Type1 type;
};

template<typename Type1,typename Type2>
class type_if<false,Type1,Type2>
{
public:
    FUNCTION_CLASS(type_if);
	typedef Type2 type;
};



#endif // type_if_h__
