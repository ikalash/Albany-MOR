//*****************************************************************//
//    Albany 3.0:  Copyright 2016 Sandia Corporation               //
//    This Software is released under the BSD license detailed     //
//    in the file "license.txt" in the top-level Albany directory  //
//*****************************************************************//

#ifndef ATOT_SOLVER_H
#define ATOT_SOLVER_H

#include <iostream>

#include "LOCA.H"

#include "Thyra_ResponseOnlyModelEvaluatorBase.hpp"
#include "Albany_StateManager.hpp"
#include "Albany_Utils.hpp"
#include "ATO_Types.hpp"
#include "ATOT_Aggregator.hpp"
#include "ATOT_Optimizer.hpp"
#include "Petra_Converters.hpp" 

namespace ATO {
  
  class OptimizationProblem; 
  class Topology;

}


namespace ATOT {
  

  class SolverSubSolver;
  class SolverSubSolverData;

  typedef struct GlobalPoint{ 
    GlobalPoint();
    int    gid; 
    double coords[3]; 
  } GlobalPoint;
  bool operator< (GlobalPoint const & a, GlobalPoint const & b);

  // eventually make this a base class and derive from it to make
  // various kernels.  Also add a factory.
  class SpatialFilter{
    public:
      SpatialFilter(Teuchos::ParameterList& params);
      void buildOperator(
             Teuchos::RCP<Albany::Application> app,
             Teuchos::RCP<const Tpetra_Map>    overlapNodeMapT,
             Teuchos::RCP<const Tpetra_Map>    localNodeMapT, 
             Teuchos::RCP<Tpetra_Import>       importerT,
             Teuchos::RCP<Tpetra_Export>       exporterT);  
      Teuchos::RCP<Tpetra_CrsMatrix> FilterOperatorT(){return filterOperatorT;}
      //IKT, FIXME: remove the following function once Mark Hoemmen 
      //fixes apply method with TRANS mode in Tpetra::CrsMatrix.
      Teuchos::RCP<Tpetra_CrsMatrix> FilterOperatorTransposeT(){return filterOperatorTransposeT;}
      int getNumIterations(){return iterations;}
    protected:
      void importNeighbors(
             std::map< GlobalPoint, std::set<GlobalPoint> >& neighbors,
             Teuchos::RCP<Tpetra_Import>       importerT, 
             const Tpetra_Map& localNodeMapT,
             Teuchos::RCP<Tpetra_Export>       exporterT, 
             const Tpetra_Map& overlapNodeMapT);

      Teuchos::RCP<Tpetra_CrsMatrix> filterOperatorT;
      //IKT, FIXME: remove the following creation of filterOperatorTransposeT 
      //once Mark Hoemmen fixes apply method with TRANS mode in Tpetra::CrsMatrix.
      Teuchos::RCP<Tpetra_CrsMatrix> filterOperatorTransposeT;
      int iterations;
      double filterRadius;
      Teuchos::Array<std::string> blocks;
  };

  class OptInterface {
  public:
    virtual void Compute(const double* p, double& g, double* dgdp, double& c, double* dcdp=NULL)=0;

    virtual void ComputeMeasure(std::string measureType, double& measure)=0;

    virtual void ComputeMeasure(std::string measureType, const double* p, 
                                double& measure, double* dmdp, std::string integrationMethod)=0;
    virtual void ComputeMeasure(std::string measureType, const double* p, 
                                double& measure, std::string integrationMethod);

    virtual void ComputeMeasure(std::string measureType, const double* p, double& measure);
    virtual void ComputeMeasure(std::string measureType, const double* p, double& measure, double* dmdp);

    virtual void InitializeOptDofs(double* p)=0;
    virtual void getOptDofsLowerBound( Teuchos::Array<double>& b )=0;
    virtual void getOptDofsUpperBound( Teuchos::Array<double>& b )=0;

    virtual int GetNumOptDofs()=0;

    /* legacy */

    virtual void Compute(double* p, double& g, double* dgdp, double& c, double* dcdp=NULL)=0;
    virtual void ComputeConstraint(double* p, double& c, double* dcdp=NULL)=0;

