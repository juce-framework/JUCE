#ifndef ____header__e2121ioejwaaaaaa
#define ____header__e2121ioejwaaaaaa

class FreetypeLookAndFeel : public LookAndFeel_V3
{
public:
	void InitType(void*const ptr,size_t byteLen)
	{
		FreeTypeFaces::getInstance()->addFaceFromMemory(
            7.f     , 
            12.f    ,
            ptr     ,
            byteLen );
	}
	Typeface::Ptr getTypefaceForFont (const Font& font) override
	{
		Typeface::Ptr tf = FreeTypeFaces::createTypefaceForFont (f);
        if( tf == nullptr ) tf = LookAndFeel_V3::getTypefaceForFont(font);
        return tf;
	};
private:

};

#endif //____header__e2121ioejwaaaaaa