// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//  * Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//  * Neither the name of NVIDIA CORPORATION nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ''AS IS'' AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Copyright (c) 2008-2025 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  

#ifndef PXS_ISLAND_SIM_H
#define PXS_ISLAND_SIM_H

#include "foundation/PxAssert.h"
#include "foundation/PxBitMap.h"
#include "foundation/PxArray.h"
#include "CmPriorityQueue.h"
#include "CmBlockArray.h"
#include "PxNodeIndex.h"

namespace physx
{
struct PartitionEdge;

namespace IG
{
#define IG_INVALID_ISLAND 0xFFFFFFFFu
#define IG_INVALID_EDGE 0xFFFFFFFFu
#define IG_LIMIT_DIRTY_NODES 0
#define IG_SANITY_CHECKS 0

typedef PxU32 IslandId;
typedef PxU32 EdgeIndex;
typedef PxU32 EdgeInstanceIndex;

struct Edge
{
	//Edge instances can be implicitly calculated based on this edge index, which is an offset into the array of edges.
	//From that, the child edge index is simply the 
	//The constraint or contact referenced by this edge

	enum EdgeType
	{
		eCONTACT_MANAGER,
		eCONSTRAINT,
		eSOFT_BODY_CONTACT,
		eFEM_CLOTH_CONTACT, 
		ePARTICLE_SYSTEM_CONTACT,
		eEDGE_TYPE_COUNT
	};
	
	enum EdgeState
	{
		eINSERTED			= 1<<0,
		ePENDING_DESTROYED	= 1<<1,
		eACTIVE				= 1<<2,
		eIN_DIRTY_LIST		= 1<<3,
		eDESTROYED			= 1<<4,
		eREPORT_ONLY_DESTROY= 1<<5,
		eACTIVATING			= 1<<6
	};

	PxU16 mEdgeType;	// PT: EdgeType. Could be PxU8.
	PxU16 mEdgeState;	// PT: could be PxU8.
	
	EdgeIndex mNextIslandEdge, mPrevIslandEdge;

	PX_FORCE_INLINE void setInserted()				{ mEdgeState |= eINSERTED;				}
	PX_FORCE_INLINE void clearInserted()			{ mEdgeState &= ~eINSERTED;				}
	PX_FORCE_INLINE void clearDestroyed()			{ mEdgeState &= ~eDESTROYED;			}
	PX_FORCE_INLINE void setPendingDestroyed()		{ mEdgeState |= ePENDING_DESTROYED;		}
	PX_FORCE_INLINE void clearPendingDestroyed()	{ mEdgeState &= ~ePENDING_DESTROYED;	}
	PX_FORCE_INLINE void activateEdge()				{ mEdgeState |= eACTIVE;				}
	PX_FORCE_INLINE void deactivateEdge()			{ mEdgeState &= ~eACTIVE;				}
	PX_FORCE_INLINE void markInDirtyList()			{ mEdgeState |= eIN_DIRTY_LIST;			}
	PX_FORCE_INLINE void clearInDirtyList()			{ mEdgeState &= ~eIN_DIRTY_LIST;		}
	PX_FORCE_INLINE void setReportOnlyDestroy()		{ mEdgeState |= eREPORT_ONLY_DESTROY;	}
public:
	Edge() : mEdgeType(Edge::eCONTACT_MANAGER), mEdgeState(eDESTROYED),
		mNextIslandEdge(IG_INVALID_EDGE), mPrevIslandEdge(IG_INVALID_EDGE)
	{
	}
	PX_FORCE_INLINE PxIntBool	isInserted()			const	{ return PxIntBool(mEdgeState & eINSERTED);				}
	PX_FORCE_INLINE PxIntBool	isDestroyed()			const	{ return PxIntBool(mEdgeState & eDESTROYED);			}
	PX_FORCE_INLINE PxIntBool	isPendingDestroyed()	const	{ return PxIntBool(mEdgeState & ePENDING_DESTROYED);	}
	PX_FORCE_INLINE PxIntBool	isActive()				const	{ return PxIntBool(mEdgeState & eACTIVE);				}
	PX_FORCE_INLINE PxIntBool	isInDirtyList()			const	{ return PxIntBool(mEdgeState & eIN_DIRTY_LIST);		}
	PX_FORCE_INLINE PxIntBool	isReportOnlyDestroy()	const	{ return PxIntBool(mEdgeState & eREPORT_ONLY_DESTROY);	}
	PX_FORCE_INLINE EdgeType	getEdgeType()			const	{ return EdgeType(mEdgeType);							}
};

struct EdgeInstance
{
	EdgeInstanceIndex mNextEdge, mPrevEdge; //The next edge instance in this node's list of edge instances

	EdgeInstance() : mNextEdge(IG_INVALID_EDGE), mPrevEdge(IG_INVALID_EDGE)
	{
	}
};

template<typename Handle>
class HandleManager
{
	PxArray<Handle> mFreeHandles;
	Handle mCurrentHandle;

public:

	HandleManager() : mFreeHandles("FreeHandles"), mCurrentHandle(0)
	{
	}

	~HandleManager(){}

	Handle getHandle()
	{
		if(mFreeHandles.size())
		{
			Handle handle = mFreeHandles.popBack();
			PX_ASSERT(isValidHandle(handle));
			return handle;
		}
		return mCurrentHandle++;				
	}