    virtual void ComputeObjective(double* p, double& g, double* dgdp=NULL)=0;
    virtual void ComputeObjective(const double* p, double& g, double* dgdp=NULL)=0;
    virtual void ComputeVolume(double* p, const double* dfdp, double& v, double threshhold, double minP)=0;

   
    /* end legacy */
  };

class Solver : public Thyra::ResponseOnlyModelEvaluatorBase<ST>, public OptInterface {
public:

Solver(const Teuchos::RCP<Teuchos::ParameterList>& appParams,
    const Teuchos::RCP<const Teuchos_Comm>& comm,
    const Teuchos::RCP<const Tpetra_Vector>& initial_guess);

~Solver();  

//pure virtual from Thyra::ModelEvaluator
virtual Teuchos::RCP<const Thyra::VectorSpaceBase<ST>> get_x_space() const; 
//pure virtual from Thyra::ModelEvaluator
virtual Teuchos::RCP<const Thyra::VectorSpaceBase<ST>> get_f_space() const;  
//pure virtual from Thyra::ModelEvaluator
virtual Thyra::ModelEvaluatorBase::InArgs<ST> createInArgs() const;   
//pure virtual from Thyra::ModelEvaluator
virtual Thyra::ModelEvaluatorBase::OutArgs<ST> createOutArgsImpl() const;  
//pure virtual from Thyra::ModelEvaluator
void evalModelImpl(
Thyra::ModelEvaluatorBase::InArgs<ST> const & in_args,
Thyra::ModelEvaluatorBase::OutArgs<ST> const & out_args) const;  
//! Return parameter vector map
Teuchos::RCP<const Thyra::VectorSpaceBase<ST> > get_p_space(int l) const;

void Compute(double* p, double& f, double* dfdp, double& g, double* dgdp=NULL); 
void Compute(const double* p, double& f, double* dfdp, double& g, double* dgdp=NULL);  

void ComputeConstraint(double* p, double& c, double* dcdp=NULL);

void ComputeObjective(double* p, double& g, double* dgdp=NULL);  
void ComputeObjective(const double* p, double& g, double* dgdp=NULL); 
void writeCurrentDesign();
void InitializeOptDofs(double* p);
void getOptDofsLowerBound( Teuchos::Array<double>& b );
void getOptDofsUpperBound( Teuchos::Array<double>& b );
void ComputeVolume(double* p, const double* dfdp, double& v, double threshhold, double minP);
void ComputeMeasure(std::string measureType, const double* p, 
		double& measure, double* dmdp, std::string integrationMethod);
void ComputeMeasure(std::string measureType, double& measure);
int GetNumOptDofs();

private:

// data
int _iteration;
int _writeDesignFrequency;
int  numDims;
int _num_parameters; // for sensitiviy analysis(?)
int _num_responses;  //  ditto
// Teuchos::RCP<Tpetra_LocalMap> _tpetra_param_map;
// Teuchos::RCP<Tpetra_LocalMap> _tpetra_response_map;
Teuchos::RCP<const Tpetra_Map>      _tpetra_x_map;

int _numPhysics; // number of sub problems

std::vector<int> _wsOffset;  //index offsets to map to/from workset to/from 1D array.

bool _is_verbose;    // verbose or not for topological optimization solver
bool _is_restart;

Teuchos::RCP<Aggregator> _objAggregator;
Teuchos::RCP<Aggregator> _conAggregator;
Teuchos::RCP<Optimizer> _optimizer;


typedef struct TopologyInfoStructT {
  Teuchos::RCP<ATO::Topology>      topologyT;
  Teuchos::RCP<SpatialFilter> filterT;
  Teuchos::RCP<SpatialFilter> postFilterT;
  Teuchos::RCP<Tpetra_Vector> filteredOverlapVectorT;
  Teuchos::RCP<Tpetra_Vector> filteredVectorT;
  Teuchos::RCP<Tpetra_Vector> overlapVectorT;
  Teuchos::RCP<Tpetra_Vector> localVectorT;
  bool                        filterIsRecursiveT;
} TopologyInfoStructT;

std::vector<Teuchos::RCP<TopologyInfoStructT> > _topologyInfoStructsT;
Teuchos::RCP<ATO::TopologyArray> _topologyArrayT;

// currently all topologies must have the same entity type
std::string entityType;

std::vector<Teuchos::RCP<SpatialFilter> > filters;
Teuchos::RCP<SpatialFilter> _derivativeFilter;


typedef struct HomogenizationSet { 
std::string name;
std::string type;
int responseIndex;
int homogDim; 
std::vector<Teuchos::RCP<Teuchos::ParameterList> > homogenizationAppParams;
std::vector<SolverSubSolver> homogenizationProblems;
} HomogenizationSet;

std::vector<HomogenizationSet> _homogenizationSets;

std::vector<Teuchos::RCP<Teuchos::ParameterList> > _subProblemAppParams;
std::vector<SolverSubSolver> _subProblems;

ATO::OptimizationProblem* _atoProblem;

Teuchos::RCP<const Teuchos_Comm> _solverComm; 
Teuchos::RCP<Teuchos::ParameterList> _mainAppParams;

Teuchos::RCP<const Tpetra_Map> overlapNodeMapT;
Teuchos::RCP<const Tpetra_Map> localNodeMapT;


Teuchos::Array< Teuchos::RCP<Tpetra_Vector> > overlapObjectiveGradientVecT;
Teuchos::Array< Teuchos::RCP<Tpetra_Vector> > ObjectiveGradientVecT;

Teuchos::Array< Teuchos::RCP<Tpetra_Vector> > overlapConstraintGradientVecT;
Teuchos::Array< Teuchos::RCP<Tpetra_Vector> > ConstraintGradientVecT;

Teuchos::RCP<double> objectiveValue;
Teuchos::RCP<double> constraintValue;

Teuchos::RCP<Tpetra_Import> importerT;
Teuchos::RCP<Tpetra_Export> exporterT;

std::map<std::string, std::vector<Teuchos::RCP<const Tpetra_Vector>>> responseMapT;
std::map<std::string, std::vector<Teuchos::RCP<Tpetra_MultiVector>>> responseDerivMapT;


// methods
void copyTopologyIntoStateMgr(const double* p, Albany::StateManager& stateMgr );
void smoothTopologyT(double* p);
void smoothTopologyT(Teuchos::RCP<TopologyInfoStructT> topoStructT);
void copyTopologyFromStateMgr(double* p, Albany::StateManager& stateMgr );
void copyTopologyIntoParameter(const double* p, SolverSubSolver& sub);
void copyObjectiveFromStateMgr( double& g, double* dgdp );
void copyConstraintFromStateMgr( double& c, double* dcdp );
void zeroSet();
Teuchos::RCP<const Teuchos::ParameterList> getValidProblemParameters() const;

Teuchos::RCP<const Thyra::VectorSpaceBase<ST>> get_g_space(int j) const;

Teuchos::RCP<Teuchos::ParameterList> 
createInputFile( const Teuchos::RCP<Teuchos::ParameterList>& appParams, int physIndex) const;

SolverSubSolver CreateSubSolver(const Teuchos::RCP<Teuchos::ParameterList> appParams, 
			    const Teuchos::RCP<const Teuchos_Comm>& comm,
			    const Teuchos::RCP<const Tpetra_Vector>& initial_guess  = Teuchos::null);

Teuchos::RCP<Teuchos::ParameterList> 
createHomogenizationInputFile( 
    const Teuchos::RCP<Teuchos::ParameterList>& appParams, 
    const Teuchos::ParameterList& homog_subList, 
    int homogProblemIndex, 
    int homogSubIndex, 
    int homogDim) const;

SolverSubSolverData CreateSubSolverData(const SolverSubSolver& sub) const;

};

class SolverSubSolver {
public:
Teuchos::RCP<Albany::Application> app;
Teuchos::RCP<Thyra::ModelEvaluator<ST>> modelT;
Teuchos::RCP<Thyra::ModelEvaluatorBase::InArgs<ST>> params_inT;
Teuchos::RCP<Thyra::ModelEvaluatorBase::OutArgs<ST>> responses_outT;
    void freeUp() {app = Teuchos::null; modelT = Teuchos::null; }
  };

  class SolverSubSolverData {
  public:
    int Np;
    int Ng;
    std::vector<int> pLength;
    std::vector<int> gLength;
    Teuchos::RCP<const Tpetra_Vector> p_initT;
    Thyra::ModelEvaluatorBase::DerivativeSupport deriv_supportT;
  };

}
#endif
