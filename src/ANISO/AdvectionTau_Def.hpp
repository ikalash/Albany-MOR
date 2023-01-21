//*****************************************************************//
//    Albany 3.0:  Copyright 2016 Sandia Corporation               //
//    This Software is released under the BSD license detailed     //
//    in the file "license.txt" in the top-level Albany directory  //
//*****************************************************************//

#include "Teuchos_TestForException.hpp"
#include "Phalanx_DataLayout.hpp"
#include "PHAL_Utilities.hpp"
#include "Intrepid2_FunctionSpaceTools.hpp"

namespace ANISO {

template<typename EvalT, typename Traits>
AdvectionTau<EvalT, Traits>::
AdvectionTau(
    const Teuchos::ParameterList& p,
    const Teuchos::RCP<Albany::Layouts>& dl) :
  coord     (p.get<std::string>("Coordinate Name"), dl->vertices_vector),
  grad_bf   (p.get<std::string>("Gradient BF Name"), dl->node_qp_vector),
  kappa     (p.get<std::string>("Kappa Name"), dl->qp_scalar),
  alpha_mag (p.get<std::string>("Alpha Magnitude Name"), dl->qp_scalar),
  tau       (p.get<std::string>("Tau Name"), dl->qp_scalar) {

  num_nodes = dl->node_qp_vector->dimension(1);
  num_qps = dl->node_qp_vector->dimension(2);
  num_dims = dl->node_qp_vector->dimension(3);
  num_vertices = dl->vertices_vector->dimension(1);

  this->addDependentField(coord);
  this->addDependentField(grad_bf);
  this->addDependentField(kappa);
  this->addDependentField(alpha_mag);
  this->addEvaluatedField(tau);
  this->setName("Advection Residual");
}

template<typename EvalT, typename Traits>
void AdvectionTau<EvalT, Traits>::
postRegistrationSetup(
    typename Traits::SetupData d,
    PHX::FieldManager<Traits>& fm) {

  this->utils.setFieldData(coord, fm);
  this->utils.setFieldData(grad_bf, fm);
  this->utils.setFieldData(kappa, fm);
  this->utils.setFieldData(alpha_mag, fm);
  this->utils.setFieldData(tau, fm);
}

template<typename EvalT, typename Traits>
void AdvectionTau<EvalT, Traits>::
evaluateFields(typename Traits::EvalData workset) {

  int num_edges = (num_vertices)*(num_vertices-1)/2;
  Teuchos::Array<double> edge_lengths_2(num_edges);

  for (int cell=0; cell < workset.numCells; ++cell) {

    // compute the element edge lengths
    // this needs to change for 3d
    edge_lengths_2[0] =
      std::pow(coord(cell, 1, 0) - coord(cell, 0, 0), 2) +
      std::pow(coord(cell, 1, 1) - coord(cell, 0, 1), 2);
    edge_lengths_2[1] =
      std::pow(coord(cell, 2, 0) - coord(cell, 1, 0), 2) +
      std::pow(coord(cell, 2, 1) - coord(cell, 1, 1), 2);
    edge_lengths_2[2] =
      std::pow(coord(cell, 0, 0) - coord(cell, 2, 0), 2) +
      std::pow(coord(cell, 0, 1) - coord(cell, 2, 1), 2);

    // compute the element mesh size
    MeshScalarT h = 0.0;
    for (int i=0; i < num_edges; ++i)
      h += edge_lengths_2[i];
    h = std::sqrt(h/num_edges);

    for (int qp=0; qp < num_qps; ++qp) {

      // compute tau
      ScalarT a = h*alpha_mag(cell, qp)/(2.0*kappa(cell, qp));
      tau(cell, qp) = h/(2.0*alpha_mag(cell, qp))*(1.0/std::tanh(a)-1.0/a);
    }
  }

}

}
