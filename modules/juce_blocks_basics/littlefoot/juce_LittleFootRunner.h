/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2016 - ROLI Ltd.

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

namespace littlefoot
{

/*
    This file contains classes and definitions for executing a littlefoot
    bytecode program that was created with the littlefoot compiler.
*/

#if ! (defined (LITTLEFOOT_DEBUG_TRACE) || RUNNING_ON_REAL_BLOCK_DEVICE)
 #define LITTLEFOOT_DEBUG_TRACE 0
#endif

#if ! (defined (LITTLEFOOT_DUMP_PROGRAM) || RUNNING_ON_REAL_BLOCK_DEVICE)
 #define LITTLEFOOT_DUMP_PROGRAM 0
#endif

using int8        = char;
using uint8       = unsigned char;
using int16       = short;
using uint16      = unsigned short;
using int32       = int;
using uint32      = unsigned int;
using FunctionID  = int16;

#define LITTLEFOOT_OPCODES(OP, OP_INT8, OP_INT16, OP_INT32) \
    OP       (halt) \
    OP_INT16 (jump) \
    OP_INT16 (jumpIfTrue) \
    OP_INT16 (jumpIfFalse) \
    OP_INT16 (call) \
    OP_INT8  (retVoid) \
    OP_INT8  (retValue) \
    OP_INT16 (callNative) \
    OP       (drop) \
    OP_INT8  (dropMultiple) \
    OP_INT8  (pushMultiple0) \
    OP       (push0) \
    OP       (push1) \
    OP_INT8  (push8) \
    OP_INT16 (push16) \
    OP_INT32 (push32) \
    OP       (dup) \
    OP       (dupOffset_01) \
    OP       (dupOffset_02) \
    OP       (dupOffset_03) \
    OP       (dupOffset_04) \
    OP       (dupOffset_05) \
    OP       (dupOffset_06) \
    OP       (dupOffset_07) \
    OP_INT8  (dupOffset) \
    OP_INT16 (dupOffset16) \
    OP_INT8  (dropToStack) \
    OP_INT16 (dropToStack16) \
    OP_INT16 (dupFromGlobal) \
    OP_INT16 (dropToGlobal) \
    OP       (int32ToFloat) \
    OP       (floatToInt32) \
    OP       (add_int32) \
    OP       (add_float) \
    OP       (mul_int32) \
    OP       (mul_float) \
    OP       (sub_int32) \
    OP       (sub_float) \
    OP       (div_int32) \
    OP       (div_float) \
    OP       (mod_int32) \
    OP       (bitwiseOr) \
    OP       (bitwiseAnd) \
    OP       (bitwiseXor) \
    OP       (bitwiseNot) \
    OP       (bitShiftLeft) \
    OP       (bitShiftRight) \
    OP       (logicalOr) \
    OP       (logicalAnd) \
    OP       (logicalNot) \
    OP       (testZE_int32) \
    OP       (testNZ_int32) \
    OP       (testGT_int32) \
    OP       (testGE_int32) \
    OP       (testLT_int32) \
    OP       (testLE_int32) \
    OP       (testZE_float) \
    OP       (testNZ_float) \
    OP       (testGT_float) \
    OP       (testGE_float) \
    OP       (testLT_float) \
    OP       (testLE_float) \
    OP       (getHeapByte) \
    OP       (getHeapInt) \
    OP       (getHeapBits) \
    OP       (setHeapByte) \
    OP       (setHeapInt) \

enum class OpCode  : uint8
{
   #define LITTLEFOOT_OP(name)  name,
    LITTLEFOOT_OPCODES (LITTLEFOOT_OP, LITTLEFOOT_OP, LITTLEFOOT_OP, LITTLEFOOT_OP)
   #undef LITTLEFOOT_OP
    endOfOpcodes
};

/** Available value types */
enum class Type  : uint8
{
    void_   = 'v',
    int_    = 'i',
    bool_   = 'b',
    float_  = 'f'
};

//==============================================================================
/** Defines a native function that the program can call. */
struct NativeFunction
{
    using ImplementationFunction = int32 (*) (void*, const int32*);

