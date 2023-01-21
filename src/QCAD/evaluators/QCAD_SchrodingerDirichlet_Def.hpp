//*****************************************************************//
//    Albany 3.0:  Copyright 2016 Sandia Corporation               //
//    This Software is released under the BSD license detailed     //
//    in the file "license.txt" in the top-level Albany directory  //
//*****************************************************************//

#include "Teuchos_TestForException.hpp"
#include "Phalanx_DataLayout.hpp"
#include "Sacado_ParameterRegistration.hpp"

#include "Albany_TpetraThyraUtils.hpp"

// **********************************************************************
// Genereric Template Code for Constructor and PostRegistrationSetup
// **********************************************************************

namespace QCAD {

template<typename EvalT,typename Traits>
SchrodingerDirichletBase<EvalT, Traits>::
SchrodingerDirichletBase(Teuchos::ParameterList& p) :
  offset(p.get<int>("Equation Offset")),
  nodeSetID(p.get<std::string>("Node Set ID"))
{
  value = p.get<RealType>("Dirichlet Value");

  std::string name = p.get< std::string >("Dirichlet Name");
  PHX::Tag<ScalarT> fieldTag(name, p.get< Teuchos::RCP<PHX::DataLayout> >("Data Layout"));

  this->addEvaluatedField(fieldTag);

  this->setName(name+PHX::typeAsString<EvalT>());

  // Set up values as parameters for parameter library
  Teuchos::RCP<ParamLib> paramLib = p.get< Teuchos::RCP<ParamLib> >
               ("Parameter Library", Teuchos::null);

  this->registerSacadoParameter(name, paramLib);
}

template<typename EvalT, typename Traits>
void SchrodingerDirichletBase<EvalT, Traits>::
postRegistrationSetup(typename Traits::SetupData d,
                      PHX::FieldManager<Traits>& fm)
{
}

// **********************************************************************
// Specialization: Residual
// **********************************************************************
template<typename Traits>
SchrodingerDirichlet<PHAL::AlbanyTraits::Residual, Traits>::
SchrodingerDirichlet(Teuchos::ParameterList& p) :
  SchrodingerDirichletBase<PHAL::AlbanyTraits::Residual, Traits>(p)
{
}

// **********************************************************************
template<typename Traits>
void SchrodingerDirichlet<PHAL::AlbanyTraits::Residual, Traits>::
evaluateFields(typename Traits::EvalData dirichletWorkset)
{
  
  Teuchos::RCP<Tpetra_Vector> fT = dirichletWorkset.fT;
  Teuchos::RCP<const Tpetra_Vector> xT = Albany::getConstTpetraVector(dirichletWorkset.x);
  Teuchos::ArrayRCP<const ST> xT_constView = xT->get1dView();
  Teuchos::ArrayRCP<ST> fT_nonconstView = fT->get1dViewNonConst();
  
  // Grab the vector off node GIDs for this Node Set ID from the std::map
  const std::vector<std::vector<int> >& nsNodes = dirichletWorkset.nodeSets->find(this->nodeSetID)->second;

  for (unsigned int inode = 0; inode < nsNodes.size(); inode++) {
    int lunk = nsNodes[inode][this->offset];

    fT_nonconstView[lunk] = xT_constView[lunk] - this->value;
  }
}

// **********************************************************************
// Specialization: Jacobian
// **********************************************************************
template<typename Traits>
SchrodingerDirichlet<PHAL::AlbanyTraits::Jacobian, Traits>::
SchrodingerDirichlet(Teuchos::ParameterList& p) :
  SchrodingerDirichletBase<PHAL::AlbanyTraits::Jacobian, Traits>(p)
{
}

// **********************************************************************
template<typename Traits>
void SchrodingerDirichlet<PHAL::AlbanyTraits::Jacobian, Traits>::
evaluateFields(typename Traits::EvalData dirichletWorkset)
{

  Teuchos::RCP<Tpetra_Vector> fT = dirichletWorkset.fT;
  Teuchos::RCP<const Tpetra_Vector> xT = Albany::getConstTpetraVector(dirichletWorkset.x);
  Teuchos::ArrayRCP<const ST> xT_constView = xT->get1dView();
  Teuchos::RCP<Tpetra_CrsMatrix> jacT = dirichletWorkset.JacT;


  const RealType j_coeff = dirichletWorkset.j_coeff;
  const std::vector<std::vector<int> >& nsNodes = dirichletWorkset.nodeSets->find(this->nodeSetID)->second;

  bool fillResid = (fT != Teuchos::null);
  Teuchos::ArrayRCP<ST> fT_nonconstView;
  if (fillResid) fT_nonconstView = fT->get1dViewNonConst();

  Teuchos::Array<LO> index(1);
  Teuchos::Array<ST> value(1);
  Teuchos::Array<ST> zero(1);
  size_t numEntriesT;
  value[0] = j_coeff;
  Teuchos::Array<ST> matrixEntriesT;
  Teuchos::Array<LO> matrixIndicesT;
  int nMyRows = jacT->getNodeNumRows();
  zero[0] = 0.0;
  //std::vector<int> globalRows(nMyRows);
  //jac->RowMap().MyGlobalElements(&globalRows[0]);

  for (unsigned int inode = 0; inode < nsNodes.size(); inode++) {
      int lunk = nsNodes[inode][this->offset];
      numEntriesT = jacT->getNumEntriesInLocalRow(lunk);
      matrixEntriesT.resize(numEntriesT);
      matrixIndicesT.resize(numEntriesT);
      jacT->getLocalRowCopy(lunk, matrixIndicesT(), matrixEntriesT(), numEntriesT);
      for (int i=0; i<numEntriesT; i++) matrixEntriesT[i]=0; // zero out row
      index[0] = lunk;
      for (int i=0; i<nMyRows; i++) jacT->replaceLocalValues(i, index(), zero()); //zero out col 
      //int gunk = globalRows[lunk]; // convert local row index -> global index

      jacT->replaceLocalValues(lunk, index(), value()); //set diagonal element = j_coeff

      if (fillResid)  fT_nonconstView[lunk] = xT_constView[lunk] - this->value.val();
    
  }
}

// **********************************************************************
// Specialization: Tangent
// **********************************************************************
template<typename Traits>
SchrodingerDirichlet<PHAL::AlbanyTraits::Tangent, Traits>::
SchrodingerDirichlet(Teuchos::ParameterList& p) :
  SchrodingerDirichletBase<PHAL::AlbanyTraits::Tangent, Traits>(p)
{
}

// **********************************************************************
template<typename Traits>
void SchrodingerDirichlet<PHAL::AlbanyTraits::Tangent, Traits>::
evaluateFields(typename Traits::EvalData dirichletWorkset)
{
  Teuchos::RCP<Tpetra_Vector> fT = dirichletWorkset.fT;
  Teuchos::RCP<Tpetra_MultiVector> fpT = dirichletWorkset.fpT;
  Teuchos::RCP<Tpetra_MultiVector> JVT = dirichletWorkset.JVT;
  Teuchos::RCP<const Tpetra_Vector> xT = Albany::getConstTpetraVector(dirichletWorkset.x);
  Teuchos::RCP<const Tpetra_MultiVector> VxT = Albany::getConstTpetraMultiVector(dirichletWorkset.Vx);

  Teuchos::ArrayRCP<const ST> VxT_constView; 
  Teuchos::ArrayRCP<ST> fT_nonconstView;                                         
  if (fT != Teuchos::null) fT_nonconstView = fT->get1dViewNonConst();
  Teuchos::ArrayRCP<const ST> xT_constView = xT->get1dView();                                       
  
  const RealType j_coeff = dirichletWorkset.j_coeff;
  const std::vector<std::vector<int> >& nsNodes = 
    dirichletWorkset.nodeSets->find(this->nodeSetID)->second;

  for (unsigned int inode = 0; inode < nsNodes.size(); inode++) {
    int lunk = nsNodes[inode][this->offset];

    if (fT != Teuchos::null) {
      fT_nonconstView[lunk] = xT_constView[lunk] - this->value.val();
    }

    if (JVT != Teuchos::null) {
      Teuchos::ArrayRCP<ST> JVT_nonconstView;
      for (int i=0; i<dirichletWorkset.num_cols_x; i++) {
        JVT_nonconstView = JVT->getDataNonConst(i); 
        VxT_constView = VxT->getData(i); 
        JVT_nonconstView[lunk] = j_coeff*VxT_constView[lunk];
      }
    }
    
    if (fpT != Teuchos::null) {
      Teuchos::ArrayRCP<ST> fpT_nonconstView; 
      for (int i=0; i<dirichletWorkset.num_cols_p; i++) {
        fpT_nonconstView = fpT->getDataNonConst(i); 
        fpT_nonconstView[lunk] = -this->value.dx(dirichletWorkset.param_offset+i);
      }
    }
  }
}

// **********************************************************************
// Specialization: DistParamDeriv
// **********************************************************************
template<typename Traits>
SchrodingerDirichlet<PHAL::AlbanyTraits::DistParamDeriv, Traits>::
SchrodingerDirichlet(Teuchos::ParameterList& p) :
  SchrodingerDirichletBase<PHAL::AlbanyTraits::DistParamDeriv, Traits>(p)
{
}

// **********************************************************************
template<typename Traits>
void SchrodingerDirichlet<PHAL::AlbanyTraits::DistParamDeriv, Traits>::
evaluateFields(typename Traits::EvalData dirichletWorkset)
{

  const std::vector<std::vector<int> >& nsNodes = 
    dirichletWorkset.nodeSets->find(this->nodeSetID)->second;

  Teuchos::RCP<Tpetra_MultiVector> fpVT = dirichletWorkset.fpVT;
  Teuchos::ArrayRCP<ST> fpVT_nonconstView; 
  bool trans = dirichletWorkset.transpose_dist_param_deriv;
  int num_cols = fpVT->getNumVectors();

  if (trans) {
    Teuchos::RCP<Tpetra_MultiVector> VpT = dirichletWorkset.Vp_bcT;
    Teuchos::ArrayRCP<ST> VpT_nonconstView; 
    for (unsigned int inode = 0; inode < nsNodes.size(); inode++) {
      int lunk = nsNodes[inode][this->offset];
  
      for (int i=0; i<num_cols; i++){
        //(*Vp)[i][lunk] = 0.0;
        VpT_nonconstView = VpT->getDataNonConst(i); 
        VpT_nonconstView[lunk] = 0.0; 
      }
    }
  }

  else {
    for (unsigned int inode = 0; inode < nsNodes.size(); inode++) {
      int lunk = nsNodes[inode][this->offset];

      for (int i=0; i<num_cols; i++) {
        //(*fpV)[i][lunk] = 0.0;
        fpVT_nonconstView = fpVT->getDataNonConst(i); 
        fpVT_nonconstView[lunk] = 0.0; 
      }
    }
  }
}

// **********************************************************************
// Simple evaluator to aggregate all SchrodingerDirichlet BCs into one "field"
// **********************************************************************

template<typename EvalT, typename Traits>
SchrodingerDirichletAggregator<EvalT, Traits>::
SchrodingerDirichletAggregator(Teuchos::ParameterList& p) 
{
  Teuchos::RCP<PHX::DataLayout> dl =  p.get< Teuchos::RCP<PHX::DataLayout> >("Data Layout");

  std::vector<std::string>& dbcs = *p.get<Teuchos::RCP<std::vector<std::string> > >("DBC Names");

  for (unsigned int i=0; i<dbcs.size(); i++) {
    PHX::Tag<ScalarT> fieldTag(dbcs[i], dl);
    this->addDependentField(fieldTag);
  }

  PHX::Tag<ScalarT> fieldTag(p.get<std::string>("DBC Aggregator Name"), dl);
  this->addEvaluatedField(fieldTag);

  this->setName("SchrodingerDirichlet Aggregator" + PHX::typeAsString<EvalT>());
}

// **********************************************************************
} // namespace QCAD
