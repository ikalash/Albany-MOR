//*****************************************************************//
//    Albany 3.0:  Copyright 2016 Sandia Corporation               //
//    This Software is released under the BSD license detailed     //
//    in the file "license.txt" in the top-level Albany directory  //
//*****************************************************************//

#ifndef ALBANY_ASCII_STKMESH2DSTRUCT_HPP
#define ALBANY_ASCII_STKMESH2DSTRUCT_HPP

#include "Albany_GenericSTKMeshStruct.hpp"

//#include <Ionit_Initializer.h>

namespace Albany {

  class AsciiSTKMesh2D : public GenericSTKMeshStruct {

    public:

    AsciiSTKMesh2D(
                  const Teuchos::RCP<Teuchos::ParameterList>& params,
                  const Teuchos::RCP<const Teuchos_Comm>& commT);

    ~AsciiSTKMesh2D();

    void setFieldAndBulkData(
                  const Teuchos::RCP<const Teuchos_Comm>& commT,
                  const Teuchos::RCP<Teuchos::ParameterList>& params,
                  const unsigned int neq_,
                  const AbstractFieldContainer::FieldContainerRequirements& req,
                  const Teuchos::RCP<Albany::StateInfoStruct>& sis,
                  const unsigned int worksetSize,
                  const std::map<std::string,Teuchos::RCP<Albany::StateInfoStruct> >& side_set_sis = {},
                  const std::map<std::string,AbstractFieldContainer::FieldContainerRequirements>& side_set_req = {});

    //! Flag if solution has a restart values -- used in Init Cond
    bool hasRestartSolution() const {return false; }

    //! If restarting, convenience function to return restart data time
    double restartDataTime() const {return -1.0; }

    private:
    //Ioss::Init::Initializer ioInit;

    Teuchos::RCP<const Teuchos::ParameterList>
      getValidDiscretizationParameters() const;

    Teuchos::RCP<Teuchos::FancyOStream> out;
    bool periodic;
    int NumElemNodes; //number of nodes per element (e.g. 3 for Triangles)
    int NumNodes; //number of nodes
    int NumEles; //number of elements
    int NumBdEdges; //number of faces on basal boundary
    std::map<int,std::string> bdTagToNodeSetName;
    std::map<int,std::string> bdTagToSideSetName;
    double (*xyz)[3];
    int (*eles)[4]; //hard-coded for quads (tria will ignore last one)
    int (*be)[3]; //2 points plus label
  };

}
#endif
