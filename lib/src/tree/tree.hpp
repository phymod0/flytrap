#ifndef TREE
#define TREE


#include <memory>
#include <optional>
#include <unordered_map>


template <typename Node, typename Value> class Tree
{
      public:
	Tree& operator[](const Node& node);
	bool hasChild(const Node& node);
	Value& value();

      private:
	std::unordered_map<Node, std::shared_ptr<Tree>> children;
	std::optional<Value> maybe_value;
};


#endif /* TREE */
