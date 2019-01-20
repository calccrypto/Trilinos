//@HEADER
// ************************************************************************
//
//               ShyLU: Hybrid preconditioner package
//                 Copyright 2012 Sandia Corporation
//
// Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
// the U.S. Government retains certain rights in this software.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the Corporation nor the names of the
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY SANDIA CORPORATION "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SANDIA CORPORATION OR THE
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Questions? Contact Alexander Heinlein (alexander.heinlein@uni-koeln.de)
//
// ************************************************************************
//@HEADER

#ifndef _FROSCH_INTERFACEENTITY_DEF_HPP
#define _FROSCH_INTERFACEENTITY_DEF_HPP

#include <FROSch_InterfaceEntity_decl.hpp>

namespace FROSch {
    
    template <class SC,class LO,class GO>
    bool Node<SC,LO,GO>::operator< (const Node &n) const
    {
        return NodeIDGlobal_<n.NodeIDGlobal_;
    }
    
    template <class SC,class LO,class GO>
    bool Node<SC,LO,GO>::operator== (const Node &n) const
    {
        return NodeIDGlobal_==n.NodeIDGlobal_;
    }
    
    
    template <class SC,class LO,class GO,class NO>
    InterfaceEntity<SC,LO,GO,NO>::InterfaceEntity(EntityType type,
                                                  UN dofsPerNode,
                                                  UN multiplicity,
                                                  GO *subdomains,
                                                  EntityFlag flag) :
    Type_ (type),
    Flag_ (flag),
    NodeVector_ (0),
    SubdomainsVector_ (multiplicity),
    Ancestors_ (),
    Offspring_ (),
    CoarseNodes_ (),
    DistancesVector_ (0),
    DofsPerNode_ (dofsPerNode),
    Multiplicity_ (multiplicity),
    UniqueID_ (-1),
    LocalID_ (-1),
    CoarseNodeID_ (-1)
    {
        for (UN i=0; i<multiplicity; i++) {
            SubdomainsVector_[i] = subdomains[i];
        }
        sortunique(SubdomainsVector_);

        Ancestors_.reset(new EntitySet<SC,LO,GO,NO>(DefaultType));
        Offspring_.reset(new EntitySet<SC,LO,GO,NO>(DefaultType));
        CoarseNodes_.reset(new EntitySet<SC,LO,GO,NO>(DefaultType));
    }
    
    template <class SC,class LO,class GO,class NO>
    InterfaceEntity<SC,LO,GO,NO>::~InterfaceEntity()
    {
        
    } // Do we need sth here?
    
    template <class SC,class LO,class GO,class NO>
    int InterfaceEntity<SC,LO,GO,NO>::addNode(LO nodeIDGamma,
                                              LO nodeIDLocal,
                                              GO nodeIDGlobal,
                                              UN nDofs,
                                              const LOVecPtr dofsGamma,
                                              const LOVecPtr dofsLocal,
                                              const GOVecPtr dofsGlobal)
    {
        FROSCH_ASSERT(nDofs<=DofsPerNode_,"nDofs>NumDofs_.");
        
        FROSCH_ASSERT(dofsGamma.size()==nDofs,"dofIDs.size()!=nDofs");
        FROSCH_ASSERT(dofsLocal.size()==nDofs,"dofIDs.size()!=nDofs");
        FROSCH_ASSERT(dofsGlobal.size()==nDofs,"dofIDs.size()!=nDofs");

        Node<SC,LO,GO> node;
        
        node.NodeIDGamma_ = nodeIDGamma;
        node.NodeIDLocal_ = nodeIDLocal;
        node.NodeIDGlobal_ = nodeIDGlobal;
        
        node.DofsGamma_.deepCopy(dofsGamma());
        node.DofsLocal_.deepCopy(dofsLocal());
        node.DofsGlobal_.deepCopy(dofsGlobal());
        
        NodeVector_.push_back(node);
        return 0;
    }
    
    template <class SC,class LO,class GO,class NO>
    int InterfaceEntity<SC,LO,GO,NO>::addNode(const NodePtr &node)
    {
        return addNode(*node);
    }
    
