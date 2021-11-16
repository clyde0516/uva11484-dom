#include <assert.h>
#include <iostream>
#include <sstream>
#include <stack>
#include <unordered_map>
#include <boost/enable_shared_from_this.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/smart_ptr/make_shared_object.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>

//#define ONLINE_JUDGE

#ifndef ONLINE_JUDGE
#include <gmock/gmock.h>
#endif

const std::string END_OF_DOC("</n>");

enum class Adjacent
{
    None = 0,
    FirstChild,
    NextSibling,
    PreviousSibling,
    Parent
};

Adjacent reverse(Adjacent direction)
{
    Adjacent reversed_direction = Adjacent::None;

    switch (direction)
    {
    case Adjacent::FirstChild:
        reversed_direction = Adjacent::Parent;
        break;
    case Adjacent::NextSibling:
        reversed_direction = Adjacent::PreviousSibling;
        break;
    case Adjacent::PreviousSibling:
        reversed_direction = Adjacent::NextSibling;
        break;
    case Adjacent::Parent:
        reversed_direction = Adjacent::FirstChild;
        break;
    default:
        BOOST_ASSERT(0);
        break;
    }

    return reversed_direction;
}

class DocNode : public boost::enable_shared_from_this<DocNode>
{
public:
    typedef boost::shared_ptr<DocNode> shared_ptr_t;

public:
    DocNode(const std::string & value);

public:
    std::string value() const;

    shared_ptr_t adjacent_node(Adjacent direction) const;
    void set_adjacent_node(Adjacent direction, shared_ptr_t node);

private:
    std::string value_;
    std::unordered_map<Adjacent, shared_ptr_t> adjacent_nodes_;
};

DocNode::DocNode(const std::string & value)
{
    value_ = value;
}

std::string DocNode::value() const
{
    return value_;
}

DocNode::shared_ptr_t DocNode::adjacent_node(Adjacent direction) const
{
    auto itr = adjacent_nodes_.find(direction);
    if (itr != adjacent_nodes_.end())
    {
        return itr->second;
    }

    return DocNode::shared_ptr_t();
}

void DocNode::set_adjacent_node(Adjacent direction, shared_ptr_t node)
{
    if (adjacent_nodes_.count(direction) > 0)
    {
        BOOST_ASSERT(0);
        return;
    }

    adjacent_nodes_[direction] = node;
    auto reversed_direction = reverse(direction);
    if (node && !node->adjacent_node(reversed_direction))
    {
        node->set_adjacent_node(reversed_direction, shared_from_this());
    }
}

std::string get_value(const std::string & new_doc_tag)
{
    size_t first_quote_pos = new_doc_tag.find_first_of('\'');
    size_t last_quote_pos = new_doc_tag.find_last_of('\'');
    BOOST_ASSERT(first_quote_pos < last_quote_pos);
    return new_doc_tag.substr(first_quote_pos + 1, last_quote_pos - first_quote_pos - 1);
}

DocNode::shared_ptr_t build_dom(std::istream & is)
{
    std::stack<DocNode::shared_ptr_t> dom_nodes;
    DocNode::shared_ptr_t root_node;
    DocNode::shared_ptr_t previous_sibling;

    std::string line;
    std::getline(is, line);
    size_t line_count = boost::lexical_cast<size_t>(line);

    for (size_t i = 0; i < line_count; ++i)
    {
        std::getline(is, line);

        if (line == END_OF_DOC)
        {
            previous_sibling = dom_nodes.empty() ? nullptr : dom_nodes.top();
            dom_nodes.pop();
        }
        else
        {
            auto current = boost::make_shared<DocNode>(get_value(line));
            current->set_adjacent_node(Adjacent::Parent, dom_nodes.empty() ? nullptr : dom_nodes.top());
            current->set_adjacent_node(Adjacent::PreviousSibling, previous_sibling);

            dom_nodes.push(current);
            if (!root_node)
            {
                root_node = current;
            }
        }
    }

    return root_node;
}

std::vector<Adjacent> get_instructions(std::istream & is)
{
    std::vector<Adjacent> instructions;

    size_t instruction_count;
    is >> instruction_count;

    for (size_t i = 0; i < instruction_count; ++i)
    {
        Adjacent direction = Adjacent::None;

        std::string instruction;
        is >> instruction;

        if (instruction == "first_child")
        {
            direction = Adjacent::FirstChild;
        }
        else if (instruction == "next_sibling")
        {
            direction = Adjacent::NextSibling;
        }
        else if (instruction == "previous_sibling")
        {
            direction = Adjacent::PreviousSibling;
        }
        else if (instruction == "parent")
        {
            direction = Adjacent::Parent;
        }

        BOOST_ASSERT(direction != Adjacent::None);
        instructions.push_back(direction);
    }

    return instructions;
}

void solve_dom_problem(std::istream & is, std::ostream & os)
{
    DocNode::shared_ptr_t current_node = build_dom(is);
    size_t case_number = 0;

    while (true)
    {
        std::vector<Adjacent> directions = get_instructions(is);
        if (directions.empty())
        {
            break;
        }

        os << "Case " << boost::lexical_cast<std::string>(++case_number) << ":" << std::endl;
        for (auto & direction : directions)
        {
            auto next_node = current_node->adjacent_node(direction);
            if (next_node)
            {
                current_node = next_node;
            }
            os << current_node->value() << std::endl;
        }
    }
}

int main(int argc, char ** argv)
{
#ifdef ONLINE_JUDGE
    solve_dom_problem(std::cin, std::cout);
#else
    testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
#endif
}

#ifndef ONLINE_JUDGE
TEST(DocumentObjectModel, Sample)
{
    std::istringstream iss(
        "4\n"
        "<n value = 'parent'>\n"
        "<n value = 'child'>\n"
        "</n>\n"
        "</n>\n"
        "2\n"
        "next_sibling\n"
        "first_child\n"
        "0\n"
    );
    std::ostringstream oss;

    solve_dom_problem(iss, oss);
    EXPECT_EQ(
        "Case 1:\n"
        "parent\n"
        "child\n"
        , oss.str());
}
#endif
