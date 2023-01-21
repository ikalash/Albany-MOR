//*****************************************************************//
//    Albany 3.0:  Copyright 2016 Sandia Corporation               //
//    This Software is released under the BSD license detailed     //
//    in the file "license.txt" in the top-level Albany directory  //
//*****************************************************************//

#ifndef LANDICE_STOKESL1L2PROBLEM_HPP
#define LANDICE_STOKESL1L2PROBLEM_HPP

#include "Teuchos_RCP.hpp"
#include "Teuchos_ParameterList.hpp"

#include "Albany_AbstractProblem.hpp"
#include "Albany_GeneralPurposeFieldsNames.hpp"

#include "PHAL_Workset.hpp"
#include "PHAL_Dimension.hpp"

namespace LandIce {

  /*!
   * \brief Abstract interface for representing a 1-D finite element
   * problem.
   */
  class StokesL1L2 : public Albany::AbstractProblem {
  public:

    //! Default constructor
    StokesL1L2(const Teuchos::RCP<Teuchos::ParameterList>& params,
     const Teuchos::RCP<ParamLib>& paramLib,
     const int numDim_);

    //! Destructor
    ~StokesL1L2();

    //! Return number of spatial dimensions
    virtual int spatialDimension() const { return numDim; }

    //! Get boolean telling code if SDBCs are utilized
    virtual bool useSDBCs() const {return use_sdbcs_; }

    //! Build the PDE instantiations, boundary conditions, and initial solution
    virtual void buildProblem(
      Teuchos::ArrayRCP<Teuchos::RCP<Albany::MeshSpecsStruct> >  meshSpecs,
      Albany::StateManager& stateMgr);

    // Build evaluators
    virtual Teuchos::Array< Teuchos::RCP<const PHX::FieldTag> >
    buildEvaluators(
      PHX::FieldManager<PHAL::AlbanyTraits>& fm0,
      const Albany::MeshSpecsStruct& meshSpecs,
      Albany::StateManager& stateMgr,
      Albany::FieldManagerChoice fmchoice,
      const Teuchos::RCP<Teuchos::ParameterList>& responseList);

    //! Each problem must generate it's list of valide parameters
    Teuchos::RCP<const Teuchos::ParameterList> getValidProblemParameters() const;

  private:

    //! Private to prohibit copying
    StokesL1L2(const StokesL1L2&);

    //! Private to prohibit copying
    StokesL1L2& operator=(const StokesL1L2&);

  public:

    //! Main problem setup routine. Not directly called, but indirectly by following functions
    template <typename EvalT> Teuchos::RCP<const PHX::FieldTag>
    constructEvaluators(
      PHX::FieldManager<PHAL::AlbanyTraits>& fm0,
      const Albany::MeshSpecsStruct& meshSpecs,
      Albany::StateManager& stateMgr,
      Albany::FieldManagerChoice fmchoice,
      const Teuchos::RCP<Teuchos::ParameterList>& responseList);

    void constructDirichletEvaluators(const Albany::MeshSpecsStruct& meshSpecs);

  protected:
    int numDim;

    /// Boolean marking whether SDBCs are used
    bool use_sdbcs_;

  };
} // Namespace LandIce

#include "Intrepid2_DefaultCubatureFactory.hpp"
#include "Shards_CellTopology.hpp"

#include "Albany_Utils.hpp"
#include "Albany_ProblemUtils.hpp"
#include "Albany_EvaluatorUtils.hpp"
#include "Albany_ResponseUtilities.hpp"

#include "PHAL_DOFVecGradInterpolation.hpp"

#include "LandIce_SharedParameter.hpp"
#include "LandIce_ParamEnum.hpp"
#include "LandIce_StokesL1L2Resid.hpp"
#include "LandIce_ViscosityL1L2.hpp"
#include "LandIce_EpsilonL1L2.hpp"
#include "LandIce_StokesL1L2BodyForce.hpp"

#include "PHAL_Source.hpp"

