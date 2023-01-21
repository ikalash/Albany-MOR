//*****************************************************************//
//    Albany 3.0:  Copyright 2016 Sandia Corporation               //
//    This Software is released under the BSD license detailed     //
//    in the file "license.txt" in the top-level Albany directory  //
//*****************************************************************//

#ifndef LANDICE_SCATTER_RESIDUAL2D_HPP
#define LANDICE_SCATTER_RESIDUAL2D_HPP

#include "PHAL_ScatterResidual.hpp"



namespace PHAL {
/** \brief Scatters result from the residual fields into the
    global (epetra) data structurs.  This includes the
    post-processing of the AD data type for all evaluation
    types besides Residual.

*/
// **************************************************************
// Base Class for code that is independent of evaluation type
// **************************************************************



template<typename EvalT, typename Traits> class ScatterResidual2D;
template<typename EvalT, typename Traits> class ScatterResidualWithExtrudedField;

// **************************************************************
// **************************************************************
// * Specializations
// **************************************************************
// **************************************************************


// **************************************************************
// Residual
// **************************************************************
template<typename Traits>
class ScatterResidual2D<PHAL::AlbanyTraits::Residual,Traits>
  : public ScatterResidual<PHAL::AlbanyTraits::Residual, Traits>  {
public:
  ScatterResidual2D(const Teuchos::ParameterList& p,
                              const Teuchos::RCP<Albany::Layouts>& dl);
};

// **************************************************************
// Jacobian
// **************************************************************
template<typename Traits>
class ScatterResidual2D<PHAL::AlbanyTraits::Jacobian,Traits>
  : public ScatterResidual<PHAL::AlbanyTraits::Jacobian, Traits>  {
public:
  ScatterResidual2D(const Teuchos::ParameterList& p,
                              const Teuchos::RCP<Albany::Layouts>& dl);
  void evaluateFields(typename Traits::EvalData d);
private:
  const std::size_t numFields;
  int fieldLevel;
  std::string meshPart;
  Teuchos::RCP<const CellTopologyData> cell_topo;
  typedef typename PHAL::AlbanyTraits::Jacobian::ScalarT ScalarT;
};

// **************************************************************
// Tangent
// **************************************************************
template<typename Traits>
class ScatterResidual2D<PHAL::AlbanyTraits::Tangent,Traits>
  : public ScatterResidual<PHAL::AlbanyTraits::Tangent, Traits>  {
public:
  ScatterResidual2D(const Teuchos::ParameterList& p,
                              const Teuchos::RCP<Albany::Layouts>& dl);
//  void evaluateFields(typename Traits::EvalData d);
};

// **************************************************************
// Distributed parameter derivative
// **************************************************************
template<typename Traits>
class ScatterResidual2D<PHAL::AlbanyTraits::DistParamDeriv,Traits>
  : public ScatterResidual<PHAL::AlbanyTraits::DistParamDeriv, Traits>  {
public:
  ScatterResidual2D(const Teuchos::ParameterList& p,
                  const Teuchos::RCP<Albany::Layouts>& dl);
//  void evaluateFields(typename Traits::EvalData d);
};

// **************************************************************
// Residual
// **************************************************************
template<typename Traits>
class ScatterResidualWithExtrudedField<PHAL::AlbanyTraits::Residual,Traits>
  : public ScatterResidual<PHAL::AlbanyTraits::Residual, Traits>  {
public:
  ScatterResidualWithExtrudedField(const Teuchos::ParameterList& p,
                              const Teuchos::RCP<Albany::Layouts>& dl);
};

// **************************************************************
// Jacobian
// **************************************************************
template<typename Traits>
class ScatterResidualWithExtrudedField<PHAL::AlbanyTraits::Jacobian,Traits>
  : public ScatterResidual<PHAL::AlbanyTraits::Jacobian, Traits>  {
public:
  ScatterResidualWithExtrudedField(const Teuchos::ParameterList& p,
                              const Teuchos::RCP<Albany::Layouts>& dl);
  void evaluateFields(typename Traits::EvalData d);
private:
  const std::size_t numFields;
  int offset2DField;
  int fieldLevel;
  typedef typename PHAL::AlbanyTraits::Jacobian::ScalarT ScalarT;
};

// **************************************************************
// Tangent
// **************************************************************
template<typename Traits>
class ScatterResidualWithExtrudedField<PHAL::AlbanyTraits::Tangent,Traits>
  : public ScatterResidual<PHAL::AlbanyTraits::Tangent, Traits>  {
public:
  ScatterResidualWithExtrudedField(const Teuchos::ParameterList& p,
                              const Teuchos::RCP<Albany::Layouts>& dl);
//  void evaluateFields(typename Traits::EvalData d);
};

// **************************************************************
// Distributed parameter derivative
// **************************************************************
template<typename Traits>
class ScatterResidualWithExtrudedField<PHAL::AlbanyTraits::DistParamDeriv,Traits>
  : public ScatterResidual<PHAL::AlbanyTraits::DistParamDeriv, Traits>  {
public:
  ScatterResidualWithExtrudedField(const Teuchos::ParameterList& p,
                  const Teuchos::RCP<Albany::Layouts>& dl);
//  void evaluateFields(typename Traits::EvalData d);
};

}

#endif
