//*****************************************************************//
//    Albany 3.0:  Copyright 2016 Sandia Corporation               //
//    This Software is released under the BSD license detailed     //
//    in the file "license.txt" in the top-level Albany directory  //
//*****************************************************************//

#include <fstream>
#include "Teuchos_TestForException.hpp"
#include "Phalanx_DataLayout.hpp"
#include "Teuchos_CommHelpers.hpp"
#include "PHAL_Utilities.hpp"

template<typename EvalT, typename Traits>
LandIce::FluxDiv<EvalT, Traits>::
FluxDiv (const Teuchos::ParameterList& p,
         const Teuchos::RCP<Albany::Layouts>& dl_basal)
{
  // get and validate Response parameter list
  std::string fieldName = p.get<std::string> ("Field Name");

  const std::string& averaged_velocity_name     = p.get<std::string>("Averaged Velocity Side QP Variable Name");
  const std::string& div_averaged_velocity_name = p.get<std::string>("Averaged Velocity Side QP Divergence Name");
  const std::string& thickness_name             = p.get<std::string>("Thickness Side QP Variable Name");
  const std::string& grad_thickness_name        = p.get<std::string>("Thickness Gradient Name");
  const std::string& side_tangents_name         = p.get<std::string>("Side Tangents Name");

  averaged_velocity     = decltype(averaged_velocity)(averaged_velocity_name, dl_basal->qp_vector);
  div_averaged_velocity = decltype(div_averaged_velocity)(div_averaged_velocity_name, dl_basal->qp_scalar);
  thickness             = decltype(thickness)(thickness_name, dl_basal->qp_scalar);
  grad_thickness        = decltype(grad_thickness)(grad_thickness_name, dl_basal->qp_gradient);
  side_tangents         = decltype(side_tangents)(side_tangents_name, dl_basal->qp_tensor_cd_sd);

  flux_div              = decltype(flux_div)(fieldName, dl_basal->qp_scalar);

  // Get Dimensions
  std::vector<PHX::DataLayout::size_type> dims;
  dl_basal->qp_vector->dimensions(dims);
  numSideQPs = dims[2];
  numSideDims  = dims[3];

  sideSetName = p.get<std::string> ("Side Set Name");

  // add dependent fields
  this->addDependentField(averaged_velocity);
  this->addDependentField(div_averaged_velocity);
  this->addDependentField(thickness);
  this->addDependentField(grad_thickness);
  this->addDependentField(side_tangents);

  this->addEvaluatedField(flux_div);

  this->setName("FluxDivergence"+PHX::typeAsString<EvalT>());
}

// **********************************************************************
template<typename EvalT, typename Traits>
void LandIce::FluxDiv<EvalT, Traits>::postRegistrationSetup(typename Traits::SetupData d, PHX::FieldManager<Traits>& fm)
{
  this->utils.setFieldData(averaged_velocity, fm);
  this->utils.setFieldData(div_averaged_velocity, fm);
  this->utils.setFieldData(thickness, fm);
  this->utils.setFieldData(grad_thickness, fm);
  this->utils.setFieldData(side_tangents, fm);
  this->utils.setFieldData(flux_div, fm);
}

// **********************************************************************
template<typename EvalT, typename Traits>
void LandIce::FluxDiv<EvalT, Traits>::evaluateFields(typename Traits::EvalData workset)
{
  TEUCHOS_TEST_FOR_EXCEPTION (workset.sideSets==Teuchos::null, std::runtime_error,
                              "Side sets defined in input file but not properly specified on the mesh.\n");

  if (workset.sideSets->find(sideSetName) != workset.sideSets->end())
  {
    const std::vector<Albany::SideStruct>& sideSet = workset.sideSets->at(sideSetName);
    for (auto const& it_side : sideSet)
    {
      // Get the local data of side and cell
      const int cell = it_side.elem_LID;
      const int side = it_side.side_local_id;

      ScalarT t = 0;
      for (int qp=0; qp<numSideQPs; ++qp)
      {
        ScalarT grad_thickness_tmp[2] = {0.0, 0.0};
        for (std::size_t dim = 0; dim < numSideDims; ++dim)
        {
          grad_thickness_tmp[0] += side_tangents(cell,side,qp,0,dim)*grad_thickness(cell,side,qp,dim);
          grad_thickness_tmp[1] += side_tangents(cell,side,qp,1,dim)*grad_thickness(cell,side,qp,dim);
        }

        ScalarT divHV = div_averaged_velocity(cell, side, qp)* thickness(cell, side, qp);
        for (std::size_t dim = 0; dim < numSideDims; ++dim)
          divHV += grad_thickness_tmp[dim]*averaged_velocity(cell, side, qp, dim);
        flux_div(cell, side, qp) = divHV;
      }
    }
  }
}
