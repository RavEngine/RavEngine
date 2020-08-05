#pragma once

#include <vector>
#include <algorithm>
#include "PathAlgorithm.h"
#include "Node.h"

class AStarNode : public Node
{
	public:

		AStarNode() :
			m_f(0.0), m_g(0.0), m_h(0.0),
			closed(false), open(false)
		{}

		virtual ~AStarNode()
		{}

		void setPosition(int x, int y)
		{
			m_x = x;
			m_y = y;
		}

		void setF(float f)
		{
			m_f = f;
		}

		void setG(float g)
		{
			m_g = g;
		}

		void setH(float h)
		{
			m_h = h;
		}

		void setOpen(bool value)
		{
			open = value;
		}

		void setClosed(bool value)
		{
			closed = value;
		}

		inline unsigned int getX() const
		{
			return m_x;
		}

		inline unsigned int getY() const
		{
			return m_y;
		}

		inline float getF() const
		{
			return m_f;
		}

		inline float getG() const
		{
			return m_g;
		}

		inline float getH() const
		{
			return m_h;
		}

		inline bool isOpen() const
		{
			return open;
		}

		inline bool isClosed() const
		{
			return closed;
		}

		virtual float distanceTo(AStarNode* node) const = 0;

		void release()
		{
			open = closed = false;
			m_f = m_g = m_h = 0.0f;
			m_parent = nullptr;
		}

	protected:
		float m_f, m_g, m_h;
		unsigned int m_x, m_y;
		bool open, closed;
};

struct CompareNodes
{
	bool operator() (const AStarNode* s1, const AStarNode *s2) const
	{
		return s1->getF() < s2->getF();
	}
};

class AStar : public PathAlgorithm<AStarNode>
{
	public:

		static AStar& getInstance()
		{
			static AStar instance;
			return instance;
		}

		bool getPath(AStarNode* start, AStarNode* goal, std::vector<AStarNode*>& path);
		void clear();

	private:

		AStar();
		~AStar();

		void releaseNodes();
		void pushOpen(AStarNode* node);
		void popOpen(AStarNode* node);
		
		std::vector<AStarNode*> open, closed;
};
