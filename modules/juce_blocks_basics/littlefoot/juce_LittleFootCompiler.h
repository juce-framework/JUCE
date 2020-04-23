/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

JUCE_BEGIN_IGNORE_WARNINGS_MSVC (4702)

namespace littlefoot
{

/**
    This class compiles littlefoot source code into a littlefoot::Program object
    which can be executed by a littlefoot::Runner.

    @tags{Blocks}
*/
struct Compiler
{
    Compiler() = default;

    /** Gives the compiler a zero-terminated list of native function prototypes to
        use when parsing function calls.
    */
    void addNativeFunctions (const char* const* functionPrototypes)
    {
        for (; *functionPrototypes != nullptr; ++functionPrototypes)
            nativeFunctions.add (NativeFunction (*functionPrototypes, nullptr));
    }

    /** Tells the compiler to use the list of native function prototypes from
        this littlefoot::Runner object.
    */
    template <typename RunnerType>
    void addNativeFunctions (const RunnerType& runner)
    {
        for (int i = 0; i < runner.getNumNativeFunctions(); ++i)
            nativeFunctions.add (runner.getNativeFunction (i));
    }

    /** Compiles a littlefoot program.
        If there's an error, this returns it, otherwise the compiled bytecode is
        placed in the compiledObjectCode member.
    */
    Result compile (const String& sourceCode, uint32 defaultHeapSize, const Array<File>& searchPaths = {})
    {
        try
        {
            SyntaxTreeBuilder stb (sourceCode, nativeFunctions, defaultHeapSize, searchPaths);
            stb.compile();
            stb.simplify();

            compiledObjectCode.clear();

            CodeGenerator codeGen (compiledObjectCode, stb);
            codeGen.generateCode (stb.blockBeingParsed, stb.heapSizeRequired);
            return Result::ok();
        }
        catch (String error)
        {
            return Result::fail (error);
        }
    }

    /** After a successful compilation, this returns the finished Program. */
    Program getCompiledProgram() const noexcept
    {
        return Program (compiledObjectCode.begin(), (uint32) compiledObjectCode.size());
    }

    static File resolveIncludePath (String include, Array<File> searchPaths)
    {
        if (File::isAbsolutePath (include) && File (include).existsAsFile())
            return { include };

        auto fileName = include.fromLastOccurrenceOf ("/", false, false);

        for (auto path : searchPaths)
        {
            if (path == File())
                continue;

            if (! path.isDirectory())
                path = path.getParentDirectory();

            if (path.getChildFile (include).existsAsFile())
                return path.getChildFile (include);

            if (path.getChildFile (fileName).existsAsFile())
                return path.getChildFile (fileName);
        }

        return {};
    }

    /** After a successful call to compile(), this contains the bytecode generated.
        A littlefoot::Program object can be created directly from this array.
    */
    Array<uint8> compiledObjectCode;

private:

   #ifndef DOXYGEN

    struct Statement;
    struct Expression;
    struct Variable;
    struct BlockStatement;
    struct Function;
    struct AllocatedObject  { virtual ~AllocatedObject() = default; };
    using StatementPtr = Statement*;
    using ExpPtr = Expression*;
    using BlockPtr = BlockStatement*;
    using TokenType = const char*;

    #define LITTLEFOOT_KEYWORDS(X) \
        X(if_,      "if")       X(else_,   "else")    X(do_,     "do") \
        X(while_,   "while")    X(for_,    "for")     X(break_,  "break")   X(continue_, "continue") \
        X(void_,    "void")     X(int_,    "int")     X(float_,  "float")   X(bool_,     "bool") \
        X(return_,  "return")   X(true_,   "true")    X(false_,  "false")   X(const_,    "const")

    #define LITTLEFOOT_OPERATORS(X) \
        X(semicolon,     ";")        X(dot,          ".")       X(comma,        ",")    X(hash,       "#") \
        X(openParen,     "(")        X(closeParen,   ")")       X(openBrace,    "{")    X(closeBrace, "}") \
        X(openBracket,   "[")        X(closeBracket, "]")       X(colon,        ":")    X(question,   "?") \
        X(equals,        "==")       X(assign,       "=")       X(notEquals,    "!=")   X(logicalNot, "!") \
        X(plusEquals,    "+=")       X(plusplus,     "++")      X(plus,         "+") \
        X(minusEquals,   "-=")       X(minusminus,   "--")      X(minus,        "-") \
        X(timesEquals,   "*=")       X(times,        "*")       X(divideEquals, "/=")   X(divide,     "/") \
        X(moduloEquals,  "%=")       X(modulo,       "%")       X(xorEquals,    "^=")   X(bitwiseXor, "^")   X(bitwiseNot, "~") \
        X(andEquals,     "&=")       X(logicalAnd,   "&&")      X(bitwiseAnd,   "&") \
        X(orEquals,      "|=")       X(logicalOr,    "||")      X(bitwiseOr,    "|") \
        X(leftShiftEquals,    "<<=") X(lessThanOrEqual,  "<=")  X(leftShift,    "<<")   X(lessThan,   "<") \
        X(rightShiftUnsigned, ">>>") X(rightShiftEquals, ">>=") X(rightShift,   ">>")   X(greaterThanOrEqual, ">=")  X(greaterThan,  ">")

    struct Token
    {
        #define DECLARE_LITTLEFOOT_TOKEN(name, str)  static constexpr const char* name = str;
        LITTLEFOOT_KEYWORDS  (DECLARE_LITTLEFOOT_TOKEN)
        LITTLEFOOT_OPERATORS (DECLARE_LITTLEFOOT_TOKEN)
        DECLARE_LITTLEFOOT_TOKEN (eof,        "$eof")
        DECLARE_LITTLEFOOT_TOKEN (literal,    "$literal")
        DECLARE_LITTLEFOOT_TOKEN (identifier, "$identifier")
    };

    Array<NativeFunction> nativeFunctions;

    //==============================================================================
    struct CodeLocation
    {
        CodeLocation (const String& code, const File& srcFile) noexcept : program (code), location (program.getCharPointer()), sourceFile (srcFile) {}
        CodeLocation (const CodeLocation& other) = default;

        [[noreturn]] void throwError (const String& message) const
        {
            int col = 1, line = 1;

            for (auto i = program.getCharPointer(); i < location && ! i.isEmpty(); ++i)
            {
                ++col;
                if (*i == '\n')  { col = 1; ++line; }
            }

            auto filePath = sourceFile == File() ? String() : (sourceFile.getFullPathName() + ": ");
            throw filePath + "Line " + String (line) + ", column " + String (col) + " : " + message;
        }

        String program;
        String::CharPointerType location;
        File sourceFile;
    };

    //==============================================================================
    struct TokenIterator
    {
        TokenIterator (const String& code) : location (code, {}), p (code.getCharPointer()) { skip(); }

        TokenType skip()
        {
            skipWhitespaceAndComments();
            location.location = p;
            auto last = currentType;
            currentType = matchNextToken();
            return last;
        }

        void match (TokenType expected)
        {
            if (currentType != expected)
                throwErrorExpecting (getTokenDescription (expected));

            skip();
        }

        bool matchIf (TokenType expected)       { if (currentType == expected)  { skip(); return true; } return false; }

        template <typename... Args>
        bool matchesAny (TokenType t1, Args... others) const noexcept   { return currentType == t1 || matchesAny (others...); }
        bool matchesAny (TokenType t1) const noexcept                   { return currentType == t1; }

        void throwErrorExpecting (const String& expected)    { location.throwError ("Found " + getTokenDescription (currentType) + " when expecting " + expected); }

        CodeLocation location;
        TokenType currentType;
        var currentValue;

    protected:
        String::CharPointerType p;

    private:
        static bool isIdentifierStart (juce_wchar c) noexcept   { return CharacterFunctions::isLetter (c)        || c == '_'; }
        static bool isIdentifierBody  (juce_wchar c) noexcept   { return CharacterFunctions::isLetterOrDigit (c) || c == '_'; }

        TokenType matchNextToken()
        {
            if (isIdentifierStart (*p))
            {
                auto end = p;
                while (isIdentifierBody (*++end)) {}

                const size_t len = (size_t) (end - p);
                #define LITTLEFOOT_COMPARE_KEYWORD(name, str) if (len == sizeof (str) - 1 && matchToken (Token::name, len)) return Token::name;
                LITTLEFOOT_KEYWORDS (LITTLEFOOT_COMPARE_KEYWORD)

                currentValue = String (p, end); p = end;
                return Token::identifier;
            }

            if (p.isDigit())
            {
                if (parseHexLiteral() || parseFloatLiteral() || parseOctalLiteral() || parseDecimalLiteral())
                    return Token::literal;

                location.throwError ("Syntax error in numeric constant");
            }

            if (parseStringLiteral (*p) || (*p == '.' && parseFloatLiteral()))
                return Token::literal;

            #define LITTLEFOOT_COMPARE_OPERATOR(name, str) if (matchToken (Token::name, sizeof (str) - 1)) return Token::name;
            LITTLEFOOT_OPERATORS (LITTLEFOOT_COMPARE_OPERATOR)

            if (! p.isEmpty())
                location.throwError ("Unexpected character '" + String::charToString (*p) + "' in source");

            return Token::eof;
        }

        bool matchToken (TokenType name, const size_t len) noexcept
        {
            if (p.compareUpTo (CharPointer_ASCII (name), (int) len) != 0) return false;
            p += (int) len;  return true;
        }

        void skipWhitespaceAndComments()
        {
            for (;;)
            {
                p = p.findEndOfWhitespace();

                if (*p == '/')
                {
                    auto c2 = p[1];

                    if (c2 == '/')  { p = CharacterFunctions::find (p, (juce_wchar) '\n'); continue; }

                    if (c2 == '*')
                    {
                        location.location = p;
                        p = CharacterFunctions::find (p + 2, CharPointer_ASCII ("*/"));
                        if (p.isEmpty()) location.throwError ("Unterminated '/*' comment");
                        p += 2; continue;
                    }
                }

                break;
            }
        }

