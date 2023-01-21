//*****************************************************************//
//    Albany 3.0:  Copyright 2016 Sandia Corporation               //
//    This Software is released under the BSD license detailed     //
//    in the file "license.txt" in the top-level Albany directory  //
//*****************************************************************//

#include "PHAL_AlbanyTraits.hpp"

#include "PHAL_SaveNodalField.hpp"
#include "PHAL_SaveNodalField_Def.hpp"

template<typename EvalT, typename Traits>
const std::string PHAL::SaveNodalFieldBase<EvalT, Traits>::className = "Save_Nodal_Field";

PHAL_INSTANTIATE_TEMPLATE_CLASS(PHAL::SaveNodalField)
PHAL_INSTANTIATE_TEMPLATE_CLASS(PHAL::SaveNodalFieldBase)

