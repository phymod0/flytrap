#include "tree.hpp"


template <typename Node, typename Value>
Tree<Node, Value>& Tree<Node, Value>::operator[](const Node& node)
{
	auto it = children.find(node);
	if (it == children.end()) {
		std::shared_ptr<Tree> subtree(new Tree);
		it = children.insert({node, subtree}).first;
	}
	return *(it->second);
}


template <typename Node, typename Value>
bool Tree<Node, Value>::hasChild(const Node& node)
{
	return children.find(node) != children.end();
}


template <typename Node, typename Value> Value& Tree<Node, Value>::value()
{
	if (!maybe_value)
		maybe_value = std::move(std::optional<Value>(Value()));
	return *maybe_value;
}