        bool parseStringLiteral (juce_wchar quoteType)
        {
            if (quoteType != '"' && quoteType != '\'')
                return false;

            auto r = JSON::parseQuotedString (p, currentValue);
            if (r.failed()) location.throwError (r.getErrorMessage());
            return true;
        }

        bool parseHexLiteral()
        {
            if (*p != '0' || (p[1] != 'x' && p[1] != 'X')) return false;

            auto t = ++p;
            auto v = CharacterFunctions::getHexDigitValue (*++t);
            if (v < 0) return false;

            for (;;)
            {
                auto digit = CharacterFunctions::getHexDigitValue (*++t);
                if (digit < 0) break;
                v = v * 16 + digit;
            }

            currentValue = v; p = t;
            return true;
        }

        bool parseFloatLiteral()
        {
            int numDigits = 0;
            auto t = p;
            while (t.isDigit())  { ++t; ++numDigits; }

            const bool hasPoint = (*t == '.');

            if (hasPoint)
                while ((++t).isDigit())  ++numDigits;

            if (numDigits == 0)
                return false;

            auto c = *t;
            const bool hasExponent = (c == 'e' || c == 'E');

            if (hasExponent)
            {
                c = *++t;
                if (c == '+' || c == '-')  ++t;
                if (! t.isDigit()) return false;
                while ((++t).isDigit()) {}
            }

            if (! (hasExponent || hasPoint)) return false;

            currentValue = CharacterFunctions::getDoubleValue (p);  p = t;
            return true;
        }

        bool parseOctalLiteral()
        {
            auto t = p;
            int64 v = *t - '0';
            if (v != 0) return false;  // first digit of octal must be 0

            for (;;)
            {
                auto digit = (int) (*++t - '0');
                if (isPositiveAndBelow (digit, 8))        v = v * 8 + digit;
                else if (isPositiveAndBelow (digit, 10))  location.throwError ("Decimal digit in octal constant");
                else break;
            }

            currentValue = v;  p = t;
            return true;
        }

        bool parseDecimalLiteral()
        {
            int64 v = 0;

            for (;; ++p)
            {
                auto digit = (int) (*p - '0');
                if (isPositiveAndBelow (digit, 10))  v = v * 10 + digit;
                else break;
            }

            currentValue = v;
            return true;
        }
    };

    //==============================================================================
    //==============================================================================
    struct SyntaxTreeBuilder  : private TokenIterator
    {
        SyntaxTreeBuilder (const String& code, const Array<NativeFunction>& nativeFns, uint32 defaultHeapSize, const Array<File>& searchPathsToUse)
            : TokenIterator (code), searchPaths (searchPathsToUse), nativeFunctions (nativeFns), heapSizeRequired (defaultHeapSize) {}

        void compile()
        {
            blockBeingParsed = allocate<BlockStatement> (location, nullptr, nullptr, false);
            parseCode();
            heapSizeRequired += arrayHeapSize;
        }

        void parseCode()
        {
            const auto programHash = location.program.hashCode64();

            if (includedSourceCode.contains (programHash))
                return;

            includedSourceCode.add (programHash);

            while (currentType != Token::eof)
            {
                if (matchIf (Token::hash))
                {
                    parseCompilerDirective();
                    continue;
                }

                const bool isConstVariable = matchIf (Token::const_);

                if (! matchesAnyTypeOrVoid())
                    throwErrorExpecting ("a global variable or function");

                auto type = tokenToType (skip());
                auto name = parseIdentifier();

                if (matchIf (Token::openParen))
                {
                    if (isConstVariable)
                        location.throwError ("Return type of a function cannot be const");

                    parseFunctionDeclaration (type, name);
                    continue;
                }

                if (type == Type::void_)
                    location.throwError ("A variable type cannot be 'void'");

                parseGlobalVariableDeclaraion (isConstVariable, type, name);
            }
        }

        void simplify()
        {
            for (auto f : functions)
                f->block->simplify (*this);
        }

        const Function* findFunction (FunctionID functionID) const noexcept
        {
            for (auto f : functions)
                if (f->functionID == functionID)
                    return f;

            return nullptr;
        }

        const NativeFunction* findNativeFunction (FunctionID functionID) const noexcept
        {
            for (auto& f : nativeFunctions)
                if (f.functionID == functionID)
                    return &f;

            return nullptr;
        }

        //==============================================================================
        BlockPtr blockBeingParsed = nullptr;
        Array<Function*> functions;
        Array<File> searchPaths;
        Array<int64> includedSourceCode;
        const Array<NativeFunction>& nativeFunctions;
        uint32 heapSizeRequired;
        uint32 arrayHeapSize = 0;

        template <typename Type, typename... Args>
        Type* allocate (Args... args)   { auto o = new Type (args...); allAllocatedObjects.add (o); return o; }

    private:
        OwnedArray<AllocatedObject> allAllocatedObjects;

        //==============================================================================
        void parseCompilerDirective()
        {
            auto name = parseIdentifier();

            if (name == "heapsize")
            {
                match (Token::colon);
                heapSizeRequired = (((uint32) parseIntegerLiteral()) + 3) & ~3u;
            }
            else if (name == "include")
            {
                parseIncludeDirective();
            }
            else
            {
                location.throwError ("Unknown compiler directive");
            }
        }

        void parseIncludeDirective()
        {
            match (Token::literal);

            if (! currentValue.isString())
            {
                location.throwError ("Expected file path");
                return;
            }

            File fileToInclude = resolveIncludePath (currentValue.toString());

            if (fileToInclude == File())
                return;

            searchPaths.add (fileToInclude);
            auto codeToInclude = fileToInclude.loadFileAsString();

            auto locationToRestore = location;
            auto currentTypeToRestore = currentType;
            auto currentValueToRestore = currentValue;
            auto pToRestore = p;

            location = CodeLocation (codeToInclude, fileToInclude);
            p = codeToInclude.getCharPointer();
            skip();

            parseCode();

            location = locationToRestore;
            currentType = currentTypeToRestore;
            currentValue = currentValueToRestore;
            p = pToRestore;
        }

        File resolveIncludePath (String include)
        {
            if (include.substring (include.length() - 11) != ".littlefoot")
            {
                location.throwError ("File extension must be .littlefoot");
                return {};
            }

            auto path = Compiler::resolveIncludePath (include, searchPaths);

            if (! path.existsAsFile())
                location.throwError ("File not found: " + include);

            return path;
        }

        //TODO:   should there be a max array size?
        void parseGlobalVariableDeclaraion (bool isConst, Type type, String name)
        {
            for (;;)
            {
                if (matchIf (Token::openBracket))
                {
                    int arraySize = 0;
                    parseGlobalArray (arraySize, type, name, nullptr);
                    arrayHeapSize += uint32 (arraySize * 4);
                }
                else
                {
                    parseGlobalVariable (isConst, type, name);
                }

                if (matchIf (Token::comma))
                {
                    name = parseIdentifier();
                    continue;
                }

                match (Token::semicolon);
                break;
            }
        }

        void parseGlobalArray (int& arraySize, Type type, const String& name, Variable* parent)
        {
            const auto value = parseIntegerLiteral();
            match (Token::closeBracket);

            blockBeingParsed->addVariable ({ {}, type, true, false, {}, value, parent }, location);

            auto& newArray = blockBeingParsed->arrays.getReference (blockBeingParsed->arrays.size() - 1);

            if (parent != nullptr)
                parent->nextArray = &newArray;

            if (matchIf (Token::openBracket))
            {
                parseGlobalArray (arraySize, type, name, &newArray);
                arraySize *= value;
            }
            else
            {
                newArray.name = name;
                arraySize = value;
            }
        }

        void parseGlobalVariable (bool isConst, Type type, String name)
        {
            var constantInitialiser;

            if (isConst)
                constantInitialiser = parseConstantExpressionInitialiser (type);

            blockBeingParsed->addVariable ({ name, type, true, isConst, constantInitialiser, 0 }, location);
        }

        var parseConstantExpressionInitialiser (Type expectedType)
        {
            var result;
            match (Token::assign);
            auto e = parseExpression();

            if (auto literal = dynamic_cast<LiteralValue*> (e->simplify (*this)))
            {
                result = literal->value;

                if (getTypeOfVar (result) != expectedType)
                    location.throwError ("Expected a constant expression of type " + getTypeName (expectedType));
            }
            else
            {
                location.throwError ("Expected a constant expression");
            }

            return result;
        }

        void parseFunctionDeclaration (Type returnType, const String& name)
        {
            auto f = allocate<Function>();

            while (matchesAnyType())
            {
                auto type = tokenToType (skip());
                f->arguments.add ({ parseIdentifier(), type, false, false, 0 });

                if (f->arguments.size() > 127)
                    location.throwError ("Too many function arguments");

                if (currentType == Token::closeParen)
                    break;

                match (Token::comma);
            }

            match (Token::closeParen);
            f->functionID = createFunctionID (name, returnType, f->getArgumentTypes());

            if (findFunction (f->functionID) != nullptr || findNativeFunction (f->functionID) != nullptr)
                location.throwError ("Duplicate function declaration");

            functions.add (f);

            f->block = parseBlock (true);
            f->returnType = returnType;

            if (! f->block->alwaysReturns())
            {
                if (returnType != Type::void_)
                    location.throwError ("This function must return a value");

                f->block->statements.add (allocate<ReturnStatement> (location, f->block, nullptr));
            }
        }

        int parseIntegerLiteral()
        {
            auto e = parseExpression();

            if (auto literal = dynamic_cast<LiteralValue*> (e->simplify (*this)))
            {
                if (literal->value.isInt() || literal->value.isInt64())
                {
                    auto value = static_cast<int> (literal->value);

                    if (value > 0)
                        return value;
                }
            }

            location.throwError ("Expected an integer constant");
            return 0;
        }

        BlockPtr parseBlock (bool isMainBlockOfFunction)
        {
            match (Token::openBrace);
            auto b = allocate<BlockStatement> (location, blockBeingParsed, functions.getLast(), isMainBlockOfFunction);
            auto lastBlock = blockBeingParsed;
            blockBeingParsed = b;

            while (! matchIf (Token::closeBrace))
                b->statements.add (parseStatement());

            blockBeingParsed = lastBlock;
            return b;
        }

