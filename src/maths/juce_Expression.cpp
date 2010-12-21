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

#include "../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_Expression.h"
#include "../containers/juce_ReferenceCountedArray.h"


//==============================================================================
class Expression::Helpers
{
public:
    typedef ReferenceCountedObjectPtr<Term> TermPtr;

    // This helper function is needed to work around VC6 scoping bugs
    static const TermPtr& getTermFor (const Expression& exp) throw()       { return exp.term; }

    friend class Expression::Term; // (also only needed as a VC6 workaround)

    //==============================================================================
    class Constant  : public Term
    {
    public:
        Constant (const double value_, bool isResolutionTarget_)
            : value (value_), isResolutionTarget (isResolutionTarget_) {}

        Type getType() const throw()                            { return constantType; }
        Term* clone() const                                     { return new Constant (value, isResolutionTarget); }
        double evaluate (const EvaluationContext&, int) const   { return value; }
        int getNumInputs() const                                { return 0; }
        Term* getInput (int) const                              { return 0; }

        const TermPtr negated()
        {
            return new Constant (-value, isResolutionTarget);
        }

        const String toString() const
        {
            if (isResolutionTarget)
                return "@" + String (value);

            return String (value);
        }

        double value;
        bool isResolutionTarget;
    };

    //==============================================================================
    class Symbol  : public Term
    {
    public:
        explicit Symbol (const String& symbol_)
            : mainSymbol (symbol_.upToFirstOccurrenceOf (".", false, false).trim()),
              member (symbol_.fromFirstOccurrenceOf (".", false, false).trim())
        {}

        Symbol (const String& symbol_, const String& member_)
            : mainSymbol (symbol_),
              member (member_)
        {}

        double evaluate (const EvaluationContext& c, int recursionDepth) const
        {
            if (++recursionDepth > 256)
                throw EvaluationError ("Recursive symbol references");

            try
            {
                return getTermFor (c.getSymbolValue (mainSymbol, member))->evaluate (c, recursionDepth);
            }
            catch (...)
            {}

            return 0;
        }

        Type getType() const throw()                            { return symbolType; }
        Term* clone() const                                     { return new Symbol (mainSymbol, member); }
        int getNumInputs() const                                { return 0; }
        Term* getInput (int) const                              { return 0; }
        const String getSymbolName() const                      { return toString(); }

        const String toString() const
        {
            return member.isEmpty() ? mainSymbol
                                    : mainSymbol + "." + member;
        }

        bool referencesSymbol (const String& s, const EvaluationContext* c, int recursionDepth) const
        {
            if (s == mainSymbol)
                return true;

            if (++recursionDepth > 256)
                throw EvaluationError ("Recursive symbol references");

            try
            {
                return c != 0 && getTermFor (c->getSymbolValue (mainSymbol, member))->referencesSymbol (s, c, recursionDepth);
            }
            catch (EvaluationError&)
            {
                return false;
            }
        }

        String mainSymbol, member;
    };

    //==============================================================================
    class Function  : public Term
    {
    public:
        Function (const String& functionName_, const ReferenceCountedArray<Term>& parameters_)
            : functionName (functionName_), parameters (parameters_)
        {}

        Term* clone() const         { return new Function (functionName, parameters); }

        double evaluate (const EvaluationContext& c, int recursionDepth) const
        {
            HeapBlock <double> params (parameters.size());
            for (int i = 0; i < parameters.size(); ++i)
                params[i] = parameters.getUnchecked(i)->evaluate (c, recursionDepth);

            return c.evaluateFunction (functionName, params, parameters.size());
        }

        Type getType() const throw()                            { return functionType; }
        int getInputIndexFor (const Term* possibleInput) const  { return parameters.indexOf (possibleInput); }
        int getNumInputs() const                                { return parameters.size(); }
        Term* getInput (int i) const                            { return parameters [i]; }
        const String getFunctionName() const                    { return functionName; }

