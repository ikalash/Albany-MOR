//*****************************************************************//
//    Albany 3.0:  Copyright 2016 Sandia Corporation               //
//    This Software is released under the BSD license detailed     //
//    in the file "license.txt" in the top-level Albany directory  //
//*****************************************************************//

#ifdef ATO_USES_DOTK
#include "ATO_DOTk_Optimizer.hpp"
#endif
#include "Teuchos_TestForException.hpp"
#include "ATO_Solver.hpp"

namespace ATO {

/**********************************************************************/
Optimizer_DOTk::Optimizer_DOTk(const Teuchos::ParameterList& optimizerParams) :
Optimizer(optimizerParams)
/**********************************************************************/
{ 
  myCoOperators = NULL;
}

/******************************************************************************/
Optimizer_DOTk::~Optimizer_DOTk()
/******************************************************************************/
{
  if(myCoOperators) delete myCoOperators;
}

/******************************************************************************/
void
Optimizer_DOTk::Initialize()
/******************************************************************************/
{

  myCoOperators = new ATO_DOTk_ContinuousOperators(solverInterface, comm);

  int numOptDofs = solverInterface->GetNumOptDofs();
  vector* newVector = new vector(numOptDofs, comm);

}

/******************************************************************************/
void
Optimizer_DOTk::Optimize()
/******************************************************************************/
{
}

}