        StatementPtr parseStatement()
        {
            if (currentType == Token::openBrace)   return parseBlock (false);
            if (matchIf (Token::if_))              return parseIf();
            if (matchIf (Token::while_))           return parseDoOrWhileLoop (false);
            if (matchIf (Token::do_))              return parseDoOrWhileLoop (true);
            if (matchIf (Token::for_))             return parseForLoop();
            if (matchIf (Token::return_))          return parseReturn();
            if (matchIf (Token::break_))           return matchEndOfStatement (allocate<BreakStatement> (location, blockBeingParsed));
            if (matchIf (Token::continue_))        return matchEndOfStatement (allocate<ContinueStatement> (location, blockBeingParsed));
            if (matchIf (Token::semicolon))        return matchEndOfStatement (allocate<Statement> (location, blockBeingParsed));
            if (matchIf (Token::plusplus))         return matchEndOfStatement (parsePreIncDec (Token::plus));
            if (matchIf (Token::minusminus))       return matchEndOfStatement (parsePreIncDec (Token::minus));
            if (matchesAny (Token::openParen))     return matchEndOfStatement (parseFactor());
            if (matchIf (Token::const_))           return parseVariableDeclaration (true);
            if (matchesAnyType())                  return parseVariableDeclaration (false);

            if (matchesAny (Token::identifier, Token::literal, Token::minus))
                return matchEndOfStatement (parseExpression());

            throwErrorExpecting ("a statement");
            return nullptr;
        }

        ExpPtr parseExpression()
        {
            auto lhs = parseLogicOperator();

            if (matchIf (Token::question))          return parseTernaryOperator (lhs);
            if (matchIf (Token::plusEquals))        return parseInPlaceOpExpression (lhs, Token::plus);
            if (matchIf (Token::minusEquals))       return parseInPlaceOpExpression (lhs, Token::minus);
            if (matchIf (Token::timesEquals))       return parseInPlaceOpExpression (lhs, Token::times);
            if (matchIf (Token::divideEquals))      return parseInPlaceOpExpression (lhs, Token::divide);
            if (matchIf (Token::moduloEquals))      return parseInPlaceOpExpression (lhs, Token::modulo);
            if (matchIf (Token::leftShiftEquals))   return parseInPlaceOpExpression (lhs, Token::leftShift);
            if (matchIf (Token::rightShiftEquals))  return parseInPlaceOpExpression (lhs, Token::rightShift);

            if (matchIf (Token::assign))
            {
                auto loc = location;
                return allocate<Assignment> (loc, blockBeingParsed, lhs, parseExpression(), false);
            }

            return lhs;
        }

        ExpPtr parseTernaryOperator (ExpPtr condition)
        {
            auto e = allocate<TernaryOp> (location, blockBeingParsed);
            e->condition = condition;
            e->trueBranch = parseExpression();
            match (Token::colon);
            e->falseBranch = parseExpression();
            return e;
        }

        ExpPtr parseLogicOperator()
        {
            for (auto a = parseComparator();;)
            {
                if (! matchesAny (Token::logicalAnd, Token::logicalOr, Token::bitwiseOr,
                                  Token::bitwiseAnd, Token::bitwiseXor))
                    return a;

                auto loc = location;
                auto type = skip();
                a = allocate<BinaryOperator> (loc, blockBeingParsed, a, parseComparator(), type);
            }
        }

        ExpPtr parseComparator()
        {
            for (auto a = parseShiftOperator();;)
            {
                if (! matchesAny (Token::equals, Token::notEquals, Token::lessThan,
                                  Token::lessThanOrEqual, Token::greaterThan, Token::greaterThanOrEqual))
                    return a;

                auto loc = location;
                auto type = skip();
                a = allocate<BinaryOperator> (loc, blockBeingParsed, a, parseShiftOperator(), type);
            }
        }

        ExpPtr parseShiftOperator()
        {
            for (auto a = parseAdditionSubtraction();;)
            {
                if (! matchesAny (Token::leftShift, Token::rightShift, Token::rightShiftUnsigned))
                    return a;

                auto loc = location;
                auto type = skip();
                a = allocate<BinaryOperator> (loc, blockBeingParsed, a, parseExpression(), type);
            }
        }

        ExpPtr parseAdditionSubtraction()
        {
            for (auto a = parseMultiplyDivide();;)
            {
                if (! matchesAny (Token::plus, Token::minus))
                    return a;

                auto loc = location;
                auto type = skip();
                a = allocate<BinaryOperator> (loc, blockBeingParsed, a, parseMultiplyDivide(), type);
            }
        }

        ExpPtr parseMultiplyDivide()
        {
            for (auto a = parseUnary();;)
            {
                if (! matchesAny (Token::times, Token::divide, Token::modulo))
                    return a;

                auto loc = location;
                auto type = skip();
                a = allocate<BinaryOperator> (loc, blockBeingParsed, a, parseUnary(), type);
            }
        }

        ExpPtr parseUnary()
        {
            if (matchIf (Token::plusplus))    return parsePreIncDec (Token::plus);
            if (matchIf (Token::minusminus))  return parsePreIncDec (Token::minus);

            if (matchesAny (Token::minus, Token::logicalNot, Token::bitwiseNot))
            {
                auto loc = location;
                auto type = skip();
                return allocate<UnaryOp> (loc, blockBeingParsed, parseUnary(), type);
            }

            return parseFactor();
        }

        ExpPtr parseFactor()
        {
            if (currentType == Token::identifier)  return parseSuffixes (allocate<Identifier> (location, blockBeingParsed, parseIdentifier()));
            if (matchIf (Token::openParen))        return parseSuffixes (matchCloseParen (parseExpression()));
            if (matchIf (Token::true_))            return parseSuffixes (allocate<LiteralValue> (location, blockBeingParsed, true));
            if (matchIf (Token::false_))           return parseSuffixes (allocate<LiteralValue> (location, blockBeingParsed, false));

            if (currentType == Token::literal)
            {
                auto lit = allocate<LiteralValue> (location, blockBeingParsed, currentValue);
                skip();
                return parseSuffixes (lit);
            }

            if (matchesAny (Token::int_, Token::float_, Token::bool_))
                return parseSuffixes (parseFunctionCall (skip()));

            throwErrorExpecting ("an expression");
            return nullptr;
        }

        ExpPtr parseSuffixes (ExpPtr input)
        {
            if (currentType == Token::openParen)
            {
                if (auto functionName = dynamic_cast<Identifier*> (input))
                    return parseSuffixes (parseFunctionCall (functionName->name));

                location.throwError ("Malformed function call");
                return {};
            }

            if (matchIf (Token::openBracket)) return parseArraySubscript (input);
            if (matchIf (Token::plusplus))    return parsePostIncDec (input, Token::plus);
            if (matchIf (Token::minusminus))  return parsePostIncDec (input, Token::minus);

            return input;
        }

        ExpPtr parseArraySubscript (ExpPtr input)
        {
            auto s = allocate<ArraySubscript> (location, blockBeingParsed);
            s->object = input;
            s->index = parseExpression();
            match (Token::closeBracket);

            if (matchIf (Token::openBracket))
                return parseArraySubscript (s);

            return s;
        }

        ExpPtr parseInPlaceOpExpression (ExpPtr lhs, TokenType opType)
        {
            auto loc = location;
            auto rhs = parseExpression();
            return allocate<Assignment> (loc, blockBeingParsed, lhs,
                                         allocate<BinaryOperator> (location, blockBeingParsed, lhs, rhs, opType), false);
        }

        ExpPtr parsePreIncDec (TokenType opType)
        {
            auto lhs = parseFactor();
            auto one = allocate<LiteralValue> (location, blockBeingParsed, (int) 1);
            return allocate<Assignment> (location, blockBeingParsed, lhs,
                                         allocate<BinaryOperator> (location, blockBeingParsed, lhs, one, opType), false);
        }

        ExpPtr parsePostIncDec (ExpPtr lhs, TokenType opType)
        {
            auto one = allocate<LiteralValue> (location, blockBeingParsed, (int) 1);
            return allocate<Assignment> (location, blockBeingParsed, lhs,
                                         allocate<BinaryOperator> (location, blockBeingParsed, lhs, one, opType), true);
        }

        StatementPtr parseIf()
        {
            auto s = allocate<IfStatement> (location, blockBeingParsed);
            match (Token::openParen);
            s->condition = matchCloseParen (parseExpression());
            s->trueBranch = parseStatement();
            s->falseBranch = matchIf (Token::else_) ? parseStatement() : nullptr;
            return s;
        }

        StatementPtr parseReturn()
        {
            auto value = matchIf (Token::semicolon) ? nullptr : parseExpression();
            auto returnStatement = allocate<ReturnStatement> (location, blockBeingParsed, value);
            matchIf (Token::semicolon);
            return returnStatement;
        }

        StatementPtr parseVariableDeclaration (bool isConst)
        {
            if (isConst && ! matchesAnyType())
                throwErrorExpecting ("a type");

            auto type = tokenToType (skip());

            for (StatementPtr result = nullptr;;)
            {
                auto identifier = allocate<Identifier> (location, blockBeingParsed, parseIdentifier());
                auto loc = location;

                if (isConst)
                {
                    auto constantValue = parseConstantExpressionInitialiser (type);
                    blockBeingParsed->addVariable ({ identifier->getIdentifier(), type, false, true, constantValue }, loc);
                }
                else
                {
                    blockBeingParsed->addVariable ({ identifier->getIdentifier(), type, false, false, {} }, loc);

                    auto assignedValue = matchIf (Token::assign) ? parseExpression() : nullptr;

                    if (auto literal = dynamic_cast<LiteralValue*> (assignedValue))
                        if (static_cast<double> (literal->value) == 0)
                            assignedValue = nullptr;

                    if (assignedValue != nullptr || ! blockBeingParsed->isMainBlockOfFunction) // no need to assign 0 for variables in the outer scope
                    {
                        if (assignedValue == nullptr)
                            assignedValue = allocate<LiteralValue> (loc, blockBeingParsed, (int) 0);

                        auto assignment = allocate<Assignment> (loc, blockBeingParsed, identifier, assignedValue, false);

                        if (result == nullptr)
                        {
                            result = assignment;
                        }
                        else
                        {
                            auto block = dynamic_cast<BlockPtr> (result);

                            if (block == nullptr)
                            {
                                block = allocate<BlockStatement> (loc, blockBeingParsed, functions.getLast(), false);
                                block->statements.add (result);
                                result = block;
                            }

                            block->statements.add (assignment);
                        }
                    }
                }

                if (matchIf (Token::semicolon))
                    return result != nullptr ? result : allocate<Statement> (location, blockBeingParsed);

                match (Token::comma);
            }
        }

