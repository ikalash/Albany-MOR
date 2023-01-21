//*****************************************************************//
//    Albany 3.0:  Copyright 2016 Sandia Corporation               //
//    This Software is released under the BSD license detailed     //
//    in the file "license.txt" in the top-level Albany directory  //
//*****************************************************************//

#include "QCADT_CoupledPoissonSchrodinger.hpp"
#include "QCADT_CoupledPSJacobian.hpp"
#include "Albany_SolverFactory.hpp"
#include "Albany_ModelFactory.hpp"


/* GAHFIXME - Silence warning:
TRILINOS_DIR/../../../include/pecos_global_defs.hpp:17:0: warning: 
        "BOOST_MATH_PROMOTE_DOUBLE_POLICY" redefined [enabled by default]
Please remove when issue is resolved
*/
#undef BOOST_MATH_PROMOTE_DOUBLE_POLICY

#include "Teuchos_XMLParameterListHelpers.hpp"
#include "Teuchos_TestForException.hpp"

#include "Teuchos_ParameterList.hpp"

#include "Thyra_SpmdVectorBase.hpp"

#include "Albany_ModelFactory.hpp"
#include "Albany_Utils.hpp"
#include "Albany_SolverFactory.hpp"
#include "Albany_StateInfoStruct.hpp"
#include "Albany_EigendataInfoStructT.hpp"
#include "Albany_TpetraThyraUtils.hpp"

//For creating discretiation object without a problem object
#include "Albany_DiscretizationFactory.hpp"
#include "Albany_StateInfoStruct.hpp"
#include "Albany_AbstractFieldContainer.hpp"
#include "Albany_NullSpaceUtils.hpp"

//Ifpack includes
#include "Ifpack_ConfigDefs.h"
#include "Ifpack.h"

#define OUTPUT_TO_SCREEN

std::string QCADT::strdim(const std::string s, const int dim) {
  std::ostringstream ss;
  ss << s << " " << dim << "D";
  return ss.str();
}