    /** Creates a NativeFunction from its signature and an implementation function.
        The format of nameAndArgumentTypes is "name/[return type][arg1][arg2..]"
        So for example "int foobar (float, bool)" would be "foobar/ifb"
    */
    NativeFunction (const char* nameAndArgumentTypes, ImplementationFunction fn) noexcept
        : nameAndArguments (nameAndArgumentTypes), function (fn),
          functionID (createID (nameAndArgumentTypes)), returnType(), numArgs()
    {
        const int slash = indexOfSlash (nameAndArgumentTypes);

        if (slash > 0)
        {
            returnType = static_cast<Type> (nameAndArgumentTypes[slash + 1]);

            while (nameAndArgumentTypes[slash + 2 + numArgs] != 0)
                ++numArgs;
        }
    }

    const char* nameAndArguments;       /**< This signature must have the form "name/[return type][arg1][arg2..]" */
    ImplementationFunction function;    /**< A static function that will be called. */
    FunctionID functionID;              /**< The ID is a hash of the name + arguments, but not the return type. */
    Type returnType;                    /**< The function's return type. */
    uint8 numArgs;                      /**< The number of arguments that the function takes. */

    /** Converts a function signature to its hashed ID. */
    static FunctionID createID (const char* nameAndArgTypes) noexcept
    {
        jassert (nameAndArgTypes != nullptr && nameAndArgTypes[0] != 0); // the name cannot be an empty string!
        int hash = 0, i = 0;
        const int slash = indexOfSlash (nameAndArgTypes);

        jassert (slash > 0); // The slash can't be the first character in this string!
        jassert (nameAndArgTypes[slash + 1] != 0);  // The slash must be followed by a return type character
        jassert (juce::String (nameAndArgTypes).substring (0, slash).containsOnly ("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_"));
        jassert (! juce::String ("0123456789").containsChar (nameAndArgTypes[0]));
        jassert (juce::String (nameAndArgTypes).substring (slash + 1).containsOnly ("vif"));
        jassert (juce::String (nameAndArgTypes).substring (slash + 2).containsOnly ("if")); // arguments must only be of these types

        for (; nameAndArgTypes[i] != 0; ++i)
            if (i != slash + 1)
                hash = hash * 31 + nameAndArgTypes[i];

        return static_cast<FunctionID> (hash + i);
    }

private:
    static int indexOfSlash (const char* nameAndArgs) noexcept
    {
        for (int i = 0; nameAndArgs[i] != 0; ++i)
            if (nameAndArgs[i] == '/')
                return i;

        return -1;
    }
};

//==============================================================================
/**
    A reference to a block of memory which contains a complete program.

    Data format:
      2 bytes - program checksum
      2 bytes - program size
      2 bytes - num functions
      2 bytes - num globals
      2 bytes - amount of heap space needed (bytes)
      2 bytes - ID of function 1
      2 bytes - byte offset of function 1 code
      2 bytes - ID of function 2
      2 bytes - byte offset of function 2 code
                etc..
      ...function code...
*/
struct Program
{
    Program (const void* data, uint32 totalMemorySize) noexcept
       : programStart (static_cast<const uint8*> (data)), maxProgramSize (totalMemorySize)
    {
        jassert (data != nullptr);
    }

    uint16 getStoredChecksum() const noexcept
    {
        return (uint16) readInt16 (programStart);
    }

    uint16 calculateChecksum() const noexcept
    {
        auto size = getProgramSize();
        uint16 n = (uint16) size;

        for (uint32 i = 2; i < size; ++i)
            n += (n * 2) + programStart[i];

        return n;
    }

    bool checksumMatches() const noexcept
    {
        return calculateChecksum() == getStoredChecksum();
    }

    uint32 getProgramSize() const noexcept
    {
        auto size = (uint16) readInt16 (programStart + 2);
        return size < programHeaderSize ? programHeaderSize
                                        : (size > maxProgramSize ? maxProgramSize : size);
    }

    uint32 getNumFunctions() const noexcept
    {
        return (uint16) readInt16 (programStart + 4);
    }

    FunctionID getFunctionID (uint32 functionIndex) const noexcept
    {
        if (auto f = getFunctionEntry (functionIndex))
            return static_cast<FunctionID> (readInt16 (f));

        return {};
    }

