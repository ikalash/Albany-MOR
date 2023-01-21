//*****************************************************************//
//    Albany 3.0:  Copyright 2016 Sandia Corporation               //
//    This Software is released under the BSD license detailed     //
//    in the file "license.txt" in the top-level Albany directory  //
//*****************************************************************//
#include <iostream>

#include "Albany_Utils.hpp"
#include "Albany_SolverFactory.hpp"
#include "Piro_PerformAnalysis.hpp"
#include "Teuchos_GlobalMPISession.hpp"
#include "Teuchos_StackedTimer.hpp"
#include "Teuchos_TimeMonitor.hpp"
#include "Teuchos_VerboseObject.hpp"
#include "Teuchos_StandardCatchMacros.hpp"

int main(int argc, char *argv[]) {

  int status=0; // 0 = pass, failures are incremented
  bool success = true;
  Teuchos::GlobalMPISession mpiSession(&argc,&argv);

  Kokkos::initialize(argc, argv);

  Teuchos::RCP<Teuchos::FancyOStream> out(Teuchos::VerboseObjectBase::getDefaultOStream());

  // Command-line argument for input file
  Albany::CmdLineArgs cmd("inputAnalysis.xml");
  cmd.parse_cmdline(argc, argv, *out);

  const auto stackedTimer = Teuchos::rcp(
      new Teuchos::StackedTimer("Albany Stacked Timer"));
  Teuchos::TimeMonitor::setStackedTimer(stackedTimer);

  try {
    Teuchos::RCP<Teuchos::Time> totalTime =
      Teuchos::TimeMonitor::getNewTimer("AlbanyAnalysis: ***Total Time***");
    Teuchos::TimeMonitor totalTimer(*totalTime); //start timer

    using namespace std;

    *out << "\nStarting Albany Analysis via Piro!" << std::endl;

    // Construct a ModelEvaluator for your application;

    Teuchos::RCP<const Teuchos_Comm> comm =
      Tpetra::getDefaultComm();

    // Connect vtune for performance profiling
    if (cmd.vtune) {
      Albany::connect_vtune(comm->getRank());
    }

    Teuchos::RCP<Albany::SolverFactory> slvrfctry =
      Teuchos::rcp(new Albany::SolverFactory(cmd.xml_filename, comm));

    Teuchos::RCP<Thyra::ResponseOnlyModelEvaluatorBase<ST> > appThyra = slvrfctry->createT(comm, comm);


    Teuchos::RCP< Thyra::VectorBase<double> > p;

    // If no analysis section set in input file, default to simple "Solve"
    std::string analysisPackage = slvrfctry->getAnalysisParameters().get("Analysis Package","Solve");
    status = Piro::PerformAnalysis(*appThyra, slvrfctry->getAnalysisParameters(), p); 

//    Dakota::RealVector finalValues = dakota.getFinalSolution().continuous_variables();
//    std::cout << "\nAlbany_Dakota: Final Values from Dakota = "
//         << setprecision(8) << finalValues << std::endl;

    status =  slvrfctry->checkAnalysisTestResults(0, p);

    // Regression comparisons for Dakota runs only valid on Proc 0.
    if (mpiSession.getRank()>0)  status=0;
    else *out << "\nNumber of Failed Comparisons: " << status << std::endl;
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