        StatementPtr parseForLoop()
        {
            auto oldBlock = blockBeingParsed;
            auto block = allocate<BlockStatement> (location, oldBlock, functions.getLast(), false);
            blockBeingParsed = block;
            auto loopStatement = allocate<LoopStatement> (location, blockBeingParsed, false);
            block->statements.add (loopStatement);
            match (Token::openParen);

            loopStatement->initialiser = parseStatement();
            loopStatement->condition = matchIf (Token::semicolon) ? allocate<LiteralValue> (location, blockBeingParsed, true)
                                                                  : matchEndOfStatement (parseExpression());
            loopStatement->iterator = matchIf (Token::closeParen) ? allocate<Statement> (location, blockBeingParsed)
                                                                  : matchCloseParen (parseExpression());
            loopStatement->body = parseStatement();

            blockBeingParsed = oldBlock;
            return block;
        }

        StatementPtr parseDoOrWhileLoop (bool isDoLoop)
        {
            auto loopStatement          = allocate<LoopStatement> (location, blockBeingParsed, isDoLoop);
            loopStatement->initialiser  = allocate<Statement> (location, blockBeingParsed);
            loopStatement->iterator     = allocate<Statement> (location, blockBeingParsed);

            if (isDoLoop)
            {
                loopStatement->body = parseBlock (false);
                match (Token::while_);
            }

            match (Token::openParen);
            loopStatement->condition = matchCloseParen (parseExpression());

            if (! isDoLoop)
                loopStatement->body = parseStatement();

            return loopStatement;
        }

        String parseIdentifier()
        {
            auto name = currentValue.toString();
            match (Token::identifier);
            return name;
        }

        ExpPtr parseFunctionCall (const String& name)
        {
            auto call = allocate<FunctionCall> (location, blockBeingParsed);
            call->functionName = name;
            match (Token::openParen);

            while (currentType != Token::closeParen)
            {
                call->arguments.add (parseExpression());

                if (currentType == Token::closeParen)
                    break;

                match (Token::comma);
            }

            return matchCloseParen (call);
        }

        bool matchesAnyType() const noexcept         { return matchesAny (Token::int_, Token::float_, Token::bool_); }
        bool matchesAnyTypeOrVoid() const noexcept   { return matchesAnyType() || currentType == Token::void_; }
        ExpPtr matchCloseParen (ExpPtr e)            { match (Token::closeParen); return e; }
        template<typename ExpType> ExpType matchEndOfStatement (ExpType e)  { match (Token::semicolon); return e; }
    };

    //==============================================================================
    //==============================================================================
    struct CodeGenerator
    {
        CodeGenerator (Array<uint8>& output, SyntaxTreeBuilder& stb)
            : outputCode (output), syntaxTree (stb) {}

        void generateCode (BlockPtr outerBlock, uint32 heapSizeBytesRequired)
        {
            for (auto f : syntaxTree.functions)
            {
                f->address = createMarker();
                f->unwindAddress = createMarker();
            }

            emit ((int16) 0); // checksum
            emit ((int16) 0); // size
            emit ((int16) syntaxTree.functions.size());
            emit ((int16) outerBlock->variables.size());
            emit ((int16) heapSizeBytesRequired);

            for (auto f : syntaxTree.functions)
                emit (f->functionID, f->address);

            auto codeStart = outputCode.size();

            for (auto f : syntaxTree.functions)
                f->emit (*this);

            removeJumpsToNextInstruction (codeStart);
            resolveMarkers();

            Program::writeInt16 (outputCode.begin() + 2, (int16) outputCode.size());
            const Program program (outputCode.begin(), (uint32) outputCode.size());
            Program::writeInt16 (outputCode.begin(), (int16) program.calculateChecksum());
            jassert (program.checksumMatches());
        }

        //==============================================================================
        Array<uint8>& outputCode;
        SyntaxTreeBuilder& syntaxTree;

        struct Marker  { int index = 0; };
        struct MarkerAndAddress  { Marker marker; int address; };

        int nextMarker = 0;
        Array<MarkerAndAddress> markersToResolve, resolvedMarkers;

        Marker createMarker() noexcept  { Marker m; m.index = ++nextMarker; return m; }
        void attachMarker (Marker m)    { resolvedMarkers.add ({ m, outputCode.size() }); }

        int getResolvedMarkerAddress (Marker marker) const
        {
            for (auto m : resolvedMarkers)
                if (m.marker.index == marker.index)
                    return m.address;

            jassertfalse;
            return 0;
        }

        Marker getMarkerAtAddress (int address) const noexcept
        {
            for (auto m : markersToResolve)
                if (m.address == address)
                    return m.marker;

            jassertfalse;
            return {};
        }

        void resolveMarkers()
        {
            for (auto m : markersToResolve)
                Program::writeInt16 (outputCode.begin() + m.address, (int16) getResolvedMarkerAddress (m.marker));
        }

        void removeCode (int address, int size)
        {
            outputCode.removeRange (address, size);

            for (int i = markersToResolve.size(); --i >= 0;)
            {
                auto& m = markersToResolve.getReference (i);

                if (m.address >= address + size)
                    m.address -= size;
                else if (m.address >= address)
                    markersToResolve.remove (i);
            }

            for (auto& m : resolvedMarkers)
                if (m.address >= address + size)
                    m.address -= size;
        }

        void removeJumpsToNextInstruction (int address)
        {
            while (address < outputCode.size())
            {
                auto op = (OpCode) outputCode.getUnchecked (address);
                auto opSize = 1 + Program::getNumExtraBytesForOpcode (op);

                if (op == OpCode::jump)
                {
                    auto marker = getMarkerAtAddress (address + 1);

                    if (marker.index != 0)
                    {
                        if (getResolvedMarkerAddress (marker) == address + opSize)
                        {
                            removeCode (address, opSize);
                            continue;
                        }
                    }
                }

                address += opSize;
            }
        }

        Marker breakTarget, continueTarget;

        //==============================================================================
        void emit (OpCode op)           { emit ((int8) op); }
        void emit (Marker m)            { markersToResolve.add ({ m, outputCode.size() }); emit ((int16) 0); }
        void emit (int8 value)          { outputCode.add ((uint8) value); }
        void emit (int16 value)         { uint8 d[2]; Program::writeInt16 (d, value); outputCode.insertArray (-1, d, (int) sizeof (d)); }
        void emit (int32 value)         { uint8 d[4]; Program::writeInt32 (d, value); outputCode.insertArray (-1, d, (int) sizeof (d)); }

        template <typename Arg1, typename... Args>
        void emit (Arg1 arg1, Args... args)
        {
            emit (arg1);
            emit (args...);
        }

        void emitPush (const var& value)
        {
            if (value.isDouble())
            {
                const float v = value;

                if (v == 0)  emit (OpCode::push0);
                else         emit (OpCode::push32, Program::floatToInt (v));
            }
            else
            {
                const int v = value;

                if (v == 0)                     emit (OpCode::push0);
                else if (v == 1)                emit (OpCode::push1);
                else if (v > 0 && v < 128)      emit (OpCode::push8,  (int8)  v);
                else if (v > 0 && v < 32768)    emit (OpCode::push16, (int16) v);
                else                            emit (OpCode::push32, (int32) v);
            }
        }

        void emitCast (Type source, Type dest, const CodeLocation& location)
        {
            if (dest == source) return;
            if (dest == Type::void_)  return emit (OpCode::drop);
            if (source == Type::bool_ && dest == Type::int_) return;
            if (source == Type::int_ && dest == Type::bool_) return emit (OpCode::testNZ_int32);
            if ((source == Type::int_ || source == Type::bool_) && dest == Type::float_) return emit (OpCode::int32ToFloat);

            location.throwError ("Cannot cast from " + getTypeName (source) + " to " + getTypeName (dest));
        }

        void emitVariableRead (Type sourceType, Type requiredType, int stackDepth, int index, const CodeLocation& location)
        {
            if (index < 0)
            {
                emit (OpCode::dupFromGlobal, (int16) ((-index) - 1));
            }
            else
            {
                index += stackDepth;

                if (index == 0)
                    emit (OpCode::dup);
                else if (index < 8)
                    emit ((OpCode) ((int) OpCode::dupOffset_01 + index - 1));
                else if (index >= 128)
                    emit (OpCode::dupOffset16, (int16) index);
                else
                    emit (OpCode::dupOffset, (int8) index);
            }

            emitCast (sourceType, requiredType, location);
        }

