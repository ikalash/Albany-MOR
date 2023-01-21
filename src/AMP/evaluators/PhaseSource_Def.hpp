//*****************************************************************//
//    Albany 3.0:  Copyright 2016 Sandia Corporation               //
//    This Software is released under the BSD license detailed     //
//    in the file "license.txt" in the top-level Albany directory  //
//*****************************************************************//
#include <fstream>
#include "Sacado_ParameterRegistration.hpp"
#include "Albany_Utils.hpp"

#include "Teuchos_TestForException.hpp"
#include "Phalanx_DataLayout.hpp"

#include "Intrepid2_FunctionSpaceTools.hpp"


namespace AMP {

//**********************************************************************
template<typename EvalT, typename Traits>
PhaseSource<EvalT, Traits>::
PhaseSource(Teuchos::ParameterList& p,
                         const Teuchos::RCP<Albany::Layouts>& dl) :
  coord_      (p.get<std::string>("Coordinate Name"),
               dl->qp_vector),
  time        (p.get<std::string>("Time Name"),
               dl->workset_scalar),
  deltaTime   (p.get<std::string>("Delta Time Name"),
               dl->workset_scalar),
  source_     (p.get<std::string>("Source Name"),
               dl->qp_scalar)
{

  this->addDependentField(coord_);
  this->addDependentField(time);
  this->addDependentField(deltaTime);
  this->addEvaluatedField(source_);
 
  Teuchos::RCP<PHX::DataLayout> scalar_dl = dl->qp_scalar;
  std::vector<PHX::Device::size_type> dims;
  scalar_dl->dimensions(dims);
  workset_size_ = dims[0];
  num_qps_      = dims[1];

  Teuchos::ParameterList* cond_list =
    p.get<Teuchos::ParameterList*>("Parameter List");

  Teuchos::RCP<const Teuchos::ParameterList> reflist =
    this->getValidPhaseSourceParameters();

  cond_list->validateParameters(*reflist, 0,
      Teuchos::VALIDATE_USED_ENABLED, Teuchos::VALIDATE_DEFAULTS_DISABLED);

  std::string type = cond_list->get("Phase Source Type", "Constant");

  ScalarT value = cond_list->get("Value", 1.0);
  init_constant(value,p);
  
  //cond_list = p.get<Teuchos::ParameterList*>("Laser Source Parameter List");
  
  

  this->setName("PhaseSource"+PHX::typeAsString<EvalT>());

}

//**********************************************************************
template<typename EvalT, typename Traits>
void PhaseSource<EvalT, Traits>::
init_constant(ScalarT value, Teuchos::ParameterList& p){
  laser_power = value;
}

//**********************************************************************
template<typename EvalT, typename Traits>
void PhaseSource<EvalT, Traits>::
postRegistrationSetup(typename Traits::SetupData d,
                      PHX::FieldManager<Traits>& fm)
{
  this->utils.setFieldData(coord_,fm);
  this->utils.setFieldData(time,fm);
  this->utils.setFieldData(deltaTime,fm);
  this->utils.setFieldData(source_,fm);
}

//**********************************************************************
template<typename EvalT, typename Traits>
void PhaseSource<EvalT, Traits>::
evaluateFields(typename Traits::EvalData workset)
{
  // current time
  const RealType t = workset.current_time;
  
  AMP::LaserCenter Val;
  Val.t = t;
  
  RealType x, y, power_fraction;
  int power;
  LaserData_.getLaserPosition(t,Val,x,y,power,power_fraction);
  ScalarT Laser_center_x = x;
  ScalarT Laser_center_y = y;
  ScalarT Laser_power_fraction = power_fraction; 

  //source function
  ScalarT laser_beam_radius = 60.0e-6;
  ScalarT porosity = 0.652;
  ScalarT particle_dia = 20.0e-6;
  ScalarT powder_hemispherical_reflectivity = 0.70;
  //ScalarT lambda =2.0;
  ScalarT pi = 3.1415926535897932;

  ScalarT LaserFlux_Max;
  // laser on or off
  if ( power == 1 )
    {
      LaserFlux_Max =(3.0/(pi*laser_beam_radius*laser_beam_radius))*laser_power*Laser_power_fraction;
    }
  else
    {
      LaserFlux_Max = 0.0;
    }

  ScalarT beta = 1.5*(1.0 - porosity)/(porosity*particle_dia);
  
  //  Parameters for the depth profile of the laser heat source:
  //Depth of powder bed (Needs to be changed as per the model)
  ScalarT PB_depth = 50e-6;
  ScalarT lambda = PB_depth*beta;
  
  
  // Few parameters:
  ScalarT a = sqrt(1.0 - powder_hemispherical_reflectivity);
  ScalarT A = (1.0 - pow(powder_hemispherical_reflectivity,2))*exp(-lambda);
  ScalarT B = 3.0 + powder_hemispherical_reflectivity*exp(-2*lambda);
  ScalarT b1 = 1 - a;
  ScalarT b2 = 1 + a;
  ScalarT c1 = b2 - powder_hemispherical_reflectivity*b1;
  ScalarT c2 = b1 - powder_hemispherical_reflectivity*b2;
  ScalarT C = b1*c2*exp(-2*a*lambda) - b2*c1*exp(2*a*lambda);
  
  //  Following are few factors defined by the coder to be included while defining the depth profile
  ScalarT f1 = 1.0/(3.0 - 4.0*powder_hemispherical_reflectivity);
  ScalarT f2 = 2*powder_hemispherical_reflectivity*a*a/C;
  ScalarT f3 = 3.0*(1.0 - powder_hemispherical_reflectivity);
  
  
  //  Code for heat into substrate
  ScalarT Substrate_Top = 0.00005;

  /* The "Thin layer" upto which the volumetric heat in the substrate penetrates is given for optical thickness values for 2.5, 2, and 3. Choose only one at a time based on the porosity
  value defined in the material input file */
  /*
  //This is for optical thickness = 2.5 (porosity = 0.6, particle dia = 20 microns, powder bed thickness = 50 microns)
  ScalarT Absorptivity_PowderSurface = 0.7728; 
  ScalarT Substrate_Bot = 0.0000575;
  */
  
  //This is for optical thickness = 2 (porosity = 0.652, particle dia = 20 microns, powder bed thickness = 50 microns)
  ScalarT Absorptivity_PowderSurface = 0.7570; 
  ScalarT Substrate_Bot = 0.000060;

  /*
  //This is for optical thickness = 3 (porosity = 0.556, particle dia = 20 microns, powder bed thickness = 50 microns)
  ScalarT Absorptivity_PowderSurface = 0.7795; 
  ScalarT Substrate_Bot = 0.0000565;
  */

  ScalarT S1 = 1.0/(3.0-4.0*powder_hemispherical_reflectivity);
  ScalarT S2 = powder_hemispherical_reflectivity*a/C;
  ScalarT S3 = (A*b2 + B*c1)*(exp(2.0*a*lambda) - 1.0); 
  ScalarT S4 = (A*b1 + B*c2)*(exp(-2.0*a*lambda) - 1.0); 
  ScalarT S5 = 3.0*(1.0 - powder_hemispherical_reflectivity)*(1.0 - exp(-lambda))*(1.0 + powder_hemispherical_reflectivity*exp(-lambda)); 
  ScalarT I_value = S1*(S2*(S3 + S4) + S5);
  //std::cout<<"Abs at substrate = "<<(Absorptivity_PowderSurface - I_value)<<std::endl;
  
  //std::cout<<" ebname ="<<workset.EBName<<std::endl; 
  //std::cout<<"current time ="<<workset.current_time<<std::endl;
  // source function
  for (std::size_t cell = 0; cell < workset.numCells; ++cell) {
    for (std::size_t qp = 0; qp < num_qps_; ++qp) {
        MeshScalarT X = coord_(cell,qp,0);
        MeshScalarT Y = coord_(cell,qp,1);
        MeshScalarT Z = coord_(cell,qp,2);
		
		//Value of depth profile at z = lambda
		ScalarT depth_profile_lambda = f1*(f2*(A*(b2*exp(2.0*a*lambda)-b1*exp(-2.0*a*lambda)) - B*(c2*exp(-2.0*a*(lambda - lambda))-c1*exp(2.0*a*(lambda-lambda)))) + f3*(exp(-lambda)+powder_hemispherical_reflectivity*exp(lambda - 2.0*lambda)));
  
                           
        ScalarT radius = sqrt((X - Laser_center_x)*(X - Laser_center_x) + (Y - Laser_center_y)*(Y - Laser_center_y));
        /*
		if (radius < laser_beam_radius && Z >= Substrate_Top && Z <= Substrate_Bot)
              source_(cell,qp) = beta*LaserFlux_Max*pow((1.0-(radius*radius)/(laser_beam_radius*laser_beam_radius)),2)*(Absorptivity_PowderSurface - I_value);
        else  source_(cell,qp) =0.0;
		*/
		
		//This formula is valid only at the substrate when no melting happens
		/*
		if (radius < laser_beam_radius && Z >= Substrate_Top)
              source_(cell,qp) = beta*(Absorptivity_PowderSurface - I_value)*LaserFlux_Max*pow((1.0-(radius*radius)/(laser_beam_radius*laser_beam_radius)),2)*exp(-beta*(Z-Substrate_Top));
		else  source_(cell,qp) =0.0;		
		*/
		
		if (radius < laser_beam_radius && Z >= Substrate_Top)
              source_(cell,qp) = beta*depth_profile_lambda*LaserFlux_Max*pow((1.0-(radius*radius)/(laser_beam_radius*laser_beam_radius)),2)*exp(-beta*(Z-Substrate_Top));
		else  source_(cell,qp) =0.0;
    }
  }
}

//**********************************************************************
template<typename EvalT, typename Traits>
Teuchos::RCP<const Teuchos::ParameterList>
PhaseSource<EvalT, Traits>::
getValidPhaseSourceParameters() const
{
  Teuchos::RCP<Teuchos::ParameterList> valid_pl =
    rcp(new Teuchos::ParameterList("Valid Phase Source Params"));;
  
  valid_pl->set<std::string>("Phase Source Type", "Constant",
     "Constant phase source across the element block");
  valid_pl->set<double>("Value", 1.0, "Constant phase source value");
  
  return valid_pl;
}
//**********************************************************************
}
