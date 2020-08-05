#pragma once

#include <vector>

/**
	Represents a basic node for path finding with no added value. It can not be used
	or instanciated as-is and is provided for implementing specific nodes for algorithms.

	@see AStarNode
*/
class Node
{
	public:

		Node();
		virtual ~Node();

		/**
			@brief Assigns the parent of the node. The parent of a node will
			be evaluated when reconstituing the path form the goal.
			@param[in] parent Pointer to the node to assign as the parent.
		*/
		void setParent(Node* parent);

		/**
			@brief Returns a pointer to the parent node.
			@return A pointer to the parent node.
		*/
		Node* getParent() const;

		/**
			@brief Add a node to the children of the current node.
			@param[in] child A pointer to the child.
		*/
		void addChild(Node* child, float distance);

		/**
			@brief Returns a vector containing all the children of the current node.
			@return A vector of Node pointers.
		*/
		std::vector<std::pair<Node*, float>>& getChildren();

	protected:

		/**
			@brief Clears the children of the node.
		*/
		void clearChildren();

		/**
			Pointer to the parent node.
		*/
		Node* m_parent;

		/**
			List of all the node's children.
		*/
		std::vector<std::pair<Node*, float>> m_children;
};