        void emitArrayElementIndex (const Expression* target, BlockPtr parentBlock,
                                    int stackDepth, const CodeLocation& errorLocation)
        {
            if (auto currentSubscript = dynamic_cast<const ArraySubscript*> (target))
            {
                const auto identifier = currentSubscript->getIdentifier();
                auto array = parentBlock->getArray (identifier, errorLocation);

                ExpPtr elementIndent = nullptr;
                auto currentArray = &array;

                while (currentSubscript != nullptr && currentArray != nullptr)
                {
                    auto lhs = syntaxTree.allocate<LiteralValue> (errorLocation, parentBlock, parentBlock->getArrayElementSizeInBytes (*currentArray));
                    ExpPtr subscriptIndent = syntaxTree.allocate<BinaryOperator> (errorLocation, parentBlock, lhs, currentSubscript->index, Token::times);

                    if (elementIndent == nullptr)
                        elementIndent = subscriptIndent;
                    else
                        elementIndent = syntaxTree.allocate<BinaryOperator> (errorLocation, parentBlock, elementIndent, subscriptIndent, Token::plus);

                    currentSubscript = dynamic_cast<ArraySubscript*> (currentSubscript->object);
                    currentArray = currentArray->previousArray;
                }

                auto arrayStart = (int) (syntaxTree.heapSizeRequired - syntaxTree.arrayHeapSize) + parentBlock->getArrayStart (identifier, errorLocation);
                auto lhs = syntaxTree.allocate<LiteralValue> (errorLocation, parentBlock, arrayStart);
                elementIndent = syntaxTree.allocate<BinaryOperator> (errorLocation, parentBlock, lhs, elementIndent, Token::plus);

                elementIndent = elementIndent->simplify (syntaxTree);
                elementIndent->emit (*this, Type::int_, stackDepth);
            }
            else
            {
                errorLocation.throwError ("Cannot cast Expression to ArraySubscript");
            }
        }
    };

    //==============================================================================
    //==============================================================================
    struct Statement  : public AllocatedObject
    {
        struct Visitor
        {
            virtual ~Visitor() = default;
            virtual void operator()(StatementPtr) = 0;
        };

        Statement (const CodeLocation& l, BlockPtr parent) noexcept : location (l), parentBlock (parent) {}
        virtual void emit (CodeGenerator&, Type, int /*stackDepth*/) const {}
        virtual bool alwaysReturns() const                  { return false; }
        virtual void visitSubStatements (Visitor&) const {}
        virtual Statement* simplify (SyntaxTreeBuilder&)    { return this; }

        CodeLocation location;
        BlockPtr parentBlock;
    };

    struct Expression  : public Statement
    {
        Expression (const CodeLocation& l, BlockPtr parent) noexcept : Statement (l, parent) {}
        virtual Type getType (CodeGenerator&) const = 0;
        ExpPtr simplify (SyntaxTreeBuilder&) override    { return this; }
        virtual String getIdentifier() const { location.throwError ("This operator requires an assignable variable"); return {}; }
    };

    struct Variable
    {
        // VS2015 requires a constructor to avoid aggregate initialization
        Variable (const String& n, Type t, bool isGlobalVar, bool isConstVar, const var& cv,
                  int nElements = 0, Variable* pArray = nullptr, Variable* nArray = nullptr)
           : name (n), type (t), isGlobal (isGlobalVar), isConst (isConstVar), constantValue (cv),
             numElements (nElements), previousArray (pArray), nextArray (nArray)
        {
        }

        String name;
        Type type;
        bool isGlobal, isConst;
        var constantValue;
        int numElements = 0;
        Variable* previousArray = nullptr;
        Variable* nextArray = nullptr;
    };

    //==============================================================================
    struct Function  : public AllocatedObject
    {
        FunctionID functionID;
        Type returnType;
        Array<Variable> arguments;
        BlockPtr block;
        CodeGenerator::Marker address, unwindAddress;

        void emit (CodeGenerator& cg) const
        {
            cg.attachMarker (address);

            const int numLocals = getNumLocals();

            for (int num = numLocals; num > 0;)
            {
                if (num == 1)
                {
                    cg.emit (OpCode::push0);
                    --num;
                }
                else
                {
                    int numToDo = jmin (127, num);
                    cg.emit (OpCode::pushMultiple0, (int8) numToDo);
                    num -= numToDo;
                }
            }

            block->emit (cg, Type::void_, 0);
            cg.attachMarker (unwindAddress);

            const bool keepTop = returnType != Type::void_;

            for (int num = numLocals; num > 0;)
            {
                if (num == 1 && ! keepTop)
                {
                    cg.emit (OpCode::drop);
                    --num;
                }
                else
                {
                    int numToDo = jmin (127, num);
                    cg.emit (OpCode::dropMultiple, (int8) (keepTop ? -numToDo : numToDo));
                    num -= numToDo;
                }
            }

            cg.emit (keepTop ? OpCode::retValue : OpCode::retVoid, (int8) arguments.size());
        }

        Array<Type> getArgumentTypes() const
        {
            Array<Type> argTypes;

            for (auto& arg : arguments)
                argTypes.add (arg.type);

            return argTypes;
        }

        int getNumLocals() const noexcept
        {
            return countMaxNumLocalVariables (block);
        }

        static int countMaxNumLocalVariables (StatementPtr s) noexcept
        {
            struct Counter : Statement::Visitor
            {
                void operator() (StatementPtr sub) override
                {
                    num = jmax (num, countMaxNumLocalVariables (sub));
                }

                int num = 0;
            };

            Counter counter;

            if (s != nullptr)
                s->visitSubStatements (counter);

            if (auto block = dynamic_cast<BlockPtr> (s))
                counter.num += block->variables.size();

            return counter.num;
        }
    };

    //==============================================================================
    struct BlockStatement  : public Statement
    {
        BlockStatement (const CodeLocation& l, BlockPtr parent, Function* f, bool isMainFunctionBlock)
            : Statement (l, parent), function (f), isMainBlockOfFunction (isMainFunctionBlock) {}

        void emit (CodeGenerator& cg, Type requiredType, int stackDepth) const override
        {
            jassert (requiredType == Type::void_); ignoreUnused (requiredType);
            jassert (function != nullptr);

            for (auto s : statements)
                s->emit (cg, Type::void_, stackDepth);
        }

        bool alwaysReturns() const override
        {
            return ! statements.isEmpty() && statements.getLast()->alwaysReturns();
        }

        void visitSubStatements (Statement::Visitor& visit) const override
        {
            for (auto s : statements)
                visit (s);
        }

        Statement* simplify (SyntaxTreeBuilder& stb) override
        {
            for (int i = 0; i < statements.size(); ++i)
                statements.set (i, statements.getReference(i)->simplify (stb));

            return this;
        }

        // returns -ve values for globals
        int getVariableDepth (const String& name, const CodeLocation& locationForError) const
        {
            auto index = indexOf (variables, name);

            if (index >= 0)
                return getNumVariablesInParentBlocks() + index;

            if (! isMainBlockOfFunction)
                return parentBlock->getVariableDepth (name, locationForError);

            if (function != nullptr)
                for (int i = function->arguments.size(); --i >= 0;)
                    if (function->arguments.getReference(i).name == name)
                        return i + 1 + function->getNumLocals();

            index = indexOf (getGlobalVariables(), name);

            if (index >= 0)
                return -(index + 1);

            locationForError.throwError ("Unknown variable '" + name + "'");
        }

        Variable getArray (const String& name, const CodeLocation& locationForError) const
        {
            for (const auto& array : getGlobalArrays())
                if (array.name == name)
                    return array;

            locationForError.throwError ("Unknown array '" + name + "'");
        }

        int getArraySizeInBytes (const Variable& array) const
        {
            return array.numElements * getArrayElementSizeInBytes (array);
        }

        int getArrayElementSizeInBytes (const Variable& array) const
        {
            if (array.nextArray != nullptr)
                return getArraySizeInBytes (*array.nextArray);

            return numBytesInType;
        }

        int getArrayStart (const String& name, const CodeLocation& locationForError) const
        {
            int start = 0;

            for (const auto& array : getGlobalArrays())
            {
                if (array.name == name)
                    return start;

                if (array.name.isNotEmpty())
                    start += getArraySizeInBytes (array);
            }

            locationForError.throwError ("Unknown array '" + name + "'");
        }

        int getNumVariablesInParentBlocks() const noexcept
        {
            return isMainBlockOfFunction ? 0 : (parentBlock->getNumVariablesInParentBlocks()
                                                 + parentBlock->variables.size());
        }

        const Array<Variable>& getGlobalVariables() const noexcept  { return parentBlock != nullptr ? parentBlock->getGlobalVariables() : variables; }
        const Array<Variable>& getGlobalConstants() const noexcept  { return parentBlock != nullptr ? parentBlock->getGlobalConstants() : constants; }
        const Array<Variable>& getGlobalArrays() const noexcept     { return parentBlock != nullptr ? parentBlock->getGlobalArrays() : arrays; }

        const Variable& getVariable (const String& name, const CodeLocation& locationForError) const
        {
            for (auto& v : constants)
                if (v.name == name)
                    return v;

            for (auto& v : variables)
                if (v.name == name)
                    return v;

            if (! isMainBlockOfFunction && parentBlock != nullptr)
                return parentBlock->getVariable (name, locationForError);

            if (function != nullptr)
                for (auto& v : function->arguments)
                    if (v.name == name)
                        return v;

            for (auto& v : getGlobalConstants())
                if (v.name == name)
                    return v;

            for (auto& v : getGlobalVariables())
                if (v.name == name)
                    return v;

            for (auto& v : getGlobalArrays())
                if (v.name == name)
                    return v;

            locationForError.throwError ("Unknown variable '" + name + "'");
        }

        void addVariable (Variable v, const CodeLocation& locationForError)
        {
            if (v.name.isNotEmpty() && (indexOf (variables, v.name) >= 0 || indexOf (constants, v.name) >= 0 || indexOf (arrays, v.name) >= 0))
                locationForError.throwError ("Variable '" + v.name + "' already exists");

            (v.numElements == 0 ? (v.isConst ? constants : variables) : arrays).add (v);
        }

        static int indexOf (const Array<Variable>& vars, const String& name) noexcept
        {
            for (int i = 0; i < vars.size(); ++i)
                if (vars.getReference(i).name == name)
                    return i;

            return -1;
        }

        Function* function;
        Array<StatementPtr> statements;
        Array<Variable> variables, constants, arrays;
        bool isMainBlockOfFunction;
    };

    struct IfStatement  : public Statement
    {
        IfStatement (const CodeLocation& l, BlockPtr parent) : Statement (l, parent) {}