    const uint8* getFunctionStartAddress (uint32 functionIndex) const noexcept
    {
        if (auto f = getFunctionEntry (functionIndex))
        {
            uint32 address = (uint16) readInt16 (f + sizeof (FunctionID));

            if (address < getProgramSize())
                return programStart + address;
        }

        return {};
    }

    const uint8* getFunctionEndAddress (uint32 functionIndex) const noexcept
    {
        return ++functionIndex >= getNumFunctions() ? programStart + getProgramSize()
                                                    : getFunctionStartAddress (functionIndex);
    }

    /** Returns the number of global variables the program uses */
    uint16 getNumGlobals() const noexcept
    {
        return (uint16) readInt16 (programStart + 6);
    }

    /** Returns the number of bytes of heap space the program needs */
    uint16 getHeapSizeBytes() const noexcept
    {
        return (uint16) readInt16 (programStart + 8);
    }

   #if JUCE_DEBUG
    //==============================================================================
    /** Prints the assembly code for a given function. */
    void dumpFunctionDisassembly (juce::OutputStream& out, uint32 functionIndex) const
    {
        out << juce::newLine << "Function #" << (int) functionIndex
            << "  (" << juce::String::toHexString (getFunctionID (functionIndex)) << ")" << juce::newLine;

        if (auto codeStart = getFunctionStartAddress (functionIndex))
            if (auto codeEnd   = getFunctionEndAddress (functionIndex))
                for (auto prog = codeStart; prog < codeEnd;)
                    out << getOpDisassembly (prog) << juce::newLine;
    }

    juce::String getOpDisassembly (const uint8*& prog) const
    {
        juce::String s;
        s << juce::String::toHexString ((int) (prog - programStart)).paddedLeft ('0', 4) << ":  ";
        auto op = (OpCode) *prog++;

        switch (op)
        {
           #define LITTLEFOOT_OP(name)         case OpCode::name:  s << #name; break;
           #define LITTLEFOOT_OP_INT8(name)    case OpCode::name:  s << #name << " " << juce::String::toHexString ((int) *prog++).paddedLeft ('0', 2); break;
           #define LITTLEFOOT_OP_INT16(name)   case OpCode::name:  s << #name << " " << juce::String::toHexString ((int) readInt16 (prog)).paddedLeft ('0', 4); prog += 2; break;
           #define LITTLEFOOT_OP_INT32(name)   case OpCode::name:  s << #name << " " << juce::String::toHexString ((int) readInt32 (prog)).paddedLeft ('0', 8); prog += 4; break;
            LITTLEFOOT_OPCODES (LITTLEFOOT_OP, LITTLEFOOT_OP_INT8, LITTLEFOOT_OP_INT16, LITTLEFOOT_OP_INT32)
           #undef LITTLEFOOT_OP
           #undef LITTLEFOOT_OP_INT8
           #undef LITTLEFOOT_OP_INT16
           #undef LITTLEFOOT_OP_INT32

            default:  s << "???"; break;
        }

        return s;
    }

    /** Calls dumpFunctionDisassembly() for all functions. */
    void dumpAllFunctions (juce::OutputStream& out) const
    {
        DBG ("Program size: " << (int) getProgramSize() << " bytes");

        for (uint32 i = 0; i < getNumFunctions(); ++i)
            dumpFunctionDisassembly (out, i);
    }
   #endif

    //==============================================================================
    static float intToFloat (int32 value) noexcept          { float v; copyFloatMem (&v, &value); return v; }
    static int32 floatToInt (float value) noexcept          { int32 v; copyFloatMem (&v, &value); return v; }

    static int16 readInt16 (const uint8* d) noexcept        { return (int16) (d[0] + (((uint16) d[1]) << 8)); }
    static int32 readInt32 (const uint8* d) noexcept        { return (int32) ((uint32) readInt16 (d) + (((uint32) readInt16 (d + 2)) << 16)); }

    static void writeInt16 (uint8* d, int16 v) noexcept     { d[0] = v & 0xff; d[1] = (uint8) (v >> 8); }
    static void writeInt32 (uint8* d, int32 v) noexcept     { writeInt16 (d, (int16) (v & 0xffff)); writeInt16 (d + 2, (int16) (v >> 16)); }

