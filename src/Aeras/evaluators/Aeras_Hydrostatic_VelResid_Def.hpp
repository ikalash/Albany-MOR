//*****************************************************************//
//    Albany 3.0:  Copyright 2016 Sandia Corporation               //
//    This Software is released under the BSD license detailed     //
//    in the file "license.txt" in the top-level Albany directory  //
//*****************************************************************//

#include "Teuchos_TestForException.hpp"
#include "Teuchos_VerboseObject.hpp"
#include "Teuchos_RCP.hpp"
#include "Phalanx_DataLayout.hpp"
#include "Sacado_ParameterRegistration.hpp"
#include "PHAL_Utilities.hpp"
#include "Albany_Utils.hpp"

#include "Intrepid2_FunctionSpaceTools.hpp"
#include "Aeras_Layouts.hpp"
#include "Aeras_ShallowWaterConstants.hpp"

namespace Aeras {

//**********************************************************************
template<typename EvalT, typename Traits>
Hydrostatic_VelResid<EvalT, Traits>::
Hydrostatic_VelResid(const Teuchos::ParameterList& p,
                       const Teuchos::RCP<Aeras::Layouts>& dl) :
  wBF         (p.get<std::string> ("Weighted BF Name"),                 dl->node_qp_scalar),
  GradBF      (p.get<std::string>   ("Gradient BF Name"),               dl->node_qp_gradient),
  wGradBF     (p.get<std::string> ("Weighted Gradient BF Name"),        dl->node_qp_gradient),
  keGrad      (p.get<std::string> ("Gradient QP Kinetic Energy"),       dl->qp_gradient_level),
  PhiGrad     (p.get<std::string> ("Gradient QP GeoPotential"),         dl->qp_gradient_level),
  etadotdVelx (p.get<std::string> ("EtaDotdVelx"),                      dl->node_vector_level),
  pGrad       (p.get<std::string> ("Gradient QP Pressure"),             dl->qp_gradient_level),
  VelxNode    (p.get<std::string> ("Velx"),                             dl->node_vector_level),
  Velx        (p.get<std::string> ("QP Velx"),                          dl->node_vector_level),
  VelxDot     (p.get<std::string> ("QP Time Derivative Variable Name"), dl->node_vector_level),
  DVelx       (p.get<std::string> ("D Vel Name"),                       dl->qp_vector_level),
  density     (p.get<std::string> ("QP Density"),                       dl->node_scalar_level),
  sphere_coord  (p.get<std::string>  ("Spherical Coord Name"), dl->qp_gradient ),
  vorticity    (p.get<std::string>  ("QP Vorticity"), dl->qp_scalar_level ),
  jacobian_det  (p.get<std::string>  ("Jacobian Det Name"), dl->qp_scalar ),
  jacobian  (p.get<std::string>  ("Jacobian Name"), dl->qp_tensor ),

  Residual    (p.get<std::string> ("Residual Name"),                    dl->node_vector_level),

  viscosity   (p.get<Teuchos::ParameterList*>("Hydrostatic Problem")  ->get<double>("Viscosity", 0.0)),
  hyperviscosity (p.get<Teuchos::ParameterList*>("Hydrostatic Problem")  ->get<double>("HyperViscosity", 0.0)),
  AlphaAngle (p.get<Teuchos::ParameterList*>("Hydrostatic Problem")  ->get<double>("Rotation Angle", 0.0)),
  pureAdvection (p.get<Teuchos::ParameterList*>("Hydrostatic Problem")  ->get<bool>("Pure Advection", false)),
  //AlphaAngle (p.isParameter("XZHydrostatic Problem") ? 
  //              p.get<Teuchos::ParameterList*>("XZHydrostatic Problem")->get<double>("Rotation Angle", 0.0):
  //              p.get<Teuchos::ParameterList*>("Hydrostatic Problem")  ->get<double>("Rotation Angle", 0.0)),
  Omega( p.isParameter("Hydrostatic Problem") && 
  		 p.get<Teuchos::ParameterList*>("Hydrostatic Problem")->isParameter("NonRotating") ? 0.0 :
	     2.0*(Aeras::ShallowWaterConstants::self().pi)/(24.*3600.)),

