//*****************************************************************//
//    Albany 3.0:  Copyright 2016 Sandia Corporation               //
//    This Software is released under the BSD license detailed     //
//    in the file "license.txt" in the top-level Albany directory  //
//*****************************************************************//

#include "AAdapt_CopyRemeshT.hpp"

#include "Teuchos_TimeMonitor.hpp"

namespace AAdapt {

typedef stk::mesh::Entity Entity;
typedef stk::mesh::EntityRank EntityRank;
typedef stk::mesh::RelationIdentifier EdgeId;
typedef stk::mesh::EntityKey EntityKey;

//----------------------------------------------------------------------------
AAdapt::CopyRemeshT::
CopyRemeshT(const Teuchos::RCP<Teuchos::ParameterList>& params,
           const Teuchos::RCP<ParamLib>& param_lib,
           const Albany::StateManager& state_mgr,
           const Teuchos::RCP<const Teuchos_Comm>& comm) :
  AAdapt::AbstractAdapterT(params, param_lib, state_mgr, comm),
  remesh_file_index_(1) {

  discretization_ = state_mgr_.getDiscretization();

  stk_discretization_ =
    static_cast<Albany::STKDiscretization*>(discretization_.get());

  stk_mesh_struct_ = stk_discretization_->getSTKMeshStruct();

  bulk_data_ = stk_mesh_struct_->bulkData;
  meta_data_ = stk_mesh_struct_->metaData;

  num_dim_ = stk_mesh_struct_->numDim;

  // Save the initial output file name
  base_exo_filename_ = stk_mesh_struct_->exoOutFile;

}

//----------------------------------------------------------------------------
AAdapt::CopyRemeshT::
~CopyRemeshT() {
}

//----------------------------------------------------------------------------
bool
AAdapt::CopyRemeshT::queryAdaptationCriteria(int iter) {

  if(adapt_params_->get<std::string>("Remesh Strategy", "None").compare("Continuous") == 0){

    if(iter > 1)

      return true;

    else

      return false;

  }

  Teuchos::Array<int> remesh_iter = adapt_params_->get<Teuchos::Array<int> >("Remesh Step Number");

  for(int i = 0; i < remesh_iter.size(); i++)

    if(iter == remesh_iter[i])

      return true;

  return false;

}

//----------------------------------------------------------------------------
bool
AAdapt::CopyRemeshT::adaptMesh(){

  std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";
  std::cout << "Adapting mesh using AAdapt::CopyRemesh method       \n";
  std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";

  // Save the current results and close the exodus file

  // Create a remeshed output file naming convention by adding the
  // remeshFileIndex ahead of the period

  std::ostringstream ss;
  std::string str = base_exo_filename_;
  ss << "_" << remesh_file_index_ << ".";
  str.replace(str.find('.'), 1, ss.str());

  std::cout << "Remeshing: renaming output file to - " << str << std::endl;

  // Open the new exodus file for results
  stk_discretization_->reNameExodusOutput(str);

  remesh_file_index_++;

  // do remeshing right here if we were doing any...

  // Throw away all the Albany data structures and re-build them
  // from the mesh

  stk_discretization_->updateMesh();

  return true;

}

//----------------------------------------------------------------------------
Teuchos::RCP<const Teuchos::ParameterList>
AAdapt::CopyRemeshT::getValidAdapterParameters() const {
  Teuchos::RCP<Teuchos::ParameterList> validPL =
    this->getGenericAdapterParams("ValidCopyRemeshParameters");

  Teuchos::Array<int> defaultArgs;

  validPL->set<Teuchos::Array<int> >("Remesh Step Number", defaultArgs, "Iteration step at which to remesh the problem");
  validPL->set<std::string>("Remesh Strategy", "", "Strategy to use when remeshing: Continuous - remesh every step.");

  return validPL;
}
//----------------------------------------------------------------------------
}
