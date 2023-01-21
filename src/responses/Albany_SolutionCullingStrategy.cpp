//*****************************************************************//
//    Albany 3.0:  Copyright 2016 Sandia Corporation               //
//    This Software is released under the BSD license detailed     //
//    in the file "license.txt" in the top-level Albany directory  //
//*****************************************************************//

#include "Albany_SolutionCullingStrategy.hpp"

#include "Tpetra_GatherAllV.hpp" 
#include "Teuchos_CommHelpers.hpp"
#include "Teuchos_Array.hpp" 
#include "Tpetra_DistObject.hpp"
#include "Albany_Utils.hpp"

#include "Teuchos_Assert.hpp"


#include <algorithm>

namespace Albany {

class UniformSolutionCullingStrategy : public SolutionCullingStrategyBase {
public:
  explicit UniformSolutionCullingStrategy(int numValues);
  virtual Teuchos::Array<Tpetra_GO> selectedGIDsT(Teuchos::RCP<const Tpetra_Map> sourceMapT) const;
private:
  int numValues_;
};

} // namespace Albany

Albany::UniformSolutionCullingStrategy::
UniformSolutionCullingStrategy(int numValues) :
  numValues_(numValues)
{
  // Nothing to do
}

Teuchos::Array<Tpetra_GO>
Albany::UniformSolutionCullingStrategy::
selectedGIDsT(Teuchos::RCP<const Tpetra_Map> sourceMapT) const
{
  Teuchos::Array<Tpetra_GO> allGIDs(sourceMapT->getGlobalNumElements());
  {
    const int ierr = Tpetra::GatherAllV(
        sourceMapT->getComm(),
        sourceMapT->getNodeElementList().getRawPtr(), sourceMapT->getNodeNumElements(),
        allGIDs.getRawPtr(), allGIDs.size());
    TEUCHOS_ASSERT(ierr == 0);
  }
  std::sort(allGIDs.begin(), allGIDs.end());

  Teuchos::Array<Tpetra_GO> result(numValues_);
  const int stride = 1 + (allGIDs.size() - 1) / numValues_;
  for (int i = 0; i < numValues_; ++i) {
    result[i] = allGIDs[i * stride];
  }
  return result;
}

#include "Albany_SolutionCullingStrategy.hpp"

#include "Albany_Application.hpp"
#include "Albany_AbstractDiscretization.hpp"

#include "Tpetra_GatherAllV.hpp"
#include "Teuchos_CommHelpers.hpp"
#include "Teuchos_Array.hpp" 
#include "Tpetra_DistObject.hpp"
#include "Albany_Utils.hpp"

#include "Teuchos_Assert.hpp"

#include <string>
#include <algorithm>

namespace Albany {

class NodeSetSolutionCullingStrategy : public SolutionCullingStrategyBase {
public:
  NodeSetSolutionCullingStrategy(
      const std::string &nodeSetLabel,
      const Teuchos::RCP<const Application> &app);

  virtual Teuchos::Array<Tpetra_GO> selectedGIDsT(Teuchos::RCP<const Tpetra_Map> sourceMapT) const;
  virtual void setupT();

private:
  std::string nodeSetLabel_;
  Teuchos::RCP<const Application> app_;

  Teuchos::RCP<const AbstractDiscretization> disc_;
};

} // namespace Albany

Albany::NodeSetSolutionCullingStrategy::
NodeSetSolutionCullingStrategy(
    const std::string &nodeSetLabel,
    const Teuchos::RCP<const Application> &app) :
  nodeSetLabel_(nodeSetLabel),
  app_(app),
  disc_(Teuchos::null)
{
  // setup() must be called after the discretization has been created to finish initialization
}

void
Albany::NodeSetSolutionCullingStrategy::
setupT()
{
  disc_ = app_->getDiscretization();
  // Once the discretization has been obtained, a handle to the application is not required
  // Release the resource to avoid possible circular references
  app_.reset();
}

Teuchos::Array<Tpetra_GO>
Albany::NodeSetSolutionCullingStrategy::
selectedGIDsT(Teuchos::RCP<const Tpetra_Map> sourceMapT) const
{
  Teuchos::Array<Tpetra_GO> result;
  {
    Teuchos::Array<Tpetra_GO> mySelectedGIDs;
    {
      const NodeSetList &nodeSets = disc_->getNodeSets();

      const NodeSetList::const_iterator it = nodeSets.find(nodeSetLabel_);
      if (it != nodeSets.end()) {
        typedef NodeSetList::mapped_type NodeSetEntryList;
        const NodeSetEntryList &sampleNodeEntries = it->second;

        for (NodeSetEntryList::const_iterator jt = sampleNodeEntries.begin(); jt != sampleNodeEntries.end(); ++jt) {
          typedef NodeSetEntryList::value_type NodeEntryList;
          const NodeEntryList &sampleEntries = *jt;
          for (NodeEntryList::const_iterator kt = sampleEntries.begin(); kt != sampleEntries.end(); ++kt) {
            mySelectedGIDs.push_back(sourceMapT->getGlobalElement(*kt));
          }
        }
      }
    }

    Teuchos::RCP<const Teuchos::Comm<int> >commT = sourceMapT->getComm(); 
    {
      Tpetra_GO selectedGIDCount;
      {
        Tpetra_GO mySelectedGIDCount = mySelectedGIDs.size();
        Teuchos::reduceAll<LO, Tpetra_GO>(*commT, Teuchos::REDUCE_SUM, 1, &mySelectedGIDCount, &selectedGIDCount); 
      }
      result.resize(selectedGIDCount);
    }

    const int ierr = Tpetra::GatherAllV(
        commT,
        mySelectedGIDs.getRawPtr(), mySelectedGIDs.size(),
        result.getRawPtr(), result.size());
    TEUCHOS_ASSERT(ierr == 0);
  }

  std::sort(result.begin(), result.end());

  return result;
}


