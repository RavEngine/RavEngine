#include "Dijkstra.h"

#include <limits>
#include <algorithm>

const float DijkstraNode::infinity = std::numeric_limits<float>::infinity();

Dijkstra::Dijkstra()
{}

Dijkstra::~Dijkstra()
{}

bool Dijkstra::getPath(DijkstraNode* start, DijkstraNode* goal, std::vector<DijkstraNode*>& path)
{
	DijkstraNode *currentNode, *childNode;
	float dist;

	std::make_heap(open.begin(), open.end(), CompareNodes());
	pushOpen(start);
	start->setDistance(0.0f);

	while(!open.empty())
	{
		std::sort(open.begin(), open.end(), CompareNodes());

		currentNode = open.front();
		popOpen(currentNode);

		if(currentNode == goal)
		{
			reconstructPath(currentNode, path);
			return true;
		}

		for(const auto& children : currentNode->getChildren())
		{
			childNode = static_cast<DijkstraNode*>(children.first);
			
			dist = currentNode->getDistance() + children.second;
			if(!childNode->isClosed() && dist < childNode->getDistance())
			{
				childNode->setDistance(dist);
				childNode->setParent(currentNode);
				pushOpen(childNode);
			}
		}
	}

	return false;
}

void Dijkstra::pushOpen(DijkstraNode* node)
{
	open.push_back(node);
	std::push_heap(open.begin(), open.end(), CompareNodes());
}

void Dijkstra::popOpen(DijkstraNode* node)
{
	std::pop_heap(open.begin(), open.end(), CompareNodes());
	open.pop_back();
	node->setClosed(true);
	closed.push_back(node);
}


void Dijkstra::releaseNodes()
{
	for(const auto& node : open)
		node->release();
	for(const auto& node : closed)
		node->release();
}

void Dijkstra::clear()
{
	releaseNodes();
	open.clear();
	closed.clear();
}