        bool referencesSymbol (const String& s, const EvaluationContext* c, int recursionDepth) const
        {
            for (int i = 0; i < parameters.size(); ++i)
                if (parameters.getUnchecked(i)->referencesSymbol (s, c, recursionDepth))
                    return true;

            return false;
        }

        const String toString() const
        {
            if (parameters.size() == 0)
                return functionName + "()";

            String s (functionName + " (");

            for (int i = 0; i < parameters.size(); ++i)
            {
                s << parameters.getUnchecked(i)->toString();

                if (i < parameters.size() - 1)
                    s << ", ";
            }

            s << ')';
            return s;
        }

        const String functionName;
        ReferenceCountedArray<Term> parameters;
    };

    //==============================================================================
    class Negate  : public Term
    {
    public:
        Negate (const TermPtr& input_) : input (input_)
        {
            jassert (input_ != 0);
        }

        Type getType() const throw()                            { return operatorType; }
        int getInputIndexFor (const Term* possibleInput) const  { return possibleInput == input ? 0 : -1; }
        int getNumInputs() const                                { return 1; }
        Term* getInput (int index) const                        { return index == 0 ? static_cast<Term*> (input) : 0; }
        Term* clone() const                                     { return new Negate (input->clone()); }
        double evaluate (const EvaluationContext& c, int recursionDepth) const    { return -input->evaluate (c, recursionDepth); }
        const String getFunctionName() const                    { return "-"; }

        const TermPtr negated()
        {
            return input;
        }

        const TermPtr createTermToEvaluateInput (const EvaluationContext& context, const Term* input_, double overallTarget, Term* topLevelTerm) const
        {
            (void) input_;
            jassert (input_ == input);

            const Term* const dest = findDestinationFor (topLevelTerm, this);

            return new Negate (dest == 0 ? new Constant (overallTarget, false)
                                         : dest->createTermToEvaluateInput (context, this, overallTarget, topLevelTerm));
        }

        const String toString() const
        {
            if (input->getOperatorPrecedence() > 0)
                return "-(" + input->toString() + ")";
            else
                return "-" + input->toString();
        }

        bool referencesSymbol (const String& s, const EvaluationContext* c, int recursionDepth) const
        {
            return input->referencesSymbol (s, c, recursionDepth);
        }

    private:
        const TermPtr input;
    };

    //==============================================================================
    class BinaryTerm  : public Term
    {
    public:
        BinaryTerm (Term* const left_, Term* const right_) : left (left_), right (right_)
        {
            jassert (left_ != 0 && right_ != 0);
        }

        int getInputIndexFor (const Term* possibleInput) const
        {
            return possibleInput == left ? 0 : (possibleInput == right ? 1 : -1);
        }

        Type getType() const throw()        { return operatorType; }

        int getNumInputs() const            { return 2; }
        Term* getInput (int index) const    { return index == 0 ? static_cast<Term*> (left) : (index == 1 ? static_cast<Term*> (right) : 0); }

        bool referencesSymbol (const String& s, const EvaluationContext* c, int recursionDepth) const
        {
            return left->referencesSymbol (s, c, recursionDepth)
                || right->referencesSymbol (s, c, recursionDepth);
        }

        const String toString() const
        {
            String s;

            const int ourPrecendence = getOperatorPrecedence();
            if (left->getOperatorPrecedence() > ourPrecendence)
                s << '(' << left->toString() << ')';
            else
                s = left->toString();

            s << ' ' << getFunctionName() << ' ';

            if (right->getOperatorPrecedence() >= ourPrecendence)
                s << '(' << right->toString() << ')';
            else
                s << right->toString();

            return s;
        }

    protected:
        const TermPtr left, right;

