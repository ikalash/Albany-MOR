//*****************************************************************//
//    Albany 3.0:  Copyright 2016 Sandia Corporation               //
//    This Software is released under the BSD license detailed     //
//    in the file "license.txt" in the top-level Albany directory  //
//*****************************************************************//


#ifndef AADAPT_UNIFORMREFINE_HPP
#define AADAPT_UNIFORMREFINE_HPP

#include "Albany_APFDiscretization.hpp"
#include <ma.h>
#include "Albany_StateManager.hpp"
#include "AAdapt_MeshAdaptMethod.hpp"

namespace AAdapt {

class UniformRefine : public MeshAdaptMethod {

  public:
    UniformRefine(const Teuchos::RCP<Albany::APFDiscretization>& disc);

    ~UniformRefine();

    void adaptMesh(const Teuchos::RCP<Teuchos::ParameterList>& adapt_params_);

    void setParams(const Teuchos::RCP<Teuchos::ParameterList>& p);

    void preProcessShrunkenMesh();

    void preProcessOriginalMesh();
    void postProcessFinalMesh() {}
    void postProcessShrunkenMesh() {}

  private:

};

}

#endif

