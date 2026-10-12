#include "../include/lunasvg.h"
#include "svgelement.h"
#include "svgparserutils.h"

#include <cassert>
namespace {

namespace lunasvg {

struct SimpleSelector;

using Selector = std::vector<SimpleSelector>;
using SelectorList = std::vector<Selector>;

struct AttributeSelector {
    enum class MatchType {
        None,
        Equals,
        Contains,
        Includes,
        StartsWith,
        EndsWith,
        DashEquals
    };

    MatchType matchType{MatchType::None};
    PropertyID id{PropertyID::Unknown};
    std::string value;
};

struct PseudoClassSelector {
    enum class Type {
        Unknown,
        Empty,
        Root,
        Is,
        Not,
        FirstChild,
        LastChild,
        OnlyChild,
        FirstOfType,
        LastOfType,
        OnlyOfType
    };

    Type type{Type::Unknown};
    SelectorList subSelectors;
};

struct SimpleSelector {
    enum class Combinator {
        None,
        Descendant,
        Child,
        DirectAdjacent,
        InDirectAdjacent
    };

    explicit SimpleSelector(Combinator combinator) : combinator(combinator) {}

    Combinator combinator{Combinator::Descendant};
    ElementID id{ElementID::Star};
    std::vector<AttributeSelector> attributeSelectors;
    std::vector<PseudoClassSelector> pseudoClassSelectors;
};

struct Declaration {
    int specificity;
    PropertyID id;
    std::string value;
};

using DeclarationList = std::vector<Declaration>;

struct Rule {
    SelectorList selectors;
    DeclarationList declarations;
};

class RuleData {
public:
    RuleData(const Selector& selector, const DeclarationList& declarations, size_t specificity, size_t position)
        : m_selector(selector), m_declarations(declarations), m_specificity(specificity), m_position(position)
    {}

    bool isLessThan(const RuleData& rule) const { return std::tie(m_specificity, m_position) < std::tie(rule.m_specificity, rule.m_position); }

    const Selector& selector() const { return m_selector; }
    const DeclarationList& declarations() const { return m_declarations; }
    size_t specificity() const { return m_specificity; }
    size_t position() const { return m_position; }

