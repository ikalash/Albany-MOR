//*****************************************************************//
//    Albany 3.0:  Copyright 2016 Sandia Corporation               //
//    This Software is released under the BSD license detailed     //
//    in the file "license.txt" in the top-level Albany directory  //
//*****************************************************************//

#ifndef AMP_ENERGY_INTEGRAL_HPP
#define AMP_ENERGY_INTEGRAL_HPP

namespace apf {
  class Mesh;
}

namespace Albany {

double computeAMPEnergyIntegral(apf::Mesh* m);
void debugAMPMesh(apf::Mesh* m, char const* prefix);

}

#endif