	bool isNotFreeHandle(Handle handle)	const
	{
		for(PxU32 a = 0; a < mFreeHandles.size(); ++a)
		{
			if(mFreeHandles[a] == handle)
				return false;
		}
		return true;
	}

	void freeHandle(Handle handle)
	{
		PX_ASSERT(isValidHandle(handle));
		PX_ASSERT(isNotFreeHandle(handle));
		if(handle == mCurrentHandle)
			mCurrentHandle--;
		else
			mFreeHandles.pushBack(handle);
	}

	bool isValidHandle(Handle handle)	const
	{
		return handle < mCurrentHandle;
	}

	PX_FORCE_INLINE PxU32 getTotalHandles() const { return mCurrentHandle; }
};

class Node
{
public:
	enum NodeType
	{
		eRIGID_BODY_TYPE,
		eARTICULATION_TYPE,
		eDEFORMABLE_SURFACE_TYPE,
		eDEFORMABLE_VOLUME_TYPE,
		ePARTICLESYSTEM_TYPE,
		eTYPE_COUNT
	};

	enum State
	{
		eREADY_FOR_SLEEPING = 1u << 0,	//! Ready to go to sleep
		eACTIVE				= 1u << 1,	//! Active
		eKINEMATIC			= 1u << 2,	//! Kinematic
		eDELETED			= 1u << 3,	//! Is pending deletion
		eDIRTY				= 1u << 4,	//! Is dirty (i.e. lost a connection)
		eACTIVATING			= 1u << 5	//! Is in the activating list
	};

	EdgeInstanceIndex mFirstEdgeIndex;

	PxU8 mFlags;
	PxU8 mType;
	PxU16 mStaticTouchCount;
	//PxU32 mActiveNodeIndex; //! Look-up for this node in the active nodes list, activating list or deactivating list...

	PxNodeIndex mNextNode, mPrevNode;

	//A counter for the number of active references to this body. Whenever an edge is activated, this is incremented. 
	//Whenver an edge is deactivated, this is decremented. This is used for kinematic bodies to determine if they need
	//to be in the active kinematics list
	PxU32 mActiveRefCount;
	
	//A node can correspond with one kind of user-defined object
	void*	mObject;

	PX_FORCE_INLINE Node() : mType(eRIGID_BODY_TYPE)	{ reset();	}
	PX_FORCE_INLINE ~Node()								{			}

	PX_FORCE_INLINE void reset()
	{
		mFirstEdgeIndex = IG_INVALID_EDGE;
		mFlags = eDELETED;
		mObject = NULL;
		mActiveRefCount = 0;
		mStaticTouchCount = 0;
	}

	PX_FORCE_INLINE void setActive()			{ mFlags |= eACTIVE;			}
	PX_FORCE_INLINE void clearActive()			{ mFlags &= ~eACTIVE;			}

	PX_FORCE_INLINE void setActivating()		{ mFlags |= eACTIVATING;		}
	PX_FORCE_INLINE void clearActivating()		{ mFlags &= ~eACTIVATING;		}

	//Activates a body/node.
	PX_FORCE_INLINE void setIsReadyForSleeping()	{ mFlags |= eREADY_FOR_SLEEPING;						}
	PX_FORCE_INLINE void clearIsReadyForSleeping()	{ mFlags &= (~eREADY_FOR_SLEEPING);						}
	PX_FORCE_INLINE void setIsDeleted()				{ mFlags |= eDELETED;									}
	PX_FORCE_INLINE void setKinematicFlag()			{ PX_ASSERT(!isKinematic());	mFlags |= eKINEMATIC;	}
	PX_FORCE_INLINE void clearKinematicFlag()		{ PX_ASSERT(isKinematic());	mFlags &= (~eKINEMATIC);	}
	PX_FORCE_INLINE void markDirty()				{ mFlags |= eDIRTY;										}
	PX_FORCE_INLINE void clearDirty()				{ mFlags &= (~eDIRTY);									}

public:

	PX_FORCE_INLINE PxIntBool isActive()				const { return PxIntBool(mFlags & eACTIVE);					}
	PX_FORCE_INLINE PxIntBool isActiveOrActivating()	const { return PxIntBool(mFlags & (eACTIVE | eACTIVATING));	}
	PX_FORCE_INLINE PxIntBool isActivating()			const { return PxIntBool(mFlags & eACTIVATING);				}
	PX_FORCE_INLINE PxIntBool isKinematic()				const { return PxIntBool(mFlags & eKINEMATIC);				}
	PX_FORCE_INLINE PxIntBool isDeleted()				const { return PxIntBool(mFlags & eDELETED);				}
	PX_FORCE_INLINE PxIntBool isDirty()					const { return PxIntBool(mFlags & eDIRTY);					}
	PX_FORCE_INLINE PxIntBool isReadyForSleeping()		const { return PxIntBool(mFlags & eREADY_FOR_SLEEPING);		}
	PX_FORCE_INLINE NodeType getNodeType()				const { return NodeType(mType);								}
};

struct Island
{
	PxNodeIndex mRootNode;
	PxNodeIndex mLastNode;
	PxU32 mNodeCount[Node::eTYPE_COUNT];
	PxU32 mActiveIndex;

