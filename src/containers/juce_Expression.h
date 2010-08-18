/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#ifndef __JUCE_EXPRESSION_JUCEHEADER__
#define __JUCE_EXPRESSION_JUCEHEADER__

#include "juce_ReferenceCountedObject.h"
#include "juce_Array.h"
#include "juce_ScopedPointer.h"


//==============================================================================
/**
    A class for dynamically evaluating simple numeric expressions.

    This class can parse a simple C-style string expression involving floating point
    numbers, named symbols and functions. The basic arithmetic operations of +, -, *, /
    are supported, as well as parentheses, and any alphanumeric identifiers are
    assumed to be named symbols which will be resolved when the expression is
    evaluated.

    Expressions which use identifiers and functions require a subclass of
    Expression::EvaluationContext to be supplied when evaluating them, and this object
    is expected to be able to resolve the symbol names and perform the functions that
    are used.
*/
class JUCE_API  Expression
{
public:
    //==============================================================================
    /** Creates a simple expression with a value of 0. */
    Expression();

    /** Destructor. */
    ~Expression();

    /** Creates a simple expression with a specified constant value. */
    explicit Expression (const double constant);

    /** Creates a copy of an expression. */
    Expression (const Expression& other);

    /** Copies another expression. */
    Expression& operator= (const Expression& other);

    /** Creates an expression by parsing a string.
        If there's a syntax error in the string, this will throw a ParseError exception.
        @throws ParseError
    */
    explicit Expression (const String& stringToParse);

    /** Returns a string version of the expression. */
    const String toString() const;

    /** Returns an expression which is an addtion operation of two existing expressions. */
    const Expression operator+ (const Expression& other) const;
    /** Returns an expression which is a subtraction operation of two existing expressions. */
    const Expression operator- (const Expression& other) const;
    /** Returns an expression which is a multiplication operation of two existing expressions. */
    const Expression operator* (const Expression& other) const;
    /** Returns an expression which is a division operation of two existing expressions. */
    const Expression operator/ (const Expression& other) const;
    /** Returns an expression which is a negation operation of two existing expressions. */
    const Expression operator-() const;

    /** Returns an Expression which is an identifier reference. */
    static const Expression symbol (const String& symbol);

    /** Returns an Expression which is a function call. */
    static const Expression function (const String& functionName, const Array<Expression>& parameters);

    /** Returns an Expression which parses a string from a specified character index.

        The index value is incremented so that on return, it indicates the character that follows
        the end of the expression that was parsed.

        If there's a syntax error in the string, this will throw a ParseError exception.
        @throws ParseError
    */
    static const Expression parse (const String& stringToParse, int& textIndexToStartFrom);

    //==============================================================================
    /** When evaluating an Expression object, this class is used to resolve symbols and
        perform functions that the expression uses.
    */
    class EvaluationContext
    {
    public:
        EvaluationContext();
        virtual ~EvaluationContext();

        /** Returns the value of a symbol.
            If the symbol is unknown, this can throw an Expression::EvaluationError exception.
            @throws Expression::EvaluationError
        */
        virtual const Expression getSymbolValue (const String& symbol) const;

        /** Executes a named function.
            If the function name is unknown, this can throw an Expression::EvaluationError exception.
            @throws Expression::EvaluationError
        */
        virtual double evaluateFunction (const String& functionName, const double* parameters, int numParams) const;
    };

    /** Evaluates this expression, without using an EvaluationContext.
        Without an EvaluationContext, no symbols can be used, and only basic functions such as sin, cos, tan,
        min, max are available.
    */
    double evaluate() const;

    /** Evaluates this expression, providing a context that should be able to evaluate any symbols
        or functions that it uses.
    */
    double evaluate (const EvaluationContext& context) const;

    /** Attempts to return an expression which is a copy of this one, but with a constant adjusted
        to make the expression resolve to a target value.

        E.g. if the expression is "x + 10" and x is 5, then asking for a target value of 8 will return
        the expression "x + 3". Obviously some expressions can't be reversed in this way, in which
        case they might just be adjusted by adding a constant to them.
    */
    const Expression adjustedToGiveNewResult (double targetValue, const EvaluationContext& context) const;

    /** Returns a copy of this expression in which all instances of a given symbol have been renamed. */
    const Expression withRenamedSymbol (const String& oldSymbol, const String& newSymbol) const;

    /** Returns true if this expression makes use of the specified symbol.
        If a suitable context is supplied, the search will dereference and recursively check
        all symbols, so that it can be determined whether this expression relies on the given
        symbol at any level in its evaluation.
    */
    bool referencesSymbol (const String& symbol, const EvaluationContext& context) const;

    /** Returns true if this expression contains any symbols. */
    bool usesAnySymbols() const;

    //==============================================================================
    /** An exception that can be thrown by Expression::parse(). */
    class ParseError  : public std::exception
    {
    public:
        ParseError (const String& message);

        String description;
    };

    //==============================================================================
    /** An exception that can be thrown by Expression::evaluate(). */
    class EvaluationError  : public std::exception
    {
    public:
        EvaluationError (const String& message);

        String description;
    };

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    class Helpers;
    friend class Helpers;

    class Term  : public ReferenceCountedObject
    {
    public:
        Term() {}
        virtual ~Term() {}

        virtual Term* clone() const = 0;
        virtual double evaluate (const EvaluationContext&, int recursionDepth) const = 0;
        virtual int getNumInputs() const = 0;
        virtual Term* getInput (int index) const = 0;
        virtual int getInputIndexFor (const Term* possibleInput) const;
        virtual const String toString() const = 0;
        virtual int getOperatorPrecedence() const;
        virtual bool referencesSymbol (const String& symbol, const EvaluationContext&, int recursionDepth) const;
        virtual const ReferenceCountedObjectPtr<Term> createTermToEvaluateInput (const EvaluationContext&, Term* inputTerm,
                                                                                 double overallTarget, Term* topLevelTerm) const;
        juce_UseDebuggingNewOperator

    private:
        Term (const Term& other);
        Term& operator= (const Term&);
    };

    friend class ScopedPointer<Term>;
    ReferenceCountedObjectPtr<Term> term;

    explicit Expression (Term* term);
};

#endif   // __JUCE_EXPRESSION_JUCEHEADER__
