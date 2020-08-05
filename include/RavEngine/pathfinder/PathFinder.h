#pragma once

#include <vector>
#include <iterator>
#include "PathAlgorithm.h"


/**
	The PathFinder is the main class to use for (obviously) path finding.
	@tparam T The type of nodes you are working on. It <b>must</b> derive
	from Node and implement the virtual methods.
	@see Node
*/
template <class TNode>
class PathFinder
{
	public:

		/**
			@brief Default constructor.
		*/
		explicit PathFinder() :
			m_start(nullptr), m_goal(nullptr)
		{}

		/**
			@brief Sets the start node for the next path findings.
			@param[in] start A reference to the start node.
		*/
		void setStart(TNode& start)
		{
			m_start = &start;
		}

		/**
			@brief Sets the goal node for the next path findings.
			@param[in] start A reference to the goal node.
		*/
		void setGoal(TNode& goal)
		{
			m_goal = &goal;
		}

		/**
			@brief Returns the address of the start node for.
			@return The address of the start node.
		*/
		TNode* getStart() const
		{
			return m_start;
		}

		/**
			@brief Returns the address of the goal node for.
			@return The address of the goal node.
		*/
		TNode* getGoal() const
		{
			return m_goal;
		}

		/**
			@brief Use the specified algorithm to find a path
			between the previously set start and goal.
			@tparam U The algorithm to use (for example : AStar).
			@param[out] solution The vector receiving the path found.
			@param[in] hint Optional : gives the algorithm a hint about
			the number of nodes it is subject to have in the path. Do
			not provide if you don't know what to give.
		*/
		template <class TAlgorithm>
		bool findPath(std::vector<TNode*>& solution, int hint = 0)
		{
			std::vector<typename TAlgorithm::node_type*> path;
			TAlgorithm &algorithm = TAlgorithm::getInstance();

			if (hint > 0)
				path.reserve(hint);

			bool pathFound = algorithm.getPath(m_start, m_goal, path);

			if(!pathFound)
				return false;

			if(hint > 0)
				solution.reserve(hint);
			
			for(auto rit = std::rbegin(path); rit != std::rend(path); ++rit)
				solution.push_back( static_cast<TNode*>(*rit) );

			return true;
		}

	private:
		TNode *m_start, *m_goal;
};
