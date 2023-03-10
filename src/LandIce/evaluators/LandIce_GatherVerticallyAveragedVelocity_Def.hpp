//*****************************************************************//
//    Albany 3.0:  Copyright 2016 Sandia Corporation               //
//    This Software is released under the BSD license detailed     //
//    in the file "license.txt" in the top-level Albany directory  //
//*****************************************************************//

#include "Teuchos_TestForException.hpp"
#include "Teuchos_VerboseObject.hpp"
#include "Phalanx_DataLayout.hpp"
#include "Phalanx_TypeStrings.hpp"
#include "Sacado.hpp"

#include "Albany_TpetraThyraUtils.hpp"

//uncomment the following line if you want debug output to be printed to screen
//#define OUTPUT_TO_SCREEN

namespace LandIce {

//**********************************************************************

template<typename EvalT, typename Traits>
GatherVerticallyAveragedVelocityBase<EvalT, Traits>::
GatherVerticallyAveragedVelocityBase(const Teuchos::ParameterList& p,
                  const Teuchos::RCP<Albany::Layouts>& dl) :
  averagedVel(p.get<std::string>("Averaged Velocity Name"), dl->node_vector)
{
  Teuchos::RCP<Teuchos::FancyOStream> out(Teuchos::VerboseObjectBase::getDefaultOStream());

  this->addEvaluatedField(averagedVel);
  cell_topo = p.get<Teuchos::RCP<const CellTopologyData> >("Cell Topology");

  std::vector<PHX::DataLayout::size_type> dims;

  dl->node_vector->dimensions(dims);
  numNodes = dims[1];
  vecDimFO = dims[2];

  if (p.isParameter("Mesh Part"))
    meshPart = p.get<std::string>("Mesh Part");
  else
    meshPart = "upperside";

  this->setName("GatherVerticallyAveragedVelocity"+PHX::typeAsString<EvalT>());
}


//**********************************************************************
template<typename EvalT, typename Traits>
void GatherVerticallyAveragedVelocityBase<EvalT, Traits>::
postRegistrationSetup(typename Traits::SetupData d,
                      PHX::FieldManager<Traits>& fm)
{
    this->utils.setFieldData(averagedVel,fm);
}

//**********************************************************************



template<typename Traits>
GatherVerticallyAveragedVelocity<PHAL::AlbanyTraits::Residual, Traits>::
GatherVerticallyAveragedVelocity(const Teuchos::ParameterList& p,
          const Teuchos::RCP<Albany::Layouts>& dl)
          : GatherVerticallyAveragedVelocityBase<PHAL::AlbanyTraits::Residual, Traits>(p,dl)
            {}

template<typename Traits>
void GatherVerticallyAveragedVelocity<PHAL::AlbanyTraits::Residual, Traits>::
evaluateFields(typename Traits::EvalData workset)
{
  Teuchos::RCP<const Tpetra_Vector> xT = Albany::getConstTpetraVector(workset.x);
  Teuchos::ArrayRCP<const ST> xT_constView = xT->get1dView();

  Kokkos::deep_copy(this->averagedVel.get_view(), ScalarT(0.0));

  if (workset.sideSets == Teuchos::null)
      TEUCHOS_TEST_FOR_EXCEPTION(true, std::logic_error, "Side sets defined in input file but not properly specified on the mesh" << std::endl);

  const Albany::SideSetList& ssList = *(workset.sideSets);
  Albany::SideSetList::const_iterator it = ssList.find(this->meshPart);

  if (it != ssList.end()) {
    const std::vector<Albany::SideStruct>& sideSet = it->second;

    // Loop over the sides that form the boundary condition
    const Teuchos::ArrayRCP<Teuchos::ArrayRCP<GO> >& wsElNodeID  = workset.disc->getWsElNodeID()[workset.wsIndex];
    const Albany::LayeredMeshNumbering<LO>& layeredMeshNumbering = *workset.disc->getLayeredMeshNumbering();
    const Albany::NodalDOFManager& solDOFManager = workset.disc->getOverlapDOFManager("ordinary_solution");

    const Teuchos::ArrayRCP<double>& layers_ratio = layeredMeshNumbering.layers_ratio;
    int numLayers = layeredMeshNumbering.numLayers;

    Teuchos::ArrayRCP<double> quadWeights(numLayers+1); //doing trapezoidal rule
    quadWeights[0] = 0.5*layers_ratio[0]; quadWeights[numLayers] = 0.5*layers_ratio[numLayers-1];
    for(int i=1; i<numLayers; ++i)
      quadWeights[i] = 0.5*(layers_ratio[i-1] + layers_ratio[i]);

    for (std::size_t iSide = 0; iSide < sideSet.size(); ++iSide) { // loop over the sides on this ws and name
      // Get the data that corresponds to the side
      const int elem_GID = sideSet[iSide].elem_GID;
      const int elem_LID = sideSet[iSide].elem_LID;
      const int elem_side = sideSet[iSide].side_local_id;
      const CellTopologyData_Subcell& side =  this->cell_topo->side[elem_side];
      int numSideNodes = side.topology->node_count;

      const Teuchos::ArrayRCP<GO>& elNodeID = wsElNodeID[elem_LID];

      //we only consider elements on the top.
      LO baseId, ilayer;
      for (int i = 0; i < numSideNodes; ++i) {
        std::size_t node = side.node[i];
        LO lnodeId = workset.disc->getOverlapNodeMapT()->getLocalElement(elNodeID[node]);
        layeredMeshNumbering.getIndices(lnodeId, baseId, ilayer);
        std::vector<double> avVel(this->vecDimFO,0);
        for(int il=0; il<numLayers+1; ++il)
        {
          LO inode = layeredMeshNumbering.getId(baseId, il);
          for(int comp=0; comp<this->vecDimFO; ++comp)
            avVel[comp] += xT_constView[solDOFManager.getLocalDOF(inode, comp)]*quadWeights[il];
        }
        for(int comp=0; comp<this->vecDimFO; ++comp)
          this->averagedVel(elem_LID,node,comp) = avVel[comp];
      }
    }
  }
}


template<typename Traits>
GatherVerticallyAveragedVelocity<PHAL::AlbanyTraits::Jacobian, Traits>::
GatherVerticallyAveragedVelocity(const Teuchos::ParameterList& p,
          const Teuchos::RCP<Albany::Layouts>& dl)
          : GatherVerticallyAveragedVelocityBase<PHAL::AlbanyTraits::Jacobian, Traits>(p,dl)
            {}

template<typename Traits>
void GatherVerticallyAveragedVelocity<PHAL::AlbanyTraits::Jacobian, Traits>::
evaluateFields(typename Traits::EvalData workset)
{
  Teuchos::RCP<const Tpetra_Vector> xT = Albany::getConstTpetraVector(workset.x);
  Teuchos::ArrayRCP<const ST> xT_constView = xT->get1dView();
  
  int neq = workset.wsElNodeEqID.dimension(2);

  if (workset.sideSets == Teuchos::null)
      TEUCHOS_TEST_FOR_EXCEPTION(true, std::logic_error, "Side sets defined in input file but not properly specified on the mesh" << std::endl);


  const Albany::LayeredMeshNumbering<LO>& layeredMeshNumbering = *workset.disc->getLayeredMeshNumbering();
  int numLayers = layeredMeshNumbering.numLayers;

  Kokkos::deep_copy(this->averagedVel.get_view(), ScalarT(0.0));

  const Albany::SideSetList& ssList = *(workset.sideSets);
  Albany::SideSetList::const_iterator it = ssList.find(this->meshPart);

  if (it != ssList.end()) {
    const std::vector<Albany::SideStruct>& sideSet = it->second;

    // Loop over the sides that form the boundary condition
    const Teuchos::ArrayRCP<Teuchos::ArrayRCP<GO> >& wsElNodeID  = workset.disc->getWsElNodeID()[workset.wsIndex];
    const Albany::NodalDOFManager& solDOFManager = workset.disc->getOverlapDOFManager("ordinary_solution");

    const Teuchos::ArrayRCP<double>& layers_ratio = layeredMeshNumbering.layers_ratio;

    Teuchos::ArrayRCP<double> quadWeights(numLayers+1); //doing trapezoidal rule

    quadWeights[0] = 0.5*layers_ratio[0]; quadWeights[numLayers] = 0.5*layers_ratio[numLayers-1];
    for(int i=1; i<numLayers; ++i)
      quadWeights[i] = 0.5*(layers_ratio[i-1] + layers_ratio[i]);

    for (std::size_t iSide = 0; iSide < sideSet.size(); ++iSide) { // loop over the sides on this ws and name

      // Get the data that corresponds to the side
      const int elem_GID = sideSet[iSide].elem_GID;
      const int elem_LID = sideSet[iSide].elem_LID;
      const int elem_side = sideSet[iSide].side_local_id;
      const CellTopologyData_Subcell& side =  this->cell_topo->side[elem_side];
      int numSideNodes = side.topology->node_count;

      const Teuchos::ArrayRCP<GO>& elNodeID = wsElNodeID[elem_LID];
      std::vector<double> velx(this->numNodes,0), vely(this->numNodes,0);

      LO baseId, ilayer;
      for (int i = 0; i < numSideNodes; ++i) {
        std::size_t node = side.node[i];
        LO lnodeId = workset.disc->getOverlapNodeMapT()->getLocalElement(elNodeID[node]);
        layeredMeshNumbering.getIndices(lnodeId, baseId, ilayer);
        std::vector<double> avVel(this->vecDimFO,0);
        for(int il=0; il<numLayers+1; ++il)
        {
          LO inode = layeredMeshNumbering.getId(baseId, il);
          for(int comp=0; comp<this->vecDimFO; ++comp)
            avVel[comp] += xT_constView[solDOFManager.getLocalDOF(inode, comp)]*quadWeights[il];
        }

        for(int comp=0; comp<this->vecDimFO; ++comp) {
          this->averagedVel(elem_LID,node,comp) = FadType(this->averagedVel(elem_LID,node,comp).size(), avVel[comp]);
          for(int il=0; il<numLayers+1; ++il)
            this->averagedVel(elem_LID,node,comp).fastAccessDx(neq*(this->numNodes+numSideNodes*il+i)+comp) = quadWeights[il]*workset.j_coeff;
        }
      }
    }
  }
}

template<typename Traits>
GatherVerticallyAveragedVelocity<PHAL::AlbanyTraits::Tangent, Traits>::
GatherVerticallyAveragedVelocity(const Teuchos::ParameterList& p,
          const Teuchos::RCP<Albany::Layouts>& dl)
          : GatherVerticallyAveragedVelocityBase<PHAL::AlbanyTraits::Tangent, Traits>(p,dl)
            {}

template<typename Traits>
void GatherVerticallyAveragedVelocity<PHAL::AlbanyTraits::Tangent, Traits>::
evaluateFields(typename Traits::EvalData workset)
{
  Teuchos::RCP<const Tpetra_Vector> xT = Albany::getConstTpetraVector(workset.x);
  Teuchos::RCP<const Tpetra_MultiVector> VxT = Albany::getConstTpetraMultiVector(workset.Vx);
  Teuchos::ArrayRCP<const ST> xT_constView = xT->get1dView();


  Kokkos::deep_copy(this->averagedVel.get_view(), ScalarT(0.0));

  if (workset.sideSets == Teuchos::null)
      TEUCHOS_TEST_FOR_EXCEPTION(true, std::logic_error, "Side sets defined in input file but not properly specified on the mesh" << std::endl);

  const Albany::SideSetList& ssList = *(workset.sideSets);
  Albany::SideSetList::const_iterator it = ssList.find(this->meshPart);

  if (it != ssList.end()) {
    const std::vector<Albany::SideStruct>& sideSet = it->second;

    // Loop over the sides that form the boundary condition
    const Teuchos::ArrayRCP<Teuchos::ArrayRCP<GO> >& wsElNodeID  = workset.disc->getWsElNodeID()[workset.wsIndex];
    const Albany::LayeredMeshNumbering<LO>& layeredMeshNumbering = *workset.disc->getLayeredMeshNumbering();
    const Albany::NodalDOFManager& solDOFManager = workset.disc->getOverlapDOFManager("ordinary_solution");

    const Teuchos::ArrayRCP<double>& layers_ratio = layeredMeshNumbering.layers_ratio;
    int numLayers = layeredMeshNumbering.numLayers;

    Teuchos::ArrayRCP<double> quadWeights(numLayers+1); //doing trapezoidal rule
    quadWeights[0] = 0.5*layers_ratio[0]; quadWeights[numLayers] = 0.5*layers_ratio[numLayers-1];
    for(int i=1; i<numLayers; ++i)
      quadWeights[i] = 0.5*(layers_ratio[i-1] + layers_ratio[i]);

    for (std::size_t iSide = 0; iSide < sideSet.size(); ++iSide) { // loop over the sides on this ws and name
      // Get the data that corresponds to the side
      const int elem_GID = sideSet[iSide].elem_GID;
      const int elem_LID = sideSet[iSide].elem_LID;
      const int elem_side = sideSet[iSide].side_local_id;
      const CellTopologyData_Subcell& side =  this->cell_topo->side[elem_side];
      int numSideNodes = side.topology->node_count;

      const Teuchos::ArrayRCP<GO>& elNodeID = wsElNodeID[elem_LID];

      //we only consider elements on the top.
      LO baseId, ilayer;
      for (int i = 0; i < numSideNodes; ++i) {
        std::size_t node = side.node[i];
        LO lnodeId = workset.disc->getOverlapNodeMapT()->getLocalElement(elNodeID[node]);
        layeredMeshNumbering.getIndices(lnodeId, baseId, ilayer);
        std::vector<double> avVel(this->vecDimFO,0);
        for(int il=0; il<numLayers+1; ++il)
        {
          LO inode = layeredMeshNumbering.getId(baseId, il);
          for(int comp=0; comp<this->vecDimFO; ++comp)
            avVel[comp] += xT_constView[solDOFManager.getLocalDOF(inode, comp)]*quadWeights[il];
        }
        for(int comp=0; comp<this->vecDimFO; ++comp) {
          this->averagedVel(elem_LID,node,comp) = avVel[comp];
          if (VxT != Teuchos::null && workset.j_coeff != 0.0) {
            TEUCHOS_TEST_FOR_EXCEPTION(true, std::logic_error, "Not Implemented yet" << std::endl);
          }
        }
      }
    }
  }
}


template<typename Traits>
GatherVerticallyAveragedVelocity<PHAL::AlbanyTraits::DistParamDeriv, Traits>::
GatherVerticallyAveragedVelocity(const Teuchos::ParameterList& p,
          const Teuchos::RCP<Albany::Layouts>& dl)
          : GatherVerticallyAveragedVelocityBase<PHAL::AlbanyTraits::DistParamDeriv, Traits>(p,dl)
            {}

template<typename Traits>
void GatherVerticallyAveragedVelocity<PHAL::AlbanyTraits::DistParamDeriv, Traits>::
evaluateFields(typename Traits::EvalData workset)
{
  Teuchos::RCP<const Tpetra_Vector> xT = Albany::getConstTpetraVector(workset.x);
  Teuchos::ArrayRCP<const ST> xT_constView = xT->get1dView();

  Kokkos::deep_copy(this->averagedVel.get_view(), ScalarT(0.0));

  if (workset.sideSets == Teuchos::null)
      TEUCHOS_TEST_FOR_EXCEPTION(true, std::logic_error, "Side sets defined in input file but not properly specified on the mesh" << std::endl);

  const Albany::SideSetList& ssList = *(workset.sideSets);
  Albany::SideSetList::const_iterator it = ssList.find(this->meshPart);

  if (it != ssList.end()) {
    const std::vector<Albany::SideStruct>& sideSet = it->second;

    // Loop over the sides that form the boundary condition
    const Teuchos::ArrayRCP<Teuchos::ArrayRCP<GO> >& wsElNodeID  = workset.disc->getWsElNodeID()[workset.wsIndex];
    const Albany::LayeredMeshNumbering<LO>& layeredMeshNumbering = *workset.disc->getLayeredMeshNumbering();
    const Albany::NodalDOFManager& solDOFManager = workset.disc->getOverlapDOFManager("ordinary_solution");

    const Teuchos::ArrayRCP<double>& layers_ratio = layeredMeshNumbering.layers_ratio;
    int numLayers = layeredMeshNumbering.numLayers;

    Teuchos::ArrayRCP<double> quadWeights(numLayers+1); //doing trapezoidal rule
    quadWeights[0] = 0.5*layers_ratio[0]; quadWeights[numLayers] = 0.5*layers_ratio[numLayers-1];
    for(int i=1; i<numLayers; ++i)
      quadWeights[i] = 0.5*(layers_ratio[i-1] + layers_ratio[i]);

    for (std::size_t iSide = 0; iSide < sideSet.size(); ++iSide) { // loop over the sides on this ws and name
      // Get the data that corresponds to the side
      const int elem_GID = sideSet[iSide].elem_GID;
      const int elem_LID = sideSet[iSide].elem_LID;
      const int elem_side = sideSet[iSide].side_local_id;
      const CellTopologyData_Subcell& side =  this->cell_topo->side[elem_side];
      int numSideNodes = side.topology->node_count;

      const Teuchos::ArrayRCP<GO>& elNodeID = wsElNodeID[elem_LID];

      //we only consider elements on the top.
      LO baseId, ilayer;
      for (int i = 0; i < numSideNodes; ++i) {
        std::size_t node = side.node[i];
        LO lnodeId = workset.disc->getOverlapNodeMapT()->getLocalElement(elNodeID[node]);
        layeredMeshNumbering.getIndices(lnodeId, baseId, ilayer);
        std::vector<double> avVel(this->vecDimFO,0);
        for(int il=0; il<numLayers+1; ++il)
        {
          LO inode = layeredMeshNumbering.getId(baseId, il);
          for(int comp=0; comp<this->vecDimFO; ++comp)
            avVel[comp] += xT_constView[solDOFManager.getLocalDOF(inode, comp)]*quadWeights[il];
        }
        for(int comp=0; comp<this->vecDimFO; ++comp)
          this->averagedVel(elem_LID,node,comp) = avVel[comp];
      }
    }
  }
}

} // namespace FELIX