QCADT::CoupledPoissonSchrodinger::
CoupledPoissonSchrodinger(const Teuchos::RCP<Teuchos::ParameterList>& appParams_,
			  const Teuchos::RCP<const Teuchos_Comm>& comm,
			  const Teuchos::RCP<const Tpetra_Vector>& initial_guess, 
                          Teuchos::RCP<Thyra::LinearOpWithSolveFactoryBase<ST> const> const &solver_factory):
  solver_factory_(solver_factory)
{
  std::cout << "DEBUG: " << __PRETTY_FUNCTION__ << "\n";
  using std::string;
  myComm = comm;  
  // make a copy of the appParams, since we modify them below (e.g. discretization list)
  Teuchos::RCP<Teuchos::ParameterList> appParams = Teuchos::rcp( new Teuchos::ParameterList(*appParams_) );

  // Get sub-problem input xml files from problem parameters
  Teuchos::ParameterList& problemParams = appParams->sublist("Problem");

  // Validate Problem parameters against list for this specific problem
  problemParams.validateParameters(*getValidProblemParameters(),0);

  // Get the number of dimensions
  numDims = 0;
  string name = problemParams.get<std::string>("Name");
  if(name == "Poisson Schrodinger 1D") numDims = 1;
  else if(name == "Poisson Schrodinger 2D") numDims = 2;
  else if(name == "Poisson Schrodinger 3D") numDims = 3;
  else TEUCHOS_TEST_FOR_EXCEPTION (true, Teuchos::Exceptions::InvalidParameter, std::endl 
			  << "Error!  Invalid problem name " << name << std::endl);

  // Get parameters from Problem sublist used to generate poisson and schrodinger app lists
  int vizDetail         = problemParams.get<int>("Phalanx Graph Visualization Detail");
  double Temp           = problemParams.get<double>("Temperature");
  double lenUnit        = problemParams.get<double>("Length Unit In Meters", 1e-6);
  double energyUnit     = problemParams.get<double>("Energy Unit In Electron Volts", 1.0);
  std::string matrlFile = problemParams.get<std::string>("MaterialDB Filename", "materials.xml");
  bool   bXCPot         = problemParams.get<bool>("Include exchange-correlation potential",false);
  bool   bQBOnly        = problemParams.get<bool>("Only solve schrodinger in quantum blocks",true);
  
  nEigenvals   = problemParams.get<int>("Number of Eigenvalues");
  int myRank = myComm->getRank(); 
  int nProcs = myComm->getSize(); 
  int nExtra = nEigenvals % nProcs;
  my_nEigenvals_ = (nEigenvals / nProcs) + ((myRank < nExtra) ? 1 : 0);
  Teuchos::ParameterList& discList = appParams->sublist("Discretization");
  Teuchos::ParameterList& poisson_subList = problemParams.sublist("Poisson Problem", false);
  Teuchos::ParameterList& schro_subList = problemParams.sublist("Schrodinger Problem", false);

  // Process debug options to write poisson and schrodinger app params to files
  Teuchos::ParameterList& debugList = appParams->sublist("Debug Output", true);
  std::string poissonXmlFile       = debugList.get("Poisson XML Input", "");
  std::string schrodingerXmlFile   = debugList.get("Schrodinger XML Input", "");
  std::string poissonExoOutput     = debugList.get("Poisson Exodus Output", "");
  std::string schrodingerExoOutput = debugList.get("Schrodinger Exodus Output", "");


  // Create input parameter list for poission app which mimics a separate input file
  Teuchos::RCP<Teuchos::ParameterList> poisson_appParams = 
    Teuchos::createParameterList("Poisson Application Parameters");
  Teuchos::ParameterList& poisson_probParams = poisson_appParams->sublist("Problem",false);
  
  poisson_probParams.set("Name", QCADT::strdim("Poisson",numDims));
  poisson_probParams.set("Phalanx Graph Visualization Detail", vizDetail);
  poisson_probParams.set("Length Unit In Meters",lenUnit);
  poisson_probParams.set("Energy Unit In Electron Volts",energyUnit);
  poisson_probParams.set("Temperature",Temp);
  poisson_probParams.set("MaterialDB Filename", matrlFile);

  {
    Teuchos::ParameterList auto_sourceList;
    auto_sourceList.set("Factor",1.0);
    auto_sourceList.set("Device","elementblocks");
    auto_sourceList.set("Quantum Region Source", "schrodinger");
    auto_sourceList.set("Non Quantum Region Source", bQBOnly ? "semiclassical" : "schrodinger");
    auto_sourceList.set("Eigenvectors to Import", nEigenvals);
    auto_sourceList.set("Use predictor-corrector method", false);
    auto_sourceList.set("Include exchange-correlation potential", bXCPot);

    Teuchos::ParameterList& sourceList = poisson_probParams.sublist("Poisson Source", false);
    if(poisson_subList.isSublist("Poisson Source"))
      sourceList.setParameters( poisson_subList.sublist("Poisson Source") );
    sourceList.setParametersNotAlreadySet( auto_sourceList );
  }

  {
    Teuchos::ParameterList auto_permList;
    auto_permList.set("Permittivity Type","Block Dependent");

    Teuchos::ParameterList& permList = poisson_probParams.sublist("Permittivity", false);
    if(!poisson_subList.isSublist("Permittivity"))
      permList.setParameters( poisson_subList.sublist("Permittivity") );
    permList.setParametersNotAlreadySet( auto_permList );
  }  

			      
  if(poisson_subList.isSublist("Dirichlet BCs")) {
    Teuchos::ParameterList& tmp = poisson_probParams.sublist("Dirichlet BCs", false);
    tmp.setParameters(poisson_subList.sublist("Dirichlet BCs"));
  }

  // Copy Parameter list over, or create an empty list if it was omitted (so validation passes)
  Teuchos::ParameterList& poisson_params = poisson_probParams.sublist("Parameters", false);
  if(poisson_subList.isSublist("Parameters"))
    poisson_params.setParameters(poisson_subList.sublist("Parameters"));
  else poisson_params.set("Number",0);

  // Copy Response Functions list over, or create an empty list if it was omitted (so validation passes)
  Teuchos::ParameterList& poisson_resps = poisson_probParams.sublist("Response Functions", false);
  if(poisson_subList.isSublist("Response Functions"))
    poisson_resps.setParameters(poisson_subList.sublist("Response Functions"));
  else poisson_resps.set("Number",0);  
  

  Teuchos::ParameterList& poisson_discList = poisson_appParams->sublist("Discretization", false);
  poisson_discList.setParameters(discList);
  if(poissonExoOutput.length() > 0) 
    poisson_discList.set("Exodus Output File Name",poissonExoOutput);
  else poisson_discList.remove("Exodus Output File Name",false); 

  if(poissonXmlFile.length() > 0 and myComm->getRank() == 0)
    Teuchos::writeParameterListToXmlFile(*poisson_appParams, poissonXmlFile);



  // Create input parameter list for schrodinger app which mimics a separate input file
  Teuchos::RCP<Teuchos::ParameterList> schro_appParams = 
    Teuchos::createParameterList("Schrodinger Application Parameters");  
  Teuchos::ParameterList& schro_probParams = schro_appParams->sublist("Problem",false);

  schro_probParams.set("Name", QCADT::strdim("Schrodinger",numDims));
  schro_probParams.set("Solution Method", "Continuation");
  schro_probParams.set("Phalanx Graph Visualization Detail", vizDetail);
  schro_probParams.set("Energy Unit In Electron Volts",energyUnit);
  schro_probParams.set("Length Unit In Meters",lenUnit);
  schro_probParams.set("MaterialDB Filename", matrlFile);
  schro_probParams.set("Only solve in quantum blocks", bQBOnly);

  {
    Teuchos::ParameterList auto_potList;
    auto_potList.set("Type","From Aux Data Vector");
    auto_potList.set("Aux Index", 0); //we import potential using aux vector 0
    
    Teuchos::ParameterList& potList = schro_probParams.sublist("Potential", false);
    if(schro_subList.isSublist("Potential"))
      potList.setParameters(schro_subList.sublist("Potential"));
    potList.setParametersNotAlreadySet(auto_potList);
  }

  if(!schro_subList.isSublist("Dirichlet BCs") && poisson_subList.isSublist("Dirichlet BCs")) {
    Teuchos::ParameterList& schro_dbcList = schro_probParams.sublist("Dirichlet BCs", false);
    const Teuchos::ParameterList& poisson_dbcList = poisson_subList.sublist("Dirichlet BCs");
    Teuchos::ParameterList::ConstIterator it;
    for(it = poisson_dbcList.begin(); it != poisson_dbcList.end(); ++it) {
      std::string dbcName = poisson_dbcList.name(it);
      std::size_t k = dbcName.find("Phi");
      if( k != std::string::npos ) {
	dbcName.replace(k, 3, "psi");  // replace Phi -> psi
	schro_dbcList.set( dbcName, 0.0 ); //copy all poisson DBCs but set to zero
      }
    }
  }

  // Copy Parameter list over, or create an empty list if it was omitted (so validation passes)
  Teuchos::ParameterList& schro_params = schro_probParams.sublist("Parameters", false);
  if(schro_subList.isSublist("Parameters"))
    schro_params.setParameters(schro_subList.sublist("Parameters"));
  else schro_params.set("Number",0);

  // Copy Response Functions list over, or create an empty list if it was omitted (so validation passes)
  Teuchos::ParameterList& schro_resps = schro_probParams.sublist("Response Functions", false);
  if(schro_subList.isSublist("Response Functions"))
    schro_resps.setParameters(schro_subList.sublist("Response Functions"));
  else schro_resps.set("Number",0);

  Teuchos::ParameterList& schro_discList = schro_appParams->sublist("Discretization", false);
  schro_discList.setParameters(discList);
  if(schrodingerExoOutput.length() > 0) 
    schro_discList.set("Exodus Output File Name",schrodingerExoOutput);
  else schro_discList.remove("Exodus Output File Name",false); 

  if(schrodingerXmlFile.length() > 0 and myComm->getRank() == 0)
    Teuchos::writeParameterListToXmlFile(*schro_appParams, schrodingerXmlFile);

    
  //TODO: need to add meshmover initialization, as in Albany::Application constructor??

  //Create a dummy solverFactory for validating application parameter lists
  Albany::SolverFactory validFactory( Teuchos::createParameterList("Empty dummy for Validation"), myComm );
  Teuchos::RCP<const Teuchos::ParameterList> validAppParams = validFactory.getValidAppParameters();
  Teuchos::RCP<const Teuchos::ParameterList> validParameterParams = validFactory.getValidParameterParameters();
  Teuchos::RCP<const Teuchos::ParameterList> validResponseParams = validFactory.getValidResponseParameters();

  ////////////////////////////////////////////////////////////////////////////////////////////////////////
  //! Create Poisson application object (similar logic in Albany::SolverFactory::createAlbanyAppAndModel)
  ////////////////////////////////////////////////////////////////////////////////////////////////////////

  saved_initial_guess = initial_guess;

  // Validate common parts of poisson app param list: may move inside individual Problem class
  poisson_appParams->validateParametersAndSetDefaults(*validAppParams,0);
  poisson_appParams->sublist("Problem").sublist("Parameters").validateParameters(*validParameterParams, 0);
  poisson_appParams->sublist("Problem").sublist("Response Functions").validateParameters(*validResponseParams, 0);
  poissonApp = Teuchos::rcp(new Albany::Application(myComm, poisson_appParams, Teuchos::null)); //validates problem params

  // Create model evaluator
  Albany::ModelFactory poissonModelFactory(poisson_appParams, poissonApp);
  poissonModel = poissonModelFactory.createT();

  //Get Poisson Jacobian 
  Teuchos::RCP<Tpetra_Operator> const Jac_Poisson_temp =
        Teuchos::nonnull(poissonModel->create_W_op()) ?
            ConverterT::getTpetraOperator(poissonModel->create_W_op()) :
            Teuchos::null;
  Jac_Poisson =
        Teuchos::nonnull(Jac_Poisson_temp) ?
            Teuchos::rcp_dynamic_cast<Tpetra_CrsMatrix>(Jac_Poisson_temp, true) :
            Teuchos::null;


  ////////////////////////////////////////////////////////////////////////////////////////////////////////
  //! Create Schrodinger application object (similar logic in Albany::SolverFactory::createAlbanyAppAndModel)
  ////////////////////////////////////////////////////////////////////////////////////////////////////////

  // Validate common parts of schrodinger app param list: may move inside individual Problem class
  schro_appParams->validateParametersAndSetDefaults(*validAppParams,0);
  schro_appParams->sublist("Problem").sublist("Parameters").validateParameters(*validParameterParams, 0);
  schro_appParams->sublist("Problem").sublist("Response Functions").validateParameters(*validResponseParams, 0);
  schrodingerApp = Teuchos::rcp(new Albany::Application(myComm, schro_appParams, Teuchos::null)); //validates problem params

  // Create model evaluator
  Albany::ModelFactory schrodingerModelFactory(schro_appParams, schrodingerApp);
  schrodingerModel = schrodingerModelFactory.createT();
  
  //Get Schrodinger Jacobian 
  Teuchos::RCP<Tpetra_Operator> const Jac_Schrodinger_temp =
        Teuchos::nonnull(poissonModel->create_W_op()) ?
            ConverterT::getTpetraOperator(schrodingerModel->create_W_op()) :
            Teuchos::null;
  Jac_Schrodinger =
        Teuchos::nonnull(Jac_Schrodinger_temp) ?
            Teuchos::rcp_dynamic_cast<Tpetra_CrsMatrix>(Jac_Schrodinger_temp, true) :
            Teuchos::null;

  //Save the discretization's maps for convenience (should be the same for Poisson and Schrodinger apps)
  disc_map = poissonApp->getMapT();
  disc_overlap_map =  poissonApp->getStateMgr().getDiscretization()->getOverlapMapT();
  //Allocate Schrodinger solution 
  x_schrodinger = Teuchos::rcp(new Tpetra_MultiVector(disc_map, nEigenvals));

  // Parameter vectors:  Parameter vectors of coupled PS model evaluator are just the parameter vectors
  //   of the Poisson then Schrodinger model evaluators (in order).

  //Get the number of parameter vectors of the Poisson model evaluator
  Thyra::ModelEvaluatorBase::InArgs<ST> poisson_inArgs = poissonModel->createInArgs();
  num_poisson_param_vecs = poisson_inArgs.Np();

  //Get the number of parameter vectors of the Schrodginer model evaluator
  Thyra::ModelEvaluatorBase::InArgs<ST> schrodinger_inArgs = schrodingerModel->createInArgs();
  num_schrodinger_param_vecs = schrodinger_inArgs.Np();

  num_param_vecs = num_poisson_param_vecs + num_schrodinger_param_vecs;

  // Create sacado parameter vectors of appropriate size for use in evalModel
  poisson_sacado_param_vec.resize(num_poisson_param_vecs);
  schrodinger_sacado_param_vec.resize(num_schrodinger_param_vecs);

  // Response vectors:  Response vectors of coupled PS model evaluator are just the response vectors
  //   of the Poisson then Schrodinger model evaluators (in order).
  num_response_vecs = poissonApp->getNumResponses() + schrodingerApp->getNumResponses();


  // Set member variables based on parameters from the main list
  temperature = Temp;
  length_unit_in_m  = lenUnit;
  energy_unit_in_eV = energyUnit;


  // Get conduction band offset from reference level (solution to poisson problem), as this
  //   is needed to convert the poisson solution vector to conduction band values expected by the schrodinger problem

    // Material database

  std::string mtrlDbFilename = "materials.xml";
  if(problemParams.isType<std::string>("MaterialDB Filename"))
    mtrlDbFilename = problemParams.get<std::string>("MaterialDB Filename");
  materialDB = Teuchos::rcp(new Albany::MaterialDatabase(mtrlDbFilename, myComm));
  
  std::string refMtrlName = materialDB->getParam<std::string>("Reference Material");
  double refmatChi = materialDB->getMaterialParam<double>(refMtrlName,"Electron Affinity");

  // compute energy reference
  double qPhiRef;
  {
    const double kB = 8.617332e-5;  // eV/K
    std::string category = materialDB->getMaterialParam<std::string>(refMtrlName,"Category");
    if (category == "Semiconductor") 
    {
      // Same qPhiRef needs to be used for the entire structure
      double mdn = materialDB->getMaterialParam<double>(refMtrlName,"Electron DOS Effective Mass");
      double mdp = materialDB->getMaterialParam<double>(refMtrlName,"Hole DOS Effective Mass");
      double Chi = materialDB->getMaterialParam<double>(refMtrlName,"Electron Affinity");
      double Eg0 = materialDB->getMaterialParam<double>(refMtrlName,"Zero Temperature Band Gap");
      double alpha = materialDB->getMaterialParam<double>(refMtrlName,"Band Gap Alpha Coefficient");
      double beta = materialDB->getMaterialParam<double>(refMtrlName,"Band Gap Beta Coefficient");
      
      double Eg = Eg0 - alpha*pow(temperature,2.0)/(beta+temperature); // in [eV]
      double kbT = kB * temperature;      // in [eV]
      double Eic = -Eg/2. + 3./4.*kbT*log(mdp/mdn);  // (Ei-Ec) in [eV]
      qPhiRef = Chi - Eic;  // (Evac-Ei) in [eV] where Evac = vacuum level
    }
    else 
      TEUCHOS_TEST_FOR_EXCEPTION (true, Teuchos::Exceptions::InvalidParameter, std::endl 
			  << "Error!  Invalid category " << category << " for reference material !" << std::endl);
  } 

  // NOTE: this works for element blocks of the reference material, but really needs to have refmatChi replaced by the
  //     chi (electron affinity) for the element block that owns each node...
  this->offset_to_CB = qPhiRef - refmatChi; // Conduction Band = offset - poisson_solution


  // Add discretization parameters to appParams to describe to discretization object how to 
  //   interpret the "combined" solution vector used by the coupled P-S solver.
  std::vector<std::string> solnVecComps( 2*(1+nEigenvals) ), residVecComps( 2*(1+nEigenvals) );
  
  solnVecComps[0] = "solution"; solnVecComps[1] = "S"; //was "Potential" but keep as "solution" for backward compatibility
  residVecComps[0] = "PoissonRes"; residVecComps[1] = "S";
  for(int i=0; i<nEigenvals; i++) {
    std::ostringstream ss1; ss1 << "Eigenvector" << i;
    solnVecComps[ 2*(i+1) ] = ss1.str(); solnVecComps[ 2*(i+1)+1 ] = "S";
    std::ostringstream ss2; ss2 << "SchroRes" << i;
    residVecComps[ 2*(i+1) ] = ss2.str(); ; residVecComps[ 2*(i+1)+1 ] = "S";
  }
  
  discList.set("Solution Vector Components", Teuchos::Array<std::string>(solnVecComps));
  discList.set("Residual Vector Components", Teuchos::Array<std::string>(residVecComps));
  discList.set("Interleaved Ordering", false); //combined vector is concatenated, not "interleaved"
  /* -- Example XML this would generate --
     <Parameter name="Solution Vector Components" type="Array(string)" value="{Potential, S, Eigenvector0, S, Eigenvector1, S}"/>
     <Parameter name="Residual Vector Components" type="Array(string)" value="{PoissonRes, S, SchroRes0, S, SchroRes1, S}"/>
     <Parameter name="Interleaved Ordering" type="bool" value="false"/>
  */

  // Create discretization object solely for producing collected output
  Albany::DiscretizationFactory discFactory(appParams, myComm);

  // Get mesh specification object: worksetSize, cell topology, etc
  Teuchos::ArrayRCP<Teuchos::RCP<Albany::MeshSpecsStruct> > meshSpecs =
    discFactory.createMeshSpecs();

  Albany::AbstractFieldContainer::FieldContainerRequirements requirements; //empty?
  Teuchos::RCP<Albany::StateInfoStruct> stateInfo = poissonApp->getStateMgr().getStateInfoStruct(); //for now, just use Poisson app's states
                            //Teuchos::rcp(new Albany::StateInfoStruct); //empty

  int neq = 1+nEigenvals; // number of mesh-equations
  Teuchos::RCP<Albany::RigidBodyModes> rigidBodyModes(Teuchos::rcp(new Albany::RigidBodyModes(neq)));
  disc = discFactory.createDiscretization(neq, stateInfo,requirements,rigidBodyModes);

  
  //------------------Setup nominal values----------------
  //We are coupling 2+nEigenvals models: 1 Poisson eqn + nEigenvals Schrodinger eqns + 1 nEigenvals eigenvalue eqns
  num_models_ = 2 + nEigenvals; 
  dist_eigenval_map = Teuchos::rcp(new Tpetra_Map(nEigenvals, my_nEigenvals_, 0, myComm));
  Tpetra::LocalGlobal lg = Tpetra::LocallyReplicated;
  local_eigenval_map = Teuchos::rcp(new Tpetra_Map(nEigenvals, 0, myComm, lg));
  eigenvals = Teuchos::rcp(new Tpetra_Vector(local_eigenval_map)); 
  nominal_values_ = this->createInArgsImpl();
  allocateVectors(); //sets x and x_dot in nominal_values_

  //set p_init
  for (int l = 0; l < num_param_vecs; ++l) {
    std::vector<Teuchos::RCP<Thyra::VectorSpaceBase<ST> const>> vs_array;
    if(l < num_poisson_param_vecs) {
      //Poisson model: 
      vs_array.push_back(poissonModel->get_p_space(l)); 
    }
    else {
      //Schrodinger model:  
      vs_array.push_back(schrodingerModel->get_p_space(l - num_poisson_param_vecs));
    }
    Teuchos::RCP<Thyra::DefaultProductVectorSpace<ST> const> p_space = Thyra::productVectorSpace<ST>(vs_array);
    
    std::vector<Teuchos::RCP<Thyra::VectorBase<ST> const>> pvecs_array;
    if(l < num_poisson_param_vecs) { 
      //Poisson model: 
      pvecs_array.push_back(poissonModel->getNominalValues().get_p(l)); 
    }
    else {
      //Schrodinger model:  
      pvecs_array.push_back(schrodingerModel->getNominalValues().get_p(l - num_poisson_param_vecs)); 
    }
  
    Teuchos::RCP<Thyra::DefaultProductVector<ST>> p_prod_vec = Thyra::defaultProductVector<ST>(p_space, pvecs_array);

    if (Teuchos::is_null(p_prod_vec) == true) continue;

    nominal_values_.set_p(l, p_prod_vec);
  }
  // Get material parameters for quantum region, used in computing quantum density
  quantumMtrlName = materialDB->getParam<std::string>("Quantum Material");
  valleyDegeneracyFactor = materialDB->getMaterialParam<int>(quantumMtrlName,"Number of conduction band min",2);
  effMass = materialDB->getMaterialParam<double>(quantumMtrlName,"Transverse Electron Effective Mass");
}

