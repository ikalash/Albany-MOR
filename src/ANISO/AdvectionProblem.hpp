//*****************************************************************//
//    Albany 3.0:  Copyright 2016 Sandia Corporation               //
//    This Software is released under the BSD license detailed     //
//    in the file "license.txt" in the top-level Albany directory  //
//*****************************************************************//

#ifndef ADVECTION_PROBLEM_HPP
#define ADVECTION_PROBLEM_HPP

#include "Teuchos_RCP.hpp"
#include "Teuchos_ParameterList.hpp"
#include "Teuchos_TestForException.hpp"

#include "Albany_AbstractProblem.hpp"

#include "PHAL_Workset.hpp"
#include "PHAL_Dimension.hpp"
#include "Albany_ProblemUtils.hpp"
#include "Albany_MaterialDatabase.hpp"

namespace Albany {

class AdvectionProblem : public AbstractProblem {

  public:
 
    AdvectionProblem(
      const Teuchos::RCP<Teuchos::ParameterList>& params,
      const Teuchos::RCP<ParamLib>& param_lib,
      const int num_dims,
      Teuchos::RCP<const Teuchos::Comm<int> >& commT);

    ~AdvectionProblem();

    int spatialDimension() const { return num_dims_; }

    bool useSDBCs() const {return use_sdbcs_; }

    void buildProblem(
        Teuchos::ArrayRCP<Teuchos::RCP<Albany::MeshSpecsStruct> >  meshSpecs,
        StateManager& stateMgr);

    Teuchos::Array< Teuchos::RCP<const PHX::FieldTag> > buildEvaluators(
        PHX::FieldManager<PHAL::AlbanyTraits>& fm0,
        const Albany::MeshSpecsStruct& meshSpecs,
        Albany::StateManager& stateMgr,
        Albany::FieldManagerChoice fmchoice,
        const Teuchos::RCP<Teuchos::ParameterList>& responseList);

    Teuchos::RCP<const Teuchos::ParameterList>
    getValidProblemParameters() const;

  private:

    AdvectionProblem(const AdvectionProblem&);
    
    AdvectionProblem& operator=(const AdvectionProblem&);

  public:

    template <typename EvalT>
    Teuchos::RCP<const PHX::FieldTag> constructEvaluators(
        PHX::FieldManager<PHAL::AlbanyTraits>& fm0,
        const Albany::MeshSpecsStruct& meshSpecs,
        Albany::StateManager& stateMgr,
        Albany::FieldManagerChoice fmchoice,
        const Teuchos::RCP<Teuchos::ParameterList>& responseList);

    void constructDirichletEvaluators(
        const std::vector<std::string>& nodeSetIDs);

    void constructNeumannEvaluators(
        const Teuchos::RCP<Albany::MeshSpecsStruct>& meshSpecs);

  protected:

    int num_dims_;

    Teuchos::RCP<Albany::MaterialDatabase> material_db_;

    Teuchos::RCP<Albany::Layouts> dl_;
  
    /// Boolean marking whether SDBCs are used 
    bool use_sdbcs_; 

};

}

#include "Intrepid2_DefaultCubatureFactory.hpp"
#include "Shards_CellTopology.hpp"
#include "Albany_Utils.hpp"
#include "Albany_BCUtils.hpp"
#include "Albany_ProblemUtils.hpp"
#include "Albany_EvaluatorUtils.hpp"
#include "Albany_ResponseUtilities.hpp"
#include "PHAL_SaveStateField.hpp"

#include "ANISO_Time.hpp"
#include "AdvectionKappa.hpp"
#include "AdvectionAlpha.hpp"
#include "AdvectionTau.hpp"
#include "AdvectionResidual.hpp"