    //==============================================================================
    static constexpr uint32 programHeaderSize = 10;
    const uint8* programStart = 0;
    const uint32 maxProgramSize;

private:
    const uint8* getFunctionEntry (uint32 index) const noexcept
    {
        auto offset = programHeaderSize + index * (sizeof (FunctionID) + sizeof (int16));
        return offset <= (uint32) (getProgramSize() - 4) ? programStart + offset : nullptr;
    }

    static void copyFloatMem (void* dest, const void* src) noexcept
    {
        for (int i = 0; i < 4; ++i)
            ((uint8*) dest)[i] = ((const uint8*) src)[i];
    }
};


//==============================================================================
/**
    Loads a program, and lets the user execute its functions.
    The programAndHeapSpace is the number of bytes allocated for program + heap.
    stackAndGlobalsSpace is the size of the globals + stack area.

    Memory layout:

    Program code goes at address 0, followed by any shared data the program needs
    globals are at the top end of the buffer
    stack space stretches downwards from the start of the globals
*/
template <int programAndHeapSpace, int stackAndGlobalsSpace>
struct Runner
{
    Runner() noexcept  : program (allMemory, sizeof (allMemory))  { reset(); }

    /** Installs an array of native functions that the code can use.
        Note that this doesn't take ownership of any memory involved, so the caller mustn't pass any dangling pointers
    */
    void setNativeFunctions (const NativeFunction* functions, int numFunctions, void* userDataForCallback) noexcept
    {
        nativeFunctions = functions;
        numNativeFunctions = numFunctions;
        nativeFunctionCallbackContext = userDataForCallback;
    }

    /** Returns the number of native functions available. */
    int getNumNativeFunctions() const noexcept                              { return numNativeFunctions; }

    /** Returns one of the native functions available. The index must not be out of range. */
    const NativeFunction& getNativeFunction (int index) const noexcept      { jassert (index >= 0 && index < numNativeFunctions); return nativeFunctions[index]; }

    /** Clears the memory state. */
    void reset() noexcept
    {
        for (uint32 i = 0; i < sizeof (allMemory); ++i)
            allMemory[i] = 0;
    }

    /** Return codes from a function call */
    enum class ErrorCode  : uint8
    {
        ok = 0,
        executionTimedOut,
        unknownInstruction,
        stackOverflow,
        stackUnderflow,
        illegalAddress,
        divisionByZero,
        unknownFunction
    };

    /** Calls one of the functions in the program, by its textual signature. */
    ErrorCode callFunction (const char* functionSignature) noexcept
    {
        return FunctionExecutionContext (*this, functionSignature).run();
    }

    /** Calls one of the functions in the program, by its function ID. */
    ErrorCode callFunction (FunctionID function) noexcept
    {
        return FunctionExecutionContext (*this, function).run();
    }

    /** */
    static constexpr uint32 totalProgramAndHeapSpace = programAndHeapSpace;

    /** */
    static constexpr uint32 totalStackAndGlobalsSpace = stackAndGlobalsSpace;

    /** */
    static uint32 getMaximumProgramSize() noexcept      { return programAndHeapSpace; }

    /** */
    uint8* getProgramAndDataStart() const noexcept      { return const_cast<uint8*> (allMemory); }
    /** */
    uint8* getProgramAndDataEnd() const noexcept        { return reinterpret_cast<uint8*> (stackStart); }
    /** */
    uint32 getProgramAndDataSize() const noexcept       { return (uint32) (getProgramAndDataEnd() - getProgramAndDataStart()); }

    /** */
    uint8* getProgramHeapStart() const noexcept         { return heapStart; }
    /** */
    uint8* getProgramHeapEnd() const noexcept           { return getProgramAndDataEnd(); }
    /** */
    uint16 getProgramHeapSize() const noexcept          { return heapSize; }

    /** */
    void setDataByte (uint32 index, uint8 value) noexcept
    {
        if (index < programAndHeapSpace)
        {
            if (index < program.getProgramSize())
                heapStart = nullptr; // force a re-initialise of the memory layout when the program changes

            getProgramAndDataStart()[index] = value;
        }
    }

