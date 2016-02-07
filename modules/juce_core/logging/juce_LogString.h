/*
  ==============================================================================

	Proposed addition to the JUCE library.
	Copyright (c) 2016 - Jerry Evans. https://github.com/g40

	Permission is granted to use this software under the terms of either:
	a) the GPL v2 (or any later version)
	b) the Affero GPL v3

	Details of these licenses can be found at: www.gnu.org/licenses

	JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
	WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

	------------------------------------------------------------------------------

	To release a closed-source product which uses JUCE, commercial licenses are
	available: visit www.juce.com for more information.

==============================================================================
*/

#ifndef JUCE_LOGSTRING_H_INCLUDED
#define JUCE_LOGSTRING_H_INCLUDED


//==============================================================================
/**
	Very light-weight stream type string builder for simplified debugging
	which is both thread and type safe.

	Adds DBM macro which is used in debug mode. 
	arg can now be comprised of a variable number of logString instances
	as in:
	
	DBM("Hello World " << 42 << ':' << true);


@see Logger
*/
class LogString
{
	// 
	String buffer;

public:
	
	LogString()	{}

	// automatically locate origin
	LogString(const char* file,int line) 
	{
		// make this navigable in Visual Studio
		buffer += file;
		buffer += "(";
		buffer += line;
		buffer += ") : ";
	}

	LogString& operator<<(const String& arg)
	{
		//
		buffer += arg;
		return (*this);
	}

	LogString& operator<<(const char* arg)
	{
		//
		if (arg)
		{
			buffer += arg;
		}
		return (*this);
	}

	//
	LogString& operator<<(char arg)
	{
		//
		if (arg)
		{
			buffer += arg;
		}
		return (*this);
	}

	LogString& operator<<(int arg)
	{
		//
		buffer += arg;
		return (*this);
	}

	LogString& operator<<(bool arg)
	{
		//
		buffer += (arg ? " true " : " false ");
		return (*this);
	}

	//
	LogString& operator<<(const void* arg)
	{
		//
		if (arg)
		{
			buffer += String::toHexString(arg,sizeof(void*));
		}
		else
		{
			buffer += "0x00";
		}
		return (*this);
	}

	operator const String() const
	{
		return buffer;
	}

	static void Debug(const juce::String& arg)
	{
		Logger::outputDebugString(arg);
	}

};

#ifdef DEBUG
// arg can now be comprised of a variable number of logString instances
// as in DBM("Hello World " << 42 << true);
#define DBM(arg) LogString::Debug(LogString(__FILE__,__LINE__) << arg);
#else
#define DBM(arg) 
#endif

#endif   // JUCE_LOGSTRING_H_INCLUDED