QCADT::CoupledPoissonSchrodinger::~CoupledPoissonSchrodinger()
{
}

void
QCADT::CoupledPoissonSchrodinger::allocateVectors()
{
#ifdef OUTPUT_TO_SCREEN
  std::cout << "DEBUG: " << __PRETTY_FUNCTION__ << "\n";
#endif
  Teuchos::Array<Teuchos::RCP<Thyra::VectorSpaceBase<ST> const>> spaces(num_models_); 
  
  //Poisson and Schrodinger have same disc_map
  for (int m=0; m<1+nEigenvals; ++m)
    spaces[m] = Thyra::createVectorSpace<ST>(disc_map);

  //last space is eigenvalue space
  spaces[1+nEigenvals] = Thyra::createVectorSpace<ST>(dist_eigenval_map); 

  
  Teuchos::RCP<Thyra::DefaultProductVectorSpace<ST> const> space = Thyra::productVectorSpace<ST>(spaces);
  Teuchos::ArrayRCP<Teuchos::RCP<Thyra::VectorBase<ST>>> xT_vecs;

  Teuchos::ArrayRCP<Teuchos::RCP<Thyra::VectorBase<ST>>>  x_dotT_vecs;

  xT_vecs.resize(num_models_);
  x_dotT_vecs.resize(num_models_);

  const Teuchos::RCP<const Tpetra_MultiVector> xMV_poisson = poissonApp->getAdaptSolMgrT()->getInitialSolution();
  Teuchos::RCP<Tpetra_Vector> xT_poisson = Teuchos::rcp(new Tpetra_Vector(*xMV_poisson->getVector(0)));
  TEUCHOS_TEST_FOR_EXCEPTION(xMV_poisson->getNumVectors() < 2, std::logic_error,
     "QCADT_CoupledPoissonSchrodinger Poisson Error! Time derivative data is not present!");
  Teuchos::RCP<Tpetra_Vector> x_dotT_poisson = Teuchos::rcp(new Tpetra_Vector(*xMV_poisson->getVector(1)));

  const Teuchos::RCP<const Tpetra_MultiVector> xMV_schro = schrodingerApp->getAdaptSolMgrT()->getInitialSolution();
  Teuchos::RCP<Tpetra_Vector> xT_schro = Teuchos::rcp(new Tpetra_Vector(*xMV_schro->getVector(0)));
  TEUCHOS_TEST_FOR_EXCEPTION(xMV_schro->getNumVectors() < 2, std::logic_error,
     "QCADT_CoupledPoissonSchrodinger Schrodinger Error! Time derivative data is not present!");
  Teuchos::RCP<Tpetra_Vector> x_dotT_schro = Teuchos::rcp(new Tpetra_Vector(*xMV_schro->getVector(1)));

  //Poisson initial solution
  xT_vecs[0] = Thyra::createVector(xT_poisson, spaces[0]);
  x_dotT_vecs[0] = Thyra::createVector(x_dotT_poisson, spaces[0]);
  
  //Schrodinger initial solutions -- they are the same for all nEigenvals Schrodinger equations
  for (int m=1; m<1+nEigenvals; ++m) {
    xT_vecs[m] = Thyra::createVector(xT_schro, spaces[m]);
    x_dotT_vecs[m] = Thyra::createVector(x_dotT_schro, spaces[m]);
  }
  
  //All zero initial solutions for eigenvalues
  Teuchos::RCP<Tpetra_Vector> xT_eval = Teuchos::rcp(new Tpetra_Vector(dist_eigenval_map)); 
  Teuchos::RCP<Tpetra_Vector> xdotT_eval = Teuchos::rcp(new Tpetra_Vector(dist_eigenval_map)); 
  xT_eval->putScalar(0.0); 
  xdotT_eval->putScalar(0.0); 
  xT_vecs[1+nEigenvals] = Thyra::createVector(xT_eval); 
  x_dotT_vecs[1+nEigenvals] = Thyra::createVector(xdotT_eval); 

  Teuchos::RCP<Thyra::DefaultProductVector<ST>> xT_prod_vec = Thyra::defaultProductVector<ST>(space, xT_vecs());
  Teuchos::RCP<Thyra::DefaultProductVector<ST>> x_dotT_prod_vec = Thyra::defaultProductVector<ST>(space, x_dotT_vecs());

  nominal_values_.set_x(xT_prod_vec);
  nominal_values_.set_x_dot(x_dotT_prod_vec);
  
}

Teuchos::RCP<const Thyra::VectorSpaceBase<ST>> QCADT::CoupledPoissonSchrodinger::get_x_space() const
{
  std::cout << "DEBUG: " << __PRETTY_FUNCTION__ << "\n";
  return createCombinedRangeSpace(); 
}

Teuchos::RCP<const Thyra::VectorSpaceBase<ST>> QCADT::CoupledPoissonSchrodinger::get_f_space() const
{
  std::cout << "DEBUG: " << __PRETTY_FUNCTION__ << "\n";
  return createCombinedRangeSpace(); 
}

Teuchos::RCP<Thyra::VectorSpaceBase<ST> const>
QCADT::CoupledPoissonSchrodinger::createCombinedRangeSpace() const
{
#ifdef OUTPUT_TO_SCREEN
  std::cout << "DEBUG: " << __PRETTY_FUNCTION__ << "\n";
#endif
  Teuchos::RCP<Thyra::ProductVectorSpaceBase<ST> > range_space; 
  // loop over all vectors and build the vector space
  std::vector<Teuchos::RCP<Thyra::VectorSpaceBase<ST> const>> vs_array;

  //Poisson and Schrodinger models have the same map: disc_map 
  for (int m = 0; m < 1+nEigenvals; ++m) { 
    vs_array.push_back(Thyra::createVectorSpace<ST, LO, Tpetra_GO, KokkosNode>(disc_map));
  }
  //Last map is eigenvalue map
  vs_array.push_back(Thyra::createVectorSpace<ST, LO, Tpetra_GO, KokkosNode>(dist_eigenval_map));
  range_space = Thyra::productVectorSpace<ST>(vs_array);
  return range_space;
}


Teuchos::RCP<const Thyra::VectorSpaceBase<ST>> QCADT::CoupledPoissonSchrodinger::get_p_space(int l) const
{
  #ifdef OUTPUT_TO_SCREEN
  std::cout << "DEBUG: " << __PRETTY_FUNCTION__ << "\n";
#endif
  TEUCHOS_TEST_FOR_EXCEPTION(l >= num_param_vecs || l < 0, Teuchos::Exceptions::InvalidParameter,
                     std::endl <<
                     "Error in QCADT::CoupledPoissonSchrodinger::get_p_space():  " <<
                     "Invalid parameter index l = " << l << std::endl);
  
  std::vector<Teuchos::RCP<Thyra::VectorSpaceBase<ST> const>> vs_array;
  if(l < num_poisson_param_vecs) {
    //Poisson model: 
    vs_array.push_back(poissonModel->get_p_space(l)); 
  }
  else {
    //Schrodinger model:  
    vs_array.push_back(schrodingerModel->get_p_space(l - num_poisson_param_vecs));
  }
  return Thyra::productVectorSpace<ST>(vs_array);
 
}

Teuchos::RCP<const Thyra::VectorSpaceBase<ST>> QCADT::CoupledPoissonSchrodinger::get_g_space(int j) const
{
#ifdef OUTPUT_TO_SCREEN
  std::cout << "DEBUG: " << __PRETTY_FUNCTION__ << "\n";
#endif
  TEUCHOS_TEST_FOR_EXCEPTION(j > num_response_vecs || j < 0, Teuchos::Exceptions::InvalidParameter,
                     std::endl <<
                     "Error in QCADT::CoupledPoissonSchrodinger::get_g_space():  " <<
                     "Invalid response index j = " << j << std::endl);
  
  std::vector<Teuchos::RCP<Thyra::VectorSpaceBase<ST> const>> vs_array;
  if(j < poissonApp->getNumResponses()) {
    //Poisson model: 
    vs_array.push_back(Thyra::createVectorSpace<ST, LO, Tpetra_GO, KokkosNode>(
                poissonApp->getResponse(j)->responseMapT())); 
  }
  else {
    //Schrodinger model:  
    vs_array.push_back(Thyra::createVectorSpace<ST, LO, Tpetra_GO, KokkosNode>(
                schrodingerApp->getResponse(j - poissonApp->getNumResponses())->responseMapT())); 
  }
  return Thyra::productVectorSpace<ST>(vs_array);
}

Teuchos::RCP<const Teuchos::Array<std::string> > QCADT::CoupledPoissonSchrodinger::get_p_names(int l) const
{
#ifdef OUTPUT_TO_SCREEN
  std::cout << "DEBUG: " << __PRETTY_FUNCTION__ << "\n";
#endif
  TEUCHOS_TEST_FOR_EXCEPTION(l >= num_param_vecs || l < 0, 
		     Teuchos::Exceptions::InvalidParameter,
                     std::endl << 
                     "Error!  QCADT::CoupledPoissonSchrodinger::get_p_names():  " <<
                     "Invalid parameter index l = " << l << std::endl);
  if(l < num_poisson_param_vecs)
    return poissonModel->get_p_names(l);
  else
    return schrodingerModel->get_p_names(l - num_poisson_param_vecs);
  
}

Thyra::ModelEvaluatorBase::InArgs<ST>
QCADT::CoupledPoissonSchrodinger::getNominalValues() const
{
#ifdef OUTPUT_TO_SCREEN
  std::cout << "DEBUG: " << __PRETTY_FUNCTION__ << "\n";
#endif
  return nominal_values_;
}

Thyra::ModelEvaluatorBase::InArgs<ST>
QCADT::CoupledPoissonSchrodinger::getLowerBounds() const
{
#ifdef OUTPUT_TO_SCREEN
  std::cout << "DEBUG: " << __PRETTY_FUNCTION__ << "\n";
#endif
  return Thyra::ModelEvaluatorBase::InArgs<ST>(); // Default value
}

Thyra::ModelEvaluatorBase::InArgs<ST>
QCADT::CoupledPoissonSchrodinger::getUpperBounds() const
{
#ifdef OUTPUT_TO_SCREEN
  std::cout << "DEBUG: " << __PRETTY_FUNCTION__ << "\n";
#endif
  return Thyra::ModelEvaluatorBase::InArgs<ST>(); // Default value
}


Teuchos::RCP<Thyra::LinearOpBase<ST>>
QCADT::CoupledPoissonSchrodinger::create_W_op() const
{
#ifdef OUTPUT_TO_SCREEN
  std::cout << "DEBUG: " << __PRETTY_FUNCTION__ << "\n";
#endif
  QCADT::CoupledPSJacobian psJac(nEigenvals, disc_map, 
                          numDims, valleyDegeneracyFactor, temperature,
                          length_unit_in_m, energy_unit_in_eV, effMass, offset_to_CB, eigenvals, x_schrodinger, myComm);
  return psJac.getThyraCoupledJacobian(Jac_Poisson, Jac_Schrodinger, Jac_Schrodinger); 
}