    /** */
    void setHeapByte (uint32 index, uint8 value) noexcept
    {
        auto* addr = getProgramHeapStart() + index;

        if (addr < getProgramHeapEnd())
            *addr = value;
    }

    /** */
    uint8 getHeapByte (uint32 index) const noexcept
    {
        const auto* addr = getProgramHeapStart() + index;
        return addr < getProgramHeapEnd() ? *addr : 0;
    }

    /** */
    uint32 getHeapBits (uint32 startBit, uint32 numBits) const noexcept
    {
        if (startBit + numBits > 8 * getProgramHeapSize())
        {
            jassertfalse;
            return 0;
        }

        return readLittleEndianBitsInBuffer (getProgramHeapStart(), startBit, numBits);
    }

    /** */
    int32 setHeapInt (uint32 byteOffset, uint32 value) noexcept
    {
        if (byteOffset < (uint32) (getProgramHeapSize() - 3))
            Program::writeInt32 (getProgramHeapStart() + byteOffset, (int32) value);

        return 0;
    }

    /** */
    int32 getHeapInt (uint32 byteOffset) const noexcept
    {
        return byteOffset < getProgramHeapSize() - 3 ? Program::readInt32 (getProgramHeapStart() + byteOffset) : 0;
    }

    //==============================================================================
    /** */
    uint8 allMemory[((programAndHeapSpace + stackAndGlobalsSpace) + 3) & ~3];

    /** */
    Program program;

    //==============================================================================
    /**
    */
    struct FunctionExecutionContext
    {
        FunctionExecutionContext() noexcept     : programCounter (nullptr) {}
        FunctionExecutionContext (const FunctionExecutionContext&) noexcept = default;
        FunctionExecutionContext& operator= (const FunctionExecutionContext&) noexcept = default;

        /** */
        FunctionExecutionContext (Runner& r, const char* functionSignature) noexcept
            : FunctionExecutionContext (r, NativeFunction::createID (functionSignature)) {}

        /** */
        FunctionExecutionContext (Runner& r, FunctionID function) noexcept
            : runner (&r.reinitialiseProgramLayoutIfProgramHasChanged()),
              programBase (r.program.programStart), heapStart (r.heapStart),
              stack (r.stackEnd), stackStart (r.stackStart), stackEnd (r.stackEnd),
              globals (r.globals), heapSize (r.heapSize),
              programSize (r.program.getProgramSize()),
              numGlobals (r.program.getNumGlobals())
        {
            if (r.heapStart != nullptr)
            {
                auto& prog = r.program;
                auto numFunctions = prog.getNumFunctions();

                for (uint32 i = 0; i < numFunctions; ++i)
                {
                    if (prog.getFunctionID (i) == function)
                    {
                        programCounter  = prog.getFunctionStartAddress (i);
                        functionEnd     = prog.getFunctionEndAddress (i);
                        tos             = *--stack = 0;
                        return;
                    }
                }
            }

            programCounter = nullptr;
        }

        /** */
        bool isValid() const noexcept
        {
            return programCounter != nullptr && runner->heapStart != nullptr;
        }

        /** */
        void reset() noexcept
        {
            programCounter = nullptr;
        }

        /** */
        template <typename... Args>
        void setArguments (Args... args) noexcept   { pushArguments (args...); push0(); /* (dummy return address) */ }

        /** */
        template <typename TimeOutCheckFunction>
        ErrorCode run (TimeOutCheckFunction hasTimedOut) noexcept
        {
            if (! isValid())
                return ErrorCode::unknownFunction;

            error = ErrorCode::unknownInstruction;
            uint16 opsPerformed = 0;

            for (;;)
            {
                if (programCounter >= functionEnd)
                    return error;

                if ((++opsPerformed & 63) == 0 && hasTimedOut())
                    return ErrorCode::executionTimedOut;

                dumpDebugTrace();

                auto op = (OpCode) *programCounter++;

                #define LITTLEFOOT_PERFORM_OP(name)          case OpCode::name: name(); break;
                #define LITTLEFOOT_PERFORM_OP_INT8(name)     case OpCode::name: name ((int8) *programCounter++); break;
                #define LITTLEFOOT_PERFORM_OP_INT16(name)    case OpCode::name: name (readProgram16()); break;
                #define LITTLEFOOT_PERFORM_OP_INT32(name)    case OpCode::name: name (readProgram32()); break;

                switch (op)
                {
                    LITTLEFOOT_OPCODES (LITTLEFOOT_PERFORM_OP, LITTLEFOOT_PERFORM_OP_INT8, LITTLEFOOT_PERFORM_OP_INT16, LITTLEFOOT_PERFORM_OP_INT32)
                    default:  setError (ErrorCode::unknownInstruction); break;
                }

                jassert (programCounter != nullptr);
            }
        }

