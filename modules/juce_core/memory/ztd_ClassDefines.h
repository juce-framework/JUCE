#ifndef ztd_ClassDef_h__
#define ztd_ClassDef_h__

/*****************************************************
	将此宏加入到类声明中，表明该类只能在栈上创建 
******************************************************/
#define STACK_CLASS(classname) \
	void* operator new(size_t)=delete; \
	void operator delete(void*)=delete; \
	void* operator new[](size_t)=delete; \
	void operator delete[](void*)=delete;


/*******************************************************
	将此宏加入到类声明中，表明该类只提供static的方法，用户不能创建该类的实例 
******************************************************/
#define FUNCTION_CLASS(classname) \
	classname()=delete; \
	~classname()=delete; \
	STACK_CLASS(classname)


/*******************************************************
	将此宏加入到类声明中，该类将自动持有默认的构造函数，拷贝构造函数，赋值函数，析构函数 
********************************************************/
#if JUCE_DEBUG
#	define TRIVIAL_CLASS(classname) \
		classname(){}; \
		classname(const classname& other){ memcpy(this,&other,sizeof(classname)); };\
		classname& operator=(const classname& other){ memcpy(this,&other,sizeof(classname));return *this; };\
		~classname(){} \
		JUCE_LEAK_DETECTOR(classname) //这个宏只管new和delete,所以拷贝构造和"="操作符不受影响
#else
#	define TRIVIAL_CLASS(classname) \
	classname()=default; \
	classname(const classname& other)=default;\
	classname& operator=(const classname& other)=default;\
	~classname()=default;
#endif

/********************************************************* 
	将此宏加入到类声明中，该类将自动持有默认的构造函数，析构函数,但没有拷贝构造函数 
******************************************************/
#define TRIVIAL_NOCOPYABLE_CLASS(classname) \
	classname() = default; \
	classname(const classname&) = delete; \
	classname& operator=( const classname& ) = delete; \
	~classname() = default; \
	JUCE_LEAK_DETECTOR(classname)



/******************************************************** 
	将此宏加入到类声明中，该类将禁止拷贝构造函数和拷贝赋值函数
******************************************************/
#define NONCOPYABLE_CLASS(classname) \
	classname(const classname&)=delete;\
	classname& operator=(const classname&)=delete;\
	JUCE_LEAK_DETECTOR(classname)


#endif // ztd_ClassDef_h__
