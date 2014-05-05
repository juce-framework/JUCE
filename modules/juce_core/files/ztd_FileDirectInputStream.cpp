

#if JUCE_MSVC
#pragma warning (push)
#pragma warning (disable: 4996)
#endif

/*

FileDirectInputStream::FileDirectInputStream(const File& fileToRead)
	:file(fileToRead)
	,m_status(Result::ok())
	,fileHandle(-1)
	,m_fileLength(0)
	,m_pos(0)
	,needToSeek(false)
{
	fileHandle=_wopen(file.getFullPathName().toWideCharPointer(),_O_RDONLY|_O_BINARY|_O_U8TEXT);
	if(!openedOk()) {
		m_status=Result::fail("can not open file");
	}
}

FileDirectInputStream::~FileDirectInputStream()
{
    if(openedOk()) _close(fileHandle);
}

int FileDirectInputStream::read(void* destBuffer ,int maxBytesToRead)
{
	jassert(openedOk());
	if (needToSeek) {
		if (_lseeki64(fileHandle ,m_pos ,SEEK_SET)==(-1L)) return 0;
		needToSeek=false;
	}
	const int thisReadLen=_read(fileHandle ,destBuffer ,maxBytesToRead);

	m_pos+=thisReadLen;
	return thisReadLen;
}

bool FileDirectInputStream::setPosition(int64 pos)
{
	jassert(openedOk());
	if (pos!=m_pos) {
		pos=jlimit((int64)0 ,getTotalLength() ,pos);

		needToSeek|=(m_pos!=pos);
		m_pos=pos;
	}
	return true;
}


int64 FileDirectInputStream::getPosition()
{
	return m_pos;
}

bool FileDirectInputStream::isExhausted()
{
	return m_pos>=m_fileLength;
}

int64 FileDirectInputStream::getTotalLength()
{
	return m_fileLength;
}

const File& FileDirectInputStream::getFile() const
{
	return file;
}

bool FileDirectInputStream::openedOk() const
{
	return fileHandle != -1;
}

bool FileDirectInputStream::failedToOpen() const noexcept
{
	return !openedOk();
}
*/

#if JUCE_MSVC
#pragma warning (pop)
#endif