	EdgeIndex mFirstEdge[Edge::eEDGE_TYPE_COUNT], mLastEdge[Edge::eEDGE_TYPE_COUNT];
	PxU32 mEdgeCount[Edge::eEDGE_TYPE_COUNT];

	Island() : mActiveIndex(IG_INVALID_ISLAND)
	{
		for(PxU32 a = 0; a < Edge::eEDGE_TYPE_COUNT; ++a)
		{
			mFirstEdge[a] = IG_INVALID_EDGE;
			mLastEdge[a] = IG_INVALID_EDGE;
			mEdgeCount[a] = 0;
		}

		for(PxU32 a = 0; a < Node::eTYPE_COUNT; ++a)
		{
			mNodeCount[a] = 0;
		}
	}
};

struct TraversalState
{
	PxNodeIndex mNodeIndex;
	PxU32 mCurrentIndex;
	PxU32 mPrevIndex;
	PxU32 mDepth;

	TraversalState()
	{
	}

	TraversalState(	PxNodeIndex nodeIndex, PxU32 currentIndex, PxU32 prevIndex, PxU32 depth) : 
					mNodeIndex(nodeIndex), mCurrentIndex(currentIndex), mPrevIndex(prevIndex), mDepth(depth)
	{
	}
};

struct QueueElement
{
	TraversalState* mState;
	PxU32 mHopCount;

	QueueElement()
	{
	}

	QueueElement(TraversalState* state, PxU32 hopCount) : mState(state), mHopCount(hopCount)
	{
	}
};

struct NodeComparator
{
	NodeComparator()
	{
	}

	bool operator() (const QueueElement& node0, const QueueElement& node1) const
	{
		return node0.mHopCount < node1.mHopCount;
	}
private:
	NodeComparator& operator = (const NodeComparator&);
};

// PT: island-manager data used by both CPU & GPU code.
// This is managed by external code (e.g. SimpleIslandManager) and passed as const data to IslandSim.
class CPUExternalData
{
	public:

	PX_FORCE_INLINE PxNodeIndex	getNodeIndex1(IG::EdgeIndex index) const { return mEdgeNodeIndices[2 * index];		}
	PX_FORCE_INLINE PxNodeIndex	getNodeIndex2(IG::EdgeIndex index) const { return mEdgeNodeIndices[2 * index + 1];	}

	//KS - stores node indices for a given edge. Node index 0 is at 2* edgeId and NodeIndex1 is at 2*edgeId + 1
	//can also be used for edgeInstance indexing so there's no need to figure out outboundNode ID either!
	Cm::BlockArray<PxNodeIndex>	mEdgeNodeIndices;
};

// PT: island-manager data only needed for the GPU version, but stored in CPU code.
// This is managed by external code (e.g. SimpleIslandManager) and passed as non-const data to only one of the IslandSims.
// (It is otherwise optional). IslandSim will create/update this data during island gen.
class GPUExternalData
{
	public:
	GPUExternalData() :
		mFirstPartitionEdges		("mFirstPartitionEdges"), 
		mDestroyedPartitionEdges	("mDestroyedPartitionEdges"),
		mNpIndexPtr					(NULL)
	{
	}

	///////////////////////////////////////////////////////////////////////////

	PX_FORCE_INLINE PartitionEdge*				getFirstPartitionEdge(IG::EdgeIndex edgeIndex)							const { return mFirstPartitionEdges[edgeIndex];				}
	PX_FORCE_INLINE void						setFirstPartitionEdge(IG::EdgeIndex edgeIndex, PartitionEdge* partitionEdge)  { mFirstPartitionEdges[edgeIndex] = partitionEdge;	}

					PxArray<PartitionEdge*>		mFirstPartitionEdges;

	///////////////////////////////////////////////////////////////////////////

	PX_FORCE_INLINE PxU32						getNbDestroyedPartitionEdges()	const	{ return mDestroyedPartitionEdges.size();		}
	PX_FORCE_INLINE const PartitionEdge*const*	getDestroyedPartitionEdges()	const	{ return mDestroyedPartitionEdges.begin();		}
	PX_FORCE_INLINE PartitionEdge**				getDestroyedPartitionEdges()			{ return mDestroyedPartitionEdges.begin();		}
	PX_FORCE_INLINE	void						clearDestroyedPartitionEdges()			{ mDestroyedPartitionEdges.forceSize_Unsafe(0);	}

					PxArray<PartitionEdge*>		mDestroyedPartitionEdges;

	///////////////////////////////////////////////////////////////////////////

	PX_FORCE_INLINE const PxBitMap&				getActiveContactManagerBitmap() const	{ return mActiveContactEdges;	}

					PxBitMap					mActiveContactEdges;

	///////////////////////////////////////////////////////////////////////////

	// PT: these ones are strange, used to store an unrelated ptr from the outside, and only for GPU
	PX_FORCE_INLINE void						setEdgeNodeIndexPtr(PxU32* ptr)		{ mNpIndexPtr = ptr;	}
	PX_FORCE_INLINE PxU32*						getEdgeNodeIndexPtr()		const	{ return mNpIndexPtr;	}