    template <class SC,class LO,class GO,class NO>
    int InterfaceEntity<SC,LO,GO,NO>::addNode(const Node<SC,LO,GO> &node)
    {
        FROSCH_ASSERT(node.DofsGamma_.size()<=DofsPerNode_,"node.DofsGamma_ is too large.");
        FROSCH_ASSERT(node.DofsLocal_.size()<=DofsPerNode_,"node.DofsLocal_ is too large.");
        FROSCH_ASSERT(node.DofsGlobal_.size()<=DofsPerNode_,"node.DofsGlobal_ is too large.");
        
        NodeVector_.push_back(node);
        
        return 0;
    }
    
    template <class SC,class LO,class GO,class NO>
    int InterfaceEntity<SC,LO,GO,NO>::resetGlobalDofs(UN iD,
                                                      UN nDofs,
                                                      UN *dofIDs,
                                                      GO *dofsGlobal)
    {
        FROSCH_ASSERT(iD<getNumNodes(),"iD=>getNumNodes()");
        FROSCH_ASSERT(nDofs<=DofsPerNode_,"nDofs>DofsPerNode_.");
        
        for (unsigned i=0; i<nDofs; i++) {
            if ((0<=dofIDs[i])&&(dofIDs[i]<=DofsPerNode_)) {
                NodeVector_[iD].DofsGlobal_[dofIDs[i]] = dofsGlobal[dofIDs[i]];
            } else {
                FROSCH_ASSERT(false,"dofIDs[i] is out of range.");
            }
        }
        
        return 0;
    }
    
    template <class SC,class LO,class GO,class NO>
    int InterfaceEntity<SC,LO,GO,NO>::removeNode(UN iD)
    {
        FROSCH_ASSERT(iD<getNumNodes(),"iD=>getNumNodes()");
        NodeVector_.erase(NodeVector_.begin()+iD);
        return 0;
    }
    
    template <class SC,class LO,class GO,class NO>
    int InterfaceEntity<SC,LO,GO,NO>::sortByGlobalID()
    {
        sortunique(NodeVector_);
        return 0;
    }
    
    template <class SC,class LO,class GO,class NO>
    int InterfaceEntity<SC,LO,GO,NO>::setUniqueID(GO uniqueID)
    {
        UniqueID_ = uniqueID;
        return 0;
    }
    
    template <class SC,class LO,class GO,class NO>
    int InterfaceEntity<SC,LO,GO,NO>::setLocalID(LO localID)
    {
        LocalID_ = localID;
        return 0;
    }
    
    template <class SC,class LO,class GO,class NO>
    int InterfaceEntity<SC,LO,GO,NO>::setCoarseNodeID(LO coarseNodeID)
    {
        CoarseNodeID_ = coarseNodeID;
        return 0;
    }
    
    template <class SC,class LO,class GO,class NO>
    int InterfaceEntity<SC,LO,GO,NO>::setUniqueIDToFirstGlobalID()
    {
        UniqueID_ = NodeVector_[0].NodeIDGlobal_;
        return 0;
    }
    
    template <class SC,class LO,class GO,class NO>
    int InterfaceEntity<SC,LO,GO,NO>::resetEntityType(EntityType type)
    {
        Type_ = type;
        return 0;
    }
    
    template <class SC,class LO,class GO,class NO>
    int InterfaceEntity<SC,LO,GO,NO>::resetEntityFlag(EntityFlag flag)
    {
        Flag_ = flag;
        return 0;
    }
    
    template <class SC,class LO,class GO,class NO>
    int InterfaceEntity<SC,LO,GO,NO>::findAncestorsInSet(EntitySetPtr entitySet)
    {
        EntitySetPtr ancestors(new EntitySet<SC,LO,GO,NO>(*entitySet));
        GOVec tmpVector;
        for (UN i=0; i<Multiplicity_; i++) {
            UN length = ancestors->getNumEntities();
            for (UN j=0; j<length; j++) {
                tmpVector = ancestors->getEntity(length-1-j)->getSubdomainsVector();
                if (ancestors->getEntity(length-1-j)->getMultiplicity()<=this->getMultiplicity() || !std::binary_search(tmpVector.begin(),tmpVector.end(),SubdomainsVector_[i])) {
                    ancestors->removeEntity(length-1-j);
                }
            }
        }
        
        for (UN i=0; i<ancestors->getNumEntities(); i++) {
            Ancestors_->addEntity(ancestors->getEntity(i));
        }
        Ancestors_->sortUnique();
        return 0;
    }
    
