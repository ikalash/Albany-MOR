//*****************************************************************//
//    Albany 3.0:  Copyright 2016 Sandia Corporation               //
//    This Software is released under the BSD license detailed     //
//    in the file "license.txt" in the top-level Albany directory  //
//*****************************************************************//

#include "Albany_DataTypes.hpp"
#include "Albany_Utils.hpp"

#include "Albany_DiscretizationFactory.hpp"
#include "Albany_STKDiscretization.hpp"

#include "Albany_MORDiscretizationUtils.hpp"
#include "Albany_StkEpetraMVSource.hpp"

#include "MOR_EpetraMVSource.hpp"
#include "MOR_ConcatenatedEpetraMVSource.hpp"
#include "MOR_SnapshotPreprocessor.hpp"
#include "MOR_SnapshotPreprocessorFactory.hpp"
#include "MOR_SnapshotBlockingUtils.hpp"
#include "MOR_SingularValuesHelpers.hpp"

#include "RBGen_EpetraMVMethodFactory.h"
#include "RBGen_PODMethod.hpp"

#include "Epetra_Comm.h"
#include "Epetra_MultiVector.h"
#include "Epetra_Import.h"

#include "Teuchos_Ptr.hpp"
#include "Teuchos_RCP.hpp"
#include "Teuchos_ArrayRCP.hpp"
#include "Teuchos_Array.hpp"
#include "Teuchos_GlobalMPISession.hpp"
#include "Teuchos_Comm.hpp"
#include "Teuchos_FancyOStream.hpp"
#include "Teuchos_VerboseObject.hpp"
#include "Teuchos_ParameterList.hpp"
#include "Teuchos_XMLParameterListHelpers.hpp"

//#include "EpetraExt_MultiVectorOut.h"

#include <string>
#include <limits>

//Control whether we reduce DOFs by removing DBC DOFs or not
#define INTERNAL_DOFS

Teuchos::Array<int> getMyBlockLIDs(
    std::string nodeSetLabel,
	int dofRank,
    const Albany::AbstractDiscretization &disc)
{
  Teuchos::Array<int> result;

  const Albany::NodeSetList &nodeSets = disc.getNodeSets();
  const Albany::NodeSetList::const_iterator it = nodeSets.find(nodeSetLabel);
  TEUCHOS_TEST_FOR_EXCEPT(it == nodeSets.end());
  {
    typedef Albany::NodeSetList::mapped_type NodeSetEntryList;
    const NodeSetEntryList &nodeEntries = it->second;

    for (NodeSetEntryList::const_iterator jt = nodeEntries.begin(); jt != nodeEntries.end(); ++jt) {
      typedef NodeSetEntryList::value_type NodeEntryList;
      const NodeEntryList &entries = *jt;
      result.push_back(entries[dofRank]);
    }
  }

  return result;
}

std::vector<std::string> split(const char *str, char c = ' ')
{
  std::vector<std::string> result;
  do
  {
    const char *begin = str;
    while(*str != c && *str)
        str++;
    result.push_back(std::string(begin, str));
  } while (0 != *str++);
  return result;
}

bool extract_DBC_data(Teuchos::RCP<Teuchos::ParameterList> myDBCParams, Teuchos::RCP<Teuchos::ParameterList> myrbgenParams, Teuchos::RCP<Albany::AbstractDiscretization> mybaseDisc, Teuchos::RCP<Epetra_MultiVector>& rawSnapshots, Teuchos::Array<Teuchos::RCP<const Epetra_Vector> >& blockVectors, Teuchos::Array<int>& blockedLIDs)
{
	bool isUnique = myrbgenParams->get<bool>("Unique DBC", false);
    bool isComplete = myrbgenParams->get<bool>("Complete DBC", false);

	for (auto it=myDBCParams->begin(); it!=myDBCParams->end(); it++)
	{
		std::string this_name = myDBCParams->name(it);
		std::vector<std::string> token_name = split(this_name.c_str());

		int offset;
		bool time_varying = token_name[0].compare("Time") == 0 ? offset = 2 : offset = 0;
		//bool sdbc = token_name[offset].compare("SDBC")==0;
		std::string name = token_name[offset+3];
		std::string DOF = token_name[offset+6];
		int dof=-1;
		if (DOF=="X")
			dof = 0;
		else if (DOF=="Y")
			dof = 1;
		else if (DOF=="Z")
			dof = 2;
		else if (DOF=="T")
			dof = 3;

		Teuchos::Array<int> mySelectedLIDs = getMyBlockLIDs(name, dof, *mybaseDisc);
		std::cout << "DOF " << DOF << " of " << name << " has " << mySelectedLIDs.size() << " selected LIDs: " << mySelectedLIDs << std::endl;

		if (isUnique)
		{
		for (auto it=mySelectedLIDs.begin(); it!=mySelectedLIDs.end(); )
		{
		  if (std::find(blockedLIDs.begin(), blockedLIDs.end(), *it) != blockedLIDs.end())
		  {
			std::cout << "deleting element " << *it << " of mySelectedLIDs" << std::endl;
			mySelectedLIDs.erase(it);
		  }
		  else
			it++;
		}
		}

		for (int j=0; j<mySelectedLIDs.size(); j++)
		{
		  blockedLIDs.push_back(mySelectedLIDs[j]);
		}

		if (!isComplete)
		{
		  blockVectors.push_back(MOR::isolateUniformBlock(mySelectedLIDs, *rawSnapshots));
		}
	}
	return isComplete;
}