					PxU32*						mNpIndexPtr;
};

class IslandSim
{
	PX_NOCOPY(IslandSim)

	HandleManager<IslandId>							mIslandHandles;								//! Handle manager for islands

	// PT: these arrays are parallel, all indexed by PxNodeIndex::index()
	PxArray<Node>									mNodes;										//! The nodes used in the constraint graph
	PxArray<PxU32>									mActiveNodeIndex;							//! The active node index for each node
	PxArray<PxU32>									mHopCounts;									//! The observed number of "hops" from a given node to its root node. May be inaccurate but used to accelerate searches.
	PxArray<PxNodeIndex>							mFastRoute;									//! The observed last route from a given node to the root node. We try the fast route (unless its broken) before trying others.
	PxArray<IslandId>								mIslandIds;									//! The array of per-node island ids
	//

	Cm::BlockArray<Edge>							mEdges;
	Cm::BlockArray<EdgeInstance>					mEdgeInstances;								//! Edges used to connect nodes in the constraint graph
	PxArray<Island>									mIslands;									//! The array of islands
	PxArray<PxU32>									mIslandStaticTouchCount;					//! Array of static touch counts per-island

	PxArray<PxNodeIndex>							mActiveNodes[Node::eTYPE_COUNT];			//! An array of active nodes
	PxArray<PxNodeIndex>							mActiveKinematicNodes;						//! An array of active or referenced kinematic nodes
	PxArray<EdgeIndex>								mActivatedEdges[Edge::eEDGE_TYPE_COUNT];	//! An array of active edges

	PxU32											mActiveEdgeCount[Edge::eEDGE_TYPE_COUNT];
	
	PxBitMap										mIslandAwake;								//! Indicates whether an island is awake or not

	//An array of active islands
	PxArray<IslandId>								mActiveIslands;

	PxU32											mInitialActiveNodeCount[Edge::eEDGE_TYPE_COUNT];

	PxArray<PxNodeIndex>							mNodesToPutToSleep[Node::eTYPE_COUNT];

	//Input to this frame's island management (changed nodes/edges)

	//Input list of changes observed this frame. If there no changes, no work to be done.
	PxArray<EdgeIndex>								mDirtyEdges[Edge::eEDGE_TYPE_COUNT];
	//Dirty nodes. These nodes lost at least one connection so we need to recompute islands from these nodes
	//PxArray<NodeIndex>							mDirtyNodes;
	PxBitMap										mDirtyMap;
#if IG_LIMIT_DIRTY_NODES
	PxU32											mLastMapIndex;
#endif
	//An array of nodes to activate
	PxArray<PxNodeIndex>							mActivatingNodes;
	PxArray<EdgeIndex>								mDestroyedEdges;

	//Temporary, transient data used for traversals. TODO - move to PxsSimpleIslandManager. Or if we keep it here, we can 
	//process multiple island simulations in parallel
	Cm::PriorityQueue<QueueElement, NodeComparator>	mPriorityQueue;								//! Priority queue used for graph traversal
	PxArray<TraversalState>							mVisitedNodes;								//! The list of nodes visited in the current traversal
	PxBitMap										mVisitedState;								//! Indicates whether a node has been visited
	PxArray<EdgeIndex>								mIslandSplitEdges[Edge::eEDGE_TYPE_COUNT];

	PxArray<EdgeIndex>								mDeactivatingEdges[Edge::eEDGE_TYPE_COUNT];
public:
	// PT: we could perhaps instead pass these as param whenever needed. The coupling otherwise makes it more difficult to unit-test IslandSim in isolation.
	const CPUExternalData&							mCpuData;	// PT: from the simple island manager, shared between accurate/speculative island sim
	GPUExternalData*								mGpuData;	// PT: from the simple island manager, for accurate island sim (null otherwise) and only needed for the GPU version.
protected:

	const PxU64										mContextId;

public:

	IslandSim(const CPUExternalData& cpuData, GPUExternalData* gpuData, PxU64 contextID);
	~IslandSim() {}

	void addNode(bool isActive, bool isKinematic, Node::NodeType type, PxNodeIndex nodeIndex, void* object);

	void activateNode(PxNodeIndex index);
	void deactivateNode(PxNodeIndex index);
	void putNodeToSleep(PxNodeIndex index);

	void removeConnection(EdgeIndex edgeIndex);

	PX_FORCE_INLINE PxU32				getNbActiveNodes(Node::NodeType type)			const { return mActiveNodes[type].size();	}
	PX_FORCE_INLINE const PxNodeIndex*	getActiveNodes(Node::NodeType type)				const { return mActiveNodes[type].begin();	}

	PX_FORCE_INLINE PxU32				getNbActiveKinematics()							const { return mActiveKinematicNodes.size();	}
	PX_FORCE_INLINE const PxNodeIndex*	getActiveKinematics()							const { return mActiveKinematicNodes.begin();	}

	PX_FORCE_INLINE PxU32				getNbNodesToActivate(Node::NodeType type)		const { return mActiveNodes[type].size() - mInitialActiveNodeCount[type];	}
	PX_FORCE_INLINE const PxNodeIndex*	getNodesToActivate(Node::NodeType type)			const { return mActiveNodes[type].begin() + mInitialActiveNodeCount[type];	}