    template <class SC,class LO,class GO,class NO>
    int InterfaceEntity<SC,LO,GO,NO>::clearAncestors()
    {
        Ancestors_.reset(new EntitySet<SC,LO,GO,NO>(DefaultType));
        return 0;
    }
    
    template <class SC,class LO,class GO,class NO>
    int InterfaceEntity<SC,LO,GO,NO>::addOffspring(InterfaceEntityPtr interfaceEntity)
    {
        Offspring_->addEntity(interfaceEntity);
        Offspring_->sortUnique();
        return 0;
    }
    
    template <class SC,class LO,class GO,class NO>
    int InterfaceEntity<SC,LO,GO,NO>::clearOffspring()
    {
        Offspring_.reset(new EntitySet<SC,LO,GO,NO>(DefaultType));
        return 0;
    }
    
    template <class SC,class LO,class GO,class NO>
    typename InterfaceEntity<SC,LO,GO,NO>::EntitySetPtr InterfaceEntity<SC,LO,GO,NO>::findCoarseNodes()
    {
        if (CoarseNodes_->getNumEntities()) {            
            return CoarseNodes_;
        }
        for (UN i=0; i<Ancestors_.size(); i++) {
            EntitySetPtr tmpCoarseNodes = Ancestors_[i]->findCoarseNodes();
            for (UN j=0; j<tmpCoarseNodes->getNumEntities(); j++) {
                CoarseNodes_->addEntitiy(tmpCoarseNodes[j]);
            }
        }
        CoarseNodes_->sortUnique();
        if (CoarseNodes_->getNumEntities()) {
            return CoarseNodes_;
        } else {
            EntitySetPtr thisEntity(new EntitySet<SC,LO,GO,NO>(DefaultType));
            thisEntity->addEntity(rcp(this)); // These are non-owning RCPs
            return thisEntity;
        }
    }
    
    template <class SC,class LO,class GO,class NO>
    int InterfaceEntity<SC,LO,GO,NO>::clearCoarseNodes()
    {
        CoarseNodes_.reset(new EntitySet<SC,LO,GO,NO>(DefaultType));
        return 0;
    }
    
    template <class SC,class LO,class GO,class NO>
    int InterfaceEntity<SC,LO,GO,NO>::computeDistancesToCoarseNodes(UN dimension,
                                                                    MultiVectorPtr &nodeList,
                                                                    DistanceFunction distanceFunction)
    {
        DistancesVector_.resize(NodeVector_.size());
        for (UN i=0; i<NodeVector_.size(); i++) {
            DistancesVector_[i].resize(CoarseNodes_->getNumEntities()+1,std::numeric_limits<SC>::max());
        }
        
        switch (distanceFunction) {
            case ConstantDistanceFunction:
                for (UN i=0; i<NodeVector_.size(); i++) {
                    for (UN j=0; j<CoarseNodes_->getNumEntities(); j++) {
                        DistancesVector_[i][j] = 1.0;
                    }
                }
                break;
            case InverseEuclideanDistanceFunction:
                FROSCH_ASSERT(!nodeList.is_null(),"The inverse euclidean distance cannot be calculated without coordinates of the nodes!");
                FROSCH_ASSERT(dimension==nodeList->getNumVectors(),"Inconsistent Dimension.");
                for (UN i=0; i<CoarseNodes_->getNumEntities(); i++) {
                    for (UN j=0; j<CoarseNodes_->getEntity(i)->getNumNodes(); j++) {
                        // Coordinates of the nodes of the coarse node
                        SCVecPtr CN(dimension);
                        for (UN k=0; k<dimension; k++) {
                            CN[k] = nodeList->getData(k)[CoarseNodes_->getEntity(i)->getLocalNodeID(j)];
                        }
                        for (UN k=0; k<NodeVector_.size(); k++) {
                            // Coordinates of the nodes of the entity
                            SCVecPtr E(dimension);
                            SC distance = 0.0;
                            // Compute quadratic distance
                            for (UN l=0; l<dimension; l++) {
                                distance += (nodeList->getData(l)[this->getLocalNodeID(k)]-CN[l]) * (nodeList->getData(l)[this->getLocalNodeID(k)]-CN[l]);
                            }
                            // Compute inverse euclidean distance
                            distance = std::sqrt(distance);
                            DistancesVector_[k][i] = std::min(DistancesVector_[k][i],distance);
                        }
                    }
                }
                for (UN i=0; i<NodeVector_.size(); i++) {
                    for (UN j=0; j<CoarseNodes_->getNumEntities(); j++) {
                        DistancesVector_[i][j] = 1.0/DistancesVector_[i][j];
                    }
                }
                break;
            default:
                break;
        }
        
        // In the last "row", we store the sum of the distances for all coarse nodes
        for (UN i=0; i<NodeVector_.size(); i++) {
            DistancesVector_[i][CoarseNodes_->getNumEntities()] = 0.0;
            for (UN j=0; j<CoarseNodes_->getNumEntities(); j++) {
                DistancesVector_[i][CoarseNodes_->getNumEntities()] += DistancesVector_[i][j];
            }
        }
        return 0;
    }
    