Teuchos::RCP<Thyra::PreconditionerBase<ST>>
QCADT::CoupledPoissonSchrodinger::create_W_prec() const
{
#ifdef OUTPUT_TO_SCREEN
  std::cout << "DEBUG: " << __PRETTY_FUNCTION__ << "\n";
#endif
  //Analog of EpetraExt::ModelEvaluator::Preconditioner does not exist
  //in Thyra yet!  So problem will run for now with no
  //preconditioner...
   bool const
  W_prec_not_supported = true;

  TEUCHOS_TEST_FOR_EXCEPT(W_prec_not_supported);
  return Teuchos::null;
}

Teuchos::RCP<const Thyra::LinearOpWithSolveFactoryBase<ST>>
QCADT::CoupledPoissonSchrodinger::get_W_factory() const
{
#ifdef OUTPUT_TO_SCREEN
  std::cout << "DEBUG: " << __PRETTY_FUNCTION__ << "\n";
#endif
  return solver_factory_;
}

void
QCADT::CoupledPoissonSchrodinger::reportFinalPoint(
    Thyra::ModelEvaluatorBase::InArgs<ST> const & final_point,
    bool const was_solved)
{
#ifdef OUTPUT_TO_SCREEN
  std::cout << "DEBUG: " << __PRETTY_FUNCTION__ << "\n";
#endif
  TEUCHOS_TEST_FOR_EXCEPTION(true,
      Teuchos::Exceptions::InvalidParameter,
      "Calling reportFinalPoint in QCAD_CoupledPoissonSchrodinger.cpp" << '\n');
}


Teuchos::RCP<Thyra::LinearOpBase<ST>>
QCADT::CoupledPoissonSchrodinger::
create_DgDx_op_impl(int j) const
{
#ifdef OUTPUT_TO_SCREEN
  std::cout << "DEBUG: " << __PRETTY_FUNCTION__ << "\n";
#endif
  //FIXME, IKT, 5/21/15: save for later. 
/*  TEUCHOS_TEST_FOR_EXCEPTION(
    j >= num_response_vecs || j < 0, 
    Teuchos::Exceptions::InvalidParameter,
    std::endl << 
    "Error!  Albany::ModelEvaluator::create_DgDx_op():  " << 
    "Invalid response index j = " << j << std::endl);
  
  if(j < poissonApp->getNumResponses())
    return poissonApp->getResponse(j)->createGradientOp();
  else
    return schrodingerApp->getResponse(j - poissonApp->getNumResponses())->createGradientOp();
*/
  return Teuchos::null;
}

Teuchos::RCP<Thyra::LinearOpBase<ST>>
QCADT::CoupledPoissonSchrodinger::create_DgDx_dot_op_impl(int j) const
{
#ifdef OUTPUT_TO_SCREEN
  std::cout << "DEBUG: " << __PRETTY_FUNCTION__ << "\n";
#endif
  //FIXME, IKT, 5/21/15: save for later. 
/*
  TEUCHOS_TEST_FOR_EXCEPTION(
    j >= num_response_vecs || j < 0, 
    Teuchos::Exceptions::InvalidParameter,
    std::endl << 
    "Error!  Albany::ModelEvaluator::create_DgDx_dot_op():  " << 
    "Invalid response index j = " << j << std::endl);
  
  if(j < poissonApp->getNumResponses())
    return poissonApp->getResponse(j)->createGradientOp();
  else
    return schrodingerApp->getResponse(j - poissonApp->getNumResponses())->createGradientOp();
*/
  return Teuchos::null;
}

Thyra::ModelEvaluatorBase::InArgs<ST>
QCADT::CoupledPoissonSchrodinger::
createInArgsImpl() const
{
#ifdef OUTPUT_TO_SCREEN
  std::cout << "DEBUG: " << __PRETTY_FUNCTION__ << "\n";
#endif
  Thyra::ModelEvaluatorBase::InArgsSetup<ST> inArgs;
  inArgs.setModelEvalDescription("QCAD Coupled Poisson-Schrodinger Model Evaluator");

  inArgs.setSupports(IN_ARG_t,true);
  inArgs.setSupports(IN_ARG_x,true);
  inArgs.setSupports(IN_ARG_x_dot,true);
  inArgs.setSupports(IN_ARG_alpha,true);
  inArgs.setSupports(IN_ARG_beta,true);
  inArgs.set_Np(num_param_vecs);

  // Note: no SG support yet...

  return inArgs;

}

Thyra::ModelEvaluatorBase::InArgs<ST>
QCADT::CoupledPoissonSchrodinger::createInArgs() const
{
#ifdef OUTPUT_TO_SCREEN
  std::cout << "DEBUG: " << __PRETTY_FUNCTION__ << "\n";
#endif
  return this->createInArgsImpl();
}


Thyra::ModelEvaluatorBase::OutArgs<ST> 
QCADT::CoupledPoissonSchrodinger::createOutArgsImpl() const
{
#ifdef OUTPUT_TO_SCREEN
  std::cout << "DEBUG: " << __PRETTY_FUNCTION__ << "\n";
#endif

  Thyra::ModelEvaluatorBase::OutArgsSetup<ST> outArgs; 
  outArgs.setModelEvalDescription("QCAD Coupled Poisson-Schrodinger Model Evaluator");

  int n_g = num_response_vecs;
  bool bScalarResponse;

  // Deterministic
  outArgs.setSupports(Thyra::ModelEvaluatorBase::OUT_ARG_f,true);
  outArgs.setSupports(Thyra::ModelEvaluatorBase::OUT_ARG_W_op,true);
  outArgs.set_W_properties(
      Thyra::ModelEvaluatorBase::DerivativeProperties(
          Thyra::ModelEvaluatorBase::DERIV_LINEARITY_UNKNOWN,
          Thyra::ModelEvaluatorBase::DERIV_RANK_FULL,
          true));
  outArgs.set_Np_Ng(num_param_vecs, n_g);
/*
 * FIXME, IKT, 5/21/15, save for later
  for (int i=0; i<num_param_vecs; i++)
    outArgs.setSupports(Thyra::ModelEvaluatorBase::OUT_ARG_DfDp, i, DerivativeSupport(DERIV_MV_BY_COL));
  for (int i=0; i<n_g; i++) {

    if(i < poissonApp->getNumResponses())
      bScalarResponse = poissonApp->getResponse(i)->isScalarResponse();
    else
      bScalarResponse = schrodingerApp->getResponse(i - poissonApp->getNumResponses())->isScalarResponse();

    if(bScalarResponse) {
      outArgs.setSupports(OUT_ARG_DgDx, i,
                          DerivativeSupport(DERIV_TRANS_MV_BY_ROW));
      outArgs.setSupports(OUT_ARG_DgDx_dot, i,
                          DerivativeSupport(DERIV_TRANS_MV_BY_ROW));
    }
    else {
      outArgs.setSupports(OUT_ARG_DgDx, i,
                          DerivativeSupport(DERIV_LINEAR_OP));
      outArgs.setSupports(OUT_ARG_DgDx_dot, i,
                          DerivativeSupport(DERIV_LINEAR_OP));
    }

    for (int j=0; j<num_param_vecs; j++)
      outArgs.setSupports(OUT_ARG_DgDp, i, j,
                          DerivativeSupport(DERIV_MV_BY_COL));
  }*/

  //Note: no SG support yet...

  return outArgs;
  
}