	PX_FORCE_INLINE PxU32				getNbNodesToDeactivate(Node::NodeType type)		const { return mNodesToPutToSleep[type].size();		}
	PX_FORCE_INLINE const PxNodeIndex*	getNodesToDeactivate(Node::NodeType type)		const { return mNodesToPutToSleep[type].begin();	}

	PX_FORCE_INLINE PxU32				getNbActivatedEdges(Edge::EdgeType type)		const { return mActivatedEdges[type].size();	} 
	PX_FORCE_INLINE const EdgeIndex*	getActivatedEdges(Edge::EdgeType type)			const { return mActivatedEdges[type].begin();	}

	PX_FORCE_INLINE PxU32				getNbActiveEdges(Edge::EdgeType type)			const { return mActiveEdgeCount[type];			}

	PX_FORCE_INLINE void* getObject(PxNodeIndex nodeIndex, Node::NodeType type) const
	{
		const Node& node = mNodes[nodeIndex.index()];
		PX_ASSERT(node.mType == type);
		PX_UNUSED(type);
		return node.mObject;
	}

	PX_FORCE_INLINE void clearDeactivations()
	{
		for (PxU32 i = 0; i < Node::eTYPE_COUNT; ++i)
		{
			mNodesToPutToSleep[i].forceSize_Unsafe(0);
			mDeactivatingEdges[i].forceSize_Unsafe(0);
		}
	}

	PX_FORCE_INLINE const Island&				getIsland(IG::IslandId islandIndex)		const { return mIslands[islandIndex]; }
	PX_FORCE_INLINE const Island&				getIsland(const PxNodeIndex& nodeIndex)	const { PX_ASSERT(mIslandIds[nodeIndex.index()] != IG_INVALID_ISLAND); return mIslands[mIslandIds[nodeIndex.index()]]; }

	PX_FORCE_INLINE PxU32						getNbActiveIslands()	const	{ return mActiveIslands.size();		}
	PX_FORCE_INLINE const IslandId*				getActiveIslands()		const	{ return mActiveIslands.begin();	}

	PX_FORCE_INLINE PxU32						getNbDeactivatingEdges(const IG::Edge::EdgeType edgeType)	const	{ return mDeactivatingEdges[edgeType].size();	}
	PX_FORCE_INLINE const EdgeIndex*			getDeactivatingEdges(const IG::Edge::EdgeType edgeType)		const	{ return mDeactivatingEdges[edgeType].begin();	}

	// PT: this is not actually used externally
	//PX_FORCE_INLINE PxU32						getNbDestroyedEdges()	const	{ return mDestroyedEdges.size();	}
	//PX_FORCE_INLINE const EdgeIndex*			getDestroyedEdges()		const	{ return mDestroyedEdges.begin();	}

	// PT: this is not actually used externally. Still used internally in IslandSim.
	//PX_FORCE_INLINE PxU32						getNbDirtyEdges(IG::Edge::EdgeType type)	const	{ return mDirtyEdges[type].size();	}
	//PX_FORCE_INLINE const EdgeIndex*			getDirtyEdges(IG::Edge::EdgeType type)		const	{ return mDirtyEdges[type].begin();	}

	PX_FORCE_INLINE PxU32						getNbEdges()					const	{ return mEdges.size();		}
	PX_FORCE_INLINE const Edge&					getEdge(EdgeIndex edgeIndex)	const	{ return mEdges[edgeIndex];	}
	PX_FORCE_INLINE Edge&						getEdge(EdgeIndex edgeIndex)			{ return mEdges[edgeIndex];	}

	PX_FORCE_INLINE PxU32						getNbNodes()							const { return mNodes.size();				}
	PX_FORCE_INLINE const Node&					getNode(const PxNodeIndex& nodeIndex)	const { return mNodes[nodeIndex.index()];	}

	PX_FORCE_INLINE PxU32						getActiveNodeIndex(const PxNodeIndex& nodeIndex)	const { return mActiveNodeIndex[nodeIndex.index()];	}
	PX_FORCE_INLINE const PxU32*				getActiveNodeIndex()								const { return mActiveNodeIndex.begin();			}
	//PX_FORCE_INLINE PxU32						getNbActiveNodeIndex()								const { return mActiveNodeIndex.size();				}
	
	PX_FORCE_INLINE	PxU32						getNbIslands()				const { return mIslandStaticTouchCount.size(); }
	PX_FORCE_INLINE	const PxU32*				getIslandStaticTouchCount()	const { return mIslandStaticTouchCount.begin(); }
	PX_FORCE_INLINE PxU32						getIslandStaticTouchCount(const PxNodeIndex& nodeIndex) const
												{
													PX_ASSERT(mIslandIds[nodeIndex.index()] != IG_INVALID_ISLAND);
													return mIslandStaticTouchCount[mIslandIds[nodeIndex.index()]];
												}

	PX_FORCE_INLINE	const IG::IslandId*			getIslandIds()				const { return mIslandIds.begin(); }

	PX_FORCE_INLINE	PxU64						getContextId()				const { return mContextId;	}

	void setKinematic(PxNodeIndex nodeIndex);

	void setDynamic(PxNodeIndex nodeIndex);