  numNodes    ( dl->node_scalar             ->dimension(1)),
  numQPs      ( dl->node_qp_scalar          ->dimension(2)),
  numDims     ( dl->node_qp_gradient        ->dimension(3)),
  numLevels   ( dl->node_scalar_level       ->dimension(2))
{
  this->addDependentField(keGrad);
  this->addDependentField(PhiGrad);
  this->addDependentField(density);
  this->addDependentField(etadotdVelx);
  this->addDependentField(DVelx);
  this->addDependentField(pGrad);
  this->addDependentField(VelxNode);
  this->addDependentField(Velx);
  this->addDependentField(VelxDot);
  this->addDependentField(wBF);
  this->addDependentField(wGradBF);
  this->addDependentField(sphere_coord);
  this->addDependentField(vorticity);
  this->addDependentField(jacobian);
  this->addDependentField(jacobian_det);

  this->addEvaluatedField(Residual);

  this->setName("Aeras::Hydrostatic_VelResid" + PHX::typeAsString<EvalT>());

  //refWeights        .resize               (numQPs);
  //grad_at_cub_points.resize     (numNodes, numQPs, 2);
  //refPoints         .resize               (numQPs, 2);

  //cubature->getCubature(refPoints, refWeights);
  //intrepidBasis->getValues(grad_at_cub_points, refPoints, Intrepid2::OPERATOR_GRAD);

}

//**********************************************************************
template<typename EvalT, typename Traits>
void Hydrostatic_VelResid<EvalT, Traits>::
postRegistrationSetup(typename Traits::SetupData d,
                      PHX::FieldManager<Traits>& fm)
{
  this->utils.setFieldData(keGrad     , fm);
  this->utils.setFieldData(PhiGrad    , fm);
  this->utils.setFieldData(density    , fm);
  this->utils.setFieldData(etadotdVelx, fm);
  this->utils.setFieldData(DVelx      , fm);
  this->utils.setFieldData(pGrad      , fm);
  this->utils.setFieldData(VelxNode   , fm);
  this->utils.setFieldData(Velx       , fm);
  this->utils.setFieldData(VelxDot    , fm);
  this->utils.setFieldData(wBF        , fm);
  this->utils.setFieldData(wGradBF    , fm);
  this->utils.setFieldData(sphere_coord,fm);
  this->utils.setFieldData(vorticity  , fm);
  this->utils.setFieldData(jacobian, fm);
  this->utils.setFieldData(jacobian_det, fm);

  this->utils.setFieldData(Residual,fm);
}

//**********************************************************************
// Kokkos kernels
#ifdef ALBANY_KOKKOS_UNDER_DEVELOPMENT
template<typename EvalT, typename Traits>
KOKKOS_INLINE_FUNCTION
void Hydrostatic_VelResid<EvalT, Traits>::
operator() (const Hydrostatic_VelResid_Tag& tag, const int& cell) const{
  for (int node=0; node < numNodes; ++node) {
    // Compute Coriolis
    const double alpha = AlphaAngle;
    const MeshScalarT lambda = sphere_coord(cell, node, 0);
    const MeshScalarT theta = sphere_coord(cell, node, 1);
    const ScalarT coriolis = 2*Omega*( -cos(lambda)*cos(theta)*sin(alpha) + sin(theta)*cos(alpha));

    // Compute Residual
    for (int level=0; level < numLevels; ++level) {
      for (int dim=0; dim < numDims; ++dim) {
        Residual(cell,node,level,dim) =  ( keGrad(cell,node,level,dim) + PhiGrad(cell,node,level,dim) )
                                      +  ( pGrad (cell,node,level,dim)/density(cell,node,level) )
                                      +   etadotdVelx(cell,node,level,dim)
                                      +   VelxDot(cell,node,level,dim);
      }
      Residual(cell,node,level,0) -= (vorticity(cell,node,level) + coriolis)*Velx(cell,node,level,1);
      Residual(cell,node,level,1) += (vorticity(cell,node,level) + coriolis)*Velx(cell,node,level,0);
      Residual(cell,node,level,0) *= wBF(cell,node,node);
      Residual(cell,node,level,1) *= wBF(cell,node,node);
    }
    for (int qp=0; qp < numQPs; ++qp) {
      for (int level = 0; level < 2; ++level ) {
        for (int dim = 0; dim < numDims; ++dim) {
          Residual(cell, node, level, dim) += viscosity * DVelx(cell, qp, level, dim) * wGradBF(cell, node, qp, dim);  
    	}
      }
    }
  }
}

template<typename EvalT, typename Traits>
KOKKOS_INLINE_FUNCTION
void Hydrostatic_VelResid<EvalT, Traits>::
operator() (const Hydrostatic_VelResid_pureAdvection_Tag& tag, const int& cell) const{
  for (int node=0; node < numNodes; ++node)
    for (int level=0; level < numLevels; ++level)
      for (int dim=0; dim < numDims; ++dim)
        Residual(cell,node,level,dim) =   VelxDot(cell,node,level,dim) *wBF(cell,node,node); 
}

#endif

//**********************************************************************
template<typename EvalT, typename Traits>
void Hydrostatic_VelResid<EvalT, Traits>::
evaluateFields(typename Traits::EvalData workset)
{
  double j_coeff = workset.j_coeff;
  double n_coeff = workset.n_coeff;
  obtainLaplaceOp = ((n_coeff == 22.0)&&(j_coeff == 1.0)) ? true : false;

#ifndef ALBANY_KOKKOS_UNDER_DEVELOPMENT
  PHAL::set(Residual, 0.0);
  //std::cout <<"In velocity resid: Laplace = " << obtainLaplaceOp << "\n";

  //OG I had an segfault when moving this statement uder (if not Laplace op), where it belongs.
  //To be looked at later.
  Kokkos::DynRankView<ScalarT, PHX::Device>  coriolis = Kokkos::createDynRankView(Velx.get_view(), "XXX",numQPs);
  //Kokkos::DynRankView<ScalarT, PHX::Device>  vorticity(numQPs);

  if ( !obtainLaplaceOp ) {
    if (!pureAdvection ) {
      for (int cell=0; cell < workset.numCells; ++cell) {
		for (int level=0; level < numLevels; ++level) {
	  	get_coriolis(cell, coriolis);
	  	//get_vorticity(VelxNode, cell, level, vorticity);
	  		for (int qp=0; qp < numQPs; ++qp) {
				int node = qp;
				for (int dim=0; dim < numDims; ++dim) {
				  Residual(cell,node,level,dim) += ( keGrad(cell,qp,level,dim) + PhiGrad(cell,qp,level,dim) );
				  Residual(cell,node,level,dim) += ( pGrad (cell,qp,level,dim)/density(cell,qp,level) )      ;
				  Residual(cell,node,level,dim) +=   etadotdVelx(cell,qp,level,dim)                          ;
				  Residual(cell,node,level,dim) +=   VelxDot(cell,qp,level,dim)                              ;
				}
				Residual(cell,node,level,0) -= (vorticity(cell,qp,level) + coriolis(qp))*Velx(cell,qp,level,1);
				Residual(cell,node,level,1) += (vorticity(cell,qp,level) + coriolis(qp))*Velx(cell,qp,level,0);
				Residual(cell,node,level,0) *= wBF(cell,node,qp);
				Residual(cell,node,level,1) *= wBF(cell,node,qp);
		    }
		}
		for (int level = 0; level < 2; ++level) {
			for ( int dim = 0; dim < numDims; ++dim ) {
				for ( int qp = 0; qp < numQPs; ++qp ) {
					int node = qp;
				Residual(cell, node, level, dim) += viscosity * DVelx(cell, qp, level, dim) * wGradBF(cell, node, qp, dim);
				}
			}
		}
      }
    }//end of (if not pureAdvection)

    else {
      for (int cell=0; cell < workset.numCells; ++cell)
        for (int level=0; level < numLevels; ++level)
          for (int node=0; node < numNodes; ++node)
	    for (int dim=0; dim < numDims; ++dim)
	      Residual(cell,node,level,dim) +=   VelxDot(cell,node,level,dim) *wBF(cell,node,node); 
    }
  }//end of (if not Laplace operator)

  else {
	  //to be implemented
  }

#else
  if ( !obtainLaplaceOp ) {
    if (!pureAdvection ) {
      Kokkos::parallel_for(Hydrostatic_VelResid_Policy(0,workset.numCells),*this);
      cudaCheckError();
    }

    else {
      Kokkos::parallel_for(Hydrostatic_VelResid_pureAdvection_Policy(0,workset.numCells),*this);
      cudaCheckError();
    }
  }

  else {
    //to be implemented
    PHAL::set(Residual, 0.0);
  }

#endif
}

template<typename EvalT,typename Traits>
void
Hydrostatic_VelResid<EvalT,Traits>::get_coriolis(std::size_t cell, Kokkos::DynRankView<ScalarT, PHX::Device>  & coriolis) {

  // AGS: not needed == coriolis is = not += below.
  //Kokkos::deep_copy(coriolis, 0.0);
  double alpha = AlphaAngle; 

//  std::cout << "In coriolis alpha = " << alpha << "\n";

  for (std::size_t qp=0; qp < numQPs; ++qp) {
    const MeshScalarT lambda = sphere_coord(cell, qp, 0);
    const MeshScalarT theta = sphere_coord(cell, qp, 1);
    coriolis(qp) = 2*Omega*( -cos(lambda)*cos(theta)*sin(alpha) + sin(theta)*cos(alpha));
  }

}

// *********************************************************************

//template<typename EvalT,typename Traits>
//void
//Hydrostatic_VelResid<EvalT,Traits>::get_vorticity(const Kokkos::DynRankView<ScalarT, PHX::Device>  & nodalVector,
//    std::size_t cell, std::size_t level, Kokkos::DynRankView<ScalarT, PHX::Device>  & curl) {
//
//  Kokkos::DynRankView<ScalarT, PHX::Device>& covariantVector = wrk_;
//  covariantVector.initialize();
//
//  fill_nodal_metrics(cell);
//
//  covariantVector.initialize();
//  curl.initialize();
//
//  for (std::size_t node=0; node < numNodes; ++node) {
//
//    const MeshScalarT j00 = jacobian(node, 0, 0);
//    const MeshScalarT j01 = jacobian(node, 0, 1);
//    const MeshScalarT j10 = jacobian(node, 1, 0);
//    const MeshScalarT j11 = jacobian(node, 1, 1);
//
//    covariantVector(node, 0 ) = j00*nodalVector(cell, node, level, 0) + j10*nodalVector(cell, node, level, 1);
//    covariantVector(node, 1 ) = j01*nodalVector(cell, node, level, 0) + j11*nodalVector(cell, node, level, 1);
//  }
//
//
//  for (std::size_t qp=0; qp < numQPs; ++qp) {
//    for (std::size_t node=0; node < numNodes; ++node) {
//
//      curl(qp) +=   covariantVector(node, 1)*grad_at_cub_points(node, qp,0)
//                  - covariantVector(node, 0)*grad_at_cub_points(node, qp,1);
//    }
//    curl(qp) = curl(qp)/jacobian_det(cell,qp);
//  }
//
//}

}