template <typename EvalT>
Teuchos::RCP<const PHX::FieldTag>
LandIce::StokesL1L2::constructEvaluators(
  PHX::FieldManager<PHAL::AlbanyTraits>& fm0,
  const Albany::MeshSpecsStruct& meshSpecs,
  Albany::StateManager& stateMgr,
  Albany::FieldManagerChoice fieldManagerChoice,
  const Teuchos::RCP<Teuchos::ParameterList>& responseList)
{
  using Teuchos::RCP;
  using Teuchos::rcp;
  using Teuchos::ParameterList;
  using PHX::DataLayout;
  using PHX::MDALayout;
  using std::vector;
  using std::string;
  using std::map;
  using PHAL::AlbanyTraits;

  RCP<Intrepid2::Basis<PHX::Device, RealType, RealType> >
    intrepidBasis = Albany::getIntrepid2Basis(meshSpecs.ctd);
  RCP<shards::CellTopology> cellType = rcp(new shards::CellTopology (&meshSpecs.ctd));

  const int numNodes = intrepidBasis->getCardinality();
  const int worksetSize = meshSpecs.worksetSize;

  Intrepid2::DefaultCubatureFactory cubFactory;
  RCP <Intrepid2::Cubature<PHX::Device> > cubature = cubFactory.create<PHX::Device, RealType, RealType>(*cellType, meshSpecs.cubatureDegree);

  const int numQPts = cubature->getNumPoints();
  const int numVertices = cellType->getNodeCount();

  *out << "Field Dimensions: Workset=" << worksetSize
       << ", Vertices= " << numVertices
       << ", Nodes= " << numNodes
       << ", QuadPts= " << numQPts
       << ", Dim= " << numDim << std::endl;

   int vecDim = neq;

   RCP<Albany::Layouts> dl = rcp(new Albany::Layouts(worksetSize,numVertices,numNodes,numQPts,numDim, vecDim));
   Albany::EvaluatorUtils<EvalT, PHAL::AlbanyTraits> evalUtils(dl);
   bool supportsTransient=true;
   int offset=0;

   // Temporary variable used numerous times below
   Teuchos::RCP<PHX::Evaluator<AlbanyTraits> > ev;

   // Define Field Names

     Teuchos::ArrayRCP<std::string> dof_names(1);
     Teuchos::ArrayRCP<std::string> dof_names_dot(1);
     Teuchos::ArrayRCP<std::string> resid_names(1);
     dof_names[0] = "Velocity";
     dof_names_dot[0] = dof_names[0]+"_dot";
     resid_names[0] = "Stokes Residual";
     fm0.template registerEvaluator<EvalT>
       (evalUtils.constructGatherSolutionEvaluator(true, dof_names, dof_names_dot, offset));

     fm0.template registerEvaluator<EvalT>
       (evalUtils.constructDOFVecInterpolationEvaluator(dof_names[0]));

     fm0.template registerEvaluator<EvalT>
       (evalUtils.constructDOFVecInterpolationEvaluator(dof_names_dot[0]));

     fm0.template registerEvaluator<EvalT>
       (evalUtils.constructDOFVecGradInterpolationEvaluator(dof_names[0]));

     fm0.template registerEvaluator<EvalT>
       (evalUtils.constructScatterResidualEvaluator(true, resid_names,offset, "Scatter Stokes"));
     offset += numDim;

   fm0.template registerEvaluator<EvalT>
     (evalUtils.constructGatherCoordinateVectorEvaluator());

   fm0.template registerEvaluator<EvalT>
     (evalUtils.constructMapToPhysicalFrameEvaluator(cellType, cubature));

   fm0.template registerEvaluator<EvalT>
     (evalUtils.constructComputeBasisFunctionsEvaluator(cellType, intrepidBasis, cubature));

   { // Specialized DofVecGrad Interpolation for this problem

     RCP<ParameterList> p = rcp(new ParameterList("DOFVecGrad Interpolation "+dof_names[0]));
     // Input
     p->set<std::string>("Variable Name", dof_names[0]);

     p->set<std::string>("Gradient BF Name", Albany::grad_bf_name);

     // Output (assumes same Name as input)
     p->set<std::string>("Gradient Variable Name", dof_names[0]+" Gradient");

     ev = rcp(new PHAL::DOFVecGradInterpolation<EvalT,AlbanyTraits>(*p,dl));
     fm0.template registerEvaluator<EvalT>(ev);
   }

  { // Stokes Resid
    RCP<ParameterList> p = rcp(new ParameterList("Stokes Resid"));

    //Input
    p->set<std::string>("Weighted BF Name", Albany::weighted_bf_name);
    p->set<std::string>("Weighted Gradient BF Name", Albany::weighted_grad_bf_name);
    p->set<std::string>("QP Variable Name", "Velocity");
    p->set<std::string>("QP Time Derivative Variable Name", "Velocity_dot");
    p->set<std::string>("Gradient QP Variable Name", "Velocity Gradient");
    p->set<std::string>("Velocity Gradient QP Variable Name", "Velocity Gradient");
    p->set<std::string>("Body Force Name", "Body Force");
    p->set<std::string>("LandIce Viscosity QP Variable Name", "LandIce Viscosity");
    p->set<std::string>("LandIce EpsilonXX QP Variable Name", "LandIce EpsilonXX");
    p->set<std::string>("LandIce EpsilonYY QP Variable Name", "LandIce EpsilonYY");
    p->set<std::string>("LandIce EpsilonXY QP Variable Name", "LandIce EpsilonXY");

    //Output
    p->set<std::string>("Residual Name", "Stokes Residual");

    ev = rcp(new LandIce::StokesL1L2Resid<EvalT,AlbanyTraits>(*p,dl));
    fm0.template registerEvaluator<EvalT>(ev);
  }
  {
    //--- Shared Parameter for Continuation:  ---//
    RCP<ParameterList> p = rcp(new ParameterList("Homotopy Parameter"));

    std::string param_name = "Glen's Law Homotopy Parameter";
    p->set<std::string>("Parameter Name", param_name);
    p->set<RCP<ParamLib> >("Parameter Library", paramLib);

    RCP<LandIce::SharedParameter<EvalT,PHAL::AlbanyTraits,LandIce::ParamEnum,LandIce::ParamEnum::Homotopy>> ptr_homotopy;
    ptr_homotopy = rcp(new LandIce::SharedParameter<EvalT,PHAL::AlbanyTraits,LandIce::ParamEnum,LandIce::ParamEnum::Homotopy>(*p,dl));
    ptr_homotopy->setNominalValue(params->sublist("Parameters"),params->sublist("LandIce Viscosity").get<double>(param_name,-1.0));
    fm0.template registerEvaluator<EvalT>(ptr_homotopy);
  }
  { // LandIce viscosity
    RCP<ParameterList> p = rcp(new ParameterList("LandIce Viscosity"));

    //Input
    p->set<std::string>("Gradient QP Variable Name", "Velocity Gradient");

    p->set<RCP<ParamLib> >("Parameter Library", paramLib);
    Teuchos::ParameterList& paramList = params->sublist("LandIce Viscosity");
    p->set<Teuchos::ParameterList*>("Parameter List", &paramList);
    p->set<std::string>("Coordinate Vector Name", "Coord Vec");
    p->set<std::string>("LandIce EpsilonB QP Variable Name", "LandIce EpsilonB");

    //Output
    p->set<std::string>("LandIce Viscosity QP Variable Name", "LandIce Viscosity");

    ev = rcp(new LandIce::ViscosityL1L2<EvalT,AlbanyTraits>(*p,dl));
    fm0.template registerEvaluator<EvalT>(ev);

  }
  { // LandIce epsilon
    RCP<ParameterList> p = rcp(new ParameterList("LandIce Epsilon"));

    //Input
    p->set<std::string>("Gradient QP Variable Name", "Velocity Gradient");

    p->set<RCP<ParamLib> >("Parameter Library", paramLib);
    Teuchos::ParameterList& paramList = params->sublist("LandIce Viscosity");
    p->set<Teuchos::ParameterList*>("Parameter List", &paramList);

    //Output
    p->set<std::string>("LandIce EpsilonXX QP Variable Name", "LandIce EpsilonXX");
    p->set<std::string>("LandIce EpsilonXY QP Variable Name", "LandIce EpsilonXY");
    p->set<std::string>("LandIce EpsilonYY QP Variable Name", "LandIce EpsilonYY");
    p->set<std::string>("LandIce EpsilonB QP Variable Name", "LandIce EpsilonB");

    ev = rcp(new LandIce::EpsilonL1L2<EvalT,AlbanyTraits>(*p,dl));
    fm0.template registerEvaluator<EvalT>(ev);
  }

  { // Body Force
    RCP<ParameterList> p = rcp(new ParameterList("Body Force"));

    //Input
    p->set<std::string>("LandIce Viscosity QP Variable Name", "LandIce Viscosity");
    p->set<std::string>("Coordinate Vector Name", "Coord Vec");

    Teuchos::ParameterList& paramList = params->sublist("Body Force");
    p->set<Teuchos::ParameterList*>("Parameter List", &paramList);


    //Output
    p->set<std::string>("Body Force Name", "Body Force");

    ev = rcp(new LandIce::StokesL1L2BodyForce<EvalT,AlbanyTraits>(*p,dl));
    fm0.template registerEvaluator<EvalT>(ev);
  }
  if (fieldManagerChoice == Albany::BUILD_RESID_FM)  {
    PHX::Tag<typename EvalT::ScalarT> res_tag("Scatter Stokes", dl->dummy);
    fm0.requireField<EvalT>(res_tag);
  }
  else if (fieldManagerChoice == Albany::BUILD_RESPONSE_FM) {
    Albany::ResponseUtilities<EvalT, PHAL::AlbanyTraits> respUtils(dl);
    return respUtils.constructResponses(fm0, *responseList, Teuchos::null, stateMgr);
  }

  return Teuchos::null;
}

#endif // LANDICE_STOKESL1L2PROBLEM_HPP