        void emit (CodeGenerator& cg, Type requiredType, int stackDepth) const override
        {
            jassert (requiredType == Type::void_); ignoreUnused (requiredType);

            condition->emit (cg, Type::bool_, stackDepth);
            auto endOfStatement = cg.createMarker();

            if (falseBranch == nullptr)
            {
                cg.emit (OpCode::jumpIfFalse, endOfStatement);
                trueBranch->emit (cg, Type::void_, stackDepth);
            }
            else
            {
                auto elseTarget = cg.createMarker();
                cg.emit (OpCode::jumpIfFalse, elseTarget);
                trueBranch->emit (cg, Type::void_, stackDepth);
                cg.emit (OpCode::jump, endOfStatement);
                cg.attachMarker (elseTarget);
                falseBranch->emit (cg, Type::void_, stackDepth);
            }

            cg.attachMarker (endOfStatement);
        }

        bool alwaysReturns() const override
        {
            return trueBranch->alwaysReturns() && falseBranch != nullptr && falseBranch->alwaysReturns();
        }

        void visitSubStatements (Statement::Visitor& visit) const override
        {
            visit (condition); visit (trueBranch); visit (falseBranch);
        }

        Statement* simplify (SyntaxTreeBuilder& stb) override
        {
            condition   = condition->simplify (stb);
            trueBranch  = trueBranch->simplify (stb);
            falseBranch = falseBranch != nullptr ? falseBranch->simplify (stb) : nullptr;

            if (auto literal = dynamic_cast<LiteralValue*> (condition))
                return literal->value ? trueBranch : (falseBranch != nullptr ? falseBranch : stb.allocate<Statement> (location, parentBlock));

            return this;
        }

        ExpPtr condition;
        StatementPtr trueBranch, falseBranch;
    };

    struct TernaryOp  : public Expression
    {
        TernaryOp (const CodeLocation& l, BlockPtr parent)  : Expression (l, parent) {}

        void emit (CodeGenerator& cg, Type requiredType, int stackDepth) const override
        {
            condition->emit (cg, Type::bool_, stackDepth);
            auto endOfStatement = cg.createMarker();
            auto elseTarget = cg.createMarker();
            cg.emit (OpCode::jumpIfFalse, elseTarget);
            trueBranch->emit (cg, requiredType, stackDepth);
            cg.emit (OpCode::jump, endOfStatement);
            cg.attachMarker (elseTarget);
            falseBranch->emit (cg, requiredType, stackDepth);
            cg.attachMarker (endOfStatement);
        }

        Type getType (CodeGenerator& cg) const override
        {
            auto type = trueBranch->getType (cg);

            if (type == Type::void_)                location.throwError ("The ternary operator cannot take void arguments");
            if (type != falseBranch->getType (cg))  location.throwError ("Expected both branches of this ternary operator to have the same type");

            return type;
        }

        void visitSubStatements (Statement::Visitor& visit) const override
        {
            visit (condition); visit (trueBranch); visit (falseBranch);
        }

        ExpPtr simplify (SyntaxTreeBuilder& stb) override
        {
            condition   = condition->simplify (stb);
            trueBranch  = trueBranch->simplify (stb);
            falseBranch = falseBranch->simplify (stb);

            if (auto literal = dynamic_cast<LiteralValue*> (condition))
                return literal->value ? trueBranch : falseBranch;

            return this;
        }

        ExpPtr condition, trueBranch, falseBranch;
    };

    struct LoopStatement  : public Statement
    {
        LoopStatement (const CodeLocation& l, BlockPtr parent, bool isDo) noexcept   : Statement (l, parent), isDoLoop (isDo) {}

        void emit (CodeGenerator& cg, Type, int stackDepth) const override
        {
            initialiser->emit (cg, Type::void_, stackDepth);

            auto loopStart = cg.createMarker();
            cg.attachMarker (loopStart);

            auto oldBreakTarget = cg.breakTarget;
            auto oldContinueTarget = cg.continueTarget;
            cg.breakTarget = cg.createMarker();
            cg.continueTarget = cg.createMarker();

            if (isDoLoop)
            {
                body->emit (cg, Type::void_, stackDepth);
                cg.attachMarker (cg.continueTarget);
                condition->emit (cg, Type::bool_, stackDepth);
                cg.emit (OpCode::jumpIfTrue, loopStart);
            }
            else
            {
                condition->emit (cg, Type::bool_, stackDepth);
                cg.emit (OpCode::jumpIfFalse, cg.breakTarget);
                body->emit (cg, Type::void_, stackDepth);
                cg.attachMarker (cg.continueTarget);
                iterator->emit (cg, Type::void_, stackDepth);
                cg.emit (OpCode::jump, loopStart);
            }

            cg.attachMarker (cg.breakTarget);
            cg.breakTarget = oldBreakTarget;
            cg.continueTarget = oldContinueTarget;
        }

        StatementPtr simplify (SyntaxTreeBuilder& stb) override
        {
            initialiser = initialiser->simplify (stb);
            iterator = iterator->simplify (stb);
            body = body->simplify (stb);
            condition = condition->simplify (stb);
            return this;
        }

        void visitSubStatements (Statement::Visitor& visit) const override
        {
            visit (condition); visit (initialiser); visit (iterator); visit (body);
        }

        StatementPtr initialiser, iterator, body;
        ExpPtr condition;
        bool isDoLoop;
    };

    struct ReturnStatement  : public Statement
    {
        ReturnStatement (const CodeLocation& l, BlockPtr parent, ExpPtr v) noexcept  : Statement (l, parent), returnValue (v) {}

        void emit (CodeGenerator& cg, Type, int stackDepth) const override
        {
            if (auto fn = parentBlock->function)
            {
                if (returnValue != nullptr)
                    returnValue->emit (cg, fn->returnType, stackDepth);
                else if (fn->returnType != Type::void_)
                    location.throwError ("Cannot return a value from a void function");

                cg.emit (OpCode::jump, fn->unwindAddress);
                return;
            }

            location.throwError ("The return statement can only be used inside a function");
        }

        bool alwaysReturns() const override     { return true; }

        StatementPtr simplify (SyntaxTreeBuilder& stb) override
        {
            if (returnValue != nullptr)
                returnValue = returnValue->simplify (stb);

            return this;
        }

        void visitSubStatements (Statement::Visitor& visit) const override
        {
            visit (returnValue);
        }

        ExpPtr returnValue;
    };

    struct BreakStatement  : public Statement
    {
        BreakStatement (const CodeLocation& l, BlockPtr parent) : Statement (l, parent) {}

        void emit (CodeGenerator& cg, Type, int) const override
        {
            if (cg.breakTarget.index == 0)
                location.throwError ("The break statement can only be used inside a loop");

            cg.emit (OpCode::jump, cg.breakTarget);
        }
    };

    struct ContinueStatement  : public Statement
    {
        ContinueStatement (const CodeLocation& l, BlockPtr parent) : Statement (l, parent) {}

        void emit (CodeGenerator& cg, Type, int) const override
        {
            if (cg.continueTarget.index == 0)
                location.throwError ("The continue statement can only be used inside a loop");

            cg.emit (OpCode::jump, cg.continueTarget);
        }
    };

    struct LiteralValue  : public Expression
    {
        LiteralValue (const CodeLocation& l, BlockPtr parent, const var& v) noexcept : Expression (l, parent), value (v) {}

        void emit (CodeGenerator& cg, Type requiredType, int) const override
        {
            if (requiredType != Type::void_)
            {
                auto type = getType (cg);

                if (type != requiredType && value != var ((int) 0))
                {
                    if (type == Type::int_ && requiredType == Type::bool_)   return cg.emitPush (static_cast<bool> (value));
                    if (type == Type::int_ && requiredType == Type::float_)  return cg.emitPush (static_cast<float> (value));

                    if (! (type == Type::bool_ && requiredType == Type::int_))
                        location.throwError ("Cannot cast from " + getTypeName (type) + " to " + getTypeName (requiredType));
                }

                cg.emitPush (value);
            }
        }

        Type getType (CodeGenerator&) const override
        {
            auto t = getTypeOfVar (value);

            if (t == Type::void_)
                location.throwError ("Unsupported literal type");

            return t;
        }

        var value;
    };

    struct Identifier  : public Expression
    {
        Identifier (const CodeLocation& l, BlockPtr parent, const String& n) noexcept : Expression (l, parent), name (n) {}

        void emit (CodeGenerator& cg, Type requiredType, int stackDepth) const override
        {
            cg.emitVariableRead (getType (cg), requiredType, stackDepth,
                                 parentBlock->getVariableDepth (name, location), location);
        }

        Type getType (CodeGenerator&) const override
        {
            return parentBlock->getVariable (name, location).type;
        }

        ExpPtr simplify (SyntaxTreeBuilder& stb) override
        {
            auto& v = parentBlock->getVariable (name, location);

            if (v.isConst)
                return stb.allocate<LiteralValue> (location, parentBlock, v.constantValue);

            return this;
        }

        String getIdentifier() const override
        {
            return name;
        }

        String name;
    };

    struct UnaryOp  : public Expression
    {
        UnaryOp (const CodeLocation& l, BlockPtr parent, ExpPtr a, TokenType op) noexcept
            : Expression (l, parent), source (a), operation (op) {}

        ExpPtr source;
        TokenType operation;

        void emit (CodeGenerator& cg, Type requiredType, int stackDepth) const override
        {
            auto sourceType = source->getType (cg);

            if (operation == Token::minus)
            {
                cg.emitPush ((int) 0);
                source->emit (cg, sourceType, stackDepth + 1);
                cg.emit (sourceType == Type::float_ ? OpCode::sub_float : OpCode::sub_int32);
                cg.emitCast (sourceType, requiredType, location);
            }
            else
            {
                if (sourceType == Type::float_)
                    location.throwError ("Cannot perform this operation on a float");

                if (operation == Token::logicalNot)
                {
                    source->emit (cg, sourceType, stackDepth);
                    cg.emit (OpCode::logicalNot);
                    cg.emitCast (Type::bool_, requiredType, location);
                }
                else if (operation == Token::bitwiseNot)
                {
                    source->emit (cg, Type::int_, stackDepth);
                    cg.emit (OpCode::bitwiseNot);
                    cg.emitCast (Type::int_, requiredType, location);
                }
            }
        }

