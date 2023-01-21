/*
 * LandIce_LiquidWaterFraction_Def.hpp
 *
 *  Created on: Jun 6, 2016
 *      Author: abarone
 */

#include "Teuchos_TestForException.hpp"
#include "Teuchos_VerboseObject.hpp"
#include "Phalanx_DataLayout.hpp"
#include "Phalanx_TypeStrings.hpp"

namespace LandIce
{

  template<typename EvalT, typename Traits, typename Type>
  LiquidWaterFraction<EvalT,Traits,Type>::
  LiquidWaterFraction(const Teuchos::ParameterList& p, const Teuchos::RCP<Albany::Layouts>& dl):
  enthalpyHs (p.get<std::string> ("Enthalpy Hs Variable Name"), dl->node_scalar),
  enthalpy   (p.get<std::string> ("Enthalpy Variable Name"), dl->node_scalar),
  phi        (p.get<std::string> ("Water Content Variable Name"), dl->node_scalar)
  {
    // Get Dimensions
    std::vector<PHX::DataLayout::size_type> dims;
    dl->node_qp_vector->dimensions(dims);
    numNodes = dims[1];

    this->addDependentField(enthalpyHs);
    this->addDependentField(enthalpy);

    this->addEvaluatedField(phi);
    this->setName("Phi");

    // Setting parameters
    Teuchos::ParameterList& physics = *p.get<Teuchos::ParameterList*>("LandIce Physical Parameters");
    rho_w = physics.get<double>("Water Density");//, 1000.0);
    L = physics.get<double>("Latent heat of fusion");//, 334000.0);

    printedAlpha = -1.0;

  }

  template<typename EvalT, typename Traits, typename Type>
  void LiquidWaterFraction<EvalT,Traits,Type>::
  postRegistrationSetup(typename Traits::SetupData d, PHX::FieldManager<Traits>& fm)
  {
    this->utils.setFieldData(enthalpyHs,fm);
    this->utils.setFieldData(enthalpy,fm);

    this->utils.setFieldData(phi,fm);
  }

  template<typename EvalT, typename Traits, typename Type>
  void LiquidWaterFraction<EvalT,Traits,Type>::
  evaluateFields(typename Traits::EvalData d)
  {
    const double pow6 = 1e6; //[k^{-2}], k =1000
    //  double pi = atan(1.) * 4.;
    ScalarT phiNode;

    for (std::size_t cell = 0; cell < d.numCells; ++cell)
    {
      for (std::size_t node = 0; node < numNodes; ++node)
      {
        phi(cell,node) =  ( enthalpy(cell,node) < enthalpyHs(cell,node) ) ? ScalarT(0) : pow6 * (enthalpy(cell,node) - enthalpyHs(cell,node)) / (rho_w * L);
      }
    }
  }


}





