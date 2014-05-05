#ifndef ____header__e2121ioejwaaaaaa
#define ____header__e2121ioejwaaaaaa

class FreetypeLookAndFeel : public LookAndFeel_V3
{
public:
	void InitType(void*const ptr,size_t byteLen)
	{
		FreeTypeFaces::addFaceFromMemory(7.f, 20.f, true, ptr, byteLen);
	}
	Typeface::Ptr getTypefaceForFont (const Font& font) override
	{
		Typeface::Ptr tf = FreeTypeFaces::createTypefaceForFont (font);
        if( tf == nullptr ) tf = LookAndFeel_V3::getTypefaceForFont(font);
        return tf;
	};
private:

};

#endif //____header__e2121ioejwaaaaaa