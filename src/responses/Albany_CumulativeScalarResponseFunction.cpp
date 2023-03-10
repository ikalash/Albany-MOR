//*****************************************************************//
//    Albany 3.0:  Copyright 2016 Sandia Corporation               //
//    This Software is released under the BSD license detailed     //
//    in the file "license.txt" in the top-level Albany directory  //
//*****************************************************************//


#include "Albany_CumulativeScalarResponseFunction.hpp"
#include "Albany_Application.hpp"

using Teuchos::RCP;
using Teuchos::rcp;

Albany::CumulativeScalarResponseFunction::
CumulativeScalarResponseFunction(
  const Teuchos::RCP<const Teuchos_Comm>& commT,
  const Teuchos::Array< Teuchos::RCP<ScalarResponseFunction> >& responses_) :
  SamplingBasedScalarResponseFunction(commT),
  responses(responses_),
  num_responses(0)
{
  if(responses.size() > 0)
    num_responses = responses[0]->numResponses();
}

void
Albany::CumulativeScalarResponseFunction::
setup()
{
  typedef Teuchos::Array<Teuchos::RCP<ScalarResponseFunction> > ResponseArray;
  for (ResponseArray::iterator it = responses.begin(), it_end = responses.end(); it != it_end; ++it) {
    (*it)->setup();
  }
}

void
Albany::CumulativeScalarResponseFunction::
postRegSetup()
{
  typedef Teuchos::Array<Teuchos::RCP<ScalarResponseFunction> > ResponseArray;
  for (ResponseArray::iterator it = responses.begin(), it_end = responses.end(); it != it_end; ++it) {
    (*it)->postRegSetup();
  }
}

Albany::CumulativeScalarResponseFunction::
~CumulativeScalarResponseFunction()
{
}

unsigned int
Albany::CumulativeScalarResponseFunction::
numResponses() const 
{
  return num_responses;
}

void
Albany::CumulativeScalarResponseFunction::
evaluateResponse(const double current_time,
    const Teuchos::RCP<const Thyra_Vector>& x,
    const Teuchos::RCP<const Thyra_Vector>& xdot,
    const Teuchos::RCP<const Thyra_Vector>& xdotdot,
		const Teuchos::Array<ParamVec>& p,
		Tpetra_Vector& gT)
{
  gT.putScalar(0);

  Teuchos::ArrayRCP<ST> gT_nonconstView = gT.get1dViewNonConst();
  for (unsigned int i=0; i<responses.size(); i++) {
    Teuchos::RCP<const Teuchos::Comm<int> > commT = responses[i]->getComm(); 
    Tpetra::LocalGlobal lg = Tpetra::LocallyReplicated;
    Teuchos::RCP<Tpetra_Map> local_response_map = Teuchos::rcp(new Tpetra_Map(num_responses, 0, commT, lg));
    
    // Create Tpetra_Vector for response function
    Teuchos::RCP<Tpetra_Vector> local_gT = Teuchos::rcp(new Tpetra_Vector(local_response_map));
  
    // Evaluate response function
    responses[i]->evaluateResponse(current_time, x, xdot, xdotdot, p, *local_gT);
    
    //get views of g and local_g for element access
    Teuchos::ArrayRCP<const ST> local_gT_constView = local_gT->get1dView();

    // Add result into cumulative result
    for (unsigned int j=0; j<num_responses; j++)
      gT_nonconstView[j] += local_gT_constView[j];
  }
}