        Type getType (CodeGenerator& cg) const override
        {
            if (operation == Token::minus)         return source->getType (cg);
            if (operation == Token::logicalNot)    return Type::bool_;
            return  Type::int_;
        }

        void visitSubStatements (Statement::Visitor& visit) const override
        {
            visit (source);
        }

        ExpPtr simplify (SyntaxTreeBuilder& stb) override
        {
            source = source->simplify (stb);

            if (auto literal = dynamic_cast<LiteralValue*> (source))
            {
                auto type = getTypeOfVar (literal->value);

                if (type == Type::int_    && operation == Token::minus)         { literal->value =  -static_cast<int>    (literal->value); return literal; }
                if (type == Type::int_    && operation == Token::bitwiseNot)    { literal->value =  ~static_cast<int>    (literal->value); return literal; }
                if (type == Type::int_    && operation == Token::logicalNot)    { literal->value = ! static_cast<int>    (literal->value); return literal; }
                if (type == Type::bool_   && operation == Token::logicalNot)    { literal->value = ! static_cast<bool>   (literal->value); return literal; }
                if (type == Type::float_  && operation == Token::minus)         { literal->value =  -static_cast<double> (literal->value); return literal; }
            }

            return this;
        }
    };

    struct BinaryOperator  : public Expression
    {
        BinaryOperator (const CodeLocation& l, BlockPtr parent, ExpPtr a, ExpPtr b, TokenType op) noexcept
            : Expression (l, parent), lhs (a), rhs (b), operation (op) {}

        ExpPtr lhs, rhs;
        TokenType operation;

        void emit (CodeGenerator& cg, Type requiredType, int stackDepth) const override
        {
            auto typeA = lhs->getType (cg);
            auto typeB = rhs->getType (cg);

            if (typeA == Type::float_ || typeB == Type::float_)
            {
                lhs->emit (cg, Type::float_, stackDepth);
                rhs->emit (cg, Type::float_, stackDepth + 1);

                if (operation == Token::plus)     return emitOpAndCast (cg, requiredType, OpCode::add_float);
                if (operation == Token::minus)    return emitOpAndCast (cg, requiredType, OpCode::sub_float);
                if (operation == Token::times)    return emitOpAndCast (cg, requiredType, OpCode::mul_float);
                if (operation == Token::divide)   return emitOpAndCast (cg, requiredType, OpCode::div_float);

                cg.emit (OpCode::sub_float);

                if (operation == Token::equals)                return emitOpAndCast (cg, requiredType, OpCode::testZE_float);
                if (operation == Token::notEquals)             return emitOpAndCast (cg, requiredType, OpCode::testNZ_float);
                if (operation == Token::lessThan)              return emitOpAndCast (cg, requiredType, OpCode::testLT_float);
                if (operation == Token::lessThanOrEqual)       return emitOpAndCast (cg, requiredType, OpCode::testLE_float);
                if (operation == Token::greaterThan)           return emitOpAndCast (cg, requiredType, OpCode::testGT_float);
                if (operation == Token::greaterThanOrEqual)    return emitOpAndCast (cg, requiredType, OpCode::testGE_float);

                location.throwError ("The operator " + getTokenDescription (operation) + " cannot take floating point arguments");
            }

            auto type = (typeA == Type::bool_ || typeB == Type::bool_) ? Type::bool_
                                                                       : Type::int_;
            lhs->emit (cg, type, stackDepth);
            rhs->emit (cg, type, stackDepth + 1);

            if (operation == Token::plus)          return emitOpAndCast (cg, requiredType, OpCode::add_int32);
            if (operation == Token::minus)         return emitOpAndCast (cg, requiredType, OpCode::sub_int32);
            if (operation == Token::times)         return emitOpAndCast (cg, requiredType, OpCode::mul_int32);
            if (operation == Token::divide)        return emitOpAndCast (cg, requiredType, OpCode::div_int32);
            if (operation == Token::modulo)        return emitOpAndCast (cg, requiredType, OpCode::mod_int32);
            if (operation == Token::logicalOr)     return emitOpAndCast (cg, requiredType, OpCode::logicalOr);
            if (operation == Token::logicalAnd)    return emitOpAndCast (cg, requiredType, OpCode::logicalAnd);
            if (operation == Token::bitwiseOr)     return emitOpAndCast (cg, requiredType, OpCode::bitwiseOr);
            if (operation == Token::bitwiseAnd)    return emitOpAndCast (cg, requiredType, OpCode::bitwiseAnd);
            if (operation == Token::bitwiseXor)    return emitOpAndCast (cg, requiredType, OpCode::bitwiseXor);
            if (operation == Token::leftShift)     return emitOpAndCast (cg, requiredType, OpCode::bitShiftLeft);
            if (operation == Token::rightShift)    return emitOpAndCast (cg, requiredType, OpCode::bitShiftRight);

            cg.emit (OpCode::sub_int32);

            if (operation == Token::equals)                return emitOpAndCast (cg, requiredType, OpCode::testZE_int32);
            if (operation == Token::notEquals)             return emitOpAndCast (cg, requiredType, OpCode::testNZ_int32);
            if (operation == Token::lessThan)              return emitOpAndCast (cg, requiredType, OpCode::testLT_int32);
            if (operation == Token::lessThanOrEqual)       return emitOpAndCast (cg, requiredType, OpCode::testLE_int32);
            if (operation == Token::greaterThan)           return emitOpAndCast (cg, requiredType, OpCode::testGT_int32);
            if (operation == Token::greaterThanOrEqual)    return emitOpAndCast (cg, requiredType, OpCode::testGE_int32);

            location.throwError ("Unsupported operator");
            jassertfalse;
        }

        void emitOpAndCast (CodeGenerator& cg, Type requiredType, OpCode op) const
        {
            cg.emit (op);
            cg.emitCast (getType (cg), requiredType, location);
        }

        Type getResultType (Type typeA, Type typeB) const noexcept
        {
            if (operation == Token::logicalOr || operation == Token::logicalAnd
                 || operation == Token::equals || operation == Token::notEquals
                 || operation == Token::lessThan || operation == Token::lessThanOrEqual
                 || operation == Token::greaterThan || operation == Token::greaterThanOrEqual)
                return Type::bool_;

            if (operation == Token::plus || operation == Token::minus
                 || operation == Token::times || operation == Token::divide)
            {
                if (typeA == Type::float_ || typeB == Type::float_)
                    return Type::float_;
            }

            return Type::int_;
        }

        Type getType (CodeGenerator& cg) const override
        {
            return getResultType (lhs->getType (cg), rhs->getType (cg));
        }

        void visitSubStatements (Statement::Visitor& visit) const override
        {
            visit (lhs); visit (rhs);
        }

        ExpPtr simplifyFloat (double a, double b, LiteralValue* literal)
        {
            if (operation == Token::plus)                 { literal->value = a + b;  return literal; }
            if (operation == Token::minus)                { literal->value = a - b;  return literal; }
            if (operation == Token::times)                { literal->value = a * b;  return literal; }
            if (operation == Token::divide)               { literal->value = a / b;  return literal; }
            if (operation == Token::equals)               { literal->value = a == b; return literal; }
            if (operation == Token::notEquals)            { literal->value = a != b; return literal; }
            if (operation == Token::lessThan)             { literal->value = a <  b; return literal; }
            if (operation == Token::lessThanOrEqual)      { literal->value = a <= b; return literal; }
            if (operation == Token::greaterThan)          { literal->value = a >  b; return literal; }
            if (operation == Token::greaterThanOrEqual)   { literal->value = a >= b; return literal; }
            return this;
        }

        ExpPtr simplifyBool (bool a, bool b, LiteralValue* literal)
        {
            if (operation == Token::logicalOr)            { literal->value = a || b; return literal; }
            if (operation == Token::logicalAnd)           { literal->value = a && b; return literal; }
            return this;
        }

        ExpPtr simplifyInt (int a, int b, LiteralValue* literal)
        {
            if (operation == Token::plus)                 { literal->value = a +  b; return literal; }
            if (operation == Token::minus)                { literal->value = a -  b; return literal; }
            if (operation == Token::times)                { literal->value = a *  b; return literal; }
            if (operation == Token::divide)               { literal->value = a /  b; return literal; }
            if (operation == Token::equals)               { literal->value = a == b; return literal; }
            if (operation == Token::notEquals)            { literal->value = a != b; return literal; }
            if (operation == Token::lessThan)             { literal->value = a <  b; return literal; }
            if (operation == Token::lessThanOrEqual)      { literal->value = a <= b; return literal; }
            if (operation == Token::greaterThan)          { literal->value = a >  b; return literal; }
            if (operation == Token::greaterThanOrEqual)   { literal->value = a >= b; return literal; }
            if (operation == Token::modulo)               { literal->value = a %  b; return literal; }
            if (operation == Token::logicalOr)            { literal->value = a || b; return literal; }
            if (operation == Token::logicalAnd)           { literal->value = a && b; return literal; }
            if (operation == Token::bitwiseOr)            { literal->value = a |  b; return literal; }
            if (operation == Token::bitwiseAnd)           { literal->value = a &  b; return literal; }
            if (operation == Token::bitwiseXor)           { literal->value = a ^  b; return literal; }
            if (operation == Token::leftShift)            { literal->value = a << b; return literal; }
            if (operation == Token::rightShift)           { literal->value = a >> b; return literal; }
            return this;
        }

        ExpPtr simplify (SyntaxTreeBuilder& stb) override
        {
            lhs = lhs->simplify (stb);
            rhs = rhs->simplify (stb);

            if (auto literal1 = dynamic_cast<LiteralValue*> (lhs))
            {
                if (auto literal2 = dynamic_cast<LiteralValue*> (rhs))
                {
                    auto resultType = getResultType (getTypeOfVar (literal1->value),
                                                     getTypeOfVar (literal2->value));

                    if (resultType == Type::bool_)  return simplifyBool  (literal1->value, literal2->value, literal1);
                    if (resultType == Type::int_)   return simplifyInt   (literal1->value, literal2->value, literal1);
                    if (resultType == Type::float_) return simplifyFloat (literal1->value, literal2->value, literal1);
                }
            }

            return this;
        }
    };