template <typename EvalT>
Teuchos::RCP<const PHX::FieldTag>
Albany::AdvectionProblem::constructEvaluators(
  PHX::FieldManager<PHAL::AlbanyTraits>& fm0,
  const Albany::MeshSpecsStruct& meshSpecs,
  Albany::StateManager& stateMgr,
  Albany::FieldManagerChoice fieldManagerChoice,
  const Teuchos::RCP<Teuchos::ParameterList>& responseList) {

  using Teuchos::RCP;
  using Teuchos::rcp;
  using Teuchos::ParameterList;
  using PHX::DataLayout;
  using PHX::MDALayout;
  using std::vector;
  using std::string;
  using PHAL::AlbanyTraits;

  const CellTopologyData* const elem_top = &meshSpecs.ctd;
  std::string eb_name = meshSpecs.ebName;
  std::string material_name =
    material_db_->getElementBlockParam<std::string>(
      eb_name, "material");

  RCP<Intrepid2::Basis<PHX::Device, RealType, RealType> >
    intrepid_basis = Albany::getIntrepid2Basis(*elem_top);

  RCP<shards::CellTopology> elem_type =
    rcp(new shards::CellTopology(elem_top));

  Intrepid2::DefaultCubatureFactory cub_factory;

  RCP<Intrepid2::Cubature<PHX::Device> > elem_cubature =
    cub_factory.create<PHX::Device, RealType, RealType>(
        *elem_type, meshSpecs.cubatureDegree);


  const int workset_size = meshSpecs.worksetSize;
  const int num_vertices = elem_type->getNodeCount();
  const int num_nodes = intrepid_basis->getCardinality();
  const int num_qps = elem_cubature->getNumPoints();

  dl_ = rcp(new Albany::Layouts(
        workset_size, num_vertices, num_nodes, num_qps, num_dims_));

  Teuchos::ArrayRCP<std::string> dof_names(1);
  Teuchos::ArrayRCP<std::string> resid_names(1);
  dof_names[0] = "Phi";
  resid_names[0] = "Phi Residual";

  Albany::EvaluatorUtils<EvalT, PHAL::AlbanyTraits> eval_utils(dl_);
  Teuchos::RCP<PHX::Evaluator<AlbanyTraits> > ev;

  fm0.template registerEvaluator<EvalT>(
      eval_utils.constructGatherSolutionEvaluator_noTransient(
        false, dof_names));

  fm0.template registerEvaluator<EvalT>(
      eval_utils.constructScatterResidualEvaluator(
        false, resid_names));

  fm0.template registerEvaluator<EvalT>(
      eval_utils.constructDOFInterpolationEvaluator(dof_names[0]));

  fm0.template registerEvaluator<EvalT>(
      eval_utils.constructDOFGradInterpolationEvaluator(dof_names[0]));

  fm0.template registerEvaluator<EvalT>(
      eval_utils.constructComputeBasisFunctionsEvaluator(
        elem_type, intrepid_basis, elem_cubature));

  fm0.template registerEvaluator<EvalT>(
      eval_utils.constructGatherCoordinateVectorEvaluator());

  fm0.template registerEvaluator<EvalT>(
      eval_utils.constructMapToPhysicalFrameEvaluator(
        elem_type, elem_cubature));

  // set up the problem variables
  std::string kappa_val = material_db_->getElementBlockParam<std::string>(
      eb_name, "Kappa");
  std::string alpha_x =  material_db_->getElementBlockParam<std::string>(
      eb_name, "Alpha_x");
  std::string alpha_y = material_db_->getElementBlockParam<std::string>(
      eb_name, "Alpha_y");
  Teuchos::Array<std::string> alpha_val(2);
  alpha_val[0] = alpha_x;
  alpha_val[1] = alpha_y;

  { // Time
    RCP<ParameterList> p = rcp(new ParameterList("Time"));
    p->set<RCP<ParamLib> >("Parameter Library", paramLib);
    p->set<std::string>("Time Name", "Time");
    p->set<std::string>("Delta Time Name", "Delta Time");
    ev = rcp(new ANISO::Time<EvalT, PHAL::AlbanyTraits>(*p, dl_));
    fm0.template registerEvaluator<EvalT>(ev);
    p = stateMgr.registerStateVariable(
        "Time", dl_->workset_scalar, dl_->dummy, eb_name, "scalar", 0.0, true);
    ev = rcp(new PHAL::SaveStateField<EvalT, PHAL::AlbanyTraits>(*p));
    fm0.template registerEvaluator<EvalT>(ev);
  }

  { // Kappa - diffusivity coeffecient
    RCP<ParameterList> p = rcp(new ParameterList("Kappa"));
    p->set<std::string>("Coordinate Name", "Coord Vec");
    p->set<std::string>("Kappa Value", kappa_val);
    p->set<std::string>("Kappa Name", "Kappa");
    ev = rcp(new ANISO::AdvectionKappa<EvalT, PHAL::AlbanyTraits>(*p, dl_));
    fm0.template registerEvaluator<EvalT>(ev);
  }

  { // Alpha - advective coefficient
    RCP<ParameterList> p = rcp(new ParameterList("Alpha"));
    p->set<std::string>("Coordinate Name", "Coord Vec");
    p->set<Teuchos::Array<std::string> >("Alpha Value", alpha_val);
    p->set<std::string>("Alpha Name", "Alpha");
    p->set<std::string>("Alpha Magnitude Name", "Alpha Magnitude");
    ev = rcp(new ANISO::AdvectionAlpha<EvalT, PHAL::AlbanyTraits>(*p, dl_));
    fm0.template registerEvaluator<EvalT>(ev);
  }

  { // SUPG tau
    RCP<ParameterList> p = rcp(new ParameterList("Tau"));
    p->set<std::string>("Kappa Name", "Kappa");
    p->set<std::string>("Alpha Magnitude Name", "Alpha Magnitude");
    p->set<std::string>("Coordinate Name", "Coord Vec");
    p->set<std::string>("Gradient BF Name", "Grad BF");
    p->set<std::string>("Tau Name", "Tau");
    ev = rcp(new ANISO::AdvectionTau<EvalT, PHAL::AlbanyTraits>(*p, dl_));
    fm0.template registerEvaluator<EvalT>(ev);
  }

  { // SUPG stabilized advection-diffusion residual
    RCP<ParameterList> p = rcp(new ParameterList("Advection Residual"));
    p->set<std::string>("Kappa Name", "Kappa");
    p->set<std::string>("Alpha Name", "Alpha");
    p->set<std::string>("Weighted BF Name", "wBF");
    p->set<std::string>("Weighted Gradient BF Name", "wGrad BF");
    p->set<std::string>("Kappa Name", "Kappa");
    p->set<std::string>("Concentration Name", "Phi");
    p->set<std::string>("Concentration Gradient Name", "Phi Gradient");
    p->set<std::string>("Tau Name", "Tau");
    p->set<std::string>("Source Name", "Source");
    p->set<std::string>("Residual Name", "Phi Residual");
    ev = rcp(new ANISO::AdvectionResidual<EvalT, PHAL::AlbanyTraits>(*p, dl_));
    fm0.template registerEvaluator<EvalT>(ev);
  }

  if (fieldManagerChoice == Albany::BUILD_RESID_FM) {
    PHX::Tag<typename EvalT::ScalarT> res_tag("Scatter", dl_->dummy);
    fm0.requireField<EvalT>(res_tag);
    return res_tag.clone();
  }

  else if (fieldManagerChoice == Albany::BUILD_RESPONSE_FM) {
    RCP<ParameterList> pFromProb = rcp(new ParameterList("Params from problem"));
    Albany::ResponseUtilities<EvalT, PHAL::AlbanyTraits> resp_utils(dl_);
    return resp_utils.constructResponses(fm0, *responseList, pFromProb, stateMgr);
  }

  return Teuchos::null;

}

#endif