	bool checkInternalConsistency() const;

	PX_INLINE void activateNode_ForGPUSolver(PxNodeIndex index)
	{
		IG::Node& node = mNodes[index.index()];
		node.clearIsReadyForSleeping(); //Clear the "isReadyForSleeping" flag. Just in case it was set
	}
	PX_INLINE void deactivateNode_ForGPUSolver(PxNodeIndex index)
	{
		IG::Node& node = mNodes[index.index()];
		node.setIsReadyForSleeping();
	}

	// PT: these three functions added for multithreaded implementation of Sc::Scene::islandInsertion
	void preallocateConnections(EdgeIndex handle);
	bool addConnectionPreallocated(PxNodeIndex nodeHandle1, PxNodeIndex nodeHandle2, Edge::EdgeType edgeType, EdgeIndex handle);
	void addDelayedDirtyEdges(PxU32 nbHandles, const EdgeIndex* handles);

	// PT: called by SimpleIslandManager. Made public to remove friendship, make the API clearer, and unit-testable.
	void addConnection(PxNodeIndex nodeHandle1, PxNodeIndex nodeHandle2, Edge::EdgeType edgeType, EdgeIndex handle);
	void wakeIslands();	// PT: this is always followed by a call to processNewEdges(). Merge the two?
	void wakeIslands2();
	void processNewEdges();

	// PT: called by ThirdPassTask::runInternal. Made public to remove friendship, make the API clearer, and unit-testable.
	void removeDestroyedEdges();	// PT: this is always followed by a call to processLostEdges(). Merge the two?
	void processLostEdges(const PxArray<PxNodeIndex>& destroyedNodes, bool allowDeactivation, bool permitKinematicDeactivation, PxU32 dirtyNodeLimit);

private:
	void wakeIslandsInternal(bool flag);

	void insertNewEdges();

	void removeConnectionInternal(EdgeIndex edgeIndex);

	void addConnectionToGraph(EdgeIndex index);
	void removeConnectionFromGraph(EdgeIndex edgeIndex);

	//Merges 2 islands together. The returned id is the id of the merged island
	IslandId mergeIslands(IslandId island0, IslandId island1, PxNodeIndex node0, PxNodeIndex node1);

	void mergeIslandsInternal(Island& island0, Island& island1, IslandId islandId0, IslandId islandId1, PxNodeIndex node0, PxNodeIndex node1);
	
	void unwindRoute(PxU32 traversalIndex, PxNodeIndex lastNode, PxU32 hopCount, IslandId id);

	void activateIslandInternal(const Island& island);

	void activateIsland(IslandId island);

	void deactivateIsland(IslandId island);

#if IG_SANITY_CHECKS
	bool canFindRoot(PxNodeIndex startNode, PxNodeIndex targetNode, PxArray<PxNodeIndex>* visitedNodes);
#endif
	bool tryFastPath(PxNodeIndex startNode, PxNodeIndex targetNode, IslandId islandId);

	bool findRoute(PxNodeIndex startNode, PxNodeIndex targetNode, IslandId islandId);

#if PX_DEBUG
	bool isPathTo(PxNodeIndex startNode, PxNodeIndex targetNode)	const;
#endif
	void activateNodeInternal(PxNodeIndex index);
	void deactivateNodeInternal(PxNodeIndex index);

	PX_FORCE_INLINE void makeEdgeActive(EdgeInstanceIndex index, bool testEdgeType);

	IslandId addNodeToIsland(PxNodeIndex nodeIndex1, PxNodeIndex nodeIndex2, IslandId islandId2, bool active1, bool active2);

/*	PX_FORCE_INLINE  void notifyReadyForSleeping(const PxNodeIndex nodeIndex)
	{
		Node& node = mNodes[nodeIndex.index()];
		//PX_ASSERT(node.isActive());
		node.setIsReadyForSleeping();
	}

	PX_FORCE_INLINE  void notifyNotReadyForSleeping(const PxNodeIndex nodeIndex)
	{
		Node& node = mNodes[nodeIndex.index()];
		PX_ASSERT(node.isActive() || node.isActivating());
		node.clearIsReadyForSleeping();
	}*/

	PX_FORCE_INLINE void markIslandActive(IslandId islandId)
	{
		Island& island = mIslands[islandId];
		PX_ASSERT(!mIslandAwake.test(islandId));
		PX_ASSERT(island.mActiveIndex == IG_INVALID_ISLAND);

		mIslandAwake.set(islandId);
		island.mActiveIndex = mActiveIslands.size();
		mActiveIslands.pushBack(islandId);
	}

	PX_FORCE_INLINE void markIslandInactive(IslandId islandId)
	{
		Island& island = mIslands[islandId];
		PX_ASSERT(mIslandAwake.test(islandId));
		PX_ASSERT(island.mActiveIndex != IG_INVALID_ISLAND);
		PX_ASSERT(mActiveIslands[island.mActiveIndex] == islandId);
		IslandId replaceId = mActiveIslands[mActiveIslands.size()-1];
		PX_ASSERT(mIslandAwake.test(replaceId));
		Island& replaceIsland = mIslands[replaceId];
		replaceIsland.mActiveIndex = island.mActiveIndex;
		mActiveIslands[island.mActiveIndex] = replaceId;
		mActiveIslands.forceSize_Unsafe(mActiveIslands.size()-1);
		island.mActiveIndex = IG_INVALID_ISLAND;
		mIslandAwake.reset(islandId);
	}