    bool match(const SVGElement* element) const;

private:
    Selector m_selector;
    DeclarationList m_declarations;
    size_t m_specificity;
    size_t m_position;
};

inline bool operator<(const RuleData& a, const RuleData& b) { return a.isLessThan(b); }

using RuleDataList = std::vector<RuleData>;

constexpr bool equals(const std::string_view& value, const std::string_view& subvalue)
{
    return value.compare(subvalue) == 0;
}

constexpr bool contains(const std::string_view& value, const std::string_view& subvalue)
{
    return value.find(subvalue) != std::string_view::npos;
}

constexpr bool includes(const std::string_view& value, const std::string_view& subvalue)
{
    if(subvalue.empty() || subvalue.length() > value.length())
        return false;
    std::string_view input(value);
    while(!input.empty()) {
        skipOptionalSpaces(input);
        std::string_view start(input);
        while(!input.empty() && !IS_WS(input.front()))
            input.remove_prefix(1);
        if(subvalue == start.substr(0, start.length() - input.length())) {
            return true;
        }
    }

    return false;
}

constexpr bool startswith(const std::string_view& value, const std::string_view& subvalue)
{
    if(subvalue.empty() || subvalue.length() > value.length())
        return false;
    return subvalue == value.substr(0, subvalue.size());
}

constexpr bool endswith(const std::string_view& value, const std::string_view& subvalue)
{
    if(subvalue.empty() || subvalue.length() > value.length())
        return false;
    return subvalue == value.substr(value.size() - subvalue.size(), subvalue.size());
}

constexpr bool dashequals(const std::string_view& value, const std::string_view& subvalue)
{
    if(startswith(value, subvalue))
        return (value.length() == subvalue.length() || value.at(subvalue.length()) == '-');
    return false;
}

static bool matchAttributeSelector(const AttributeSelector& selector, const SVGElement* element)
{
    const auto& value = element->getAttribute(selector.id);
    if(selector.matchType == AttributeSelector::MatchType::None)
        return !value.empty();
    if(selector.matchType == AttributeSelector::MatchType::Equals)
        return equals(value, selector.value);
    if(selector.matchType == AttributeSelector::MatchType::Contains)
        return contains(value, selector.value);
    if(selector.matchType == AttributeSelector::MatchType::Includes)
        return includes(value, selector.value);
    if(selector.matchType == AttributeSelector::MatchType::StartsWith)
        return startswith(value, selector.value);
    if(selector.matchType == AttributeSelector::MatchType::EndsWith)
        return endswith(value, selector.value);
    if(selector.matchType == AttributeSelector::MatchType::DashEquals)
        return dashequals(value, selector.value);
    return false;
}

static bool matchSimpleSelector(const SimpleSelector& selector, const SVGElement* element);

static bool matchPseudoClassSelector(const PseudoClassSelector& selector, const SVGElement* element)
{
    if(selector.type == PseudoClassSelector::Type::Empty)
        return element->children().empty();
    if(selector.type == PseudoClassSelector::Type::Root)
        return element->isRootElement();
    if(selector.type == PseudoClassSelector::Type::Is) {
        for(const auto& subSelector : selector.subSelectors) {
            for(const auto& simpleSelector : subSelector) {
                if(!matchSimpleSelector(simpleSelector, element)) {
                    return false;
                }
            }
        }

        return true;
    }

    if(selector.type == PseudoClassSelector::Type::Not) {
        for(const auto& subSelector : selector.subSelectors) {
            for(const auto& simpleSelector : subSelector) {
                if(matchSimpleSelector(simpleSelector, element)) {
                    return false;
                }
            }
        }

        return true;
    }

    if(selector.type == PseudoClassSelector::Type::FirstChild)
        return !element->previousElement();
    if(selector.type == PseudoClassSelector::Type::LastChild)
        return !element->nextElement();
    if(selector.type == PseudoClassSelector::Type::OnlyChild)
        return !(element->previousElement() || element->nextElement());
    if(selector.type == PseudoClassSelector::Type::FirstOfType) {
        auto sibling = element->previousElement();
        while(sibling) {
            if(sibling->id() == element->id())
                return false;
            sibling = element->previousElement();
        }

        return true;
    }

    if(selector.type == PseudoClassSelector::Type::LastOfType) {
        auto sibling = element->nextElement();
        while(sibling) {
            if(sibling->id() == element->id())
                return false;
            sibling = element->nextElement();
        }

        return true;
    }

    return false;
}

static bool matchSimpleSelector(const SimpleSelector& selector, const SVGElement* element)
{
    if(selector.id != ElementID::Star && selector.id != element->id())
        return false;
    for(const auto& sel : selector.attributeSelectors) {
        if(!matchAttributeSelector(sel, element)) {
            return false;
        }
    }

    for(const auto& sel : selector.pseudoClassSelectors) {
        if(!matchPseudoClassSelector(sel, element)) {
            return false;
        }
    }

    return true;
}

static bool matchSelector(const Selector& selector, const SVGElement* element)
{
    if(selector.empty())
        return false;
    auto it = selector.rbegin();
    auto end = selector.rend();
    if(!matchSimpleSelector(*it, element)) {
        return false;
    }

    auto combinator = it->combinator;
    ++it;

    while(it != end) {
        switch(combinator) {
        case SimpleSelector::Combinator::Child:
        case SimpleSelector::Combinator::Descendant:
            element = element->parentElement();
            break;
        case SimpleSelector::Combinator::DirectAdjacent:
        case SimpleSelector::Combinator::InDirectAdjacent:
            element = element->previousElement();
            break;
        case SimpleSelector::Combinator::None:
            assert(false);
        }

        if(element == nullptr)
            return false;
        if(matchSimpleSelector(*it, element)) {
            combinator = it->combinator;
            ++it;
        } else if(combinator != SimpleSelector::Combinator::Descendant
            && combinator != SimpleSelector::Combinator::InDirectAdjacent) {
            return false;
        }
    }

    return true;
}

bool RuleData::match(const SVGElement* element) const
{
    return matchSelector(m_selector, element);
}

constexpr bool IS_CSS_STARTNAMECHAR(int c) { return IS_ALPHA(c) || c == '_' || c == '-'; }
constexpr bool IS_CSS_NAMECHAR(int c) { return IS_CSS_STARTNAMECHAR(c) || IS_NUM(c); }

inline bool readCSSIdentifier(std::string_view& input, std::string& output)
{
    if(input.empty() || !IS_CSS_STARTNAMECHAR(input.front()))
        return false;
    output.clear();
    do {
        output.push_back(input.front());
        input.remove_prefix(1);
    } while(!input.empty() && IS_CSS_NAMECHAR(input.front()));
    return true;
}

static bool parseTagSelector(std::string_view& input, SimpleSelector& simpleSelector)
{
    std::string name;
    if(skipDelimiter(input, '*'))
        simpleSelector.id = ElementID::Star;
    else if(readCSSIdentifier(input, name))
        simpleSelector.id = elementid(name);
    else
        return false;
    return true;
}

static bool parseIdSelector(std::string_view& input, SimpleSelector& simpleSelector)
{
    AttributeSelector a;
    a.id = PropertyID::Id;
    a.matchType = AttributeSelector::MatchType::Equals;
    if(!readCSSIdentifier(input, a.value))
        return false;
    simpleSelector.attributeSelectors.push_back(std::move(a));
    return true;
}

static bool parseClassSelector(std::string_view& input, SimpleSelector& simpleSelector)
{
    AttributeSelector a;
    a.id = PropertyID::Class;
    a.matchType = AttributeSelector::MatchType::Includes;
    if(!readCSSIdentifier(input, a.value))
        return false;
    simpleSelector.attributeSelectors.push_back(std::move(a));
    return true;
}

static bool parseAttributeSelector(std::string_view& input, SimpleSelector& simpleSelector)
{
    std::string name;
    skipOptionalSpaces(input);
    if(!readCSSIdentifier(input, name))
        return false;
    AttributeSelector a;
    a.id = propertyid(name);
    a.matchType = AttributeSelector::MatchType::None;
    if(skipDelimiter(input, '='))
        a.matchType = AttributeSelector::MatchType::Equals;
    else if(skipString(input, "*="))
        a.matchType = AttributeSelector::MatchType::Contains;
    else if(skipString(input, "~="))
        a.matchType = AttributeSelector::MatchType::Includes;
    else if(skipString(input, "^="))
        a.matchType = AttributeSelector::MatchType::StartsWith;
    else if(skipString(input, "$="))
        a.matchType = AttributeSelector::MatchType::EndsWith;
    else if(skipString(input, "|="))
        a.matchType = AttributeSelector::MatchType::DashEquals;
    if(a.matchType != AttributeSelector::MatchType::None) {
        skipOptionalSpaces(input);
        if(!readCSSIdentifier(input, a.value)) {
            if(input.empty() || !(input.front() == '\"' || input.front() == '\''))
                return false;
            auto quote = input.front();
            input.remove_prefix(1);
            auto n = input.find(quote);
            if(n == std::string_view::npos)
                return false;
            a.value.assign(input.substr(0, n));
            input.remove_prefix(n + 1);
        }
    }

    skipOptionalSpaces(input);
    if(!skipDelimiter(input, ']'))
        return false;
    simpleSelector.attributeSelectors.push_back(std::move(a));
    return true;
}

static bool parseSelectors(std::string_view& input, SelectorList& selectors);

static bool parsePseudoClassSelector(std::string_view& input, SimpleSelector& simpleSelector)
{
    std::string name;
    if(!readCSSIdentifier(input, name))
        return false;
    PseudoClassSelector selector;
    if(name.compare("empty") == 0)
        selector.type = PseudoClassSelector::Type::Empty;
    else if(name.compare("root") == 0)
        selector.type = PseudoClassSelector::Type::Root;
    else if(name.compare("not") == 0)
        selector.type = PseudoClassSelector::Type::Not;
    else if(name.compare("first-child") == 0)
        selector.type = PseudoClassSelector::Type::FirstChild;
    else if(name.compare("last-child") == 0)
        selector.type = PseudoClassSelector::Type::LastChild;
    else if(name.compare("only-child") == 0)
        selector.type = PseudoClassSelector::Type::OnlyChild;
    else if(name.compare("first-of-type") == 0)
        selector.type = PseudoClassSelector::Type::FirstOfType;
    else if(name.compare("last-of-type") == 0)
        selector.type = PseudoClassSelector::Type::LastOfType;
    else if(name.compare("only-of-type") == 0)
        selector.type = PseudoClassSelector::Type::OnlyOfType;
    if(selector.type == PseudoClassSelector::Type::Is || selector.type == PseudoClassSelector::Type::Not) {
        skipOptionalSpaces(input);
        if(!skipDelimiter(input, '('))
            return false;
        skipOptionalSpaces(input);
        if(!parseSelectors(input, selector.subSelectors))
            return false;
        skipOptionalSpaces(input);
        if(!skipDelimiter(input, ')')) {
            return false;
        }
    }

    simpleSelector.pseudoClassSelectors.push_back(std::move(selector));
    return true;
}

static bool parseSimpleSelector(std::string_view& input, SimpleSelector& simpleSelector, bool& failed)
{
    auto consumed = parseTagSelector(input, simpleSelector);
    do {
        if(skipDelimiter(input, '#'))
            failed = !parseIdSelector(input, simpleSelector);
        else if(skipDelimiter(input, '.'))
            failed = !parseClassSelector(input, simpleSelector);
        else if(skipDelimiter(input, '['))
            failed = !parseAttributeSelector(input, simpleSelector);
        else if(skipDelimiter(input, ':'))
            failed = !parsePseudoClassSelector(input, simpleSelector);
        else
            break;
        consumed = true;
    } while(!failed);
    return consumed && !failed;
}

static bool parseCombinator(std::string_view& input, SimpleSelector::Combinator& combinator)
{
    combinator = SimpleSelector::Combinator::None;
    while(!input.empty() && IS_WS(input.front())) {
        combinator = SimpleSelector::Combinator::Descendant;
        input.remove_prefix(1);
    }

    if(skipDelimiterAndOptionalSpaces(input, '>'))
        combinator = SimpleSelector::Combinator::Child;
    else if(skipDelimiterAndOptionalSpaces(input, '+'))
        combinator = SimpleSelector::Combinator::DirectAdjacent;
    else if(skipDelimiterAndOptionalSpaces(input, '~'))
        combinator = SimpleSelector::Combinator::InDirectAdjacent;
    return combinator != SimpleSelector::Combinator::None;
}

static bool parseSelector(std::string_view& input, Selector& selector)
{
    auto combinator = SimpleSelector::Combinator::None;
    do {
        bool failed = false;
        SimpleSelector simpleSelector(combinator);
        if(!parseSimpleSelector(input, simpleSelector, failed))
            return !failed && (combinator == SimpleSelector::Combinator::Descendant);
        selector.push_back(std::move(simpleSelector));
    } while(parseCombinator(input, combinator));
    return true;
}

static bool parseSelectors(std::string_view& input, SelectorList& selectors)
{
    do {
        Selector selector;
        if(!parseSelector(input, selector))
            return false;
        selectors.push_back(std::move(selector));
    } while(skipDelimiterAndOptionalSpaces(input, ','));
    return true;
}

static bool parseDeclarations(std::string_view& input, DeclarationList& declarations)
{
    if(!skipDelimiter(input, '{'))
        return false;
    skipOptionalSpaces(input);
    do {
        std::string name;
        if(!readCSSIdentifier(input, name))
            return false;
        skipOptionalSpaces(input);
        if(!skipDelimiter(input, ':'))
            return false;
        skipOptionalSpaces(input);
        std::string_view value(input);
        while(!input.empty() && !(input.front() == '!' || input.front() == ';' || input.front() == '}'))
            input.remove_prefix(1);
        value.remove_suffix(input.length());
        stripTrailingSpaces(value);

        Declaration declaration;
        declaration.specificity = 0x10;
        declaration.id = csspropertyid(name);
        declaration.value.assign(value);
        if(skipDelimiter(input, '!')) {
            skipOptionalSpaces(input);
            if(!skipString(input, "important"))
                return false;
            declaration.specificity = 0x1000;
        }

        if(declaration.id != PropertyID::Unknown)
            declarations.push_back(std::move(declaration));
        skipOptionalSpacesOrDelimiter(input, ';');
    } while(!input.empty() && input.front() != '}');
    return skipDelimiter(input, '}');
}

static bool parseRule(std::string_view& input, Rule& rule)
{
    if(!parseSelectors(input, rule.selectors))
        return false;
    return parseDeclarations(input, rule.declarations);
}

static RuleDataList parseStyleSheet(std::string_view input)
{
    RuleDataList rules;
    while(!input.empty()) {
        skipOptionalSpaces(input);
        if(skipDelimiter(input, '@')) {
            int depth = 0;
            while(!input.empty()) {
                auto ch = input.front();
                input.remove_prefix(1);
                if(ch == ';' && depth == 0)
                    break;
                if(ch == '{') ++depth;
                else if(ch == '}' && depth > 0) {
                    if(depth == 1)
                        break;
                    --depth;
                }
            }

            continue;
        }

        Rule rule;
        if(!parseRule(input, rule))
            break;
        for(const auto& selector : rule.selectors) {
            size_t specificity = 0;
            for(const auto& simpleSelector : selector) {
                specificity += (simpleSelector.id == ElementID::Star) ? 0x0 : 0x1;
                for(const auto& attributeSelector : simpleSelector.attributeSelectors) {
                    specificity += (attributeSelector.id == PropertyID::Id) ? 0x10000 : 0x100;
                }
            }

            rules.emplace_back(selector, rule.declarations, specificity, rules.size());
        }
    }

    return rules;
}

static SelectorList parseQuerySelectors(std::string_view input)
{
    SelectorList selectors;
    stripLeadingAndTrailingSpaces(input);
    if(!parseSelectors(input, selectors)
        || !input.empty()) {
        return SelectorList();
    }

    return selectors;
}

inline void parseInlineStyle(std::string_view input, SVGElement* element)
{
    std::string name;
    skipOptionalSpaces(input);
    while(readCSSIdentifier(input, name)) {
        skipOptionalSpaces(input);
        if(!skipDelimiter(input, ':'))
            return;
        std::string value;
        while(!input.empty() && input.front() != ';') {
            value.push_back(input.front());
            input.remove_prefix(1);
        }

        auto id = csspropertyid(name);
        if(id != PropertyID::Unknown)
            element->setAttribute(0x100, id, value);
        skipOptionalSpacesOrDelimiter(input, ';');
    }
}

inline void removeStyleComments(std::string& value)
{
    auto start = value.find("/*");
    while(start != std::string::npos) {
        auto end = value.find("*/", start + 2);
        value.erase(start, end - start + 2);
        start = value.find("/*");
    }
}

inline bool decodeText(std::string_view input, std::string& output)
{
    output.clear();
    while(!input.empty()) {
        auto ch = input.front();
        input.remove_prefix(1);
        if(ch != '&') {
            output.push_back(ch);
            continue;
        }

        if(skipDelimiter(input, '#')) {
            int base = 10;
            if(skipDelimiter(input, 'x'))
                base = 16;
            unsigned int cp;
            if(!parseInteger(input, cp, base))
                return false;
            char c[5] = {0, 0, 0, 0, 0};
            if(cp < 0x80) {
                c[1] = 0;
                c[0] = char(cp);
            } else if(cp < 0x800) {
                c[2] = 0;
                c[1] = char((cp & 0x3F) | 0x80);
                cp >>= 6;
                c[0] = char(cp | 0xC0);
            } else if(cp < 0x10000) {
                c[3] = 0;
                c[2] = char((cp & 0x3F) | 0x80);
                cp >>= 6;
                c[1] = char((cp & 0x3F) | 0x80);
                cp >>= 6;
                c[0] = char(cp | 0xE0);
            } else if(cp < 0x200000) {
                c[4] = 0;
                c[3] = char((cp & 0x3F) | 0x80);
                cp >>= 6;
                c[2] = char((cp & 0x3F) | 0x80);
                cp >>= 6;
                c[1] = char((cp & 0x3F) | 0x80);
                cp >>= 6;
                c[0] = char(cp | 0xF0);
            }

            output.append(c);
        } else {
            if(skipString(input, "amp")) {
                output.push_back('&');
            } else if(skipString(input, "lt")) {
                output.push_back('<');
            } else if(skipString(input, "gt")) {
                output.push_back('>');
            } else if(skipString(input, "quot")) {
                output.push_back('\"');
            } else if(skipString(input, "apos")) {
                output.push_back('\'');
            } else {
                return false;
            }
        }

        if(!skipDelimiter(input, ';')) {
            return false;
        }
    }

    return true;
}

constexpr bool IS_STARTNAMECHAR(int c) { return IS_ALPHA(c) ||  c == '_' || c == ':'; }
constexpr bool IS_NAMECHAR(int c) { return IS_STARTNAMECHAR(c) || IS_NUM(c) || c == '-' || c == '.'; }

inline bool readIdentifier(std::string_view& input, std::string& output)
{
    if(input.empty() || !IS_STARTNAMECHAR(input.front()))
        return false;
    output.clear();
    do {
        output.push_back(input.front());
        input.remove_prefix(1);
    } while(!input.empty() && IS_NAMECHAR(input.front()));
    return true;
}

bool Document::parse(const char* data, size_t length)
{
    std::string buffer;
    std::string styleSheet;
    SVGElement* currentElement = nullptr;
    int ignoring = 0;
    auto handleText = [&](const std::string_view& text, bool in_cdata) {
        if(text.empty() || currentElement == nullptr || ignoring > 0)
            return;
        if(currentElement->id() != ElementID::Text && currentElement->id() != ElementID::Tspan && currentElement->id() != ElementID::Style) {
            return;
        }

        if(in_cdata) {
            buffer.assign(text);
        } else {
            decodeText(text, buffer);
        }

        if(currentElement->id() == ElementID::Style) {
            removeStyleComments(buffer);
            styleSheet.append(buffer);
        } else {
            auto node = std::make_unique<SVGTextNode>(this);
            node->setData(buffer);
            currentElement->addChild(std::move(node));
        }
    };

    std::string_view input(data, length);
    while(!input.empty()) {
        if(currentElement) {
            auto text = input.substr(0, input.find('<'));
            handleText(text, false);
            input.remove_prefix(text.length());
        } else {
            if(!skipOptionalSpaces(input)) {
                break;
            }
        }

        if(!skipDelimiter(input, '<'))
            return false;
        if(skipDelimiter(input, '?')) {
            if(!readIdentifier(input, buffer))
                return false;
            auto n = input.find("?>");
            if(n == std::string_view::npos)
                return false;
            input.remove_prefix(n + 2);
            continue;
        }

        if(skipDelimiter(input, '!')) {
            if(skipString(input, "--")) {
                auto n = input.find("-->");
                if(n == std::string_view::npos)
                    return false;
                handleText(input.substr(0, n), false);
                input.remove_prefix(n + 3);
                continue;
            }

            if(skipString(input, "[CDATA[")) {
                auto n = input.find("]]>");
                if(n == std::string_view::npos)
                    return false;
                handleText(input.substr(0, n), true);
                input.remove_prefix(n + 3);
                continue;
            }

            if(skipString(input, "DOCTYPE")) {
                while(!input.empty() && input.front() != '>') {
                    if(input.front() == '[') {
                        int depth = 1;
                        input.remove_prefix(1);
                        while(!input.empty() && depth > 0) {
                            if(input.front() == '[') ++depth;
                            else if(input.front() == ']') --depth;
                            input.remove_prefix(1);
                        }
                    } else {
                        input.remove_prefix(1);
                    }
                }

                if(!skipDelimiter(input, '>'))
                    return false;
                continue;
            }

            return false;
        }

        if(skipDelimiter(input, '/')) {
            if(currentElement == nullptr && ignoring == 0)
                return false;
            if(!readIdentifier(input, buffer))
                return false;
            if(ignoring == 0) {
                auto id = elementid(buffer);
                if(id != currentElement->id())
                    return false;
                currentElement = currentElement->parentElement();
            } else {
                --ignoring;
            }

            skipOptionalSpaces(input);
            if(!skipDelimiter(input, '>'))
                return false;
            continue;
        }

        if(!readIdentifier(input, buffer))
            return false;
        SVGElement* element = nullptr;
        if(ignoring > 0) {
            ++ignoring;
        } else {
            auto id = elementid(buffer);
            if(id == ElementID::Unknown) {
                ignoring = 1;
            } else {
                if(m_rootElement && currentElement == nullptr)
                    return false;
                if(m_rootElement == nullptr) {
                    if(id != ElementID::Svg)
                        return false;
                    m_rootElement = std::make_unique<SVGRootElement>(this);
                    element = m_rootElement.get();
                } else {
                    auto child = SVGElement::create(this, id);
                    element = child.get();
                    currentElement->addChild(std::move(child));
                }
            }
        }

        skipOptionalSpaces(input);
        while(readIdentifier(input, buffer)) {
            skipOptionalSpaces(input);
            if(!skipDelimiter(input, '='))
                return false;
            skipOptionalSpaces(input);
            if(input.empty() || !(input.front() == '\"' || input.front() == '\''))
                return false;
            auto quote = input.front();
            input.remove_prefix(1);
            auto n = input.find(quote);
            if(n == std::string_view::npos)
                return false;
            auto id = PropertyID::Unknown;
            if(element != nullptr)
                id = propertyid(buffer);
            if(id != PropertyID::Unknown) {
                decodeText(input.substr(0, n), buffer);
                if(id == PropertyID::Style) {
                    removeStyleComments(buffer);
                    parseInlineStyle(buffer, element);
                } else {
                    if(id == PropertyID::Id)
                        m_rootElement->addElementById(buffer, element);
                    element->setAttribute(0x1, id, buffer);
                }
            }

            input.remove_prefix(n + 1);
            skipOptionalSpaces(input);
        }

        if(skipDelimiter(input, '>')) {
            if(element != nullptr)
                currentElement = element;
            continue;
        }

        if(skipDelimiter(input, '/')) {
            if(!skipDelimiter(input, '>'))
                return false;
            if(ignoring > 0)
                --ignoring;
            continue;
        }

        return false;
    }

    if(m_rootElement == nullptr || ignoring > 0 || !input.empty())
        return false;
    applyStyleSheet(styleSheet);
    m_rootElement->build();
    return true;
}

void Document::applyStyleSheet(const std::string& content)
{
    auto rules = parseStyleSheet(content);
    if(!rules.empty()) {
        std::sort(rules.begin(), rules.end());
        m_rootElement->transverse([&rules](SVGElement* element) {
            for(const auto& rule : rules) {
                if(rule.match(element)) {
                    for(const auto& declaration : rule.declarations()) {
                        element->setAttribute(declaration.specificity, declaration.id, declaration.value);
                    }
                }
            }
        });
    }
}

ElementList Document::querySelectorAll(const std::string& content) const
{
    auto selectors = parseQuerySelectors(content);
    if(selectors.empty())
        return ElementList();
    ElementList elements;
    m_rootElement->transverse([&](SVGElement* element) {
        for(const auto& selector : selectors) {
            if(matchSelector(selector, element)) {
                elements.push_back(element);
                break;
            }
        }
    });

    return elements;
}

} // namespace lunasvg
}
