#ifndef value_min_h__
#define value_min_h__



template<int V1,int V2>
class value_min
{
public:
	enum { value=value_if< V1<=V2,V1,V2 >::value };
};



#endif // value_min_h__
