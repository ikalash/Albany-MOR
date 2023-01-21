//*****************************************************************//
//    Albany 3.0:  Copyright 2016 Sandia Corporation               //
//    This Software is released under the BSD license detailed     //
//    in the file "license.txt" in the top-level Albany directory  //
//*****************************************************************//

#ifndef QCAD_SCHRODINGERDIRICHLET_HPP
#define QCAD_SCHRODINGERDIRICHLET_HPP

#include "Phalanx_config.hpp"
#include "Phalanx_Evaluator_WithBaseImpl.hpp"
#include "Phalanx_Evaluator_Derived.hpp"
#include "Phalanx_MDField.hpp"

#include "Teuchos_ParameterList.hpp"

#include "Sacado_ParameterAccessor.hpp"
#include "PHAL_AlbanyTraits.hpp"

namespace QCAD {
/** \brief Similar to PHAL::Dirichlet, but in Jacobian specialization
    zeros out the column as well as the row corresponding to a DBC to 
    preserve the symmetry of the Schrodinger problem's Hamiltonian (its
    jacobian).
*/
// **************************************************************
// Generic Template Impelementation for constructor and PostReg
// **************************************************************

template<typename EvalT, typename Traits>
class SchrodingerDirichletBase
  : public PHX::EvaluatorWithBaseImpl<Traits>,
    public PHX::EvaluatorDerived<EvalT, Traits>,
    public Sacado::ParameterAccessor<EvalT, SPL_Traits>  
   {
  
private:

  //typedef typename Traits::Residual::ScalarT ScalarT;
  typedef typename EvalT::ScalarT ScalarT;

public:
  
  SchrodingerDirichletBase(Teuchos::ParameterList& p);
  
  void postRegistrationSetup(typename Traits::SetupData d,
                             PHX::FieldManager<Traits>& vm);
  
  // This function will be overloaded with template specialized code
  void evaluateFields(typename Traits::EvalData d)=0;
  
  virtual ScalarT& getValue(const std::string &n) { return value; }

protected:
  const int offset;
  ScalarT value;
  std::string nodeSetID;
};

// **************************************************************
// **************************************************************
// * Specializations
// **************************************************************
// **************************************************************

template<typename EvalT, typename Traits> class SchrodingerDirichlet;

// **************************************************************
// Residual 
// **************************************************************
template<typename Traits>
class SchrodingerDirichlet<PHAL::AlbanyTraits::Residual,Traits>
   : public SchrodingerDirichletBase<PHAL::AlbanyTraits::Residual, Traits> {
public:
  SchrodingerDirichlet(Teuchos::ParameterList& p);
  void evaluateFields(typename Traits::EvalData d);
};

// **************************************************************
// Jacobian
// **************************************************************
template<typename Traits>
class SchrodingerDirichlet<PHAL::AlbanyTraits::Jacobian,Traits>
   : public SchrodingerDirichletBase<PHAL::AlbanyTraits::Jacobian, Traits> {
public:
  SchrodingerDirichlet(Teuchos::ParameterList& p);
  void evaluateFields(typename Traits::EvalData d);
};

// **************************************************************
// Tangent
// **************************************************************
template<typename Traits>
class SchrodingerDirichlet<PHAL::AlbanyTraits::Tangent,Traits>
   : public SchrodingerDirichletBase<PHAL::AlbanyTraits::Tangent, Traits> {
public:
  SchrodingerDirichlet(Teuchos::ParameterList& p);
  void evaluateFields(typename Traits::EvalData d);
};

// **************************************************************
// DistParamDeriv
// **************************************************************
template<typename Traits>
class SchrodingerDirichlet<PHAL::AlbanyTraits::DistParamDeriv,Traits>
   : public SchrodingerDirichletBase<PHAL::AlbanyTraits::DistParamDeriv, Traits> {
public:
  SchrodingerDirichlet(Teuchos::ParameterList& p);
  void evaluateFields(typename Traits::EvalData d);
};

// **************************************************************
// **************************************************************
// Evaluator to aggregate all SchrodingerDirichlet BCs into one "field"
// **************************************************************
template<typename EvalT, typename Traits>
class SchrodingerDirichletAggregator
  : public PHX::EvaluatorWithBaseImpl<Traits>,
    public PHX::EvaluatorDerived<EvalT, Traits>
{
private:

  typedef typename EvalT::ScalarT ScalarT;

public:
  
  SchrodingerDirichletAggregator(Teuchos::ParameterList& p);
  
  void postRegistrationSetup(typename Traits::SetupData d,
                             PHX::FieldManager<Traits>& vm) {};
  
  // This function will be overloaded with template specialized code
  void evaluateFields(typename Traits::EvalData d) {};
};
}

#endif