int run_rb_gen(Teuchos::RCP<const Teuchos::Comm<int>> teuchosComm, const std::string& inputFileName) {
  using Teuchos::RCP;

  // Standard output
  const RCP<Teuchos::FancyOStream> out = Teuchos::VerboseObjectBase::getDefaultOStream();

  // Parse XML input file
  const RCP<Teuchos::ParameterList> topLevelParams = Teuchos::createParameterList("Albany Parameters");
  Teuchos::updateParametersFromXmlFileAndBroadcast(inputFileName, topLevelParams.ptr(), *teuchosComm);
  topLevelParams->print();

  // Create base discretization, used to retrieve the snapshot map and output the basis
  const Teuchos::RCP<Teuchos::ParameterList> baseTopLevelParams(new Teuchos::ParameterList(*topLevelParams));
  const RCP<Albany::AbstractDiscretization> baseDisc = Albany::discretizationNew(baseTopLevelParams, teuchosComm);

  const RCP<Teuchos::ParameterList> rbgenParams =
    Teuchos::sublist(topLevelParams, "Reduced Basis", /*sublistMustExist =*/ true);

  const RCP<Teuchos::ParameterList> discParams =
		  Teuchos::sublist(topLevelParams, "Discretization", /*sublistMustExist =*/ true);

  const RCP<Teuchos::ParameterList> probParams =
		  Teuchos::sublist(topLevelParams, "Problem", /*sublistMustExist =*/ true);

  const RCP<Teuchos::ParameterList> DBCParams =
		  Teuchos::sublist(probParams, "Dirichlet BCs", /*sublistMustExist =*/ true);

  typedef Teuchos::Array<std::string> FileNameList;
  FileNameList snapshotFiles;
  {
    const RCP<Teuchos::ParameterList> snapshotSourceParams = Teuchos::sublist(rbgenParams, "Snapshot Sources");
    snapshotFiles = snapshotSourceParams->get("File Names", snapshotFiles);
  }

  typedef Teuchos::Array<RCP<Albany::STKDiscretization> > DiscretizationList;
  DiscretizationList discretizations;
  RCP<Albany::STKDiscretization> baseSTKDisc = Teuchos::rcp_dynamic_cast<Albany::STKDiscretization>(baseDisc, /*throw_on_fail =*/ true);
  if (snapshotFiles.empty()) {
    discretizations.push_back(baseSTKDisc);
  } else {
    discretizations.reserve(snapshotFiles.size());
    for (FileNameList::const_iterator it = snapshotFiles.begin(), it_end = snapshotFiles.end(); it != it_end; ++it) {
      const Teuchos::RCP<Teuchos::ParameterList> localTopLevelParams(new Teuchos::ParameterList(*topLevelParams));
      {
        // Replace discretization parameters to read snapshot file
        Teuchos::ParameterList localDiscParams;
        localDiscParams.set("Method", "Ioss");
        localDiscParams.set("Exodus Input File Name", *it);
        localDiscParams.setParametersNotAlreadySet(*discParams);
        localTopLevelParams->set("Discretization", localDiscParams);
      }
      const RCP<Albany::AbstractDiscretization> disc = Albany::discretizationNew(localTopLevelParams, teuchosComm);
      discretizations.push_back(Teuchos::rcp_dynamic_cast<Albany::STKDiscretization>(disc, /*throw_on_fail =*/ true));
    }
  }

  typedef Teuchos::Array<RCP<MOR::EpetraMVSource> > SnapshotSourceList;
  SnapshotSourceList snapshotSources;
  for (DiscretizationList::const_iterator it = discretizations.begin(), it_end = discretizations.end(); it != it_end; ++it) {
    snapshotSources.push_back(Teuchos::rcp(new Albany::StkEpetraMVSource(*it)));
  }

  MOR::ConcatenatedEpetraMVSource snapshotSource(*baseDisc->getMap(), snapshotSources);
  *out << "Total snapshot count = " << snapshotSource.vectorCount() << "\n";
  Teuchos::RCP<Epetra_MultiVector> rawSnapshots = snapshotSource.multiVectorNew();


  Epetra_BlockMap map_all = rawSnapshots->Map();
  int mpi_rank, mpi_size;
  mpi_size = map_all.Comm().NumProc();
  mpi_rank = map_all.Comm().MyPID();
  printf("Processor %d out of %d\n",mpi_rank,mpi_size);
  //map_all.Print(std::cout);

  // Isolate Dirichlet BC
  Teuchos::Array<RCP<const Epetra_Vector> > blockVectors;
  Teuchos::Array<RCP<const Epetra_Vector> > completeBlockVectors;
  Teuchos::Array<int>  myBlockedLIDs;
  // I suppose you're grepping for "Blocking" or "Blocking List"...
  // Those are depreciated options in your RBGen input where you would
  // Explicitly define which DBCs you wanted to block off.
  // We now just pull that data from the "Dirichlet BCs" section, which
  // is IMHO a much better solution - why would you want to write your  
  // entire set of DBCs twice, and in a longer and more complicated format?
  // Either way, if for some reason you did still want to block off only a 
  // few of your defined DBCs, just delete all of the DBCs you don't want to 
  // block from your "Dirichlet BCs" section and keep the rest - we don't 
  // use that section for anything else, and the ROM input has its own 
  // DBC section.
  // If you're working with an old input file it should be fine - whatever 
  // you have defined in those sections will just be ignored.
  bool isComplete = extract_DBC_data(DBCParams, rbgenParams, baseDisc, rawSnapshots, blockVectors, myBlockedLIDs);

  if (!isComplete)
  {
    printf("blockVectors has %d entries\n",blockVectors.size());
  }
  int num_blocked_LIDs = 0;
  num_blocked_LIDs = myBlockedLIDs.size();
  printf("There are %d total Blocked LIDs on processor %d (unsorted, contains duplicate entries)\n", num_blocked_LIDs, mpi_rank);
  *out << "Blocked LIDs = " << myBlockedLIDs << "\n";

  int num_local_DOFs = 0;
  num_local_DOFs = rawSnapshots->MyLength();
  printf("There are %d total local DOFs on processor %d\n", num_local_DOFs, mpi_rank);

  Teuchos::Array<int>  myInternalLIDs;
  Teuchos::Array<int>  myBlockedLIDs_sorted;
  int currentGID = 0;
  int blockedGID = 0;
  for (int i=0; i<num_local_DOFs; i++)
  {
    currentGID = map_all.GID(i);
    bool found = false;
    for (int j=0; j<num_blocked_LIDs; j++)
    {
      blockedGID = map_all.GID(myBlockedLIDs[j]);
      //printf("i = %d, j = %d, myBlockedLIDs[j] = %d\n", i, j, myBlockedLIDs[j]);
      //if (i == myBlockedLIDs[j])
      if (currentGID == blockedGID)
      {
        found = true;
        break;
      }
    }
    if (rbgenParams->isSublist("Blocking"))
    {
      blockedGID = map_all.GID(myBlockedLIDs[0]);
      if (currentGID == blockedGID)
        found = true;
    }
    if (found == false)
      myInternalLIDs.push_back(i);
    else
      myBlockedLIDs_sorted.push_back(i);
  }
  printf(" num_local_DOFs - num_blocked_LIDs = %d on processor %d (includes duplicate entries)\n", num_local_DOFs - num_blocked_LIDs, mpi_rank);
  int num_internal_LIDs = 0;
  num_internal_LIDs = myInternalLIDs.size();
  printf("There are %d total Internal LIDs on processor %d\n", num_internal_LIDs, mpi_rank);
  printf("There are %d total Internal LIDs on processor %d\n", myInternalLIDs.size(), mpi_rank);
  *out << "Internal LIDs = " << myInternalLIDs << "\n";

  printf("There are %d total unique Blocked LIDs on processor %d\n", myBlockedLIDs_sorted.size(), mpi_rank);
  *out << "Blocked LIDs = " << myBlockedLIDs_sorted << "\n";
  TEUCHOS_ASSERT(num_local_DOFs == (myInternalLIDs.size() + myBlockedLIDs_sorted.size()));


  int num_DBC = myBlockedLIDs_sorted.size();
  if (isComplete)
  {
    std::cout << "Complete option chosen... " << num_DBC << " total number of DBC dofs" << std::endl;
    for (int pos=0; pos<num_DBC; pos++ )
    {
      Teuchos::Array<int> singleDBC(1,myBlockedLIDs_sorted[pos]);
      completeBlockVectors.push_back(MOR::isolateUniformBlock(singleDBC, *rawSnapshots));
      //std::cout << "@" << myBlockedLIDs_sorted[pos] << std::endl;
      //completeBlockVectors[pos]->Print(std::cout);
    }
  }

  int* myInternalGIDs = new int[num_internal_LIDs];
  for (int i=0; i<num_internal_LIDs; i++)
  {
    //printf("processor %d, i = %d, internal LID = %d, internal GID = %d\n", mpi_rank, i, myInternalLIDs[i], map_all.GID(myInternalLIDs[i]));
    myInternalGIDs[i] = map_all.GID(myInternalLIDs[i]);
  }

  int total_internal_IDs = 0;
  map_all.Comm().SumAll(&num_internal_LIDs, &total_internal_IDs, 1);
  printf("processor %d has %d internal LIDs out of %d total\n", mpi_rank, num_internal_LIDs, total_internal_IDs);

#ifdef INTERNAL_DOFS
  Epetra_BlockMap map_internal(total_internal_IDs, num_internal_LIDs, myInternalGIDs, 1, 0, map_all.Comm());
  delete[] myInternalGIDs;
  //map_internal.Print(std::cout);

  Epetra_Import import_all2internal(map_internal, map_all);
  Epetra_Import import_internal2all(map_all, map_internal);
  //import_all2internal.Print(std::cout);
  //import_internal2all.Print(std::cout);

  Teuchos::RCP<Epetra_MultiVector> rawSnapshots_internal =  Teuchos::rcp(new Epetra_MultiVector(map_internal, rawSnapshots->NumVectors(), true));
  rawSnapshots_internal->Import(*rawSnapshots, import_all2internal, Insert);
  //rawSnapshots_internal->Print(std::cout);

#endif //INTERNAL_DOFS

  // Preprocess raw snapshots
  const RCP<Teuchos::ParameterList> preprocessingParams = Teuchos::sublist(rbgenParams, "Snapshot Preprocessing");

  MOR::SnapshotPreprocessorFactory preprocessorFactory;
  const Teuchos::RCP<MOR::SnapshotPreprocessor> snapshotPreprocessor = preprocessorFactory.instanceNew(preprocessingParams);
#ifdef INTERNAL_DOFS
  snapshotPreprocessor->rawSnapshotSetIs(rawSnapshots_internal);
#else
  snapshotPreprocessor->rawSnapshotSetIs(rawSnapshots);
#endif //INTERNAL_DOFS
  const RCP<const Epetra_MultiVector> modifiedSnapshots = snapshotPreprocessor->modifiedSnapshotSet();

#ifdef INTERNAL_DOFS
  const RCP<const Epetra_Vector> origin_internal = snapshotPreprocessor->origin();
  const bool nonzeroOrigin = Teuchos::nonnull(origin_internal);

  Teuchos::RCP<Epetra_Vector> origin = Teuchos::null;
  if (Teuchos::nonnull(origin_internal))
  {
    origin = Teuchos::rcp(new Epetra_Vector(map_all, true));
    origin->Import(*origin_internal, import_internal2all, Insert);

    //origin_internal->Print(std::cout);
    //origin->Print(std::cout);
  }
#else
  const RCP<const Epetra_Vector> origin = snapshotPreprocessor->origin();
  const bool nonzeroOrigin = Teuchos::nonnull(origin);
#endif //INTERNAL_DOFS

  *out << "After preprocessing, " << modifiedSnapshots->NumVectors() << " snapshot vectors and "
    << static_cast<int>(nonzeroOrigin) << " origin\n";

  // By default, compute as many basis vectors as snapshots
  (void) Teuchos::sublist(rbgenParams, "Reduced Basis Method")->get("Basis Size", modifiedSnapshots->NumVectors());

  // Compute reduced basis
  RBGen::EpetraMVMethodFactory methodFactory;
  const RCP<RBGen::Method<Epetra_MultiVector, Epetra_Operator> > method = methodFactory.create(*rbgenParams);
  method->Initialize(rbgenParams, modifiedSnapshots);
  method->computeBasis();
#ifdef INTERNAL_DOFS
  const RCP<const Epetra_MultiVector> basis_internal = method->getBasis();

  Teuchos::RCP<Epetra_MultiVector> basis = Teuchos::rcp(new Epetra_MultiVector(map_all, basis_internal->NumVectors(), true));
  basis->Import(*basis_internal, import_internal2all, Insert);
#else
  const RCP<const Epetra_MultiVector> basis = method->getBasis();
#endif //INTERNAL_DOFS

  *out << "Computed " << basis->NumVectors() << " left-singular vectors\n";

  // Compute discarded energy fraction for each left-singular vector
  // (relative residual energy corresponding to a basis truncation after current vector)
  const RCP<const RBGen::PODMethod<double> > pod_method = Teuchos::rcp_dynamic_cast<RBGen::PODMethod<double> >(method);
  const Teuchos::Array<double> singularValues = pod_method->getSingularValues();

  *out << "Singular values: " << singularValues << "\n";

  const Teuchos::Array<double> discardedEnergyFractions = MOR::computeDiscardedEnergyFractions(singularValues);

  *out << "Discarded energy fractions: " << discardedEnergyFractions << "\n";

  // Output results
  {
    // Setup overlapping map and vector
    const Epetra_Map outputMap = *baseDisc->getOverlapMap();
    const Epetra_Import outputImport(outputMap, snapshotSource.vectorMap());
    Epetra_Vector outputVector(outputMap, /*zeroOut =*/ false);

    int snapshot0_step = discParams->get("Restart Index", -1);
    if (snapshot0_step != -1)
    {    
      double snapshot0_time = baseSTKDisc->getSTKMeshStruct()->getSolutionFieldHistoryStamp(snapshot0_step-1);
      Epetra_Vector snapshot0(Copy, *baseSTKDisc->getSolutionFieldHistory(snapshot0_step-1, snapshot0_step), 0);
      baseDisc->writeSolution(snapshot0, snapshot0_time, /*zeroOut =*/ true);
    }

    if (nonzeroOrigin) {
      const double stamp = -1.0; // Stamps must be increasing
      outputVector.Import(*origin, outputImport, Insert);
      baseDisc->writeSolution(outputVector, stamp, /*overlapped =*/ true);
    }
    double prev_stamp = -1.0;
    if (isComplete)
    {
      for (int i=0; i<num_DBC; i++)
      {
        const double stamp = -1.0 + (i+1)*std::numeric_limits<double>::epsilon();
        TEUCHOS_ASSERT(stamp != -1.0);
        TEUCHOS_ASSERT(stamp != prev_stamp);
        prev_stamp = stamp;

        outputVector.Import(*completeBlockVectors[i], outputImport, Insert);

        baseDisc->writeSolution(outputVector, stamp, /*overlapped =*/ true);
      }
    }
    else
    {
      for (int i=0; i<blockVectors.size(); i++)
      {
        const double stamp = -1.0 + (i+1)*std::numeric_limits<double>::epsilon();
        TEUCHOS_ASSERT(stamp != -1.0);
        TEUCHOS_ASSERT(stamp != prev_stamp);
        prev_stamp = stamp;
        outputVector.Import(*blockVectors[i], outputImport, Insert);
        baseDisc->writeSolution(outputVector, stamp, /*overlapped =*/ true);
      }
    }
    int num_zeros = 0;
    for (int i = 0; i < basis->NumVectors(); ++i) {
      double stamp = -discardedEnergyFractions[i]; // Stamps must be increasing
      if (stamp == 0)
      {
	stamp = stamp + num_zeros*std::numeric_limits<double>::epsilon();
	num_zeros++;
      }
      const Epetra_Vector vec(View, *basis, i);

      outputVector.Import(vec, outputImport, Insert);

      baseDisc->writeSolution(outputVector, stamp, /*overlapped =*/ true);
    }
  }

  return 0;
}

int main(int argc, char *argv[]) {
  // Communicators
  Teuchos::GlobalMPISession mpiSession(&argc, &argv);
  const Albany_MPI_Comm nativeComm = Albany_MPI_COMM_WORLD;
  const Teuchos::RCP<const Teuchos::Comm<int> > teuchosComm = Albany::createTeuchosCommFromMpiComm(nativeComm);
  const Teuchos::RCP<Teuchos::FancyOStream> out = Teuchos::VerboseObjectBase::getDefaultOStream();

  Kokkos::initialize(argc, argv);

  // Parse command-line argument for input file
  const std::string firstArg = (argc > 1) ? argv[1] : "";
  if (firstArg.empty() || firstArg == "--help") {
    *out << "AlbanyRBGen input-file-path\n";
    Kokkos::finalize_all();
    return 0;
  }

  const std::string inputFileName = argv[1];

  int status = run_rb_gen(teuchosComm,inputFileName);

  Kokkos::finalize_all();

  return status;
}