void
Albany::CumulativeScalarResponseFunction::
evaluateTangent(const double alpha, 
		const double beta,
		const double omega,
		const double current_time,
		bool sum_derivs,
    const Teuchos::RCP<const Thyra_Vector>& x,
    const Teuchos::RCP<const Thyra_Vector>& xdot,
    const Teuchos::RCP<const Thyra_Vector>& xdotdot,
		const Teuchos::Array<ParamVec>& p,
		ParamVec* deriv_p,
    const Teuchos::RCP<const Thyra_MultiVector>& Vx,
    const Teuchos::RCP<const Thyra_MultiVector>& Vxdot,
    const Teuchos::RCP<const Thyra_MultiVector>& Vxdotdot,
    const Teuchos::RCP<const Thyra_MultiVector>& Vp,
		Tpetra_Vector* gT,
		Tpetra_MultiVector* gxT,
		Tpetra_MultiVector* gpT)
{
  //zero-out vecotres
  if (gT != NULL)
    gT->putScalar(0);
  if (gxT != NULL)
    gxT->putScalar(0);
  if (gpT != NULL)
    gpT->putScalar(0);

  for (unsigned int i=0; i<responses.size(); i++) {

    // Create Tpetra_Map for response function
    unsigned int num_responses = responses[i]->numResponses();
    Teuchos::RCP<const Teuchos::Comm<int> > commT = responses[i]->getComm(); 
    Tpetra::LocalGlobal lg = Tpetra::LocallyReplicated;
    Teuchos::RCP<Tpetra_Map> local_response_map = Teuchos::rcp(new Tpetra_Map(num_responses, 0, commT, lg));

    // Create Tpetra_Vectors for response function
    RCP<Tpetra_Vector> local_gT;
    RCP<Tpetra_MultiVector> local_gxT, local_gpT;
    if (gT != NULL)
      local_gT = rcp(new Tpetra_Vector(local_response_map));
    if (gxT != NULL)
      local_gxT = rcp(new Tpetra_MultiVector(local_response_map, 
					    gxT->getNumVectors()));
    if (gpT != NULL)
      local_gpT = rcp(new Tpetra_MultiVector(local_response_map, 
					    gpT->getNumVectors()));

    // Evaluate response function
    responses[i]->evaluateTangent(alpha, beta, omega, current_time, sum_derivs,
				  x, xdot, xdotdot, p, deriv_p, Vx, Vxdot, Vxdotdot, Vp, 
				  local_gT.get(), local_gxT.get(), local_gpT.get());

    Teuchos::ArrayRCP<const ST> local_gT_constView;
    Teuchos::ArrayRCP<ST> gT_nonconstView;
    if (gT != NULL) { 
      local_gT_constView = local_gT->get1dView();
      gT_nonconstView = gT->get1dViewNonConst();
    }

    // Copy results into combined result
    for (unsigned int j=0; j<num_responses; j++) {
      if (gT != NULL)
        gT_nonconstView[j] += local_gT_constView[j];
      if (gxT != NULL) {
        Teuchos::ArrayRCP<ST> gxT_nonconstView;
        Teuchos::ArrayRCP<const ST> local_gxT_constView;
	for (int k=0; k<gxT->getNumVectors(); k++) {
          gxT_nonconstView = gxT->getDataNonConst(k); 
          local_gxT_constView = local_gxT->getData(k); 
	  gxT_nonconstView[j] += local_gxT_constView[j];
         }
      }
      if (gpT != NULL) {
        Teuchos::ArrayRCP<ST> gpT_nonconstView;
        Teuchos::ArrayRCP<const ST> local_gpT_constView;
	for (int k=0; k<gpT->getNumVectors(); k++) {
          gpT_nonconstView = gpT->getDataNonConst(k); 
          local_gpT_constView = local_gpT->getData(k); 
	  gpT_nonconstView[j] += local_gpT_constView[j];
        }
      }
    }
  }
}