void 
QCADT::CoupledPoissonSchrodinger::
evalModelImpl(
    Thyra::ModelEvaluatorBase::InArgs<ST> const & in_args,
    Thyra::ModelEvaluatorBase::OutArgs<ST> const & out_args) const
{
#ifdef OUTPUT_TO_SCREEN
  std::cout << "DEBUG: " << __PRETTY_FUNCTION__ << "\n";
#endif
  //?? Teuchos::TimeMonitor Timer(*timer); //start timer

  //
  // Get the input arguments
  
  Teuchos::RCP<const Thyra::ProductVectorBase<ST>>
  x = Teuchos::rcp_dynamic_cast<const Thyra::ProductVectorBase<ST>>(
          in_args.get_x(), true);

  Teuchos::RCP<const Thyra::ProductVectorBase<ST>>
  x_dot = Teuchos::nonnull(in_args.get_x_dot()) ?
          Teuchos::rcp_dynamic_cast<const Thyra::ProductVectorBase<ST>>(
              in_args.get_x_dot(), true) :
          Teuchos::null;
  
   // Create a Teuchos array of the xT and x_dotT for each model,
   // casting to Tpetra_Vector
 
   Teuchos::Array<Teuchos::RCP<const Thyra_Vector>> xs(num_models_);
   Teuchos::Array<Teuchos::RCP<const Thyra_Vector>> x_dots(num_models_);
 
   for (int m=0; m < num_models_; ++m) {
    //Get each Tpetra vector 
    xs[m] = x->getVectorBlock(m);
   }
   if (x_dot != Teuchos::null) {
     for (int m = 0; m < num_models_; ++m) {
        //Get each Tpetra vector 
        x_dots[m] = x_dot->getVectorBlock(m);
     }
   }
  double alpha     = 0.0;  // M coeff
  double beta      = 1.0;  // J coeff
  double curr_time = 0.0;
  if (x_dot != Teuchos::null) {
    alpha = in_args.get_alpha();
    beta = in_args.get_beta();
    curr_time  = in_args.get_t();
    std::cout << "DEBUG: WARNING: x_dot given to CoupledPoissonSchrodinger evalModel!!" << std::endl;
  }

  for (int i=0; i<in_args.Np(); i++) {
    Teuchos::RCP<Thyra::ProductVectorBase<ST> const> pT =
      Teuchos::rcp_dynamic_cast<const Thyra::ProductVectorBase<ST>>(
         in_args.get_p(i), true);
    //Teuchos::RCP<const Epetra_Vector> p = inArgs.get_p(i);
    if (pT != Teuchos::null) {
      if(i < num_poisson_param_vecs) {
        //get Poisson parameter vector
        Teuchos::RCP<Tpetra_Vector const> pT_poisson = Teuchos::rcp_dynamic_cast<const Thyra_TpetraVector>(
                                          pT->getVectorBlock(0), true)->getConstTpetraVector();
        Teuchos::ArrayRCP<ST const> pT_poisson_constView = pT_poisson->get1dView();
	for (unsigned int j=0; j<poisson_sacado_param_vec[i].size(); j++) 
	  poisson_sacado_param_vec[i][j].baseValue = pT_poisson_constView[j];
      }
      else {
        //get Schrodinger parameter vector
        Teuchos::RCP<Tpetra_Vector const> pT_schrodinger = Teuchos::rcp_dynamic_cast<const Thyra_TpetraVector>(
                                          pT->getVectorBlock(1), true)->getConstTpetraVector();
        Teuchos::ArrayRCP<ST const> pT_schrodinger_constView = pT_schrodinger->get1dView();
	for (unsigned int j=0; j<schrodinger_sacado_param_vec[i-num_poisson_param_vecs].size(); j++)
	  schrodinger_sacado_param_vec[i-num_poisson_param_vecs][j].baseValue = pT_schrodinger_constView[j];
      }
    }
  }

  //
  // Get the output arguments
  //
  Teuchos::RCP<Thyra::ProductVectorBase<ST>> f_out =
      Teuchos::nonnull(out_args.get_f()) ?
          Teuchos::rcp_dynamic_cast<Thyra::ProductVectorBase<ST>>(
              out_args.get_f(), true) :
          Teuchos::null;

  Teuchos::RCP<Thyra::LinearOpBase<ST>> W_out = Teuchos::nonnull(out_args.get_W_op()) ?
                                                    out_args.get_W_op() : Teuchos::null;


  // Get views into 'x' (and 'xdot'?) vectors to use for separate poisson and schrodinger application object calls
  //
  int disc_nMyElements = disc_map->getNodeNumElements();

  Teuchos::RCP<const Tpetra_Vector> eigenvals_dist;
  Teuchos::RCP<const Thyra_Vector> x_poisson, xdot_poisson;
  Teuchos::RCP<const Thyra_MultiVector> xdot_schrodinger = Thyra::createMembers(Albany::createConstThyraMultiVector(x_schrodinger)->range(),nEigenvals);
  std::vector<Teuchos::RCP<const Thyra_Vector>> xdot_schrodinger_vec(nEigenvals);

  //First model is Poisson.
  x_poisson = xs[0];

  //Next nEigenvals models are Schrodinger
  //IKT, can we accomplish the following with arrayviews?  
  for (int m=1; m < 1+nEigenvals; ++m) {
    auto x_spmd = Teuchos::rcp_dynamic_cast<const Thyra::SpmdVectorBase<ST>>(xs[m]);
    TEUCHOS_TEST_FOR_EXCEPTION(x_spmd.is_null(), std::runtime_error, "Error! Something went wrong while casting to SpmdVectorBase.\n");
    auto local_dim = x_spmd->spmdSpace()->localSubDim();
    Teuchos::ArrayRCP<const ST> array;
    x_spmd->getLocalData(Teuchos::ptr(&array));
    for (std::size_t i=0; i<local_dim; ++i)
       x_schrodinger->replaceLocalValue(i,m-1,array[i]);
  }
 
  //Last model is eigenvalues
  eigenvals_dist = Albany::getConstTpetraVector(xs[1+nEigenvals]);
  
  if (x_dot != Teuchos::null) {  //maybe unnecessary - it seems that the coupled PS model evaluator shouldn't support x_dot ...
    xdot_poisson = x_dots[0]; 
    for (int m=1; m < 1+nEigenvals; ++m) 
      xdot_schrodinger_vec[m-1] = x_dots[m];
  } else {
    xdot_poisson = Teuchos::null;
    for(int i=0; i<nEigenvals; i++) 
      xdot_schrodinger_vec[i] = Teuchos::null;
  }

  //
  // Communicate all the eigenvalues to every processor, since all parts of the mesh need them
  Teuchos::RCP<Tpetra_Import> eigenval_importer = Teuchos::rcp(new Tpetra_Import(dist_eigenval_map, local_eigenval_map)); 

  eigenvals->doImport(*eigenvals_dist, *eigenval_importer, Tpetra::INSERT); 

  const Teuchos::ArrayRCP<const ST> eigenvals_constView = eigenvals->get1dView();
  Teuchos::RCP<std::vector<double> > stdvec_eigenvals = 
      Teuchos::rcp(new std::vector<double>(&eigenvals_constView[0], &eigenvals_constView[0] + nEigenvals));
  //
  // Get views into 'f' residual vector to use for separate poisson and schrodinger application object calls
  //
  Teuchos::RCP<Tpetra_Vector> f_poisson, f_norm_local, f_norm_dist; 
  Teuchos::RCP<Tpetra_MultiVector> f_schrodinger;
  std::vector<Tpetra_Vector*> f_schrodinger_vec(nEigenvals);

  if (f_out != Teuchos::null) {
    //First model is Poisson.
    f_poisson = Teuchos::rcp_dynamic_cast<Thyra_TpetraVector>(
        f_out->getNonconstVectorBlock(0),
        true)->getTpetraVector();
    //Next nEigenvals models are Schrodinger
    for (int m=1; m < 1+nEigenvals; ++m) 
      f_schrodinger_vec[m-1] = Teuchos::rcp_dynamic_cast<Thyra_TpetraVector>(
        f_out->getNonconstVectorBlock(m),
        true)->getTpetraVector().getRawPtr();  
    //Last model is eigenvalue. 
    f_norm_dist = Teuchos::rcp_dynamic_cast<Thyra_TpetraVector>(
        f_out->getNonconstVectorBlock(1+nEigenvals), true)->getTpetraVector(); 
    //   (later we sum all procs contributions together and copy into distributed f_norm_dist vector)
    f_norm_local = Teuchos::rcp(new Tpetra_Vector(local_eigenval_map));
  }
  else {
    f_poisson = Teuchos::null;
    for(int i=0; i<nEigenvals; i++) f_schrodinger_vec[i] = NULL;
    f_norm_local = f_norm_dist = Teuchos::null;
  }
  const Teuchos::ArrayRCP<ST> f_norm_local_nonConstView = f_norm_local->get1dViewNonConst(); 
  const Teuchos::ArrayRCP<ST> f_norm_dist_nonConstView = f_norm_dist->get1dViewNonConst(); 
 
  // Create an eigendata struct for passing the eigenvectors to the poisson app
  //  -- note that this requires the *overlapped* eigenvectors
  Teuchos::RCP<Albany::EigendataStructT> eigenData = Teuchos::rcp( new Albany::EigendataStructT );
  eigenData->eigenvalueRe = stdvec_eigenvals;
  eigenData->eigenvectorRe = 
    Teuchos::rcp(new Tpetra_MultiVector(disc_overlap_map, nEigenvals));
  eigenData->eigenvectorIm = Teuchos::null; // no imaginary eigenvalue data... 

  // Importer for overlapped data
  Teuchos::RCP<Tpetra_Import> overlap_importer =
    Teuchos::rcp(new Tpetra_Import(disc_map, disc_overlap_map));

  
    // Overlapped eigenstate vectors
   //(eigenData->eigenvectorRe)->doImport( *x_schrodinger, *overlap_importer, Tpetra::INSERT );
   for(int i=0; i<nEigenvals; i++) {
     Teuchos::RCP<Tpetra_Vector> eigenData_i = (eigenData->eigenvectorRe)->getVectorNonConst(i); 
     eigenData_i->doImport(*(x_schrodinger->getVector(i)), *overlap_importer, Tpetra::INSERT); 
     //eigenData_i->putScalar(0.0); //DEBUG - zero out eigenvectors passed to Poisson
   }

    // set eigenvalues / eigenvectors for use in poisson problem:
  poissonApp->getStateMgr().setEigenDataT(eigenData);

  // Get overlapped version of potential (x_poisson) for passing as auxData to schrodinger app
  Teuchos::RCP<Tpetra_MultiVector> overlapped_V = Teuchos::rcp(new Tpetra_MultiVector(disc_overlap_map, 1));
  Teuchos::RCP<Tpetra_Vector> ones_vec = Teuchos::rcp(new Tpetra_Vector(disc_overlap_map));
  ones_vec->putScalar(1.0);
  Teuchos::RCP<Tpetra_Vector> overlapped_V0 = overlapped_V->getVectorNonConst(0); 
  overlapped_V0->doImport( *Albany::getConstTpetraVector(x_poisson), *overlap_importer, Tpetra::INSERT);
  overlapped_V0->update(offset_to_CB, *ones_vec, -1.0);
  //std::cout << "DEBUG: Offset to conduction band = " << offset_to_CB << std::endl;
  // set potential for use in schrodinger problem
  schrodingerApp->getStateMgr().setAuxDataT(overlapped_V);

  
  //
  // Compute the functions
  //
  bool f_poisson_already_computed = false;
  std::vector<bool> f_schrodinger_already_computed(nEigenvals, false);

  Teuchos::RCP<Tpetra_CrsMatrix> W_out_poisson_crs; //possibly used by preconditioner, so declare here
  Teuchos::RCP<Tpetra_CrsMatrix> W_out_schrodinger_crs; //possibly used by preconditioner, so declare here

  // Mass Matrix -- needed even if we don't need to compute the Jacobian, since it enters into the normalization equations
  //   --> Compute mass matrix using schrodinger equation -- independent of eigenvector so can just use 0th
  //       Note: to compute this, we need to evaluate the schrodinger problem as a transient problem, so create a dummy xdot...
  const Teuchos::RCP<Tpetra_Operator> M_out_schrodinger = Teuchos::nonnull(schrodingerModel->create_W_op()) ?
            ConverterT::getTpetraOperator(schrodingerModel->create_W_op()) :
            Teuchos::null; //maybe re-use this and not create it every time?

  Teuchos::RCP<Tpetra_CrsMatrix> M_out_schrodinger_crs =   Teuchos::nonnull(M_out_schrodinger) ?
            Teuchos::rcp_dynamic_cast<Tpetra_CrsMatrix>(M_out_schrodinger, true) :
            Teuchos::null;

  schrodingerApp->computeGlobalJacobian(1.0, 0.0, 0.0, curr_time,
              Albany::createConstThyraVector(x_schrodinger->getVector(0)),
              schrodingerModel->getNominalValues().get_x_dot(),
              Teuchos::null,
					    schrodinger_sacado_param_vec, f_schrodinger_vec[0], *M_out_schrodinger_crs);
  
  // Hamiltionan Matrix -- needed even if we don't need to compute the Jacobian, since this is how we compute the schrodinger residuals
  //   --> Computed as jacobian matrix of schrodinger equation -- independent of eigenvector so can just use 0th
  Teuchos::RCP<Tpetra_Operator> const J_out_schrodinger =
        Teuchos::nonnull(schrodingerModel->create_W_op()) ?
            ConverterT::getTpetraOperator(schrodingerModel->create_W_op()) :
            Teuchos::null;
  Teuchos::RCP<Tpetra_CrsMatrix> J_out_schrodinger_crs =
        Teuchos::nonnull(J_out_schrodinger) ?
            Teuchos::rcp_dynamic_cast<Tpetra_CrsMatrix>(J_out_schrodinger, true) :
            Teuchos::null;
  schrodingerApp->computeGlobalJacobian(0.0, 1.0, 0.0, curr_time,
              Albany::createConstThyraVector(x_schrodinger->getVector(0)),
              schrodingerModel->getNominalValues().get_x_dot(),
              Teuchos::null,
					    schrodinger_sacado_param_vec, f_schrodinger_vec[0], *J_out_schrodinger_crs);
  
  f_schrodinger_already_computed[0] = true; //residual is not affected by alpha & beta, so both of the above calls compute it.

  // W 
  if (W_out != Teuchos::null) { 
    // W = alpha*M + beta*J where M is mass mx and J is jacobian

    //if we need to compute the jacobian, get the jacobians of the poisson and schrodinger
    //  applications (as crs matrices), as well as the mass matrix (from the schrodinger problem,
    //  since it includes xdot - maybe need to fabricate this??) and from these construct a CoupledPoissonSchrodingerJacobian object (an Epetra_Operator)
    
    //TODO - how to allow general alpha and beta?  This won't work given current logic, so we should test that alpha=0, beta=1 and throw an error otherwise...

    // Compute poisson Jacobian
    const Teuchos::RCP<Tpetra_Operator> W_out_poisson = Teuchos::nonnull(poissonModel->create_W_op()) ?
            ConverterT::getTpetraOperator(poissonModel->create_W_op()) :
            Teuchos::null; //maybe re-use this and not create it every time? 

    Teuchos::RCP<Tpetra_CrsMatrix> W_out_poisson_crs =   Teuchos::nonnull(W_out_poisson) ?
            Teuchos::rcp_dynamic_cast<Tpetra_CrsMatrix>(W_out_poisson, true) :
            Teuchos::null;

    poissonApp->computeGlobalJacobian(alpha, beta, 0.0, curr_time,
              x_poisson, xdot_poisson, Teuchos::null,
				      poisson_sacado_param_vec, f_poisson.get(), *W_out_poisson_crs);
    f_poisson_already_computed = true;

    TEUCHOS_TEST_FOR_EXCEPTION(nEigenvals <= 0, Teuchos::Exceptions::InvalidParameter,"Error! The number of eigenvalues must be greater than zero.");
      
    //Compute schrodinger Jacobian using first eigenvector -- independent of eigenvector since Schro. eqn is linear
    // (test for cases we've already computed above)
    if(alpha == 1.0 && beta == 0.0)
      W_out_schrodinger_crs = M_out_schrodinger_crs;
    else if(alpha == 0.0 && beta == 1.0)
      W_out_schrodinger_crs = J_out_schrodinger_crs;
    else {
      const Teuchos::RCP<Tpetra_Operator> W_out_schrodinger = Teuchos::nonnull(schrodingerModel->create_W_op()) ?
              ConverterT::getTpetraOperator(schrodingerModel->create_W_op()) :
              Teuchos::null; //maybe re-use this and not create it every time? 
      Teuchos::RCP<Tpetra_CrsMatrix> W_out_schrodinger_crs =   Teuchos::nonnull(W_out_schrodinger) ?
              Teuchos::rcp_dynamic_cast<Tpetra_CrsMatrix>(W_out_schrodinger, true) :
              Teuchos::null;
      schrodingerApp->computeGlobalJacobian(alpha, beta, 0.0, curr_time,
                                            Albany::createConstThyraVector(x_schrodinger->getVector(0)),
                                            xdot_schrodinger_vec[0], Teuchos::null,
                                					  schrodinger_sacado_param_vec, f_schrodinger_vec[0], *W_out_schrodinger_crs);
      f_schrodinger_already_computed[0] = true;
    }    
    if (W_out != Teuchos::null) {
      QCADT::CoupledPSJacobian psJac(nEigenvals, disc_map,
                          numDims, valleyDegeneracyFactor, temperature,
                          length_unit_in_m, energy_unit_in_eV, effMass, offset_to_CB, eigenvals, x_schrodinger, myComm);
      W_out = psJac.getThyraCoupledJacobian(W_out_poisson_crs, W_out_schrodinger_crs, M_out_schrodinger_crs); 
    } 

    /*
    // DEBUG --- JACOBIAN TEST -----------------------------------------------------------------------------------------
     
    Teuchos::RCP<Epetra_Vector> x_plus_dx = Teuchos::rcp( new Epetra_Vector(*combined_SP_map) );
    Teuchos::RCP<Epetra_Vector> dx_test = Teuchos::rcp( new Epetra_Vector(*combined_SP_map) );
    double eps;
    
    //Init dx_test
    Teuchos::RCP<Epetra_Vector> dx_test_poisson, dx_test_eigenvals;
    Teuchos::RCP<Epetra_MultiVector> dx_test_schrodinger;
    separateCombinedVector(dx_test, dx_test_poisson, dx_test_schrodinger, dx_test_eigenvals);

    eps = 1e-7;
    dx_test->PutScalar(1.0);
    //dx_test_poisson->PutScalar(1.0);
    //dx_test_schrodinger->PutScalar(1.0);
    //dx_test_eigenvals->PutScalar(1.0);

    Teuchos::RCP<Epetra_Vector> f_test_manual = Teuchos::rcp( new Epetra_Vector(*combined_SP_map) );
    Teuchos::RCP<Epetra_Vector> f_test_tmp = Teuchos::rcp( new Epetra_Vector(*combined_SP_map) );
    Teuchos::RCP<Epetra_Vector> f_test_jac = Teuchos::rcp( new Epetra_Vector(*combined_SP_map) );

    computeResidual(x, f_test_manual, M_out_schrodinger_crs);
    //f_test_manual->Print( std::cout << "JACOBIAN TEST:  MANUAL 1" << std::endl);
    x_plus_dx->Update(1.0, *x, eps, *dx_test, 0.0); // x + eps*dx
    computeResidual(x_plus_dx, f_test_tmp, M_out_schrodinger_crs);
    //f_test_tmp->Print( std::cout << "JACOBIAN TEST:  MANUAL 2" << std::endl);
    f_test_manual->Update(1.0/eps, *f_test_tmp, -1.0/eps); // f_test_manual = (resid(x+eps*dx) - resid(x)) / eps ~ jacobian * dx

    W_out_psj->Apply(*dx_test, *f_test_jac);


    //x_schrodinger->Print( std::cout << "JACOBIAN TEST:  X_SCHRODINGER" << std::endl);
    f_test_manual->Print( std::cout << "JACOBIAN TEST:  MANUAL DIFF" << std::endl);
    f_test_jac->Print( std::cout << "JACOBIAN TEST:  JACOBIAN DIFF" << std::endl);

    f_test_tmp->Update(1.0, *f_test_manual, -1.0, *f_test_jac, 0.0);
    f_test_tmp->Print( std::cout << "JACOBIAN TEST:  COMPARISON VECTOR" << std::endl);
    
    double test_norm;
    f_test_tmp->Norm2(&test_norm);
    std::cout << "JACOBIAN TEST: COMPARISON VECTOR 2-NORM = " << test_norm << std::endl;

    // DEBUG --- JACOBIAN TEST -----------------------------------------------------------------------------------------
    */
  }

  //IKT, 5/26/15: removed all preconditioner stuff since we'll be using Teko. 

/*
  //IKT, FIXME 5/26/15: do dfdp later.
  // df/dp
  for (int i=0; i<outArgs.Np(); i++) {
    Teuchos::RCP<Epetra_MultiVector> dfdp_out = 
      outArgs.get_DfDp(i).getMultiVector();
    if (dfdp_out != Teuchos::null) {


      // Get views into dfdp_out vectors for poisson and schrodinger parts
      //  Note that df/dp will be zero for parts of f corresponding to an app
      //    different from the one owning the p vector.  E.g. if i==0 corresponds
      //    to p being a poisson parameter vector then df/dp == 0 for all the schrodinger
      //    parts of f.

      int nParamComponents = dfdp_out->NumVectors();
      Teuchos::RCP<Epetra_MultiVector> dfdp_poisson;
      std::vector< Teuchos::RCP<Epetra_MultiVector> > dfdp_schrodinger(nEigenvals);

      double *dfdp_data; int myLDA;
      if(dfdp_out->ExtractView(&dfdp_data, &myLDA) != 0) 
	TEUCHOS_TEST_FOR_EXCEPTION(true, Teuchos::Exceptions::InvalidParameter,
				   "Error!  QCADT::CoupledPoissonSchrodinger -- cannot extract dgdp vector view");

      dfdp_poisson = Teuchos::rcp(new Epetra_MultiVector(::View, *disc_map, &(dfdp_data[0]), myLDA, nParamComponents));
      for(int k=0; k<nEigenvals; k++)
	dfdp_schrodinger[k] = Teuchos::rcp(new Epetra_MultiVector(::View, *disc_map, &(dfdp_data[(k+1)*disc_nMyElements]), myLDA, nParamComponents));

      // Assemble p_vec
      Teuchos::Array<int> p_indexes = 
        outArgs.get_DfDp(i).getDerivativeMultiVector().getParamIndexes();
      Teuchos::RCP<ParamVec> p_vec;

      Teuchos::Array<ParamVec>& sacado_param_vec = 
	(i < num_poisson_param_vecs) ? poisson_sacado_param_vec : schrodinger_sacado_param_vec;
      int offset = (i < num_poisson_param_vecs) ? 0 : num_poisson_param_vecs;

      if (p_indexes.size() == 0)
        p_vec = Teuchos::rcp(&sacado_param_vec[i-offset],false);
      else {
        p_vec = Teuchos::rcp(new ParamVec);
        for (int j=0; j<p_indexes.size(); j++)
          p_vec->addParam(sacado_param_vec[i-offset][p_indexes[j]].family, 
                          sacado_param_vec[i-offset][p_indexes[j]].baseValue);
      }

      dfdp_out->PutScalar(0.0);

      // Compute full dfdp by computing non-zero parts and leaving zeros in others
      if (i < num_poisson_param_vecs) {
	// "Poisson-owned" param vector, so only poisson part of dfdp vector can be nonzero
	poissonApp->computeGlobalTangent(0.0, 0.0, 0.0, curr_time, false, xdot_poisson.get(), NULL, *x_poisson, 
				  poisson_sacado_param_vec, p_vec.get(),
				  NULL, NULL, NULL, NULL, f_poisson.get(), NULL, 
				  dfdp_poisson.get());

	f_poisson_already_computed=true;
      }
      else {
	// "Schrodinger-owned" param vector, so only schrodinger parts of dfdp vector can be nonzero
	for(int k=0; k<nEigenvals; k++) {
	  schrodingerApp->computeGlobalTangent(0.0, 0.0, 0.0, curr_time, false, xdot_schrodinger_vec[k], NULL, *((*x_schrodinger)(k)),
					       schrodinger_sacado_param_vec, p_vec.get(),
					       NULL, NULL, NULL, NULL, f_schrodinger_vec[k], NULL, 
					       dfdp_schrodinger[k].get());	
	  f_schrodinger_already_computed[k]=true;
	}
      }
    }
  }
*/
  // f
  /*if (app->is_adjoint) {  //TODO: support Adjoints?
    TEUCHOS_TEST_FOR_EXCEPTION(true, Teuchos::Exceptions::InvalidParameter,
				   "Error!  QCADT::CoupledPoissonSchrodinger -- adjoints not implemented yet");
    Derivative f_deriv(ut, DERIV_TRANS_MV_BY_ROW);
    int response_index = 0; // need to add capability for sending this in
    app->evaluateResponseDerivative(response_index, curr_time, x_dot.get(), *x, 
				    sacado_param_vec, NULL, 
				    NULL, f_deriv, Derivative(), Derivative());
  }
  else {  */
    if (f_out != Teuchos::null) { 
      Teuchos::RCP<Tpetra_Vector> M_vec = Teuchos::rcp(new Tpetra_Vector(disc_map));  
      //temp storage for mass matrix times vec -- maybe don't allocate this on the stack??

      if(!f_poisson_already_computed) {
	      poissonApp->computeGlobalResidual(
            curr_time,
            x_poisson, xdot_poisson, Teuchos::null,
					  poisson_sacado_param_vec, *f_poisson);
      }
      
      for(int i=0; i<nEigenvals; i++) {
        // Compute Mass_matrix * eigenvector[i]
        const Tpetra_Vector& vec = *x_schrodinger->getVector(i);
        M_out_schrodinger_crs->apply(vec, *M_vec, Teuchos::NO_TRANS, 1.0, 0.0);  


        // Compute the schrodinger residual f_schrodinger_vec[i]: H*eigenvector[i] - eigenvalue[i] * M * eigenvector[i]

        if(!f_schrodinger_already_computed[i]) {

            //Could call this, but multiply below is faster
          //schrodingerApp->computeGlobalResidual(curr_time, xdot_schrodinger_vec[i], *((*x_schrodinger)(i)), 
          //				      schrodinger_sacado_param_vec, *(f_schrodinger_vec[i]) );  // H*evec[i] 

          // H * Psi - E * M * Psi
          const Tpetra_CrsMatrix& Hamiltonian_crs =  *J_out_schrodinger_crs;
           Hamiltonian_crs.apply(vec, *f_schrodinger_vec[i], Teuchos::NO_TRANS, 1.0, 0.0);
        }
        /* ---- DEBUG ----
           double He_norm, Me_norm, H_expect;
           f_schrodinger_vec[i]->Norm2(&He_norm);
           M_vec.Norm2(&Me_norm);
           std::cout << "DEBUG " << i << ": norm(-H*evec) = " << He_norm << ", norm(M*evec) = " << Me_norm 
           << ", eval = " << (*stdvec_eigenvals)[i] << std::endl;
           f_schrodinger_vec[i]->Dot(vec, &H_expect);
        */

        // add -eval[i]*M*evec[i] to H*evec[i] (recall evals are really negative_evals)
        double eval = (*stdvec_eigenvals)[i];
        f_schrodinger_vec[i]->update( eval, *M_vec, 1.0); 

              // Compute normalization equation residuals:  f_norm[i] = abs(1 - evec[i] . M . evec[i])
        ST vec_M_vec;
              const Teuchos::ArrayView<ST> vec_M_vec_AV = Teuchos::arrayView(&vec_M_vec, 1);
        vec.dot( *M_vec, vec_M_vec_AV );
        f_norm_local_nonConstView[i] = 1.0 - vec_M_vec;
      }

      // Fill elements of f_norm_dist that belong to this processor, i.e. loop over
      // eigenvalue indices "owned" by the current proc in the combined distributed map
      std::vector<int> eval_global_elements(my_nEigenvals_);
      eval_global_elements[0] = dist_eigenval_map->getGlobalNumElements();
      for(int i=0; i<my_nEigenvals_; i++) {
	      f_norm_dist_nonConstView[i] = f_norm_local_nonConstView[eval_global_elements[i]];
      }

      //DEBUG -- print residual in gory detail for debugging
      if(1) {
	if(myComm->getRank() == 0) std::cout << "DEBUG: ----------------- Coupled Schrodinger Poisson Info Dump ---------------------" << std::endl;
	double norm, mean;

	/*std::cout << "x map has " << x->Map().NumGlobalElements() << " global els" << std::endl;
	  std::cout << "x_poisson map has " << x_poisson->Map().NumGlobalElements() << " global els" << std::endl;
	  std::cout << "x_schrodinger map has " << x_schrodinger->Map().NumGlobalElements() << " global els (each vec)" << std::endl;
	  std::cout << "dist_eval_map has " << dist_eigenval_map.NumGlobalElements() << " global els" << std::endl;
	*/

       //IKT, 5/26/15, FIXME: 
       //The following is not defined for product vectors
       /*norm = x->norm2(); mean = x->meanValue();
	std::cout << std::setprecision(10);
	if(myComm->getRank() == 0) std::cout << "X Norm & Mean = " << norm << " , " << mean << std::endl;*/
	
  auto one = Thyra::createMember(x_poisson->space());
  Thyra::assign(one.ptr(),1.0);
	norm = x_poisson->norm_2(); mean = x_poisson->dot(*one)/x_poisson->space()->dim();
	if(myComm->getRank() == 0) std::cout << "Poisson-part X Norm & Mean = " << norm << " , " << mean << std::endl;
	for(int i=0; i<nEigenvals; i++) {
	  norm = x_schrodinger->getVector(i)->norm2();
	  if(myComm->getRank() == 0) std::cout << "Schrodinger[" << i << "]-part X Norm = " << norm << std::endl;
	}
	for(int i=0; i<nEigenvals; i++) 
	  if(myComm->getRank() == 0) std::cout << "Eigenvalue[" << i << "] = " << (*stdvec_eigenvals)[i] << std::endl;
	
	norm = f_poisson->norm2();
	if(myComm->getRank() == 0) std::cout << "Poisson-part Residual Norm = " << norm << std::endl; //f_poisson->Print(std::cout);
	for(int i=0; i<nEigenvals; i++) {
	  if(f_schrodinger_vec[i] != NULL) {
	    norm = f_schrodinger_vec[i]->norm2();
	    if(myComm->getRank() == 0) std::cout << "Schrodinger[" << i << "]-part Residual Norm = " << norm << std::endl; //f_schrodinger_vec[i]->Print(std::cout);
	  }
	}
	if(myComm->getRank() == 0) std::cout << "Eigenvalue-part Residual: " << std::endl;
	f_norm_dist->print(std::cout); // only rank 0 prints
      } 
    }
  // Response functions
  for (int i=0; i<out_args.Ng(); i++) {
   Teuchos::RCP<Thyra::ProductVectorBase<ST>>
    g_out = Teuchos::nonnull(out_args.get_g(i)) ?
          Teuchos::rcp_dynamic_cast<Thyra::ProductVectorBase<ST>>(
              out_args.get_g(i), true) :
          Teuchos::null;
   
    bool g_computed = false;
  // IKT, FIXME, 5/26/15: hold of conversion for now of dg/dx, dg/dp.
/*
    Derivative dgdx_out = outArgs.get_DgDx(i);
    Derivative dgdxdot_out = outArgs.get_DgDx_dot(i);
        
    //IK, 10/9/14
    //convert g_out to Tpetra for evaluateResponseTangentT calls
    Teuchos::RCP<const Teuchos_Comm> commT = Albany::createTeuchosCommFromEpetraComm(commE);
    Teuchos::RCP<Tpetra_Vector> g_outT; 
    if (g_out != Teuchos::null) 
      g_outT = Petra::EpetraVector_To_TpetraVectorNonConst(*g_out, commT); 
    //convert xdot_poisson and x_poisson to Tpetra  
    Teuchos::RCP<const Tpetra_Vector> x_poissonT, xdot_poissonT; 
    if (x_poisson != Teuchos::null) 
      x_poissonT  = Petra::EpetraVector_To_TpetraVectorConst(*x_poisson, commT); 
    if (xdot_poisson != Teuchos::null)  
      xdot_poissonT = Petra::EpetraVector_To_TpetraVectorConst(*xdot_poisson, commT);
    //convert xdot_schrodinger and x_schrodinger to Tpetra  
    Teuchos::RCP<const Tpetra_Vector> x_schrodingerT, xdot_schrodingerT; 
    if (x_schrodinger != Teuchos::null) 
      x_schrodingerT  = Petra::EpetraVector_To_TpetraVectorConst(*((*x_schrodinger)(0)), commT); 
    if (xdot_schrodinger != Teuchos::null)
      xdot_schrodingerT  = Petra::EpetraVector_To_TpetraVectorConst(*xdot_schrodinger_vec[0], commT); 

    // dg/dx, dg/dxdot
    if (!dgdx_out.isEmpty() || !dgdxdot_out.isEmpty()) {
      if(i < poissonApp->getNumResponses()) {
	poissonApp->evaluateResponseDerivative(i, curr_time, xdot_poisson.get(), NULL, *(x_poisson),
                                      poisson_sacado_param_vec, NULL,
                                      g_out.get(), dgdx_out,
                                      dgdxdot_out, Derivative(), Derivative());
      }
      else {
	// take response derivatives using lowest eigenstate only (is there something better??)
	schrodingerApp->evaluateResponseDerivative(i - poissonApp->getNumResponses(), curr_time, xdot_schrodinger_vec[0], NULL,
                                      *((*x_schrodinger)(0)),
                                      schrodinger_sacado_param_vec, NULL,
                                      g_out.get(), dgdx_out,
                                      dgdxdot_out, Derivative(), Derivative());
      }
      g_computed = true;
    }

    // dg/dp
    for (int j=0; j<outArgs.Np(); j++) {
      Teuchos::RCP<Epetra_MultiVector> dgdp_out =
        outArgs.get_DgDp(i,j).getMultiVector();
      if (dgdp_out != Teuchos::null) {
        Teuchos::Array<int> p_indexes =
          outArgs.get_DgDp(i,j).getDerivativeMultiVector().getParamIndexes();

        Teuchos::RCP<ParamVec> p_vec;

	Teuchos::Array<ParamVec>& sacado_param_vec = 
	  (j < num_poisson_param_vecs) ? poisson_sacado_param_vec : schrodinger_sacado_param_vec;
	int offset = (j < num_poisson_param_vecs) ? 0 : num_poisson_param_vecs;

        if (p_indexes.size() == 0)
          p_vec = Teuchos::rcp(&sacado_param_vec[j-offset],false);
        else {
          p_vec = Teuchos::rcp(new ParamVec);
          for (int k=0; k<p_indexes.size(); k++)
            p_vec->addParam(sacado_param_vec[j-offset][p_indexes[k]].family,
                            sacado_param_vec[j-offset][p_indexes[k]].baseValue);
        }
       //IK, 10/9/14: converg dgdp_out to Tpetra            
       Teuchos::RCP<Tpetra_MultiVector> dgdp_outT;
       if (dgdp_out != Teuchos::null) 
        dgdp_outT = Petra::EpetraMultiVector_To_TpetraMultiVector(*dgdp_out, commT); 

	if(i < poissonApp->getNumResponses() && j < num_poisson_param_vecs) {
	  //both response and param vectors belong to poisson problem
	  poissonApp->evaluateResponseTangentT(i, alpha, beta, 0.0, curr_time, false,
					      xdot_poissonT.get(), NULL, *x_poissonT,
					      poisson_sacado_param_vec, p_vec.get(),
					      NULL, NULL, NULL, NULL, g_outT.get(), NULL,
					      dgdp_outT.get());
          //convert g_outT to Epetra_Vector g_out
          Petra::TpetraVector_To_EpetraVector(g_outT, *g_out, commE); 	
          //convert dgdp_outT to Epetra_MultiVector dgdp_out
          Petra::TpetraMultiVector_To_EpetraMultiVector(dgdp_outT, *dgdp_out, commE); 	
	}
	else if(i >= poissonApp->getNumResponses() && j >= num_poisson_param_vecs) {
	  //both response and param vectors belong to schrodinger problem -- evaluate dg/dp using first eigenvector
	  schrodingerApp->evaluateResponseTangentT(i - poissonApp->getNumResponses(), alpha, beta, 0.0, curr_time, false,
						  xdot_schrodingerT.get(), NULL, *x_schrodingerT,
						  schrodinger_sacado_param_vec, p_vec.get(),
						  NULL, NULL, NULL, NULL, g_outT.get(), NULL,
						  dgdp_outT.get());
          //convert g_outT to Epetra_Vector g_out
          Petra::TpetraVector_To_EpetraVector(g_outT, *g_out, commE); 	
          //convert dgdp_outT to Epetra_MultiVector dgdp_out
          Petra::TpetraMultiVector_To_EpetraMultiVector(dgdp_outT, *dgdp_out, commE); 	
	}
	else {
	  // response and param vectors belong to different sub-problems (Poisson or Schrodinger)
	  dgdp_out->PutScalar(0.0);
	}

        g_computed = true;
      }
    }
    */
    if (g_out != Teuchos::null && !g_computed) {
      //Poisson model:
      Teuchos::RCP<Tpetra_Vector>
        g_out_poisson = Teuchos::rcp_dynamic_cast<Thyra_TpetraVector>(
                  g_out->getNonconstVectorBlock(0),
                  true)->getTpetraVector();
      //Schrodinger model:
      Teuchos::RCP<Tpetra_Vector>
        g_out_schrodinger = Teuchos::rcp_dynamic_cast<Thyra_TpetraVector>(
                  g_out->getNonconstVectorBlock(1),
                  true)->getTpetraVector();
      if(i < poissonApp->getNumResponses()) {
	      poissonApp->evaluateResponse(i, curr_time, x_poisson, xdot_poisson, Teuchos::null,
				                             poisson_sacado_param_vec, *g_out_poisson);
      } else {
	      schrodingerApp->evaluateResponse(i - poissonApp->getNumResponses(), curr_time,
                                         Albany::createConstThyraVector(x_schrodinger->getVector(0)),
                                         xdot_schrodinger->col(0),
                                         Teuchos::null,
                                         schrodinger_sacado_param_vec, *g_out_schrodinger);
      }
    }

  }
}

