//*****************************************************************//
//    Albany 3.0:  Copyright 2016 Sandia Corporation               //
//    This Software is released under the BSD license detailed     //
//    in the file "license.txt" in the top-level Albany directory  //
//*****************************************************************//

#include <vector>
#include <string>

#include "Teuchos_TestForException.hpp"
#include "Phalanx_DataLayout.hpp"
#include "PHAL_Utilities.hpp"

namespace PHAL {

template<typename EvalT, typename Traits, typename ScalarType>
LoadStateFieldBase<EvalT, Traits, ScalarType>::
LoadStateFieldBase(const Teuchos::ParameterList& p)
{
  if (p.isType<bool>("Enable Memoizer") && p.get<bool>("Enable Memoizer"))
    memoizer.enable_memoizer();

  fieldName =  p.get<std::string>("Field Name");
  stateName =  p.get<std::string>("State Name");

  PHX::MDField<ScalarType> f(fieldName, p.get<Teuchos::RCP<PHX::DataLayout> >("State Field Layout") );
  data = f;

  this->addEvaluatedField(data);
  this->setName("Load State Field"+PHX::typeAsString<EvalT>());
}

// **********************************************************************
template<typename EvalT, typename Traits, typename ScalarType>
void LoadStateFieldBase<EvalT, Traits, ScalarType>::postRegistrationSetup(typename Traits::SetupData d,
                      PHX::FieldManager<Traits>& fm)
{
  this->utils.setFieldData(data,fm);
}

// **********************************************************************
template<typename EvalT, typename Traits, typename ScalarType>
void LoadStateFieldBase<EvalT, Traits, ScalarType>::evaluateFields(typename Traits::EvalData workset)
{
  if (memoizer.have_stored_data(workset)) return;

  //cout << "LoadStateFieldBase importing state " << stateName << " to field "
  //     << fieldName << " with size " << data.size() << endl;

  const Albany::MDArray& stateToLoad = (*workset.stateArrayPtr)[stateName];
  PHAL::MDFieldIterator<ScalarType> d(data);
  for (int i = 0; ! d.done() && i < stateToLoad.size(); ++d, ++i)
    *d = stateToLoad[i];
  for ( ; ! d.done(); ++d) *d = 0.;
}



template<typename EvalT, typename Traits>
LoadStateField<EvalT, Traits>::
LoadStateField(const Teuchos::ParameterList& p) 
{
  if (p.isType<bool>("Enable Memoizer") && p.get<bool>("Enable Memoizer"))
    memoizer.enable_memoizer();

  fieldName =  p.get<std::string>("Field Name");
  stateName =  p.get<std::string>("State Name");

  PHX::MDField<ParamScalarT> f(fieldName, p.get<Teuchos::RCP<PHX::DataLayout> >("State Field Layout") );
  data = f;

  this->addEvaluatedField(data);
  this->setName("Load State Field"+PHX::typeAsString<EvalT>());
}

// **********************************************************************
template<typename EvalT, typename Traits> 
void LoadStateField<EvalT, Traits>::postRegistrationSetup(typename Traits::SetupData d,
                      PHX::FieldManager<Traits>& fm)
{
  this->utils.setFieldData(data,fm);
}

// **********************************************************************
template<typename EvalT, typename Traits>
void LoadStateField<EvalT, Traits>::evaluateFields(typename Traits::EvalData workset)
{
  if (memoizer.have_stored_data(workset)) return;

  //cout << "LoadStateField importing state " << stateName << " to field " 
  //     << fieldName << " with size " << data.size() << endl;

  const Albany::MDArray& stateToLoad = (*workset.stateArrayPtr)[stateName];
  PHAL::MDFieldIterator<ParamScalarT> d(data);
  for (int i = 0; ! d.done() && i < stateToLoad.size(); ++d, ++i)
    *d = stateToLoad[i];
  for ( ; ! d.done(); ++d) *d = 0.;
}

// **********************************************************************
}

