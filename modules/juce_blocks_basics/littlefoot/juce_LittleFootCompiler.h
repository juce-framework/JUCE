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

using namespace juce;

/**
    This class compiles littlefoot source code into a littlefoot::Program object
    which can be executed by a littlefoot::Runner.
*/
struct Compiler
{
    Compiler() {}

    /**
    */
    void addNativeFunctions (const char* const* functionPrototypes)
    {
        for (; *functionPrototypes != nullptr; ++functionPrototypes)
            nativeFunctions.add (NativeFunction (*functionPrototypes, nullptr));
    }

    /**
    */
    template <typename RunnerType>
    void addNativeFunctions (const RunnerType& runner)
    {
        for (int i = 0; i < runner.getNumNativeFunctions(); ++i)
            nativeFunctions.add (runner.getNativeFunction (i));
    }

    /**
    */
    Result compile (const String& sourceCode, uint32 heapSizeBytesRequired)
    {
        try
        {
            SyntaxTreeBuilder stb (sourceCode);
            stb.compile();
            stb.simplify();

            compiledObjectCode.clear();

            CodeGenerator codeGen (compiledObjectCode, nativeFunctions, stb.functions);
            codeGen.generateCode (stb.blockBeingParsed, (heapSizeBytesRequired + 3) & ~3u);
            return Result::ok();
        }
        catch (String error)
        {
            return Result::fail (error);
        }
    }

    /**
    */
    Array<uint8> compiledObjectCode;

private:
    struct Statement;
    struct Expression;
    struct BlockStatement;
    struct Function;
    struct AllocatedObject  { virtual ~AllocatedObject() noexcept {} };
    using StatementPtr = Statement*;
    using ExpPtr = Expression*;
    using BlockPtr = BlockStatement*;
    using TokenType = const char*;

    #define LITTLEFOOT_KEYWORDS(X) \
        X(if_,      "if")       X(else_,   "else")    X(do_,     "do") \
        X(while_,   "while")    X(for_,    "for")     X(break_,  "break")   X(continue_, "continue") \
        X(void_,    "void")     X(int_,    "int")     X(float_,  "float")   X(bool_,     "bool") \
        X(return_,  "return")   X(true_,   "true")    X(false_,  "false")

    #define LITTLEFOOT_OPERATORS(X) \
        X(semicolon,     ";")        X(dot,          ".")       X(comma,        ",") \
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
        CodeLocation (const String& code) noexcept        : program (code), location (program.getCharPointer()) {}
        CodeLocation (const CodeLocation& other) noexcept : program (other.program), location (other.location) {}

        void throwError (const String& message) const
        {
            int col = 1, line = 1;

            for (auto i = program.getCharPointer(); i < location && ! i.isEmpty(); ++i)
            {
                ++col;
                if (*i == '\n')  { col = 1; ++line; }
            }

            throw "Line " + String (line) + ", column " + String (col) + " : " + message;
        }

