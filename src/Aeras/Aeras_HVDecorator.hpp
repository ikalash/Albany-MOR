//*****************************************************************//
//    Albany 3.0:  Copyright 2016 Sandia Corporation               //
//    This Software is released under the BSD license detailed     //
//    in the file "license.txt" in the top-level Albany directory  //
//*****************************************************************//

#if !defined(Aeras_HVDecorator_hpp)
#define Aeras_HVDecorator_hpp

#include "Albany_ModelEvaluatorT.hpp"
#include "Albany_DataTypes.hpp"
#include "Thyra_DefaultProductVector.hpp"
#include "Thyra_DefaultProductVectorSpace.hpp"

#include "Thyra_ModelEvaluatorDefaultBase.hpp"

namespace Aeras {

///
/// \brief Definition for the HyperViscosityDecorator
///
class HVDecorator: public Albany::ModelEvaluatorT {

public:

  /// Constructor
  HVDecorator(
      const Teuchos::RCP<Albany::Application>& app,
      const Teuchos::RCP<Teuchos::ParameterList>& appParams);

  Teuchos::RCP<Tpetra_CrsMatrix> createOperator(double alpha, double beta, double omega, bool xdotdot_nonnull);
  
  //IKT, 1/20/16: added the following function for the mass matrix.  createOperator returns a non-diagonal 
  //matrix, namely the Laplace, whereas the mass matrix should be diagonal. 
  Teuchos::RCP<Tpetra_CrsMatrix> createOperatorDiag(double alpha, double beta, double omega, bool xdotdot_nonnull);

  void applyLinvML(Teuchos::RCP<const Tpetra_Vector> x_in, Teuchos::RCP<Tpetra_Vector> x_out) const; 

protected:

  //! Evaluate model on InArgs
  void evalModelImpl(
      const Thyra::ModelEvaluatorBase::InArgs<ST>& inArgs,
      const Thyra::ModelEvaluatorBase::OutArgs<ST>& outArgs) const;

private: 
  //Mass and Laplace operators
  Teuchos::RCP<Tpetra_CrsMatrix> laplace_; 
  Teuchos::RCP<Tpetra_Vector> inv_mass_diag_, wrk_;
  Teuchos::RCP<Tpetra_Vector> xtildeT; 
};

}

#endif // ALBANY_HVDECORATOR_HPP
