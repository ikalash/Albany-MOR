//*****************************************************************//
//    Albany 3.0:  Copyright 2016 Sandia Corporation               //
//    This Software is released under the BSD license detailed     //
//    in the file "license.txt" in the top-level Albany directory  //
//*****************************************************************//

#include <iostream>
#include <string>

#include "Albany_Utils.hpp"
#include "Albany_SolverFactory.hpp"
#include "Albany_NOXObserver.hpp"

#include "Piro_Epetra_StokhosSolver.hpp"
#include "Stokhos_EpetraVectorOrthogPoly.hpp"
#include "Teuchos_VerboseObject.hpp"
#include "Teuchos_StandardCatchMacros.hpp"
#include "Teuchos_StackedTimer.hpp"

#include "Petra_Converters.hpp"

/* GAH FIXME - Silence warning:
TRILINOS_DIR/../../../include/pecos_global_defs.hpp:17:0: warning:
        "BOOST_MATH_PROMOTE_DOUBLE_POLICY" redefined [enabled by default]
Please remove when issue is resolved
*/
#undef BOOST_MATH_PROMOTE_DOUBLE_POLICY

#include "Stokhos.hpp"
#include "Stokhos_Epetra.hpp"

int main(int argc, char *argv[]) {

  int status=0; // 0 = pass, failures are incremented
  bool success = true;
  Teuchos::GlobalMPISession mpiSession(&argc,&argv);

  Kokkos::initialize(argc, argv);

  Teuchos::RCP<Teuchos::FancyOStream> out(Teuchos::VerboseObjectBase::getDefaultOStream());

  // Command-line argument for input file
  Albany::CmdLineArgs cmd("input.xml", "inputSG.xml");
  cmd.parse_cmdline(argc, argv, *out);
  std::string xmlfilename;
  std::string sg_xmlfilename;
  bool do_initial_guess;
  if (cmd.has_second_xml_file) {
    xmlfilename = cmd.xml_filename;
    sg_xmlfilename = cmd.xml_filename2;
    do_initial_guess = true;
  }
  else if (cmd.has_first_xml_file) {
    xmlfilename = "";
    sg_xmlfilename = cmd.xml_filename;
    do_initial_guess = false;
  }
  else {
    xmlfilename = "";
    sg_xmlfilename = "inputSG.xml";
    do_initial_guess = false;
  }

  const auto stackedTimer = Teuchos::rcp(
      new Teuchos::StackedTimer("Albany Stacked Timer"));
  Teuchos::TimeMonitor::setStackedTimer(stackedTimer);

  try {

    Teuchos::RCP<Teuchos::Time> totalTime =
      Teuchos::TimeMonitor::getNewTimer("AlbanySG: ***Total Time***");
    Teuchos::TimeMonitor totalTimer(*totalTime); //start timer

    // Setup communication objects
    Teuchos::RCP<Epetra_Comm> globalComm =
      Albany::createEpetraCommFromMpiComm(Albany_MPI_COMM_WORLD);
    Teuchos::RCP<const Teuchos_Comm> tcomm =
      Tpetra::getDefaultComm();

    // Connect vtune for performance profiling
    if (cmd.vtune) {
      Albany::connect_vtune(tcomm->getRank());
    }

    // Parse parameters
    Albany::SolverFactory sg_slvrfctry(sg_xmlfilename, tcomm);
    Teuchos::ParameterList& albanyParams = sg_slvrfctry.getParameters();
    Teuchos::RCP< Teuchos::ParameterList> piroParams =
      Teuchos::rcp(&(albanyParams.sublist("Piro")),false);

    // Create stochastic Galerkin solver
    Teuchos::RCP<Piro::Epetra::StokhosSolver> sg_solver =
      Teuchos::rcp(new Piro::Epetra::StokhosSolver(piroParams, globalComm));

    // Get comm for spatial problem
    Teuchos::RCP<const Epetra_Comm> app_comm = sg_solver->getSpatialComm();
    Teuchos::RCP<const Teuchos_Comm> tapp_comm = Albany::createTeuchosCommFromEpetraComm(app_comm);

    // Compute initial guess if requested
    Teuchos::RCP<Epetra_Vector> ig;
    if (do_initial_guess) {

      // Create solver
      Albany::SolverFactory slvrfctry(
	xmlfilename, tcomm);
      Teuchos::RCP<EpetraExt::ModelEvaluator> solver =
	slvrfctry.create(tapp_comm, tapp_comm);

      // Setup in/out args
      EpetraExt::ModelEvaluator::InArgs params_in = solver->createInArgs();
      EpetraExt::ModelEvaluator::OutArgs responses_out =
	solver->createOutArgs();
      int np = params_in.Np();
      for (int i=0; i<np; i++) {
	Teuchos::RCP<const Epetra_Vector> p = solver->get_p_init(i);
	params_in.set_p(i, p);
      }
      int ng = responses_out.Ng();
      for (int i=0; i<ng; i++) {
	Teuchos::RCP<Epetra_Vector> g =
	  Teuchos::rcp(new Epetra_Vector(*(solver->get_g_map(i))));
	responses_out.set_g(i, g);
      }

      // Evaluate model
      solver->evalModel(params_in, responses_out);

      // Print responses (not last one since that is x)
      *out << std::endl;
      out->precision(8);
      for (int i=0; i<ng-1; i++) {
	if (responses_out.get_g(i) != Teuchos::null)
	  *out << "Response " << i << " = " << std::endl
	       << *(responses_out.get_g(i)) << std::endl;
      }

      // Get final solution as initial guess
      ig = responses_out.get_g(ng-1);

      Teuchos::TimeMonitor::summarize(std::cout,false,true,false);
    }

    // Create SG solver
    Teuchos::RCP<Albany::Application> app;
    Teuchos::RCP<const Tpetra_Vector> initial_guessT;
    if (Teuchos::nonnull(ig)) {
      initial_guessT = Petra::EpetraVector_To_TpetraVectorConst(*ig, tcomm);
    }
    Teuchos::RCP<EpetraExt::ModelEvaluator> model =
      sg_slvrfctry.createAlbanyAppAndModel(app, tcomm, initial_guessT);

    // Hack in rigid body modes for ML
    {
      Teuchos::RCP<Teuchos::ParameterList> sg_solver_params =
        Teuchos::sublist(Teuchos::sublist(piroParams, "Stochastic Galerkin"), "SG Solver Parameters");
      Teuchos::RCP<Teuchos::ParameterList> sg_prec_params =
        Teuchos::sublist(sg_solver_params, "SG Preconditioner");

      if (sg_prec_params->isParameter("Mean Preconditioner Type")) {
        if (sg_prec_params->get<std::string>("Mean Preconditioner Type") == "ML") {
          Teuchos::RCP<Teuchos::ParameterList> ml_params =
            Teuchos::sublist(sg_prec_params, "Mean Preconditioner Parameters");
          const Teuchos::RCP<Albany::RigidBodyModes>&
            rbm = app->getProblem()->getNullSpace();
          // Previously, updateMLPL called importML, but there was no coordinate
          // data yet. Now we just update the parameter list.
          rbm->updatePL(ml_params);          
          sg_solver->resetSolverParameters(*sg_solver_params);
        }
      }
    }

    // Setup SG solver
    {
      const Teuchos::RCP<NOX::Epetra::Observer > NOX_observer =
        Teuchos::rcp(new Albany_NOXObserver(app));
      sg_solver->setup(model, NOX_observer);
    }

    // Evaluate SG responses at SG parameters
    EpetraExt::ModelEvaluator::InArgs sg_inArgs = sg_solver->createInArgs();
    EpetraExt::ModelEvaluator::OutArgs sg_outArgs =
      sg_solver->createOutArgs();
    int np = sg_inArgs.Np();
    for (int i=0; i<np; i++) {
      if (sg_inArgs.supports(EpetraExt::ModelEvaluator::IN_ARG_p_sg, i)) {
	Teuchos::RCP<const Stokhos::EpetraVectorOrthogPoly> p_sg =
	  sg_solver->get_p_sg_init(i);
	sg_inArgs.set_p_sg(i, p_sg);
      }
    }

    // By default, request the sensitivities if not explicitly disabled
    const bool computeSensitivities =
      sg_slvrfctry.getAnalysisParameters().sublist("Solve").get("Compute Sensitivities", true);
    int ng = sg_outArgs.Ng();
    for (int i=0; i<ng; i++) {
      if (sg_outArgs.supports(EpetraExt::ModelEvaluator::OUT_ARG_g_sg, i)) {
	Teuchos::RCP<Stokhos::EpetraVectorOrthogPoly> g_sg =
	  sg_solver->create_g_sg(i);
	sg_outArgs.set_g_sg(i, g_sg);
      }

      for (int j=0; j<np; j++) {
	EpetraExt::ModelEvaluator::DerivativeSupport ds =
	  sg_outArgs.supports(EpetraExt::ModelEvaluator::OUT_ARG_DgDp_sg,i,j);
	if (computeSensitivities &&
	    ds.supports(EpetraExt::ModelEvaluator::DERIV_MV_BY_COL)) {
	  int ncol = sg_solver->get_p_map(j)->NumMyElements();
	  Teuchos::RCP<Stokhos::EpetraMultiVectorOrthogPoly> dgdp_sg =
	    sg_solver->create_g_mv_sg(i,ncol);
	  sg_outArgs.set_DgDp_sg(i, j, dgdp_sg);
	}
      }
    }

    sg_solver->evalModel(sg_inArgs, sg_outArgs);

    bool printResponse =
      albanyParams.sublist("Problem").get("Print Response Expansion", true);
    for (int i=0; i<ng-1; i++) {
      // Don't loop over last g which is x, since it is a long vector
      // to print out.
      if (sg_outArgs.supports(EpetraExt::ModelEvaluator::OUT_ARG_g_sg, i)) {

	// Print mean and standard deviation
	Teuchos::RCP<Stokhos::EpetraVectorOrthogPoly> g_sg =
	  sg_outArgs.get_g_sg(i);
	if (g_sg != Teuchos::null && app->getResponse(i)->isScalarResponse()) {
	  Epetra_Vector g_mean(*(sg_solver->get_g_map(i)));
	  Epetra_Vector g_std_dev(*(sg_solver->get_g_map(i)));
	  g_sg->computeMean(g_mean);
	  g_sg->computeStandardDeviation(g_std_dev);
	  out->precision(12);
	  out->setf(std::ios::scientific);
	  *out << "Response " << i << " Mean =      " << std::endl
	       << g_mean << std::endl;
	  *out << "Response " << i << " Std. Dev. = " << std::endl
	       << g_std_dev << std::endl;
	  if (printResponse) {
	    *out << "Response " << i << "           = " << std::endl
		 << *g_sg << std::endl;
	    for (int j=0; j<np; j++) {
	      EpetraExt::ModelEvaluator::DerivativeSupport ds =
		sg_outArgs.supports(EpetraExt::ModelEvaluator::OUT_ARG_DgDp_sg,i,j);
	      if (!ds.none()) {
		Teuchos::RCP<Stokhos::EpetraMultiVectorOrthogPoly> dgdp_sg =
		  sg_outArgs.get_DgDp_sg(i,j).getMultiVector();
		if (dgdp_sg != Teuchos::null)
		  *out << "Response " << i << " Derivative " << j << " = "
		       << std::endl << *dgdp_sg << std::endl;
	      }
	    }
	  }

	  status += sg_slvrfctry.checkSGTestResults(i, g_sg, &g_mean, &g_std_dev);
	}
      }
    }
    *out << "\nNumber of Failed Comparisons: " << status << std::endl;

    totalTimer.~TimeMonitor();
  }
  TEUCHOS_STANDARD_CATCH_STATEMENTS(true, std::cerr, success);
  if (!success) status+=10000;
  
  stackedTimer->stop("Albany Stacked Timer");
  Teuchos::StackedTimer::OutputOptions options;
  options.output_fraction = true;
  options.output_minmax = true;
  stackedTimer->report(std::cout, Teuchos::DefaultComm<int>::getComm(), options);

  Kokkos::finalize_all();

  return status;
}
