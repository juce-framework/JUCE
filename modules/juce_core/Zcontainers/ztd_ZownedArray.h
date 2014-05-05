#ifndef ____header__ddddddddddd33333231esgfdskjf
#define ____header__ddddddddddd33333231esgfdskjf

template<typename T,int size=-1>
class ZownedArray
{
public:
	ZownedArray()
		:m_arrayPtr(nullptr)
		,m_size(0)
	{};
	template<typename... Ipts> ZownedArray(int preSize,Ipts... ipts)
	    :ZownedArray()
	{
		resize( preSize , ipts... );
	}
	~ZownedArray()
	{
		free();
	}
	void free()
	{
		for(int i=0;i<m_size;++i) {
			(m_arrayPtr+i) -> ~T();
		}
		aligned_free(m_arrayPtr);
	}
	template<typename... Ipts> void DeleteAllAndReCreate( int newSize , Ipts... ipts )
	{
		free();
		void*const ptr= aligned_malloc(newSize*sizeof(T), value_max<ALIGNOF(T),64>::result );
		m_arrayPtr=ptr;
		for(int i=0;i<newSize;++i) {
			new(m_arrayPtr+i) T(ipts...);
		}
		m_size=newSize;
	}
	template<typename IndexType> T& operator[](const IndexType i) { return m_arrayPtr[i]; }
private:
	T* m_arrayPtr;
	int m_size;

};

#endif //____header__ddddddddddd33333231esgfdskjf