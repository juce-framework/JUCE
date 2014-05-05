#ifndef ____header__ddddddddddd33333231esgfdskjf
#define ____header__ddddddddddd33333231esgfdskjf

template<typename T,int size=-1>
class ZownedArray
{
public:
	ZownedArray():m_arrayPtr(nullptr){};
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
		delete[] m_arrayPtr;
	}
	template<typename... Ipts> DeleteAllAndReCreate( int newSize , Ipts... ipts )
	{
		free();
		m_arrayPtr=new T(ipts...)[newSize];
	}
	template<typename IndexType> T& operator[](const IndexType i) { return m_arrayPtr[i]; }
private:
	T* m_arrayPtr;

};

#endif //____header__ddddddddddd33333231esgfdskjf