        const TermPtr createDestinationTerm (const EvaluationContext& context, const Term* input, double overallTarget, Term* topLevelTerm) const
        {
            jassert (input == left || input == right);
            if (input != left && input != right)
                return 0;

            const Term* const dest = findDestinationFor (topLevelTerm, this);

            if (dest == 0)
                return new Constant (overallTarget, false);

            return dest->createTermToEvaluateInput (context, this, overallTarget, topLevelTerm);
        }
    };

    //==============================================================================
    class Add  : public BinaryTerm
    {
    public:
        Add (Term* const left_, Term* const right_) : BinaryTerm (left_, right_) {}

        Term* clone() const                     { return new Add (left->clone(), right->clone()); }
        double evaluate (const EvaluationContext& c, int recursionDepth) const  { return left->evaluate (c, recursionDepth) + right->evaluate (c, recursionDepth); }
        int getOperatorPrecedence() const       { return 2; }
        const String getFunctionName() const    { return "+"; }

        const TermPtr createTermToEvaluateInput (const EvaluationContext& c, const Term* input, double overallTarget, Term* topLevelTerm) const
        {
            const TermPtr newDest (createDestinationTerm (c, input, overallTarget, topLevelTerm));
            if (newDest == 0)
                return 0;

            return new Subtract (newDest, (input == left ? right : left)->clone());
        }

    private:
        JUCE_DECLARE_NON_COPYABLE (Add);
    };

    //==============================================================================
    class Subtract  : public BinaryTerm
    {
    public:
        Subtract (Term* const left_, Term* const right_) : BinaryTerm (left_, right_) {}

        Term* clone() const                     { return new Subtract (left->clone(), right->clone()); }
        double evaluate (const EvaluationContext& c, int recursionDepth) const  { return left->evaluate (c, recursionDepth) - right->evaluate (c, recursionDepth); }
        int getOperatorPrecedence() const       { return 2; }
        const String getFunctionName() const    { return "-"; }

        const TermPtr createTermToEvaluateInput (const EvaluationContext& c, const Term* input, double overallTarget, Term* topLevelTerm) const
        {
            const TermPtr newDest (createDestinationTerm (c, input, overallTarget, topLevelTerm));
            if (newDest == 0)
                return 0;

            if (input == left)
                return new Add (newDest, right->clone());
            else
                return new Subtract (left->clone(), newDest);
        }

    private:
        JUCE_DECLARE_NON_COPYABLE (Subtract);
    };

    //==============================================================================
    class Multiply  : public BinaryTerm
    {
    public:
        Multiply (Term* const left_, Term* const right_) : BinaryTerm (left_, right_) {}

        Term* clone() const                     { return new Multiply (left->clone(), right->clone()); }
        double evaluate (const EvaluationContext& c, int recursionDepth) const  { return left->evaluate (c, recursionDepth) * right->evaluate (c, recursionDepth); }
        const String getFunctionName() const    { return "*"; }
        int getOperatorPrecedence() const       { return 1; }

        const TermPtr createTermToEvaluateInput (const EvaluationContext& c, const Term* input, double overallTarget, Term* topLevelTerm) const
        {
            const TermPtr newDest (createDestinationTerm (c, input, overallTarget, topLevelTerm));
            if (newDest == 0)
                return 0;

            return new Divide (newDest, (input == left ? right : left)->clone());
        }

    private:
        JUCE_DECLARE_NON_COPYABLE (Multiply);
    };

    //==============================================================================
    class Divide  : public BinaryTerm
    {
    public:
        Divide (Term* const left_, Term* const right_) : BinaryTerm (left_, right_) {}

        Term* clone() const                     { return new Divide (left->clone(), right->clone()); }
        double evaluate (const EvaluationContext& c, int recursionDepth) const        { return left->evaluate (c, recursionDepth) / right->evaluate (c, recursionDepth); }
        const String getFunctionName() const    { return "/"; }
        int getOperatorPrecedence() const       { return 1; }