	PX_FORCE_INLINE void markKinematicActive(PxNodeIndex nodeIndex)
	{
		const PxU32 index = nodeIndex.index();
		const Node& node = mNodes[index];
		PX_ASSERT(node.isKinematic());
		if(node.mActiveRefCount == 0 && mActiveNodeIndex[index] == PX_INVALID_NODE)
		{
			//PX_ASSERT(mActiveNodeIndex[index] == PX_INVALID_NODE);
			//node.mActiveNodeIndex = mActiveKinematicNodes.size();
			mActiveNodeIndex[index] = mActiveKinematicNodes.size();
			mActiveKinematicNodes.pushBack(nodeIndex);
		}
	}

	PX_FORCE_INLINE void markKinematicInactive(PxNodeIndex nodeIndex)
	{
		const PxU32 index = nodeIndex.index();
		const Node& node = mNodes[index];
		PX_ASSERT(node.isKinematic());
		PX_ASSERT(mActiveNodeIndex[index] != PX_INVALID_NODE);
		PX_ASSERT(mActiveKinematicNodes[mActiveNodeIndex[index]].index() == index);

		if(node.mActiveRefCount == 0)
		{
			//Only remove from active kinematic list if it has no active contacts referencing it *and* it is asleep
			if(mActiveNodeIndex[index] != PX_INVALID_NODE)
			{
				//Need to verify active node index because there is an edge case where a node could be woken, then put to 
				//sleep in the same frame. This would mean that it would not have an active index at this stage.
				PxNodeIndex replaceIndex = mActiveKinematicNodes.back();
				PX_ASSERT(mActiveNodeIndex[replaceIndex.index()] == mActiveKinematicNodes.size() - 1);
				mActiveNodeIndex[replaceIndex.index()] = mActiveNodeIndex[index];
				mActiveKinematicNodes[mActiveNodeIndex[index]] = replaceIndex;
				mActiveKinematicNodes.forceSize_Unsafe(mActiveKinematicNodes.size() - 1);
				mActiveNodeIndex[index] = PX_INVALID_NODE;
			}
		}
	}

	PX_FORCE_INLINE void markActive(PxNodeIndex nodeIndex)
	{
		const PxU32 index = nodeIndex.index();
		const Node& node = mNodes[index];
		PX_ASSERT(!node.isKinematic());
		PX_ASSERT(mActiveNodeIndex[index] == PX_INVALID_NODE);
		mActiveNodeIndex[index] = mActiveNodes[node.mType].size();
		mActiveNodes[node.mType].pushBack(nodeIndex);
	}

	PX_FORCE_INLINE void markInactive(PxNodeIndex nodeIndex)
	{
		const PxU32 index = nodeIndex.index();
		const Node& node = mNodes[index];

		PX_ASSERT(!node.isKinematic());
		PX_ASSERT(mActiveNodeIndex[index] != PX_INVALID_NODE);

		PxArray<PxNodeIndex>& activeNodes = mActiveNodes[node.mType];

		PX_ASSERT(activeNodes[mActiveNodeIndex[index]].index() == index);
		const PxU32 initialActiveNodeCount = mInitialActiveNodeCount[node.mType];

		if(mActiveNodeIndex[index] < initialActiveNodeCount)
		{
			//It's in the initial active node set. We retain a list of active nodes, where the existing active nodes
			//are at the beginning of the array and the newly activated nodes are at the end of the array...
			//The solution is to move the node to the end of the initial active node list in this case
			PxU32 activeNodeIndex = mActiveNodeIndex[index];
			PxNodeIndex replaceIndex = activeNodes[initialActiveNodeCount - 1];
			PX_ASSERT(mActiveNodeIndex[replaceIndex.index()] == initialActiveNodeCount - 1);
			mActiveNodeIndex[index] = mActiveNodeIndex[replaceIndex.index()];
			mActiveNodeIndex[replaceIndex.index()] = activeNodeIndex;
			activeNodes[activeNodeIndex] = replaceIndex;
			activeNodes[mActiveNodeIndex[index]] = nodeIndex;
			mInitialActiveNodeCount[node.mType]--;	
		}

		PX_ASSERT(!node.isKinematic());
		PX_ASSERT(mActiveNodeIndex[index] != PX_INVALID_NODE);
		PX_ASSERT(activeNodes[mActiveNodeIndex[index]].index() == index);

		PxNodeIndex replaceIndex = activeNodes.back();
		PX_ASSERT(mActiveNodeIndex[replaceIndex.index()] == activeNodes.size() - 1);
		mActiveNodeIndex[replaceIndex.index()] = mActiveNodeIndex[index];
		activeNodes[mActiveNodeIndex[index]] = replaceIndex;
		activeNodes.forceSize_Unsafe(activeNodes.size() - 1);
		mActiveNodeIndex[index] = PX_INVALID_NODE;			
	}