    private:
        //==============================================================================
        Runner* runner;
        const uint8* programCounter;
        const uint8* functionEnd;
        const uint8* programBase;
        uint8* heapStart;
        int32* stack;
        int32* stackStart;
        int32* stackEnd;
        int32* globals;
        uint16 heapSize, programSize, numGlobals;
        int32 tos; // top of stack
        ErrorCode error;

        template <typename Type1, typename... Args> void pushArguments (Type1 arg1, Args... args) noexcept   { pushArguments (args...); push32 (arg1); }
        void pushArguments (int32 arg1) noexcept    { push32 (arg1); }
        void pushArguments (float arg1) noexcept    { push32 (Program::floatToInt (arg1)); }

        int16 readProgram16() noexcept              { auto v = Program::readInt16 (programCounter); programCounter += sizeof (int16); return v; }
        int32 readProgram32() noexcept              { auto v = Program::readInt32 (programCounter); programCounter += sizeof (int32); return v; }

        void setError (ErrorCode e) noexcept        { error = e; programCounter = functionEnd; jassert (error == ErrorCode::ok); }

        bool checkStackUnderflow() noexcept         { if (stack <= stackEnd) return true; setError (ErrorCode::stackUnderflow); return false; }
        bool flushTopToStack() noexcept             { if (--stack < stackStart) { setError (ErrorCode::stackOverflow); return false; } *stack = tos; return true; }

        using IntBinaryOp   = int32 (int32, int32);
        using FloatBinaryOp = float (float, float);

        void binaryOp (IntBinaryOp   f) noexcept    { if (checkStackUnderflow()) tos = f (*stack++, tos); }
        void binaryOp (FloatBinaryOp f) noexcept    { if (checkStackUnderflow()) tos = Program::floatToInt (f (Program::intToFloat (*stack++), Program::intToFloat (tos))); }

