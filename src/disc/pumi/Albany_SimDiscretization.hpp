//*****************************************************************//
//    Albany 3.0:  Copyright 2016 Sandia Corporation               //
//    This Software is released under the BSD license detailed     //
//    in the file "license.txt" in the top-level Albany directory  //
//*****************************************************************//


#ifndef ALBANY_SIMDISCRETIZATION_HPP
#define ALBANY_SIMDISCRETIZATION_HPP

#include "Albany_APFDiscretization.hpp"
#include "Albany_SimMeshStruct.hpp"

namespace Albany {

class SimDiscretization : public APFDiscretization {
  public:

    //! Constructor
    SimDiscretization(
       Teuchos::RCP<Albany::SimMeshStruct> meshStruct_,
       const Teuchos::RCP<const Teuchos_Comm>& commT,
       const Teuchos::RCP<Albany::RigidBodyModes>& rigidBodyModes = Teuchos::null);

    //! Destructor
    ~SimDiscretization();
};

}

#endif // ALBANY_SIMDISCRETIZATION_HPP