        const TermPtr createTermToEvaluateInput (const EvaluationContext& c, const Term* input, double overallTarget, Term* topLevelTerm) const
        {
            const TermPtr newDest (createDestinationTerm (c, input, overallTarget, topLevelTerm));
            if (newDest == 0)
                return 0;

            if (input == left)
                return new Multiply (newDest, right->clone());
            else
                return new Divide (left->clone(), newDest);
        }

    private:
        JUCE_DECLARE_NON_COPYABLE (Divide);
    };

    //==============================================================================
    static Term* findDestinationFor (Term* const topLevel, const Term* const inputTerm)
    {
        const int inputIndex = topLevel->getInputIndexFor (inputTerm);
        if (inputIndex >= 0)
            return topLevel;

        for (int i = topLevel->getNumInputs(); --i >= 0;)
        {
            Term* t = findDestinationFor (topLevel->getInput (i), inputTerm);

            if (t != 0)
                return t;
        }

        return 0;
    }

    static Constant* findTermToAdjust (Term* const term, const bool mustBeFlagged)
    {
        Constant* c = dynamic_cast<Constant*> (term);
        if (c != 0 && (c->isResolutionTarget || ! mustBeFlagged))
            return c;

        if (dynamic_cast<Function*> (term) != 0)
            return 0;

        int i;
        const int numIns = term->getNumInputs();
        for (i = 0; i < numIns; ++i)
        {
            Constant* c = dynamic_cast<Constant*> (term->getInput (i));
            if (c != 0 && (c->isResolutionTarget || ! mustBeFlagged))
                return c;
        }

        for (i = 0; i < numIns; ++i)
        {
            Constant* c = findTermToAdjust (term->getInput (i), mustBeFlagged);
            if (c != 0)
                return c;
        }

        return 0;
    }

    static bool containsAnySymbols (const Term* const t)
    {
        if (dynamic_cast <const Symbol*> (t) != 0)
            return true;

        for (int i = t->getNumInputs(); --i >= 0;)
            if (containsAnySymbols (t->getInput (i)))
                return true;

        return false;
    }

    static bool renameSymbol (Term* const t, const String& oldName, const String& newName)
    {
        Symbol* const sym = dynamic_cast <Symbol*> (t);

        if (sym != 0 && sym->mainSymbol == oldName)
        {
            sym->mainSymbol = newName;
            return true;
        }

        bool anyChanged = false;

        for (int i = t->getNumInputs(); --i >= 0;)
            if (renameSymbol (t->getInput (i), oldName, newName))
                anyChanged = true;

        return anyChanged;
    }

    //==============================================================================
    class Parser
    {
    public:
        //==============================================================================
        Parser (const String& stringToParse, int& textIndex_)
            : textString (stringToParse), textIndex (textIndex_)
        {
            text = textString;
        }

        const TermPtr readExpression()
        {
            TermPtr lhs (readMultiplyOrDivideExpression());

            char opType;
            while (lhs != 0 && readOperator ("+-", &opType))
            {
                TermPtr rhs (readMultiplyOrDivideExpression());

                if (rhs == 0)
                    throw ParseError ("Expected expression after \"" + String::charToString (opType) + "\"");

                if (opType == '+')
                    lhs = new Add (lhs, rhs);
                else
                    lhs = new Subtract (lhs, rhs);
            }

            return lhs;
        }

    private:
        const String textString;
        const juce_wchar* text;
        int& textIndex;

        //==============================================================================
        static inline bool isDecimalDigit (const juce_wchar c) throw()
        {
            return c >= '0' && c <= '9';
        }

        void skipWhitespace (int& i)
        {
            while (CharacterFunctions::isWhitespace (text [i]))
                ++i;
        }

        bool readChar (const juce_wchar required)
        {
            if (text[textIndex] == required)
            {
                ++textIndex;
                return true;
            }

            return false;
        }

