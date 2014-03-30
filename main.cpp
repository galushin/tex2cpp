#include <boost/spirit/include/qi.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <fstream>
#include <iostream>

namespace ast
{
    struct nil {};
    struct signed_;
    struct expression;
    struct fraction;

    struct expression_in_parens;

    typedef boost::variant<
            nil
          , std::string
          , unsigned int
          , boost::recursive_wrapper<signed_>
          , boost::recursive_wrapper<expression>
          , boost::recursive_wrapper<fraction>
        >
    operand;

    struct signed_
    {
        char sign;
        operand operand_;
    };

    struct operation
    {
        char operator_;
        operand operand_;
    };

    // print function for debugging
    inline std::ostream& operator<<(std::ostream& out, nil) { out << "nil"; return out; }

    struct expression
    {
        operand first;
        std::vector<operation> rest;
    };

    struct expression_in_parens
    {
        expression_in_parens() = default;

        expression body;
    };

    struct formula
    {
        std::string function_name;
        std::vector<std::string> args;
        expression body;
    };

    struct fraction
    {
        expression numerator;
        expression denominator;
    };
}
// namespace ast

BOOST_FUSION_ADAPT_STRUCT(ast::formula,
                          (std::string, function_name)
                          (std::vector<std::string>, args)
                          (ast::expression, body))

BOOST_FUSION_ADAPT_STRUCT(ast::expression_in_parens,
                          (ast::expression, body))

BOOST_FUSION_ADAPT_STRUCT(
    ast::signed_,
    (char, sign)
    (ast::operand, operand_)
)

BOOST_FUSION_ADAPT_STRUCT(
    ast::operation,
    (char, operator_)
    (ast::operand, operand_)
)

BOOST_FUSION_ADAPT_STRUCT(
    ast::expression,
    (ast::operand, first)
    (std::vector<ast::operation>, rest)
)

BOOST_FUSION_ADAPT_STRUCT(ast::fraction,
                          (ast::expression, numerator)
                          (ast::expression, denominator))

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;

template <typename Iterator>
struct calculator
 : qi::grammar<Iterator, ast::formula(), ascii::space_type>
{
    calculator()
     : calculator::base_type(formula)
    {
        formula = id >> "(" >> (id % ',') >> ")" >> "=" >> expression;
        id = +qi::alpha | ('\\' > + qi::alpha);

        using qi::char_;
        using qi::uint_;

        expression =
                term
                >> *(   (char_('+') > term)
                    |   (char_('-') > term)
                    )
                ;

            term =
                factor
                >> *(   (char_('*') > factor)
                    |   (char_('/') > factor)
                    |   (char_('^') > factor)
                    )
                ;

            factor =
                    uint_
                |   fraction
                |   id
                |   expression_in_parens
                |   (char_('-') > factor)
                |   (char_('+') > factor)
                ;

            expression_in_parens
                = ('(' > expression > ')')
                | ('{' > expression > '}');

            fraction = ("\\frac{" > expression > "}{" > expression > "}");
    }

    qi::rule<Iterator, ast::formula(), ascii::space_type> formula;
    qi::rule<Iterator, ast::expression(), ascii::space_type> expression;
    qi::rule<Iterator, ast::expression(), ascii::space_type> term;
    qi::rule<Iterator, ast::operand(), ascii::space_type> factor;
    qi::rule<Iterator, std::string(), ascii::space_type> id;
    qi::rule<Iterator, ast::expression(), ascii::space_type> expression_in_parens;
    qi::rule<Iterator, ast::fraction(), ascii::space_type> fraction;
};

template <class F>
class scope_guard
{
public:
    explicit scope_guard(F f)
     : f_{std::move(f)}
    {}

    ~ scope_guard()
    {
        try
        {
            f_();
        }
        catch(...)
        {}
    }

private:
    F f_;
};

template <class F>
scope_guard<F>
make_scope_guard(F f)
{
    return scope_guard<F>{std::move(f)};
}

class cpp_generator
{
public:
    typedef cpp_generator & result_type;

    cpp_generator()
    {
        priorities_.push_back(Never);
    }

    cpp_generator & operator()(ast::formula formula)
    {
        (*this)("double ")(formula.function_name)("(");

        auto first = formula.args.begin();
        auto last = formula.args.end();

        if(first != last)
        {
            (*this)(*first);
            ++ first;

            for(; first != last; ++ first)
            {
                (*this)(", ")(*first);
            }
        }

        (*this)(")\n")("{\n")("\t")("return ");

        (*this)(formula.body);
        return (*this)(";\n")("}");
    }

    cpp_generator & operator()(ast::expression const & e)
    {
        this->helper(e, e.rest.size());
        return *this;
    }

    cpp_generator & operator()(ast::operand const & x)
    {
        boost::apply_visitor(*this, x);
        return *this;
    }

    cpp_generator & operator()(ast::operation const & x)
    {
        return (*this)(" ")(x.operator_)(" ")(x.operand_);
    }

    cpp_generator & operator()(ast::signed_ const & x)
    {
        return (*this)(x.sign)(x.operand_);
    }

    cpp_generator & operator()(ast::expression_in_parens const & x)
    {
        return (*this)("(")(x.body)(")");
    }

    cpp_generator & operator()(ast::fraction const & x)
    {
        auto const op = '/';

        priorities_.push_back(this->make_priority(op));
        auto guard = make_scope_guard([this]() {this->priorities_.pop_back();} );

        (*this)(x.numerator)(op)(x.denominator);

        return *this;
    }

    template <class T>
    cpp_generator & operator()(T const & x)
    {
        std::cout << x;

        return *this;
    }

private:
    cpp_generator & helper(ast::expression const & e, size_t n)
    {
        if(n == 0)
        {
            return (*this)(e.first);
        }

        auto op = e.rest[n-1].operator_;
        auto priority = make_priority(op);
        auto const need_paren = priority > priorities_.back();

        priorities_.push_back(priority);
        auto guard = make_scope_guard([this](){this->priorities_.pop_back();});

        if (op == '^')
        {
            (*this)("pow(");
            this->helper(e, n-1);
            (*this)(", ")(e.rest[n-1].operand_)(")");
        }
        else
        {
            if(need_paren)
            {
                (*this)('(');
            }

            this->helper(e, n-1);
            (*this)(op)(e.rest[n-1].operand_);

            if(need_paren)
            {
                (*this)(')');
            }
        }

        return *this;
    }

    enum Priority
    {
        Division,
        Mult,
        Additive,

        Never
    };

    static Priority make_priority(char op)
    {
        switch(op)
        {
        case '/':
            return Division;
        case '*':
        case '^':
            return Mult;
        case '+':
        case '-':
            return Additive;

        default:
            assert(false);
        }
        return Never;
    }

    std::vector<Priority> priorities_;
};

int main()
{
    std::ifstream file("source.txt");

    std::string src;
    std::getline(file, src);

    std::cout << "Source:\n" << src << "\n";
    std::cout << "------------------\n";

    typedef std::string::const_iterator iterator_type;
    typedef calculator<iterator_type> calculator;

    calculator calc;

    auto iter = src.cbegin();
    auto end = src.cend();
    boost::spirit::ascii::space_type space;
    ast::formula formula;

    bool r = phrase_parse(iter, end, calc, space, formula);

    cpp_generator gen;

    if(r == true)
    {
        gen(formula);
    }
    else
    {
        std::cout << "Fail\n";
        std::copy(iter, end, std::ostream_iterator<char>(std::cout));
    }

    return 0;
}
