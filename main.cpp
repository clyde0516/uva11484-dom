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

Adjacent reverse(Adjacent adjacent)
{
    switch (adjacent)
    {
    case Adjacent::FirstChild:      return Adjacent::Parent;            break;
    case Adjacent::NextSibling:     return Adjacent::PreviousSibling;   break;
    case Adjacent::PreviousSibling: return Adjacent::NextSibling;       break;
    case Adjacent::Parent:          return Adjacent::FirstChild;        break;
    }

    BOOST_ASSERT(0);
    return Adjacent::None;
}

class DocNode : public boost::enable_shared_from_this<DocNode>
{
public:
    typedef boost::shared_ptr<DocNode> shared_ptr_t;

public:
    DocNode(const std::string & value);

public:
    std::string value() const;

    shared_ptr_t adjacent_node(Adjacent adjacent) const;
    void set_adjacent_node(Adjacent adjacent, shared_ptr_t node, bool build_reversed_link);

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

DocNode::shared_ptr_t DocNode::adjacent_node(Adjacent adjacent) const
{
    auto itr = adjacent_nodes_.find(adjacent);
    if (itr != adjacent_nodes_.end())
    {
        return itr->second;
    }

    return DocNode::shared_ptr_t();
}

void DocNode::set_adjacent_node(Adjacent adjacent, shared_ptr_t node, bool double_linked)
{
    if (adjacent_nodes_.count(adjacent) > 0)
    {
        BOOST_ASSERT(0);
        return;
    }

    adjacent_nodes_[adjacent] = node;

    if (double_linked)
    {
        auto reversed_adjacent = reverse(adjacent);
        if (node && !node->adjacent_node(reversed_adjacent))
        {
            node->set_adjacent_node(reversed_adjacent, shared_from_this(), false);
        }
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
    DocNode::shared_ptr_t root_node;
    DocNode::shared_ptr_t previous_sibling;
    std::stack<DocNode::shared_ptr_t> opened_nodes;

    std::string line_buffer;
    std::getline(is, line_buffer);
    size_t line_count = boost::lexical_cast<size_t>(line_buffer);

    for (size_t i = 0; i < line_count; ++i)
    {
        std::getline(is, line_buffer);

        if (line_buffer == END_OF_DOC)
        {
            BOOST_ASSERT(!opened_nodes.empty());
            previous_sibling = opened_nodes.top();
            opened_nodes.pop();
        }
        else
        {
            auto current_node = boost::make_shared<DocNode>(get_value(line_buffer));
            current_node->set_adjacent_node(Adjacent::Parent, opened_nodes.empty() ? nullptr : opened_nodes.top(), true);
            current_node->set_adjacent_node(Adjacent::PreviousSibling, previous_sibling, true);

            opened_nodes.push(current_node);
            if (!root_node)
            {
                root_node = current_node;
            }
        }
    }

    BOOST_ASSERT(opened_nodes.empty());
    return root_node;
}

std::vector<Adjacent> get_instructions(std::istream & is)
{
    std::vector<Adjacent> instructions;

    size_t instruction_count;
    is >> instruction_count;

    for (size_t i = 0; i < instruction_count; ++i)
    {
        Adjacent adjacent = Adjacent::None;

        std::string instruction;
        is >> instruction;

        if (instruction == "first_child")
        {
            adjacent = Adjacent::FirstChild;
        }
        else if (instruction == "next_sibling")
        {
            adjacent = Adjacent::NextSibling;
        }
        else if (instruction == "previous_sibling")
        {
            adjacent = Adjacent::PreviousSibling;
        }
        else if (instruction == "parent")
        {
            adjacent = Adjacent::Parent;
        }

        BOOST_ASSERT(adjacent != Adjacent::None);
        instructions.push_back(adjacent);
    }

    return instructions;
}

void solve_dom_problem(std::istream & is, std::ostream & os)
{
    DocNode::shared_ptr_t current_node = build_dom(is);
    size_t case_number = 0;

    while (true)
    {
        std::vector<Adjacent> adjacents = get_instructions(is);
        if (adjacents.empty())
        {
            break;
        }

        os << "Case " << boost::lexical_cast<std::string>(++case_number) << ":" << std::endl;
        for (auto & adjacent : adjacents)
        {
            auto next_node = current_node->adjacent_node(adjacent);
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
