//*****************************************************************//
//    Albany 3.0:  Copyright 2016 Sandia Corporation               //
//    This Software is released under the BSD license detailed     //
//    in the file "license.txt" in the top-level Albany directory  //
//*****************************************************************//


#ifndef ALBANY_SIMMESHSTRUCT_HPP
#define ALBANY_SIMMESHSTRUCT_HPP

#include "Albany_APFMeshStruct.hpp"

namespace Albany {

class SimMeshStruct : public APFMeshStruct {

  public:

    SimMeshStruct(
                  const Teuchos::RCP<Teuchos::ParameterList>& params,
                  const Teuchos::RCP<const Teuchos_Comm>& commT);

    ~SimMeshStruct();

    msType meshSpecsType();

    virtual apf::Field* createNodalField(char const* name, int valueType);

private:

    Teuchos::RCP<const Teuchos::ParameterList>
      getValidDiscretizationParameters() const;

};

}
#endif