        bool readOperator (const char* ops, char* const opType = 0)
        {
            skipWhitespace (textIndex);

            while (*ops != 0)
            {
                if (readChar (*ops))
                {
                    if (opType != 0)
                        *opType = *ops;

                    return true;
                }

                ++ops;
            }

            return false;
        }

        bool readIdentifier (String& identifier)
        {
            skipWhitespace (textIndex);
            int i = textIndex;

            if (CharacterFunctions::isLetter (text[i]) || text[i] == '_')
            {
                ++i;

                while (CharacterFunctions::isLetterOrDigit (text[i]) || text[i] == '_' || text[i] == '.')
                    ++i;
            }

            if (i > textIndex)
            {
                identifier = String (text + textIndex, i - textIndex);
                textIndex = i;
                return true;
            }

            return false;
        }

        Term* readNumber()
        {
            skipWhitespace (textIndex);
            int i = textIndex;

            const bool isResolutionTarget = (text[i] == '@');
            if (isResolutionTarget)
            {
                ++i;
                skipWhitespace (i);
                textIndex = i;
            }

            if (text[i] == '-')
            {
                ++i;
                skipWhitespace (i);
            }

            int numDigits = 0;

            while (isDecimalDigit (text[i]))
            {
                ++i;
                ++numDigits;
            }

            const bool hasPoint = (text[i] == '.');

            if (hasPoint)
            {
                ++i;

                while (isDecimalDigit (text[i]))
                {
                    ++i;
                    ++numDigits;
                }
            }

            if (numDigits == 0)
                return 0;

            juce_wchar c = text[i];
            const bool hasExponent = (c == 'e' || c == 'E');

            if (hasExponent)
            {
                ++i;
                c = text[i];
                if (c == '+' || c == '-')
                    ++i;

                int numExpDigits = 0;
                while (isDecimalDigit (text[i]))
                {
                    ++i;
                    ++numExpDigits;
                }

                if (numExpDigits == 0)
                    return 0;
            }

            const int start = textIndex;
            textIndex = i;
            return new Constant (String (text + start, i - start).getDoubleValue(), isResolutionTarget);
        }

        const TermPtr readMultiplyOrDivideExpression()
        {
            TermPtr lhs (readUnaryExpression());

            char opType;
            while (lhs != 0 && readOperator ("*/", &opType))
            {
                TermPtr rhs (readUnaryExpression());

                if (rhs == 0)
                    throw ParseError ("Expected expression after \"" + String::charToString (opType) + "\"");

                if (opType == '*')
                    lhs = new Multiply (lhs, rhs);
                else
                    lhs = new Divide (lhs, rhs);
            }

            return lhs;
        }

        const TermPtr readUnaryExpression()
        {
            char opType;
            if (readOperator ("+-", &opType))
            {
                TermPtr term (readUnaryExpression());

                if (term == 0)
                    throw ParseError ("Expected expression after \"" + String::charToString (opType) + "\"");

                if (opType == '-')
                    term = term->negated();

                return term;
            }

            return readPrimaryExpression();
        }

        const TermPtr readPrimaryExpression()
        {
            TermPtr e (readParenthesisedExpression());
            if (e != 0)
                return e;

            e = readNumber();
            if (e != 0)
                return e;

            String identifier;
            if (readIdentifier (identifier))
            {
                if (readOperator ("(")) // method call...
                {
                    Function* f = new Function (identifier, ReferenceCountedArray<Term>());
                    ScopedPointer<Term> func (f);  // (can't use ScopedPointer<Function> in MSVC)

                    TermPtr param (readExpression());

                    if (param == 0)
                    {
                        if (readOperator (")"))
                            return func.release();

                        throw ParseError ("Expected parameters after \"" + identifier + " (\"");
                    }

                    f->parameters.add (param);

                    while (readOperator (","))
                    {
                        param = readExpression();

                        if (param == 0)
                            throw ParseError ("Expected expression after \",\"");

                        f->parameters.add (param);
                    }

                    if (readOperator (")"))
                        return func.release();

                    throw ParseError ("Expected \")\"");
                }
                else // just a symbol..
                {
                    return new Symbol (identifier);
                }
            }

            return 0;
        }