	PX_FORCE_INLINE void markEdgeActive(EdgeIndex index, PxNodeIndex nodeIndex1, PxNodeIndex nodeIndex2)
	{
		Edge& edge = mEdges[index];

		PX_ASSERT((edge.mEdgeState & Edge::eACTIVATING) == 0);

		edge.mEdgeState |= Edge::eACTIVATING;

		mActivatedEdges[edge.mEdgeType].pushBack(index);

		mActiveEdgeCount[edge.mEdgeType]++;
		
		//Set the active bit...
		if(mGpuData && edge.mEdgeType == Edge::eCONTACT_MANAGER)
			mGpuData->mActiveContactEdges.set(index);

		const PxU32 index1 = nodeIndex1.index();
		const PxU32 index2 = nodeIndex2.index();

		if (index1 != PX_INVALID_NODE && index2 != PX_INVALID_NODE)
		{
			PX_ASSERT((!mNodes[index1].isKinematic()) || (!mNodes[index2].isKinematic()) || edge.getEdgeType() == IG::Edge::eCONTACT_MANAGER);
			{
				Node& node = mNodes[index1];
				if(node.mActiveRefCount == 0 && node.isKinematic() && !node.isActiveOrActivating())
					markKinematicActive(nodeIndex1);	//Add to active kinematic list

				node.mActiveRefCount++;
			}

			{
				Node& node = mNodes[index2];
				if(node.mActiveRefCount == 0 && node.isKinematic() && !node.isActiveOrActivating())
					markKinematicActive(nodeIndex2);	//Add to active kinematic list

				node.mActiveRefCount++;
			}
		}
	}

	void removeEdgeFromActivatingList(EdgeIndex index);

	PX_FORCE_INLINE void removeEdgeFromIsland(Island& island, EdgeIndex edgeIndex)
	{
		Edge& edge = mEdges[edgeIndex];
		if(edge.mNextIslandEdge != IG_INVALID_EDGE)
		{
			PX_ASSERT(mEdges[edge.mNextIslandEdge].mPrevIslandEdge == edgeIndex);
			mEdges[edge.mNextIslandEdge].mPrevIslandEdge = edge.mPrevIslandEdge;
		}
		else
		{
			PX_ASSERT(island.mLastEdge[edge.mEdgeType] == edgeIndex);
			island.mLastEdge[edge.mEdgeType] = edge.mPrevIslandEdge;
		}

		if(edge.mPrevIslandEdge != IG_INVALID_EDGE)
		{
			PX_ASSERT(mEdges[edge.mPrevIslandEdge].mNextIslandEdge == edgeIndex);
			mEdges[edge.mPrevIslandEdge].mNextIslandEdge = edge.mNextIslandEdge;
		}
		else
		{
			PX_ASSERT(island.mFirstEdge[edge.mEdgeType] == edgeIndex);
			island.mFirstEdge[edge.mEdgeType] = edge.mNextIslandEdge;
		}

		island.mEdgeCount[edge.mEdgeType]--;
		edge.mNextIslandEdge = edge.mPrevIslandEdge = IG_INVALID_EDGE;
	}

	PX_FORCE_INLINE void addEdgeToIsland(Island& island, EdgeIndex edgeIndex)
	{
		Edge& edge = mEdges[edgeIndex];
		PX_ASSERT(edge.mNextIslandEdge == IG_INVALID_EDGE && edge.mPrevIslandEdge == IG_INVALID_EDGE);

		if(island.mLastEdge[edge.mEdgeType] != IG_INVALID_EDGE)
		{
			PX_ASSERT(mEdges[island.mLastEdge[edge.mEdgeType]].mNextIslandEdge == IG_INVALID_EDGE);
			mEdges[island.mLastEdge[edge.mEdgeType]].mNextIslandEdge = edgeIndex;
		}
		else
		{
			PX_ASSERT(island.mFirstEdge[edge.mEdgeType] == IG_INVALID_EDGE);
			island.mFirstEdge[edge.mEdgeType] = edgeIndex;
		}

		edge.mPrevIslandEdge = island.mLastEdge[edge.mEdgeType];
		island.mLastEdge[edge.mEdgeType] = edgeIndex;
		island.mEdgeCount[edge.mEdgeType]++;
	}

	PX_FORCE_INLINE void removeNodeFromIsland(Island& island, PxNodeIndex nodeIndex)
	{
		Node& node = mNodes[nodeIndex.index()];
		if(node.mNextNode.isValid())
		{
			PX_ASSERT(mNodes[node.mNextNode.index()].mPrevNode.index() == nodeIndex.index());
			mNodes[node.mNextNode.index()].mPrevNode = node.mPrevNode;
		}
		else
		{
			PX_ASSERT(island.mLastNode.index() == nodeIndex.index());
			island.mLastNode = node.mPrevNode;
		}

		if(node.mPrevNode.isValid())
		{
			PX_ASSERT(mNodes[node.mPrevNode.index()].mNextNode.index() == nodeIndex.index());
			mNodes[node.mPrevNode.index()].mNextNode = node.mNextNode;
		}
		else
		{
			PX_ASSERT(island.mRootNode.index() == nodeIndex.index());
			island.mRootNode = node.mNextNode;
		}

		island.mNodeCount[node.mType]--;

		node.mNextNode = node.mPrevNode = PxNodeIndex();
	}
};
}
}

#endif