    template <class SC,class LO,class GO,class NO>
    typename InterfaceEntity<SC,LO,GO,NO>::InterfaceEntityPtr InterfaceEntity<SC,LO,GO,NO>::divideEntity(CrsMatrixPtr matrix,int pID)
    {
        InterfaceEntityPtr entity(new InterfaceEntity<SC,LO,GO,NO>(Type_,DofsPerNode_,Multiplicity_,&(SubdomainsVector_[0])));
        
        if (getNumNodes()>=2) {
            sortByGlobalID();
            
            GOVecPtr mapVector(getNumNodes());
            for (UN i=0; i<getNumNodes(); i++) {
                mapVector[i] = NodeVector_[i].DofsGamma_[0];
            }
            CrsMatrixPtr localMatrix,mat1,mat2,mat3;
            BuildSubmatrices(matrix,mapVector(),localMatrix,mat1,mat2,mat3);
            
            VectorPtr iterationVector = Xpetra::VectorFactory<SC,LO,GO,NO>::Build(localMatrix->getRowMap());
            iterationVector->getDataNonConst(0)[0] = 1.0;
            for (UN i=0; i<getNumNodes()-1; i++) {
                localMatrix->apply(*iterationVector,*iterationVector);
            }
            
            for (UN i=0; i<getNumNodes(); i++) {
                if (fabs(iterationVector->getData(0)[i])<1.0e-10) {
                    entity->addNode(getNode(i));
                    removeNode(i);
                }
            }
        }
        return entity;
    }
    
    /////////////////
    // Get Methods //
    /////////////////
    
    template <class SC,class LO,class GO,class NO>
    EntityType InterfaceEntity<SC,LO,GO,NO>::getEntityType() const
    {
        return Type_;
    }
    
    template <class SC,class LO,class GO,class NO>
    EntityFlag InterfaceEntity<SC,LO,GO,NO>::getEntityFlag() const
    {
        return Flag_;
    }
    
    template <class SC,class LO,class GO,class NO>
    typename InterfaceEntity<SC,LO,GO,NO>::UN InterfaceEntity<SC,LO,GO,NO>::getDofsPerNode() const
    {
        return DofsPerNode_;
    }
    
    template <class SC,class LO,class GO,class NO>
    typename InterfaceEntity<SC,LO,GO,NO>::UN InterfaceEntity<SC,LO,GO,NO>::getMultiplicity() const
    {
        return Multiplicity_;
    }
    
    template <class SC,class LO,class GO,class NO>
    GO InterfaceEntity<SC,LO,GO,NO>::getUniqueID() const
    {
        return UniqueID_;
    }
    
    template <class SC,class LO,class GO,class NO>
    LO InterfaceEntity<SC,LO,GO,NO>::getLocalID() const
    {
        return LocalID_;
    }
    
    template <class SC,class LO,class GO,class NO>
    LO InterfaceEntity<SC,LO,GO,NO>::getCoarseNodeID() const
    {
        return CoarseNodeID_;
    }
    