        const TermPtr readParenthesisedExpression()
        {
            if (! readOperator ("("))
                return 0;

            const TermPtr e (readExpression());
            if (e == 0 || ! readOperator (")"))
                return 0;

            return e;
        }

        JUCE_DECLARE_NON_COPYABLE (Parser);
    };
};

//==============================================================================
Expression::Expression()
    : term (new Expression::Helpers::Constant (0, false))
{
}

Expression::~Expression()
{
}

Expression::Expression (Term* const term_)
    : term (term_)
{
    jassert (term != 0);
}

Expression::Expression (const double constant)
    : term (new Expression::Helpers::Constant (constant, false))
{
}

Expression::Expression (const Expression& other)
    : term (other.term)
{
}

Expression& Expression::operator= (const Expression& other)
{
    term = other.term;
    return *this;
}

Expression::Expression (const String& stringToParse)
{
    int i = 0;
    Helpers::Parser parser (stringToParse, i);
    term = parser.readExpression();

    if (term == 0)
        term = new Helpers::Constant (0, false);
}

const Expression Expression::parse (const String& stringToParse, int& textIndexToStartFrom)
{
    Helpers::Parser parser (stringToParse, textIndexToStartFrom);
    const Helpers::TermPtr term (parser.readExpression());

    if (term != 0)
        return Expression (term);

    return Expression();
}

double Expression::evaluate() const
{
    return evaluate (Expression::EvaluationContext());
}

double Expression::evaluate (const Expression::EvaluationContext& context) const
{
    return term->evaluate (context, 0);
}

const Expression Expression::operator+ (const Expression& other) const  { return Expression (new Helpers::Add (term, other.term)); }
const Expression Expression::operator- (const Expression& other) const  { return Expression (new Helpers::Subtract (term, other.term)); }
const Expression Expression::operator* (const Expression& other) const  { return Expression (new Helpers::Multiply (term, other.term)); }
const Expression Expression::operator/ (const Expression& other) const  { return Expression (new Helpers::Divide (term, other.term)); }
const Expression Expression::operator-() const                          { return Expression (term->negated()); }

const String Expression::toString() const
{
    return term->toString();
}

const Expression Expression::symbol (const String& symbol)
{
    return Expression (new Helpers::Symbol (symbol));
}

const Expression Expression::function (const String& functionName, const Array<Expression>& parameters)
{
    ReferenceCountedArray<Term> params;
    for (int i = 0; i < parameters.size(); ++i)
        params.add (parameters.getReference(i).term);

    return Expression (new Helpers::Function (functionName, params));
}

const Expression Expression::adjustedToGiveNewResult (const double targetValue,
                                                      const Expression::EvaluationContext& context) const
{
    ScopedPointer<Term> newTerm (term->clone());

    Helpers::Constant* termToAdjust = Helpers::findTermToAdjust (newTerm, true);

    if (termToAdjust == 0)
        termToAdjust = Helpers::findTermToAdjust (newTerm, false);

    if (termToAdjust == 0)
    {
        newTerm = new Helpers::Add (newTerm.release(), new Helpers::Constant (0, false));
        termToAdjust = Helpers::findTermToAdjust (newTerm, false);
    }

    jassert (termToAdjust != 0);

    const Term* parent = Helpers::findDestinationFor (newTerm, termToAdjust);

    if (parent == 0)
    {
        termToAdjust->value = targetValue;
    }
    else
    {
        const Helpers::TermPtr reverseTerm (parent->createTermToEvaluateInput (context, termToAdjust, targetValue, newTerm));

        if (reverseTerm == 0)
            return Expression (targetValue);

        termToAdjust->value = reverseTerm->evaluate (context, 0);
    }

    return Expression (newTerm.release());
}