void
Albany::CumulativeScalarResponseFunction::
evaluateGradient(const double current_time,
    const Teuchos::RCP<const Thyra_Vector>& x,
    const Teuchos::RCP<const Thyra_Vector>& xdot,
    const Teuchos::RCP<const Thyra_Vector>& xdotdot,
		const Teuchos::Array<ParamVec>& p,
		ParamVec* deriv_p,
		Tpetra_Vector* gT,
		Tpetra_MultiVector* dg_dxT,
		Tpetra_MultiVector* dg_dxdotT,
		Tpetra_MultiVector* dg_dxdotdotT,
		Tpetra_MultiVector* dg_dpT)
{

  if (gT != NULL)
    gT->putScalar(0);

  if (dg_dxT != NULL)
    dg_dxT->putScalar(0);

  if (dg_dxdotT != NULL)
    dg_dxdotT->putScalar(0);

  if (dg_dxdotdotT != NULL)
    dg_dxdotdotT->putScalar(0);

  if (dg_dpT != NULL)
    dg_dpT->putScalar(0);

  for (unsigned int i=0; i<responses.size(); i++) {

    // Create Tpetra_Map for response function
    unsigned int num_responses = responses[i]->numResponses();
    Teuchos::RCP<const Teuchos::Comm<int> > commT = responses[i]->getComm(); 
    Tpetra::LocalGlobal lg = Tpetra::LocallyReplicated;
    Teuchos::RCP<Tpetra_Map> local_response_map = Teuchos::rcp(new Tpetra_Map(num_responses, 0, commT, lg));

    // Create Epetra_Vectors for response function
    RCP<Tpetra_Vector> local_gT;
    if (gT != NULL)
      local_gT = rcp(new Tpetra_Vector(local_response_map));
    RCP<Tpetra_MultiVector> local_dgdxT;
    if (dg_dxT != NULL)
      local_dgdxT = rcp(new Tpetra_MultiVector(dg_dxT->getMap(), num_responses));
    RCP<Tpetra_MultiVector> local_dgdxdotT;
    if (dg_dxdotT != NULL)
      local_dgdxdotT = rcp(new Tpetra_MultiVector(dg_dxdotT->getMap(), 
						 num_responses));
    RCP<Tpetra_MultiVector> local_dgdxdotdotT;
    if (dg_dxdotdotT != NULL)
      local_dgdxdotdotT = rcp(new Tpetra_MultiVector(dg_dxdotdotT->getMap(), num_responses));
    RCP<Tpetra_MultiVector> local_dgdpT;
    if (dg_dpT != NULL)
      local_dgdpT = rcp(new Tpetra_MultiVector(local_response_map, 
					      dg_dpT->getNumVectors()));

    // Evaluate response function
    responses[i]->evaluateGradient(
            current_time, x, xdot, xdotdot, p, deriv_p, 
				    local_gT.get(), local_dgdxT.get(), 
				    local_dgdxdotT.get(), local_dgdxdotdotT.get(), local_dgdpT.get());

    // Copy results into combined result
    for (unsigned int j=0; j<num_responses; j++) {
      if (gT != NULL) {
        const Teuchos::ArrayRCP<const ST> local_gT_constView = local_gT->get1dView();
        const Teuchos::ArrayRCP<ST> gT_nonconstView = gT->get1dViewNonConst();
        gT_nonconstView[j] += local_gT_constView[j];
      }
      if (dg_dxT != NULL) {
        Teuchos::RCP<Tpetra_Vector> dg_dxT_vec = dg_dxT->getVectorNonConst(j);
        Teuchos::RCP<const Tpetra_Vector> local_dgdxT_vec = local_dgdxT->getVector(j);
        dg_dxT_vec->update(1.0, *local_dgdxT_vec, 1.0);  
      }
      if (dg_dxdotT != NULL) {
        Teuchos::RCP<Tpetra_Vector> dg_dxdotT_vec = dg_dxdotT->getVectorNonConst(j);
        Teuchos::RCP<const Tpetra_Vector> local_dgdxdotT_vec = local_dgdxdotT->getVector(j);
        dg_dxdotT_vec->update(1.0, *local_dgdxdotT_vec, 1.0);  
        }
      if (dg_dxdotdotT != NULL){
        Teuchos::RCP<Tpetra_Vector> dg_dxdotdotT_vec = dg_dxdotdotT->getVectorNonConst(j);
        Teuchos::RCP<const Tpetra_Vector> local_dgdxdotdotT_vec = local_dgdxdotdotT->getVector(j);
        dg_dxdotdotT_vec->update(1.0, *local_dgdxdotdotT_vec, 1.0);  
      }
      if (dg_dpT != NULL) {
        Teuchos::ArrayRCP<ST> dg_dpT_nonconstView;
        Teuchos::ArrayRCP<const ST> local_dgdpT_constView;
	for (int k=0; k<dg_dpT->getNumVectors(); k++) {
          local_dgdpT_constView = local_dgdpT->getData(k); 
          dg_dpT_nonconstView = dg_dpT->getDataNonConst(k); 
	  dg_dpT_nonconstView[j] += local_dgdpT_constView[j];
        }
      }
    }
  }
}


void
Albany::CumulativeScalarResponseFunction::
evaluateDistParamDeriv(
    const double current_time,
    const Teuchos::RCP<const Thyra_Vector>& x,
    const Teuchos::RCP<const Thyra_Vector>& xdot,
    const Teuchos::RCP<const Thyra_Vector>& xdotdot,
    const Teuchos::Array<ParamVec>& param_array,
    const std::string& dist_param_name,
    Tpetra_MultiVector* dg_dpT)
{
  if (dg_dpT != NULL)
    dg_dpT->putScalar(0);

  for (unsigned int i=0; i<responses.size(); i++) {

    // Create Epetra_Map for response function
    int num_responses = responses[i]->numResponses();

    // Create Epetra_MultiVectors for response derivative function
    RCP<Tpetra_MultiVector> cumulative_dgdp;
    if (dg_dpT != NULL)
      cumulative_dgdp = rcp(new Tpetra_MultiVector(dg_dpT->getMap(),num_responses));

    // Evaluate response function
    responses[i]->evaluateDistParamDeriv(
           current_time, x, xdot, xdotdot,
           param_array, dist_param_name,
           cumulative_dgdp.get());

    // Copy results into combined result
    if (dg_dpT != NULL)
      for (unsigned int j=0; j<num_responses; j++)
        dg_dpT->getVectorNonConst(j)->update(1.0, *cumulative_dgdp->getVector(j), 1.0);
  }
}