Teuchos::RCP<Albany::Application>
QCADT::CoupledPoissonSchrodinger::getPoissonApp() const
{
  std::cout << "DEBUG: " << __PRETTY_FUNCTION__ << "\n";
  return poissonApp;
}

Teuchos::RCP<Albany::Application>
QCADT::CoupledPoissonSchrodinger::getSchrodingerApp() const
{
  std::cout << "DEBUG: " << __PRETTY_FUNCTION__ << "\n";
  return schrodingerApp;
}



// Note: we could use QCADT::separateCombinedVector(...) to implement this function and those below
/*void QCADT::CoupledPoissonSchrodinger::separateCombinedVector(const Teuchos::RCP<Epetra_Vector>& combinedVector,
							     Teuchos::RCP<Epetra_Vector>& poisson_part,
							     Teuchos::RCP<Epetra_MultiVector>& schrodinger_part) const
{
  double* data;
  int disc_nMyElements = disc_map->NumMyElements();

  if(combinedVector->ExtractView(&data) != 0)
    TEUCHOS_TEST_FOR_EXCEPTION(true, Teuchos::Exceptions::InvalidParameter,
			       "Error!  QCADT::CoupledPoissonSchrodinger cannot extract vector views");

  poisson_part = Teuchos::rcp(new Epetra_Vector(::View, *disc_map, &data[0]));
  schrodinger_part = Teuchos::rcp(new Epetra_MultiVector(::View, *disc_map, &data[disc_nMyElements], disc_nMyElements, nEigenvals));
}


void QCADT::CoupledPoissonSchrodinger::separateCombinedVector(const Teuchos::RCP<Epetra_Vector>& combinedVector,
							     Teuchos::RCP<Epetra_Vector>& poisson_part,
							     Teuchos::RCP<Epetra_MultiVector>& schrodinger_part,
							     Teuchos::RCP<Epetra_Vector>& eigenvalue_part) const
{
  this->separateCombinedVector(combinedVector, poisson_part, schrodinger_part);

  double* data;
  int disc_nMyElements = disc_map->NumMyElements();
  int my_nEigenvals = combined_SP_map->NumMyElements() - disc_nMyElements * (1+nEigenvals);
  Epetra_Map dist_eigenval_map(nEigenvals, my_nEigenvals, 0, *myComm);

  combinedVector->ExtractView(&data); //above call tests for failure
  eigenvalue_part = Teuchos::rcp(new Epetra_Vector(::View, dist_eigenval_map, &data[(1+nEigenvals)*disc_nMyElements]));
}



void QCADT::CoupledPoissonSchrodinger::separateCombinedVector(const Teuchos::RCP<const Epetra_Vector>& combinedVector,
							     Teuchos::RCP<const Epetra_Vector>& poisson_part,
							     Teuchos::RCP<const Epetra_MultiVector>& schrodinger_part) const
{
  double* data;
  int disc_nMyElements = disc_map->NumMyElements();

  if(combinedVector->ExtractView(&data) != 0)
    TEUCHOS_TEST_FOR_EXCEPTION(true, Teuchos::Exceptions::InvalidParameter,
			       "Error!  QCADT::CoupledPoissonSchrodinger cannot extract vector views");

  poisson_part = Teuchos::rcp(new const Epetra_Vector(::View, *disc_map, &data[0]));
  schrodinger_part = Teuchos::rcp(new const Epetra_MultiVector(::View, *disc_map, &data[disc_nMyElements], disc_nMyElements, nEigenvals));
}


void QCADT::CoupledPoissonSchrodinger::separateCombinedVector(const Teuchos::RCP<const Epetra_Vector>& combinedVector,
							     Teuchos::RCP<const Epetra_Vector>& poisson_part,
							     Teuchos::RCP<const Epetra_MultiVector>& schrodinger_part,
							     Teuchos::RCP<const Epetra_Vector>& eigenvalue_part) const
{
  this->separateCombinedVector(combinedVector, poisson_part, schrodinger_part);

  double* data;
  int disc_nMyElements = disc_map->NumMyElements();
  int my_nEigenvals = combined_SP_map->NumMyElements() - disc_nMyElements * (1+nEigenvals);
  Epetra_Map dist_eigenval_map(nEigenvals, my_nEigenvals, 0, *myComm);

  combinedVector->ExtractView(&data); //above call tests for failure
  eigenvalue_part = Teuchos::rcp(new const Epetra_Vector(::View, dist_eigenval_map, &data[(1+nEigenvals)*disc_nMyElements]));
}
*/


