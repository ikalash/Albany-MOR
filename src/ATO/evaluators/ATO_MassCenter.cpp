//*****************************************************************//
//    Albany 2.0:  Copyright 2012 Sandia Corporation               //
//    This Software is released under the BSD license detailed     //
//    in the file "license.txt" in the top-level Albany directory  //
//*****************************************************************//

#include "PHAL_AlbanyTraits.hpp"

#include "ATO_MassCenter.hpp"
#include "ATO_MassCenter_Def.hpp"

template<typename EvalT, typename Traits>
const std::string ATO::MassCenter<EvalT, Traits>::className = "MassCenter";

PHAL_INSTANTIATE_TEMPLATE_CLASS(ATO::MassCenter)

