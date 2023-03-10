//*****************************************************************//
//    Albany 3.0:  Copyright 2016 Sandia Corporation               //
//    This Software is released under the BSD license detailed     //
//    in the file "license.txt" in the top-level Albany directory  //
//*****************************************************************//

#ifndef AADAPT_SCALEDSIZEFIELD_HPP
#define AADAPT_SCALEDSIZEFIELD_HPP

#include "AAdapt_MeshAdaptMethod.hpp"

namespace AAdapt {

class ScaledSizeField : public MeshAdaptMethod {

  public:

    ScaledSizeField(const Teuchos::RCP<Albany::APFDiscretization>& disc);

    ~ScaledSizeField();

    void adaptMesh(const Teuchos::RCP<Teuchos::ParameterList>& adapt_params_);

    void setParams(const Teuchos::RCP<Teuchos::ParameterList>& p);

    void preProcessShrunkenMesh();

    void preProcessOriginalMesh();
    void postProcessFinalMesh() {}
    void postProcessShrunkenMesh() {}

    class ScaledIsoFunc : public ma::IsotropicFunction
    {
      public:
        virtual ~ScaledIsoFunc(){}

    /** \brief get the desired element size at this vertex */

        virtual double getValue(ma::Entity* vert){
            return factor_ * averageEdgeLength_;
        }

        double factor_;
        double averageEdgeLength_;

    } scaledIsoFunc;


};

}

#endif