        String program;
        String::CharPointerType location;
    };

    //==============================================================================
    struct TokenIterator
    {
        TokenIterator (const String& code) : location (code), p (code.getCharPointer()) { skip(); }

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

        CodeLocation location;
        TokenType currentType;
        var currentValue;

        void throwErrorExpecting (const String& expected)    { location.throwError ("Found " + getTokenDescription (currentType) + " when expecting " + expected); }

    private:
        String::CharPointerType p;

        static bool isIdentifierStart (const juce_wchar c) noexcept   { return CharacterFunctions::isLetter (c)        || c == '_'; }
        static bool isIdentifierBody  (const juce_wchar c) noexcept   { return CharacterFunctions::isLetterOrDigit (c) || c == '_'; }

        TokenType matchNextToken()
        {
            if (isIdentifierStart (*p))
            {
                String::CharPointerType end (p);
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
                    const juce_wchar c2 = p[1];

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
            String::CharPointerType t (p);
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
        SyntaxTreeBuilder (const String& code)  : TokenIterator (code) {}

        void compile()
        {
            blockBeingParsed = allocate<BlockStatement> (location, nullptr, nullptr, false);

            while (currentType != Token::eof)
            {
                if (! matchesAnyTypeOrVoid())
                    throwErrorExpecting ("a global variable or function");

                auto type = tokenToType (skip());
                auto name = parseIdentifier();

                if (matchIf (Token::openParen))
                {
                    parseFunctionDeclaration (type, name);
                    continue;
                }

                if (type == Type::void_)
                    location.throwError ("A variable type cannot be 'void'");

                int arraySize = matchIf (Token::openBracket) ? parseArraySize() : 0;

                if (arraySize > 0)
                    location.throwError ("Arrays not yet implemented!");

                while (matchIf (Token::comma))
                {
                    blockBeingParsed->addVariable (name, type, location);
                    name = parseIdentifier();
                }

                blockBeingParsed->addVariable (name, type, location);
                match (Token::semicolon);
            }
        }

        void simplify()
        {
            for (auto f : functions)
                f->block->simplify (*this);
        }

        //==============================================================================
        BlockPtr blockBeingParsed = nullptr;
        Array<Function*> functions;

        template <typename Type, typename... Args>
        Type* allocate (Args... args)   { auto o = new Type (args...); allAllocatedObjects.add (o); return o; }

    private:
        OwnedArray<AllocatedObject> allAllocatedObjects;

        //==============================================================================
        void parseFunctionDeclaration (Type returnType, const String& name)
        {
            auto f = allocate<Function>();
            functions.add (f);

            while (matchesAnyType())
            {
                auto type = tokenToType (skip());
                f->arguments.add ({ parseIdentifier(), type });

                if (f->arguments.size() > 127)
                    location.throwError ("Too many function arguments");

                if (currentType == Token::closeParen)
                    break;

                match (Token::comma);
            }

            match (Token::closeParen);
            f->functionID = createFunctionID (name, returnType, f->getArgumentTypes());
            f->block = parseBlock (true);
            f->returnType = returnType;

            if (! f->block->alwaysReturns())
            {
                if (returnType != Type::void_)
                    location.throwError ("This function must return a value");

                f->block->statements.add (allocate<ReturnStatement> (location, f->block, nullptr));
            }
        }

        int parseArraySize()
        {
            auto e = parseExpression();
            e->simplify (*this);

            if (auto literal = dynamic_cast<LiteralValue*> (e))
            {
                if (literal->value.isInt() || literal->value.isInt64())
                {
                    auto value = static_cast<int> (literal->value);

                    if (value > 0)
                        return value;
                }
            }

            location.throwError ("An array size must be a constant integer");
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
            if (matchesAnyType())                  return parseVariableDeclaration (tokenToType (skip()));

            if (matchesAny (Token::identifier, Token::literal, Token::minus))
                return matchEndOfStatement (parseExpression());

            throwErrorExpecting ("a statement");
            return nullptr;
        }

        ExpPtr parseExpression()
        {
            auto lhs = parseLogicOperator();

            if (matchIf (Token::question))          return parseTerneryOperator (lhs);
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
                return allocate<Assignment> (loc, blockBeingParsed, getIdentifierFromExpression (lhs), parseExpression(), false);
            }

            return lhs;
        }

        ExpPtr parseTerneryOperator (ExpPtr condition)
        {
            auto e = allocate<TerneryOp> (location, blockBeingParsed);
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
            if (matchIf (Token::true_))            return parseSuffixes (allocate<LiteralValue> (location, blockBeingParsed, (int) 1));
            if (matchIf (Token::false_))           return parseSuffixes (allocate<LiteralValue> (location, blockBeingParsed, (int) 0));

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

            if (matchIf (Token::openBracket))
            {
                auto s = allocate<ArraySubscript> (location, blockBeingParsed);
                s->object = input;
                s->index = parseExpression();
                match (Token::closeBracket);
                return parseSuffixes (s);
            }

            if (matchIf (Token::plusplus))   return parsePostIncDec (input, Token::plus);
            if (matchIf (Token::minusminus)) return parsePostIncDec (input, Token::minus);

            return input;
        }

        ExpPtr parseInPlaceOpExpression (ExpPtr lhs, TokenType opType)
        {
            auto loc = location;
            auto rhs = parseExpression();
            return allocate<Assignment> (loc, blockBeingParsed, getIdentifierFromExpression (lhs),
                                         allocate<BinaryOperator> (location, blockBeingParsed, lhs, rhs, opType), false);
        }

        ExpPtr parsePreIncDec (TokenType opType)
        {
            auto lhs = parseFactor();
            auto one = allocate<LiteralValue> (location, blockBeingParsed, (int) 1);
            return allocate<Assignment> (location, blockBeingParsed, getIdentifierFromExpression (lhs),
                                         allocate<BinaryOperator> (location, blockBeingParsed, lhs, one, opType), false);
        }

        ExpPtr parsePostIncDec (ExpPtr lhs, TokenType opType)
        {
            auto one = allocate<LiteralValue> (location, blockBeingParsed, (int) 1);
            return allocate<Assignment> (location, blockBeingParsed, getIdentifierFromExpression (lhs),
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

        StatementPtr parseVariableDeclaration (Type type)
        {
            for (StatementPtr result = nullptr;;)
            {
                auto name = parseIdentifier();
                auto loc = location;
                blockBeingParsed->addVariable (name, type, loc);

                auto assignedValue = matchIf (Token::assign) ? parseExpression() : nullptr;

                if (auto literal = dynamic_cast<LiteralValue*> (assignedValue))
                    if (static_cast<double> (literal->value) == 0)
                        assignedValue = nullptr;

                if (assignedValue != nullptr || ! blockBeingParsed->isMainBlockOfFunction) // no need to assign 0 for variables in the outer scope
                {
                    if (assignedValue == nullptr)
                        assignedValue = allocate<LiteralValue> (loc, blockBeingParsed, (int) 0);

                    auto assignment = allocate<Assignment> (loc, blockBeingParsed, name, assignedValue, false);

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
            String name = currentValue.toString();
            match (Token::identifier);
            return name;
        }

        String getIdentifierFromExpression (ExpPtr e)
        {
            if (auto i = dynamic_cast<Identifier*> (e))
                return i->name;

            location.throwError ("This operator requires an assignable variable");
            return {};
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
        CodeGenerator (Array<uint8>& output, const Array<NativeFunction>& nativeFns, const Array<Function*>& fns)
            : outputCode (output), nativeFunctions (nativeFns), functions (fns) {}

        void generateCode (BlockPtr outerBlock, uint32 heapSizeBytesRequired)
        {
            for (auto f : functions)
            {
                f->address = createMarker();
                f->unwindAddress = createMarker();
            }

            emit ((int16) 0); // checksum
            emit ((int16) 0); // size
            emit ((int16) functions.size());
            emit ((int16) outerBlock->variables.size());
            emit ((int16) heapSizeBytesRequired);

            for (auto f : functions)
                emit (f->functionID, f->address);

            for (auto f : functions)
                f->emit (*this);

            resolveMarkers();

            Program::writeInt16 (outputCode.begin() + 2, (int16) outputCode.size());
            const Program program (outputCode.begin(), (uint32) outputCode.size());
            Program::writeInt16 (outputCode.begin(), (int16) program.calculateChecksum());
            jassert (program.checksumMatches());
        }

        //==============================================================================
        Array<uint8>& outputCode;
        const Array<NativeFunction>& nativeFunctions;
        const Array<Function*>& functions;

        struct Marker  { int index = 0; };
        struct MarkerAndAddress  { int markerIndex, address; };

        int nextMarker = 0;
        Array<MarkerAndAddress> markersToResolve, resolvedMarkers;

        Marker createMarker() noexcept  { Marker m; m.index = ++nextMarker; return m; }
        void attachMarker (Marker m)    { resolvedMarkers.add ({ m.index, outputCode.size() }); }

        int getResolvedMarkerAddress (int markerIndex) const
        {
            for (auto m : resolvedMarkers)
                if (m.markerIndex == markerIndex)
                    return m.address;

            jassertfalse;
            return 0;
        }

        void resolveMarkers()
        {
            for (auto m : markersToResolve)
                Program::writeInt16 (outputCode.begin() + m.address, (int16) getResolvedMarkerAddress (m.markerIndex));
        }

        Marker breakTarget, continueTarget;

        //==============================================================================
        void emit (OpCode op)           { emit ((int8) op); }
        void emit (Marker m)            { markersToResolve.add ({ m.index, outputCode.size() }); emit ((int16) 0); }
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
                    emit ((OpCode) ((int) OpCode::dup));
                else if (index < 8)
                    emit ((OpCode) ((int) OpCode::dupOffset_01 + index - 1));
                else if (index >= 128)
                    emit (OpCode::dupOffset16, (int16) index);
                else
                    emit (OpCode::dupOffset, (int8) index);
            }

            emitCast (sourceType, requiredType, location);
        }

        //==============================================================================
        Function* findFunction (FunctionID functionID) const noexcept
        {
            for (auto f : functions)
                if (f->functionID == functionID)
                    return f;

            return nullptr;
        }

        NativeFunction* findNativeFunction (FunctionID functionID) const noexcept
        {
            for (auto& f : nativeFunctions)
                if (f.functionID == functionID)
                    return &f;

            return nullptr;
        }
    };

    //==============================================================================
    //==============================================================================
    struct Statement  : public AllocatedObject
    {
        Statement (const CodeLocation& l, BlockPtr parent) noexcept : location (l), parentBlock (parent) {}
        virtual void emit (CodeGenerator&, Type, int /*stackDepth*/) const {}
        virtual bool alwaysReturns() const                  { return false; }
        virtual void visitSubStatements (std::function<void(StatementPtr)>) const {}
        virtual Statement* simplify (SyntaxTreeBuilder&)    { return this; }

        CodeLocation location;
        BlockPtr parentBlock;
    };

    struct Expression  : public Statement
    {
        Expression (const CodeLocation& l, BlockPtr parent) noexcept : Statement (l, parent) {}
        virtual Type getType (CodeGenerator&) const = 0;
    };

    struct Variable
    {
        String name;
        Type type;
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
            int num = 0;

            if (s != nullptr)
                s->visitSubStatements ([&] (StatementPtr sub) { num = jmax (num, countMaxNumLocalVariables (sub)); });

            if (auto block = dynamic_cast<BlockPtr> (s))
                num += block->variables.size();

            return num;
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

        void visitSubStatements (std::function<void(StatementPtr)> visit) const override
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
            int index = indexOf (variables, name);
            if (index >= 0)
                return getNumVariablesInParentBlocks() + index;

            if (! isMainBlockOfFunction)
                return parentBlock->getVariableDepth (name, locationForError);

            for (int i = function->arguments.size(); --i >= 0;)
                if (function->arguments.getReference(i).name == name)
                    return i + 1 + function->getNumLocals();

            index = indexOf (getGlobalVariables(), name);
            if (index >= 0)
                return -(index + 1);

            locationForError.throwError ("Unknown variable '" + name + "'");
            return 0;
        }

        int getNumVariablesInParentBlocks() const noexcept
        {
            return isMainBlockOfFunction ? 0 : (parentBlock->getNumVariablesInParentBlocks()
                                                 + parentBlock->variables.size());
        }

        const Array<Variable>& getGlobalVariables() const noexcept
        {
            return parentBlock != nullptr ? parentBlock->getGlobalVariables() : variables;
        }

        Type getVariableType (const String& name, const CodeLocation& locationForError) const
        {
            for (auto& v : variables)
                if (v.name == name)
                    return v.type;

            if (! isMainBlockOfFunction)
                return parentBlock->getVariableType (name, locationForError);

            for (auto& v : function->arguments)
                if (v.name == name)
                    return v.type;

            for (auto& v : getGlobalVariables())
                if (v.name == name)
                    return v.type;

            locationForError.throwError ("Unknown variable '" + name + "'");
            return {};
        }

        void addVariable (const String& name, Type type, const CodeLocation& locationForError)
        {
            if (indexOf (variables, name) >= 0)
                locationForError.throwError ("Variable '" + name + "' already exists");

            variables.add ({ name, type });
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
        Array<Variable> variables;
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

        void visitSubStatements (std::function<void(StatementPtr)> visit) const override
        {
            visit (condition); visit (trueBranch); visit (falseBranch);
        }

        Statement* simplify (SyntaxTreeBuilder& stb) override
        {
            condition   = dynamic_cast<ExpPtr> (condition->simplify (stb));
            trueBranch  = trueBranch->simplify (stb);
            falseBranch = falseBranch != nullptr ? falseBranch->simplify (stb) : nullptr;

            if (auto literal = dynamic_cast<LiteralValue*> (condition))
                return literal->value ? trueBranch : (falseBranch != nullptr ? falseBranch : stb.allocate<Statement> (location, parentBlock));

            return this;
        }

        ExpPtr condition;
        StatementPtr trueBranch, falseBranch;
    };

    struct TerneryOp  : public Expression
    {
        TerneryOp (const CodeLocation& l, BlockPtr parent)  : Expression (l, parent) {}

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

            if (type == Type::void_)                location.throwError ("The ternery operator cannot take void arguments");
            if (type != falseBranch->getType (cg))  location.throwError ("Expected both branches of this ternery operator to have the same type");

            return type;
        }

        void visitSubStatements (std::function<void(StatementPtr)> visit) const override
        {
            visit (condition); visit (trueBranch); visit (falseBranch);
        }

        Statement* simplify (SyntaxTreeBuilder& stb) override
        {
            condition   = dynamic_cast<ExpPtr> (condition->simplify (stb));
            trueBranch  = dynamic_cast<ExpPtr> (trueBranch->simplify (stb));
            falseBranch = dynamic_cast<ExpPtr> (falseBranch->simplify (stb));

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

        void visitSubStatements (std::function<void(StatementPtr)> visit) const override
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

                if (parentBlock->statements.getLast() != this)
                    cg.emit (OpCode::jump, fn->unwindAddress);

                return;
            }

            location.throwError ("The return statement can only be used inside a function");
        }

        bool alwaysReturns() const override     { return true; }

        void visitSubStatements (std::function<void(StatementPtr)> visit) const override
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
            return parentBlock->getVariableType (name, location);
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

        void visitSubStatements (std::function<void(StatementPtr)> visit) const override
        {
            visit (source);
        }

        Statement* simplify (SyntaxTreeBuilder& stb) override
        {
            source = dynamic_cast<ExpPtr> (source->simplify (stb));

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

        void visitSubStatements (std::function<void(StatementPtr)> visit) const override
        {
            visit (lhs); visit (rhs);
        }

        Statement* simplifyFloat (double a, double b, LiteralValue* literal)
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

        Statement* simplifyBool (bool a, bool b, LiteralValue* literal)
        {
            if (operation == Token::logicalOr)            { literal->value = a || b; return literal; }
            if (operation == Token::logicalAnd)           { literal->value = a && b; return literal; }
            return this;
        }

        Statement* simplifyInt (int a, int b, LiteralValue* literal)
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

        Statement* simplify (SyntaxTreeBuilder& stb) override
        {
            lhs = dynamic_cast<ExpPtr> (lhs->simplify (stb));
            rhs = dynamic_cast<ExpPtr> (rhs->simplify (stb));

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
        Assignment (const CodeLocation& l, BlockPtr parent, const String& dest, ExpPtr source, bool isPost) noexcept
            : Expression (l, parent), target (dest), newValue (source), isPostAssignment (isPost) {}

        void emit (CodeGenerator& cg, Type requiredType, int stackDepth) const override
        {
            auto variableType = getType (cg);

            if (isPostAssignment && requiredType != Type::void_)
            {
                cg.emitVariableRead (variableType, requiredType, stackDepth,
                                     parentBlock->getVariableDepth (target, location), location);
                ++stackDepth;
                requiredType = Type::void_;
            }

            newValue->emit (cg, variableType, stackDepth);
            auto index = parentBlock->getVariableDepth (target, location);

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

        Type getType (CodeGenerator&) const override
        {
            return parentBlock->getVariableType (target, location);
        }

        void visitSubStatements (std::function<void(StatementPtr)> visit) const override
        {
            visit (newValue);
        }

        Statement* simplify (SyntaxTreeBuilder& stb) override
        {
            newValue = dynamic_cast<ExpPtr> (newValue->simplify (stb));
            return this;
        }

        String target;
        ExpPtr newValue;
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

            if (auto fn = cg.findFunction (functionID))
            {
                emitArgs (cg, fn->getArgumentTypes(), stackDepth);
                cg.emit (OpCode::call, fn->address);
                cg.emitCast (fn->returnType, requiredType, location);
                return;
            }

            if (auto nativeFn = cg.findNativeFunction (functionID))
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

            if (auto fn = cg.findFunction (functionID))
                return fn->returnType;

            if (auto nativeFn = cg.findNativeFunction (functionID))
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

        void visitSubStatements (std::function<void(StatementPtr)> visit) const override
        {
            for (auto& arg : arguments)
                visit (arg);
        }

        String functionName;
        Array<ExpPtr> arguments;
    };

    struct ArraySubscript  : public Expression
    {
        ArraySubscript (const CodeLocation& l, BlockPtr parent) noexcept  : Expression (l, parent) {}

        void emit (CodeGenerator&, Type /*requiredType*/, int /*stackDepth*/) const override
        {
            location.throwError ("Arrays are not implemented yet!");
        }

        void visitSubStatements (std::function<void(StatementPtr)> visit) const override
        {
            visit (object); visit (index);
        }

        Statement* simplify (SyntaxTreeBuilder& stb) override
        {
            object = dynamic_cast<ExpPtr> (object->simplify (stb));
            index = dynamic_cast<ExpPtr> (index->simplify (stb));
            return this;
        }

        Type getType (CodeGenerator& cg) const override
        {
            return object->getType (cg);
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
};

}