namespace Albany {

class NodeGIDsSolutionCullingStrategy : public SolutionCullingStrategyBase {
public:
  NodeGIDsSolutionCullingStrategy(
      const Teuchos::Array<int>& nodeGIDs,
      const Teuchos::RCP<const Application> &app);

  virtual Teuchos::Array<Tpetra_GO> selectedGIDsT(Teuchos::RCP<const Tpetra_Map> sourceMapT) const;
  virtual void setupT();

private:
  Teuchos::Array<int> nodeGIDs_;
  Teuchos::RCP<const Application> app_;

  Teuchos::RCP<const AbstractDiscretization> disc_;
};

} // namespace Albany

Albany::NodeGIDsSolutionCullingStrategy::
NodeGIDsSolutionCullingStrategy(
    const Teuchos::Array<int>& nodeGIDs,
    const Teuchos::RCP<const Application> &app) :
  nodeGIDs_(nodeGIDs),
  app_(app),
  disc_(Teuchos::null)
{
  // setup() must be called after the discretization has been created to finish initialization
}

void
Albany::NodeGIDsSolutionCullingStrategy::
setupT()
{
  disc_ = app_->getDiscretization();
  // Once the discretization has been obtained, a handle to the application is not required
  // Release the resource to avoid possible circular references
  app_.reset();
}

Teuchos::Array<Tpetra_GO>
Albany::NodeGIDsSolutionCullingStrategy::
selectedGIDsT(Teuchos::RCP<const Tpetra_Map> sourceMapT) const
{
  Teuchos::Array<Tpetra_GO> result;
  {
    Teuchos::Array<Tpetra_GO> mySelectedGIDs;

    // Subract 1 to convert exodus GIDs to our GIDs
    for (int i=0; i<nodeGIDs_.size(); i++)
      if (sourceMapT->isNodeGlobalElement(nodeGIDs_[i] -1) ) mySelectedGIDs.push_back(nodeGIDs_[i] - 1);

    Teuchos::RCP<const Teuchos::Comm<int> >commT = sourceMapT->getComm(); 
    {
      Tpetra_GO selectedGIDCount;
      {
        Tpetra_GO mySelectedGIDCount = mySelectedGIDs.size();
        Teuchos::reduceAll<LO, Tpetra_GO>(*commT, Teuchos::REDUCE_SUM, 1, &mySelectedGIDCount, &selectedGIDCount); 
      }
      result.resize(selectedGIDCount);
    }

    const int ierr = Tpetra::GatherAllV(
        commT,
        mySelectedGIDs.getRawPtr(), mySelectedGIDs.size(),
        result.getRawPtr(), result.size());
    TEUCHOS_ASSERT(ierr == 0);
  }

  std::sort(result.begin(), result.end());

  return result;
}


#include "Albany_Application.hpp"

#include "Teuchos_TestForException.hpp"
#include "Teuchos_CommHelpers.hpp"
#include "Teuchos_Array.hpp" 
#include "Tpetra_DistObject.hpp"
#include "Albany_Utils.hpp"

#include <string>

namespace Albany {

Teuchos::RCP<SolutionCullingStrategyBase>
createSolutionCullingStrategy(
    const Teuchos::RCP<const Application> &app,
    Teuchos::ParameterList &params)
{
  const std::string cullingStrategyToken = params.get("Culling Strategy", "Uniform");

  if (cullingStrategyToken == "Uniform") {
    const int numValues = params.get("Num Values", 10);
    return Teuchos::rcp(new UniformSolutionCullingStrategy(numValues));
  } else if (cullingStrategyToken == "Node Set") {
    const std::string nodeSetLabel = params.get<std::string>("Node Set Label");
    return Teuchos::rcp(new NodeSetSolutionCullingStrategy(nodeSetLabel, app));
  } else if (cullingStrategyToken == "Node GIDs") {
    Teuchos::Array<int> nodeGIDs = params.get<Teuchos::Array<int> >("Node GID Array");
    return Teuchos::rcp(new NodeGIDsSolutionCullingStrategy(nodeGIDs, app));
  }

  const bool unsupportedCullingStrategy = true;
  TEUCHOS_TEST_FOR_EXCEPT(unsupportedCullingStrategy);
}

} // namespace Albany