//Copied from Albany::SolverFactory -- used to validate applicaton parameters of applications not created via a SolverFactory
Teuchos::RCP<const Teuchos::ParameterList>
QCADT::CoupledPoissonSchrodinger::getValidAppParameters() const
{  
  std::cout << "DEBUG: " << __PRETTY_FUNCTION__ << "\n";
  Teuchos::RCP<Teuchos::ParameterList> validPL = Teuchos::rcp(new Teuchos::ParameterList("ValidAppParams"));;
  validPL->sublist("Problem",            false, "Problem sublist");
  validPL->sublist("Debug Output",       false, "Debug Output sublist");
  validPL->sublist("Discretization",     false, "Discretization sublist");
  validPL->sublist("Quadrature",         false, "Quadrature sublist");
  validPL->sublist("Regression Results", false, "Regression Results sublist");
  validPL->sublist("VTK",                false, "DEPRECATED  VTK sublist");
  validPL->sublist("Piro",               false, "Piro sublist");
  validPL->sublist("Coupled System",     false, "Coupled system sublist");

  return validPL;
}

Teuchos::RCP<const Teuchos::ParameterList>
QCADT::CoupledPoissonSchrodinger::getValidProblemParameters() const
{
  std::cout << "DEBUG: " << __PRETTY_FUNCTION__ << "\n";
  Teuchos::RCP<Teuchos::ParameterList> validPL = Teuchos::createParameterList("ValidPoissonSchrodingerProblemParams");

  validPL->set<std::string>("Name", "", "String to designate Problem Class");
  validPL->set<int>("Phalanx Graph Visualization Detail", 0,
                    "Flag to select output of Phalanx Graph and level of detail");

  validPL->set<double>("Length Unit In Meters",1e-6,"Length unit in meters");
  validPL->set<double>("Energy Unit In Electron Volts",1.0,"Energy (voltage) unit in electron volts");
  validPL->set<double>("Temperature",300,"Temperature in Kelvin");
  validPL->set<std::string>("MaterialDB Filename","materials.xml","Filename of material database xml file");
  validPL->set<int>("Number of Eigenvalues",0,"The number of eigenvalue-eigenvector pairs");
  validPL->set<bool>("Verbose Output",false,"Enable detailed output mode");

  validPL->set<bool>("Include exchange-correlation potential",false,"Include exchange-correlation potential in poisson source term");
  validPL->set<bool>("Only solve schrodinger in quantum blocks",true,"Limit schrodinger solution to elements blocks labeled as quantum in the materials DB");

  validPL->sublist("Poisson Problem", false, "");
  validPL->sublist("Schrodinger Problem", false, "");

  // Candidates for deprecation. Pertain to the solution rather than the problem definition.
  validPL->set<std::string>("Solution Method", "Steady", "Flag for Steady, Transient, or Continuation");
  
  return validPL;
}