const Expression Expression::withRenamedSymbol (const String& oldSymbol, const String& newSymbol) const
{
    jassert (newSymbol.toLowerCase().containsOnly ("abcdefghijklmnopqrstuvwxyz0123456789_"));

    Expression newExpression (term->clone());
    Helpers::renameSymbol (newExpression.term, oldSymbol, newSymbol);
    return newExpression;
}

bool Expression::referencesSymbol (const String& symbol, const EvaluationContext* context) const
{
    return term->referencesSymbol (symbol, context, 0);
}

bool Expression::usesAnySymbols() const
{
    return Helpers::containsAnySymbols (term);
}

Expression::Type Expression::getType() const throw()
{
    return term->getType();
}

const String Expression::getSymbol() const
{
    return term->getSymbolName();
}

const String Expression::getFunction() const
{
    return term->getFunctionName();
}

const String Expression::getOperator() const
{
    return term->getFunctionName();
}

int Expression::getNumInputs() const
{
    return term->getNumInputs();
}

const Expression Expression::getInput (int index) const
{
    return Expression (term->getInput (index));
}

//==============================================================================
int Expression::Term::getOperatorPrecedence() const
{
    return 0;
}

bool Expression::Term::referencesSymbol (const String&, const EvaluationContext*, int) const
{
    return false;
}

int Expression::Term::getInputIndexFor (const Term*) const
{
    return -1;
}

const ReferenceCountedObjectPtr<Expression::Term> Expression::Term::createTermToEvaluateInput (const EvaluationContext&, const Term*, double, Term*) const
{
    jassertfalse;
    return 0;
}

const ReferenceCountedObjectPtr<Expression::Term> Expression::Term::negated()
{
    return new Helpers::Negate (this);
}

const String Expression::Term::getSymbolName() const
{
    jassertfalse; // You should only call getSymbol() on an expression that's actually a symbol!
    return String::empty;
}

const String Expression::Term::getFunctionName() const
{
    jassertfalse; // You shouldn't call this for an expression that's not actually a function!
    return String::empty;
}

//==============================================================================
Expression::ParseError::ParseError (const String& message)
    : description (message)
{
    DBG ("Expression::ParseError: " + message);
}

Expression::EvaluationError::EvaluationError (const String& message)
    : description (message)
{
    DBG ("Expression::EvaluationError: " + description);
}

Expression::EvaluationError::EvaluationError (const String& symbol, const String& member)
    : description ("Unknown symbol: \"" + symbol + (member.isEmpty() ? "\"" : ("." + member + "\"")))
{
    DBG ("Expression::EvaluationError: " + description);
}

//==============================================================================
Expression::EvaluationContext::EvaluationContext()  {}
Expression::EvaluationContext::~EvaluationContext() {}

const Expression Expression::EvaluationContext::getSymbolValue (const String& symbol, const String& member) const
{
    throw EvaluationError (symbol, member);
}

double Expression::EvaluationContext::evaluateFunction (const String& functionName, const double* parameters, int numParams) const
{
    if (numParams > 0)
    {
        if (functionName == "min")
        {
            double v = parameters[0];
            for (int i = 1; i < numParams; ++i)
                v = jmin (v, parameters[i]);

            return v;
        }
        else if (functionName == "max")
        {
            double v = parameters[0];
            for (int i = 1; i < numParams; ++i)
                v = jmax (v, parameters[i]);

            return v;
        }
        else if (numParams == 1)
        {
            if      (functionName == "sin")     return sin (parameters[0]);
            else if (functionName == "cos")     return cos (parameters[0]);
            else if (functionName == "tan")     return tan (parameters[0]);
            else if (functionName == "abs")     return std::abs (parameters[0]);
        }
    }

    throw EvaluationError ("Unknown function: \"" + functionName + "\"");
}


END_JUCE_NAMESPACE