    struct Assignment  : public Expression
    {
        Assignment (const CodeLocation& l, BlockPtr parent, ExpPtr dest, ExpPtr source, bool isPost) noexcept
            : Expression (l, parent), target (dest), newValue (source), isPostAssignment (isPost) {}

        void emit (CodeGenerator& cg, Type requiredType, int stackDepth) const override
        {
            auto variableType = getType (cg);

            if (isPostAssignment && requiredType != Type::void_)
            {
                target->emit (cg, requiredType, stackDepth);
                ++stackDepth;
                requiredType = Type::void_;
            }

            newValue->emit (cg, variableType, stackDepth);

            if (auto a = dynamic_cast<ArraySubscript*> (target))
            {
                cg.emitArrayElementIndex (a, parentBlock, ++stackDepth, location);
                cg.emit (OpCode::setHeapInt);
            }
            else
            {
                auto index = parentBlock->getVariableDepth (target->getIdentifier(), location);

                if (requiredType != Type::void_)
                {
                    cg.emit (OpCode::dup);
                    ++stackDepth;
                }

                if (index < 0)
                {
                    cg.emit (OpCode::dropToGlobal, (int16) ((-index) - 1));
                }
                else
                {
                    index += stackDepth;

                    if (index >= 128)
                        cg.emit (OpCode::dropToStack16, (int16) index);
                    else
                        cg.emit (OpCode::dropToStack, (int8) index);
                }

                if (requiredType != Type::void_)
                    cg.emitCast (variableType, requiredType, location);
            }
        }

        Type getType (CodeGenerator&) const override
        {
            return parentBlock->getVariable (target->getIdentifier(), location).type;
        }

        void visitSubStatements (Statement::Visitor& visit) const override
        {
            visit (newValue);
        }

        ExpPtr simplify (SyntaxTreeBuilder& stb) override
        {
            newValue = newValue->simplify (stb);
            return this;
        }

        ExpPtr target, newValue;
        bool isPostAssignment;
    };

    struct FunctionCall  : public Expression
    {
        FunctionCall (const CodeLocation& l, BlockPtr parent) noexcept  : Expression (l, parent) {}

        void emit (CodeGenerator& cg, Type requiredType, int stackDepth) const override
        {
            if (functionName == Token::int_)     return emitCast (cg, Type::int_,   stackDepth);
            if (functionName == Token::float_)   return emitCast (cg, Type::float_, stackDepth);
            if (functionName == Token::bool_)    return emitCast (cg, Type::bool_,  stackDepth);

            auto functionID = getFunctionID (cg);

            if (auto fn = cg.syntaxTree.findFunction (functionID))
            {
                emitArgs (cg, fn->getArgumentTypes(), stackDepth);
                cg.emit (OpCode::call, fn->address);
                cg.emitCast (fn->returnType, requiredType, location);
                return;
            }

            if (auto nativeFn = cg.syntaxTree.findNativeFunction (functionID))
            {
                emitArgs (cg, getArgTypesFromFunctionName (nativeFn->nameAndArguments), stackDepth);
                cg.emit (OpCode::callNative, nativeFn->functionID);
                cg.emitCast (nativeFn->returnType, requiredType, location);
                return;
            }

            if (auto b = findBuiltInFunction (functionID))
            {
                emitArgs (cg, getArgTypesFromFunctionName (b->name), stackDepth);
                cg.emit (b->op);
                cg.emitCast (b->returnType, requiredType, location);
                return;
            }

            throwCannotFindFunctionError (cg, requiredType);
        }

        Type getType (CodeGenerator& cg) const override
        {
            if (arguments.size() == 1)
            {
                if (functionName == Token::float_)  return Type::float_;
                if (functionName == Token::int_)    return Type::int_;
                if (functionName == Token::bool_)   return Type::bool_;
            }

            auto functionID = getFunctionID (cg);

            if (auto fn = cg.syntaxTree.findFunction (functionID))
                return fn->returnType;

            if (auto nativeFn = cg.syntaxTree.findNativeFunction (functionID))
                return nativeFn->returnType;

            if (auto b = findBuiltInFunction (functionID))
                return b->returnType;

            throwCannotFindFunctionError (cg, Type::void_);
            return {};
        }

        struct BuiltInFunction { OpCode op; Type returnType; const char* name; };

        const BuiltInFunction* findBuiltInFunction (FunctionID functionID) const noexcept
        {
            static constexpr const BuiltInFunction builtIns[] =
            {
                { OpCode::getHeapByte, Type::int_,  "getHeapByte/ii"  },
                { OpCode::getHeapInt,  Type::int_,  "getHeapInt/ii"   },
                { OpCode::getHeapBits, Type::int_,  "getHeapBits/iii" },
                { OpCode::setHeapByte, Type::void_, "setHeapByte/vii" },
                { OpCode::setHeapInt,  Type::void_, "setHeapInt/vii"  }
            };

            for (auto& b : builtIns)
                if (functionID == NativeFunction::createID (b.name))
                    return &b;

            return nullptr;
        }

        void emitArgs (CodeGenerator& cg, const Array<Type>& argTypes, int stackDepth) const
        {
            for (int i = arguments.size(); --i >= 0;)
            {
                auto argType = argTypes[i];
                auto argValue = arguments.getUnchecked(i);

                if (argValue->getType (cg) != argType)
                    location.throwError ("Argument " + String (i + 1) + " requires an expression of type " + getTypeName (argType));

                argValue->emit (cg, argType, stackDepth++);
            }
        }

        void emitCast (CodeGenerator& cg, Type destType, int stackDepth) const
        {
            if (arguments.size() != 1)
                location.throwError (getTypeName (destType) + " cast operation requires a single argument");

            auto* arg = arguments.getReference (0);
            const auto sourceType = arg->getType (cg);
            arg->emit (cg, sourceType, stackDepth);

            const bool sourceIsFloat = (sourceType == Type::float_);
            const bool destIsFloat   = (destType   == Type::float_);

            if (sourceIsFloat != destIsFloat)
                cg.emit (destIsFloat ? OpCode::int32ToFloat : OpCode::floatToInt32);
        }

        FunctionID getFunctionID (CodeGenerator& cg) const
        {
            Array<Type> argTypes;

            for (auto arg : arguments)
                argTypes.add (arg->getType (cg));

            return createFunctionID (functionName, Type::void_, argTypes); // NB: the ID ignores the return type so void is OK
        }

        void throwCannotFindFunctionError (CodeGenerator& cg, Type returnType) const
        {
            StringArray args;
            for (auto arg : arguments)
                args.add (getTypeName (arg->getType (cg)));

            auto desc = getTypeName (returnType) + " " + functionName
                            + "(" + args.joinIntoString (", ") + ")";

            location.throwError ("Cannot find matching function: " + desc.quoted());
        }

        void visitSubStatements (Statement::Visitor& visit) const override
        {
            for (auto& arg : arguments)
                visit (arg);
        }

        ExpPtr simplify (SyntaxTreeBuilder& stb) override
        {
            for (auto& arg : arguments)
                arg = arg->simplify (stb);

            return this;
        }

        String functionName;
        Array<ExpPtr> arguments;
    };

    struct ArraySubscript  : public Expression
    {
        ArraySubscript (const CodeLocation& l, BlockPtr parent) noexcept  : Expression (l, parent) {}

        void emit (CodeGenerator& cg, Type /*requiredType*/, int stackDepth) const override
        {
            cg.emitArrayElementIndex (this, parentBlock, stackDepth, location);
            cg.emit (OpCode::getHeapInt);
        }

        void visitSubStatements (Statement::Visitor& visit) const override
        {
            visit (object); visit (index);
        }

        ExpPtr simplify (SyntaxTreeBuilder& stb) override
        {
            object = object->simplify (stb);
            index = index->simplify (stb);
            return this;
        }

        Type getType (CodeGenerator& cg) const override
        {
            return object->getType (cg);
        }

        String getIdentifier() const override
        {
            if (auto i = dynamic_cast<Identifier*> (object))
                return i->name;

            if (auto s = dynamic_cast<ArraySubscript*> (object))
                return s->getIdentifier();

            location.throwError ("This operator requires an assignable variable");
            return {};
        }

        ExpPtr object, index;
    };

    //==============================================================================
    static Array<Type> getArgTypesFromFunctionName (const char* nameAndTypes)
    {
        Array<Type> types;
        auto args = String (nameAndTypes).fromFirstOccurrenceOf ("/", false, false).substring (1);

        for (int i = 0; i < args.length(); ++i)
            types.add (static_cast<Type> (args[i]));

        return types;
    }

    static FunctionID createFunctionID (String name, Type returnType, const Array<Type>& types)
    {
        name << "/" << (char) returnType;

        for (auto t : types)
            name << (char) t;

        return NativeFunction::createID (name.toRawUTF8());
    }

    static String getTokenDescription (TokenType t)    { return t[0] == '$' ? String (t + 1) : ("'" + String (t) + "'"); }

    static String getTypeName (Type t) noexcept
    {
        if (t == Type::int_)    return "int";
        if (t == Type::bool_)   return "bool";
        if (t == Type::float_)  return "float";
        return "void";
    }

    static Type tokenToType (TokenType t) noexcept
    {
        if (t == Token::int_)    return Type::int_;
        if (t == Token::bool_)   return Type::bool_;
        if (t == Token::float_)  return Type::float_;
        return Type::void_;
    }

    static Type getTypeOfVar (const var& v) noexcept
    {
        if (v.isInt() || v.isInt64())   return Type::int_;
        if (v.isDouble())               return Type::float_;
        if (v.isBool())                 return Type::bool_;
        return Type::void_;
    }

   #endif // ! DOXYGEN
};

}

JUCE_END_IGNORE_WARNINGS_MSVC