        void halt() noexcept                        { setError (ErrorCode::ok); }
        void jump (int16 addr) noexcept             { if (((uint16) addr) >= programSize) return setError (ErrorCode::illegalAddress); programCounter = programBase + (uint16) addr; }
        void jumpIfTrue (int16 addr) noexcept       { bool v = tos; drop(); if (v)   jump (addr); }
        void jumpIfFalse (int16 addr) noexcept      { bool v = tos; drop(); if (! v) jump (addr); }
        void call (int16 fnAddr) noexcept           { if (flushTopToStack()) { tos = (int32) (programCounter - programBase); jump (fnAddr); } }
        void retVoid (int8 numArgs) noexcept        { if (tos == 0) return setError (ErrorCode::ok); auto retAddr = (int16) tos; stack += (uint8) numArgs; if (checkStackUnderflow()) { tos = *stack++; jump (retAddr); } }
        void retValue (int8 numArgs) noexcept       { auto retAddr = (int16) *stack++; if (retAddr == 0) return setError (ErrorCode::ok); stack += (uint8) numArgs; if (checkStackUnderflow()) jump (retAddr); }
        void drop() noexcept                        { if (checkStackUnderflow()) tos = *stack++; }
        void dropMultiple (int8 num) noexcept       { if (num < 0) { stack -= num; checkStackUnderflow(); } else { stack += num - 1; drop(); }}
        void pushMultiple0 (int8 num) noexcept      { if (stack - num <= stackStart) return setError (ErrorCode::stackOverflow); flushTopToStack(); for (int i = (uint8) num; --i > 0;) *--stack = 0; tos = 0; }
        void push0() noexcept                       { push32 (0); }
        void push1() noexcept                       { push32 (1); }
        void push8 (int8 value) noexcept            { push32 (value); }
        void push16 (int16 value) noexcept          { push32 (value); }
        void push32 (int32 value) noexcept          { flushTopToStack(); tos = value; }
        void dup() noexcept                         { flushTopToStack(); }
        void dupOffset_01() noexcept                { dupOffset16 (1); }
        void dupOffset_02() noexcept                { dupOffset16 (2); }
        void dupOffset_03() noexcept                { dupOffset16 (3); }
        void dupOffset_04() noexcept                { dupOffset16 (4); }
        void dupOffset_05() noexcept                { dupOffset16 (5); }
        void dupOffset_06() noexcept                { dupOffset16 (6); }
        void dupOffset_07() noexcept                { dupOffset16 (7); }
        void dupOffset (int8 offset) noexcept       { dupOffset16 ((uint8) offset); }
        void dupOffset16 (int16 offset) noexcept    { if (flushTopToStack()) { auto addr = stack + offset; if (addr < stackStart || addr >= stackEnd) return setError (ErrorCode::illegalAddress); tos = *addr; } }
        void dropToStack (int8 offset) noexcept     { dropToStack16 ((uint8) offset); }
        void dropToStack16 (int16 offset) noexcept  { auto addr = stack + offset; if (addr < stackStart || addr >= stackEnd) return setError (ErrorCode::illegalAddress); *addr = tos; drop(); }
        void dupFromGlobal (int16 index) noexcept   { if (flushTopToStack()) { if (((uint16) index) >= numGlobals) return setError (ErrorCode::illegalAddress); tos = globals [(uint16) index]; } }
        void dropToGlobal (int16 index) noexcept    { if (((uint16) index) >= numGlobals) return setError (ErrorCode::illegalAddress); globals [(uint16) index] = tos; drop(); }
        void int32ToFloat() noexcept                { tos = Program::floatToInt (static_cast<float> (tos)); }
        void floatToInt32() noexcept                { tos = static_cast<int32> (Program::intToFloat (tos)); }
        void add_int32() noexcept                   { binaryOp ([] (int32 a, int32 b) { return a + b; }); }
        void add_float() noexcept                   { binaryOp ([] (float a, float b) { return a + b; }); }
        void mul_int32() noexcept                   { binaryOp ([] (int32 a, int32 b) { return a * b; }); }
        void mul_float() noexcept                   { binaryOp ([] (float a, float b) { return a * b; }); }
        void sub_int32() noexcept                   { binaryOp ([] (int32 a, int32 b) { return a - b; }); }
        void sub_float() noexcept                   { binaryOp ([] (float a, float b) { return a - b; }); }
        void div_int32() noexcept                   { if (tos == 0) return setError (ErrorCode::divisionByZero); binaryOp ([] (int32 a, int32 b) { return a / b; }); }
        void div_float() noexcept                   { if (tos == 0) return setError (ErrorCode::divisionByZero); binaryOp ([] (float a, float b) { return a / b; }); }
        void mod_int32() noexcept                   { if (tos == 0) return setError (ErrorCode::divisionByZero); binaryOp ([] (int32 a, int32 b) { return a % b; }); }
        void bitwiseOr() noexcept                   { binaryOp ([] (int32 a, int32 b) { return a | b; }); }
        void bitwiseAnd() noexcept                  { binaryOp ([] (int32 a, int32 b) { return a & b; }); }
        void bitwiseXor() noexcept                  { binaryOp ([] (int32 a, int32 b) { return a ^ b; }); }
        void bitShiftLeft() noexcept                { binaryOp ([] (int32 a, int32 b) { return a << b; }); }
        void bitShiftRight() noexcept               { binaryOp ([] (int32 a, int32 b) { return a >> b; }); }
        void logicalOr() noexcept                   { binaryOp ([] (int32 a, int32 b) { return (int32) (a || b); }); }
        void logicalAnd() noexcept                  { binaryOp ([] (int32 a, int32 b) { return (int32) (a && b); }); }
        void logicalNot() noexcept                  { tos = ! tos; }
        void bitwiseNot() noexcept                  { tos = ~tos; }
        void testZE_int32() noexcept                { tos = (tos == 0); }
        void testNZ_int32() noexcept                { tos = (tos != 0); }
        void testGT_int32() noexcept                { tos = (tos >  0); }
        void testGE_int32() noexcept                { tos = (tos >= 0); }
        void testLT_int32() noexcept                { tos = (tos <  0); }
        void testLE_int32() noexcept                { tos = (tos <= 0); }
        void testZE_float() noexcept                { tos = (Program::intToFloat (tos) == 0.0f); }
        void testNZ_float() noexcept                { tos = (Program::intToFloat (tos) != 0.0f); }
        void testGT_float() noexcept                { tos = (Program::intToFloat (tos) >  0.0f); }
        void testGE_float() noexcept                { tos = (Program::intToFloat (tos) >= 0.0f); }
        void testLT_float() noexcept                { tos = (Program::intToFloat (tos) <  0.0f); }
        void testLE_float() noexcept                { tos = (Program::intToFloat (tos) <= 0.0f); }
        void getHeapByte() noexcept                 { tos = runner->getHeapByte ((uint32) tos); }
        void getHeapInt() noexcept                  { tos = runner->getHeapInt  ((uint32) tos); }
        void getHeapBits() noexcept                 { if (checkStackUnderflow()) tos = runner->getHeapBits ((uint32) tos, (uint32) *stack++); }
        void setHeapByte() noexcept                 { if (checkStackUnderflow()) runner->setHeapByte ((uint32) tos, (uint8)  *stack++); drop(); }
        void setHeapInt() noexcept                  { if (checkStackUnderflow()) runner->setHeapInt  ((uint32) tos, (uint32) *stack++); drop(); }