//This function is used solely for Jacobian debugging
/*void QCADT::CoupledPoissonSchrodinger::computeResidual(const Teuchos::RCP<const Epetra_Vector>& x,
						      Teuchos::RCP<Epetra_Vector>& f,
						      Teuchos::RCP<Epetra_CrsMatrix>& massMx) const
{
  double curr_time = 0.0;
  Epetra_Vector M_vec(*disc_map);  //temp storage for mass matrix times vec -- maybe don't allocate this on the stack??
  Epetra_LocalMap local_eigenval_map(nEigenvals, 0, *myComm);

  int disc_nMyElements = disc_map->NumMyElements();
  int my_nEigenvals = combined_SP_map->NumMyElements() - disc_nMyElements * (1+nEigenvals);

  Teuchos::RCP<const Epetra_Vector> x_poisson, eigenvals_dist;
  Teuchos::RCP<const Epetra_MultiVector> x_schrodinger;
  separateCombinedVector(x, x_poisson, x_schrodinger, eigenvals_dist);

  Teuchos::RCP<Epetra_Vector>   f_poisson, f_norm_local, f_norm_dist;
  Teuchos::RCP<Epetra_MultiVector> f_schrodinger;
  std::vector<Epetra_Vector*> f_schrodinger_vec(nEigenvals);
  separateCombinedVector(f, f_poisson, f_schrodinger, f_norm_dist);
  for(int i=0; i<nEigenvals; i++) f_schrodinger_vec[i] = (*f_schrodinger)(i);
  f_norm_local = Teuchos::rcp(new Epetra_Vector(local_eigenval_map));


  //update schrodinger wavefunctions for poisson

  Epetra_Import eigenval_importer(local_eigenval_map, eigenvals_dist->Map() );
  Teuchos::RCP<Epetra_Vector> eigenvals =  Teuchos::rcp(new Epetra_Vector(local_eigenval_map));
  eigenvals->Import(*eigenvals_dist, eigenval_importer, Insert);
  Teuchos::RCP<std::vector<double> > stdvec_eigenvals = Teuchos::rcp(new std::vector<double>(&(*eigenvals)[0], &(*eigenvals)[0] + nEigenvals));

  Teuchos::RCP<Albany::EigendataStruct> eigenData = Teuchos::rcp( new Albany::EigendataStruct );
  eigenData->eigenvalueRe = stdvec_eigenvals;
  eigenData->eigenvectorRe = Teuchos::rcp(new Epetra_MultiVector(*disc_overlap_map, nEigenvals));
  eigenData->eigenvectorIm = Teuchos::null;
  Teuchos::RCP<Epetra_Import> overlap_importer = Teuchos::rcp(new Epetra_Import(*disc_overlap_map, *disc_map));

  for(int i=0; i<nEigenvals; i++)
    (*(eigenData->eigenvectorRe))(i)->Import( *((*x_schrodinger)(i)), *overlap_importer, Insert );

    // set eigenvalues / eigenvectors for use in poisson problem:
  poissonApp->getStateMgr().setEigenData(eigenData);
  poissonApp->computeGlobalResidual(curr_time, NULL, *x_poisson, 
				    poisson_sacado_param_vec, *f_poisson);
      

    // Get overlapped version of potential (x_poisson) for passing as auxData to schrodinger app
  Teuchos::RCP<Epetra_MultiVector> overlapped_V = Teuchos::rcp(new Epetra_MultiVector(*disc_overlap_map, 1));
  Teuchos::RCP<Epetra_Vector> ones_vec = Teuchos::rcp(new Epetra_Vector(*disc_overlap_map));
  ones_vec->PutScalar(1.0);
  (*overlapped_V)(0)->Import( *x_poisson, *overlap_importer, Insert );
  (*overlapped_V)(0)->Update(offset_to_CB, *ones_vec, -1.0);
  schrodingerApp->getStateMgr().setAuxData(overlapped_V);

    // compute schrodinger Hamiltonian
  Teuchos::RCP<Epetra_Operator> hamMx = schrodingerModel->create_W(); //maybe re-use this and not create it every time?
  Teuchos::RCP<Epetra_CrsMatrix> hamMx_crs = Teuchos::rcp_dynamic_cast<Epetra_CrsMatrix>(hamMx, true);
  Teuchos::RCP<const Epetra_Vector> dummy_xdot = schrodingerModel->get_x_dot_init();
  schrodingerApp->computeGlobalJacobian(0.0, 1.0, 0.0, curr_time, dummy_xdot.get(), NULL, *((*x_schrodinger)(0)), 
					schrodinger_sacado_param_vec, f_schrodinger_vec[0], *hamMx_crs);

  for(int i=0; i<nEigenvals; i++) {
    const Epetra_Vector& vec = *((*x_schrodinger)(i));
    massMx->Multiply(false, vec, M_vec);  
    hamMx_crs->Multiply(false, vec, *(f_schrodinger_vec[i]));  
    f_schrodinger_vec[i]->Update( (*stdvec_eigenvals)[i], M_vec, 1.0); 

    // Compute normalization equation residuals:  f_norm[i] = abs(1 - evec[i] . M . evec[i])
    double vec_M_vec;
    vec.Dot( M_vec, &vec_M_vec );
    (*f_norm_local)[i] = 1.0 - vec_M_vec;
  }

  // Fill elements of f_norm_dist that belong to this processor, i.e. loop over
  // eigenvalue indices "owned" by the current proc in the combined distributed map
  std::vector<int> eval_global_elements(my_nEigenvals);
  eigenvals_dist->Map().MyGlobalElements(&eval_global_elements[0]);
  for(int i=0; i<my_nEigenvals; i++)
    (*f_norm_dist)[i] = (*f_norm_local)[eval_global_elements[i]];
}*/
