#ifndef JUCE_FILEDIRECTINPUTSTREAM_H_INCLUDED
#define JUCE_FILEDIRECTINPUTSTREAM_H_INCLUDED


//==============================================================================
/**
    An input stream that reads from a local file.

    @see InputStream, FileOutputStream, File::createInputStream
*/
class FileDirectInputStream : public InputStream
{
public:
	explicit FileDirectInputStream(const File& fileToRead);
	~FileDirectInputStream();
	int64 getTotalLength() override;
	bool openedOk() const ;
	int read(void* destBuffer ,int maxBytesToRead) override;
	bool isExhausted() override;
	int64 getPosition() override;
	bool setPosition(int64 pos) override;

    const Result& getStatus() const noexcept { return m_status; }
    bool failedToOpen() const noexcept;
private:
    //==============================================================================
    File file;
    Result m_status;
	int fileHandle;
	int64 m_fileLength;
	int64 m_pos;
	bool needToSeek;
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FileDirectInputStream);
};


#endif   // JUCE_FILEINPUTSTREAM_H_INCLUDED