        void callNative (FunctionID functionID) noexcept
        {
            auto numFunctions = runner->numNativeFunctions;
            auto* functions = runner->nativeFunctions;

            for (int i = 0; i < numFunctions; ++i)
            {
                const auto& f = functions[i];

                if (f.functionID == functionID)
                {
                    if (flushTopToStack())
                    {
                        tos = f.function (runner->nativeFunctionCallbackContext, stack);
                        stack += f.numArgs;

                        if (checkStackUnderflow() && f.returnType == Type::void_)
                            drop();
                    }

                    return;
                }
            }

            setError (ErrorCode::unknownFunction);
        }

        void dumpDebugTrace() const
        {
           #if LITTLEFOOT_DEBUG_TRACE // Dumps the program counter and stack, for debugging
            juce::MemoryOutputStream dump;
            auto progCopy = programCounter;
            dump << juce::String (runner->program.getOpDisassembly (progCopy)).paddedRight (' ', 26)
                 << juce::String::toHexString (tos) << ' ';

            for (auto s = stack; s < stackEnd; ++s)
                dump << juce::String::toHexString (*s) << ' ';

            DBG (dump.toString());
           #endif
        }
    };

private:
    //==============================================================================
    const NativeFunction* nativeFunctions;
    int numNativeFunctions = 0;
    void* nativeFunctionCallbackContext = nullptr;
    uint8* heapStart  = nullptr;
    int32* stackStart = nullptr;
    int32* stackEnd   = nullptr;
    int32* globals    = nullptr;
    uint16 heapSize   = 0;

    Runner& reinitialiseProgramLayoutIfProgramHasChanged() noexcept
    {
        if (heapStart == nullptr && program.checksumMatches())
        {
            auto numGlobals = program.getNumGlobals();
            globals = reinterpret_cast<int32*> (allMemory + sizeof (allMemory)) - numGlobals;
            heapStart = getProgramAndDataStart() + program.getProgramSize();
            heapSize = program.getHeapSizeBytes();
            stackEnd = globals;
            stackStart = reinterpret_cast<int32*> (heapStart + heapSize);

            if ((uint8*) globals < heapStart || stackStart + 32 > stackEnd)
            {
                jassertfalse;
                heapStart = nullptr;
            }
            else
            {
                for (uint32 i = 0; i < numGlobals; ++i)
                    globals[i] = 0; // clear globals

               #if LITTLEFOOT_DUMP_PROGRAM
                juce::MemoryOutputStream m;
                program.dumpAllFunctions (m);
                DBG (m.toString());
               #endif
            }
        }

        return *this;
    }
};
}