    template <class SC,class LO,class GO,class NO>
    const Node<SC,LO,GO>& InterfaceEntity<SC,LO,GO,NO>::getNode(UN iDNode) const
    {
        return NodeVector_[iDNode];
    }
    
    template <class SC,class LO,class GO,class NO>
    LO InterfaceEntity<SC,LO,GO,NO>::getGammaNodeID(UN iDNode) const
    {
        return NodeVector_[iDNode].NodeIDGamma_;
    }
    
    template <class SC,class LO,class GO,class NO>
    LO InterfaceEntity<SC,LO,GO,NO>::getLocalNodeID(UN iDNode) const
    {
        return NodeVector_[iDNode].NodeIDLocal_;
    }
    
    template <class SC,class LO,class GO,class NO>
    GO InterfaceEntity<SC,LO,GO,NO>::getGlobalNodeID(UN iDNode) const
    {
        return NodeVector_[iDNode].NodeIDGlobal_;
    }
    
    template <class SC,class LO,class GO,class NO>
    LO InterfaceEntity<SC,LO,GO,NO>::getGammaDofID(UN iDNode, UN iDDof) const
    {
        return NodeVector_[iDNode].DofsGamma_[iDDof];
    }
    
    template <class SC,class LO,class GO,class NO>
    LO InterfaceEntity<SC,LO,GO,NO>::getLocalDofID(UN iDNode, UN iDDof) const
    {
        return NodeVector_[iDNode].DofsLocal_[iDDof];
    }
    
    template <class SC,class LO,class GO,class NO>
    GO InterfaceEntity<SC,LO,GO,NO>::getGlobalDofID(UN iDNode, UN iDDof) const
    {
        return NodeVector_[iDNode].DofsGlobal_[iDDof];
    }
    
    template <class SC,class LO,class GO,class NO>
    const typename InterfaceEntity<SC,LO,GO,NO>::GOVec & InterfaceEntity<SC,LO,GO,NO>::getSubdomainsVector() const
    {
        return SubdomainsVector_;
    }
    
    template <class SC,class LO,class GO,class NO>
    typename InterfaceEntity<SC,LO,GO,NO>::UN InterfaceEntity<SC,LO,GO,NO>::getNumNodes() const
    {
        return NodeVector_.size();
    }
    
    template <class SC,class LO,class GO,class NO>
    const typename InterfaceEntity<SC,LO,GO,NO>::EntitySetPtr InterfaceEntity<SC,LO,GO,NO>::getAncestors() const
    {
        return Ancestors_;
    }
    
    template <class SC,class LO,class GO,class NO>
    const typename InterfaceEntity<SC,LO,GO,NO>::EntitySetPtr InterfaceEntity<SC,LO,GO,NO>::getOffspring() const
    {
        return Offspring_;
    }
    
    template <class SC,class LO,class GO,class NO>
    const typename InterfaceEntity<SC,LO,GO,NO>::EntitySetPtr InterfaceEntity<SC,LO,GO,NO>::getCoarseNodes() const
    {
        return CoarseNodes_;
    }
    
    template <class SC,class LO,class GO,class NO>
    SC InterfaceEntity<SC,LO,GO,NO>::getDistanceToCoarseNode(UN iDNode,
                                                                                                    UN iDCoarseNode) const
    {
        return DistancesVector_[iDNode][iDCoarseNode];
    }
    
    template <class SC,class LO,class GO,class NO>
    bool compareInterfaceEntities(Teuchos::RCP<InterfaceEntity<SC,LO,GO,NO> > iEa,
                                  Teuchos::RCP<InterfaceEntity<SC,LO,GO,NO> > iEb)
    {
        return iEa->getUniqueID()<iEb->getUniqueID();
    }
    
    template <class SC,class LO,class GO,class NO>
    bool equalInterfaceEntities(Teuchos::RCP<InterfaceEntity<SC,LO,GO,NO> > iEa,
                                Teuchos::RCP<InterfaceEntity<SC,LO,GO,NO> > iEb)
    {
        return iEa->getUniqueID()==iEb->getUniqueID();
    }
}

#endif
