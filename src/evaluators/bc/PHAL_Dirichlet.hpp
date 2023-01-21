//*****************************************************************//
//    Albany 3.0:  Copyright 2016 Sandia Corporation               //
//    This Software is released under the BSD license detailed     //
//    in the file "license.txt" in the top-level Albany directory  //
//*****************************************************************//

#ifndef PHAL_DIRICHLET_HPP
#define PHAL_DIRICHLET_HPP

#include "Phalanx_config.hpp"
#include "Phalanx_Evaluator_WithBaseImpl.hpp"
#include "Phalanx_Evaluator_Derived.hpp"
#include "Phalanx_MDField.hpp"

#include "Teuchos_ParameterList.hpp"

#include "Sacado_ParameterAccessor.hpp"
#include "PHAL_AlbanyTraits.hpp"

namespace PHAL {
/** \brief Gathers solution values from the Newton solution vector into
    the nodal fields of the field manager

    Currently makes an assumption that the stride is constant for dofs
    and that the nmber of dofs is equal to the size of the solution
    names vector.

*/
// **************************************************************
// Generic Template Impelementation for constructor and PostReg
// **************************************************************

template<typename EvalT, typename Traits>
class DirichletBase
  : public PHX::EvaluatorWithBaseImpl<Traits>,
    public PHX::EvaluatorDerived<EvalT, Traits>,
    public Sacado::ParameterAccessor<EvalT, SPL_Traits>
{

private:

  //typedef typename Traits::Residual::ScalarT ScalarT;
  typedef typename EvalT::ScalarT ScalarT;

public:

  DirichletBase(Teuchos::ParameterList& p);

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

template<typename EvalT, typename Traits> class Dirichlet;

// **************************************************************
// Residual
// **************************************************************
template<typename Traits>
class Dirichlet<PHAL::AlbanyTraits::Residual,Traits>
   : public DirichletBase<PHAL::AlbanyTraits::Residual, Traits> {
public:
  Dirichlet(Teuchos::ParameterList& p);
  void evaluateFields(typename Traits::EvalData d);
};

// **************************************************************
// Jacobian
// **************************************************************
template<typename Traits>
class Dirichlet<PHAL::AlbanyTraits::Jacobian,Traits>
   : public DirichletBase<PHAL::AlbanyTraits::Jacobian, Traits> {
public:
  Dirichlet(Teuchos::ParameterList& p);
  void evaluateFields(typename Traits::EvalData d);
};

// **************************************************************
// Tangent
// **************************************************************
template<typename Traits>
class Dirichlet<PHAL::AlbanyTraits::Tangent,Traits>
   : public DirichletBase<PHAL::AlbanyTraits::Tangent, Traits> {
public:
  Dirichlet(Teuchos::ParameterList& p);
  void evaluateFields(typename Traits::EvalData d);
};

// **************************************************************
// Distributed Parameter Derivative
// **************************************************************
template<typename Traits>
class Dirichlet<PHAL::AlbanyTraits::DistParamDeriv,Traits>
   : public DirichletBase<PHAL::AlbanyTraits::DistParamDeriv, Traits> {
public:
  Dirichlet(Teuchos::ParameterList& p);
  void evaluateFields(typename Traits::EvalData d);
};

// **************************************************************
// **************************************************************
// Evaluator to aggregate all Dirichlet BCs into one "field"
// **************************************************************
template<typename EvalT, typename Traits>
class DirichletAggregator
  : public PHX::EvaluatorWithBaseImpl<Traits>,
    public PHX::EvaluatorDerived<EvalT, Traits>
{
private:

  typedef typename EvalT::ScalarT ScalarT;

public:

  DirichletAggregator(Teuchos::ParameterList& p);

  void postRegistrationSetup(typename Traits::SetupData d,
                             PHX::FieldManager<Traits>& vm) {};

  // This function will be overloaded with template specialized code
  void evaluateFields(typename Traits::EvalData d) {};
};
}

#endif
