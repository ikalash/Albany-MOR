//*****************************************************************//
//    Albany 3.0:  Copyright 2016 Sandia Corporation               //
//    This Software is released under the BSD license detailed     //
//    in the file "license.txt" in the top-level Albany directory  //
//*****************************************************************//

#include "Albany_SolutionValuesResponseFunction.hpp"

#include "Albany_SolutionCullingStrategy.hpp"

#include "Teuchos_RCP.hpp"
#include "Teuchos_Array.hpp"

#include "Teuchos_Assert.hpp"

#include <iostream>

#include "Albany_TpetraThyraUtils.hpp"

//amb Hack for John M.'s work. Replace with an evaluator-based RF later so we
// can get gradient, etc.
//   By the way, I could be wrong, but I think NodeGIDsSolutionCullingStrategy
// isn't doing the right thing when the solution is other than scalar. But we're
// just whipping this together quickly for a demo, so I'll leave that issue (if
// indeed it is one) for later.
class Albany::SolutionValuesResponseFunction::SolutionPrinter {
private:
  std::string filename_;
  // No need for a separate RCP.
  const Teuchos::RCP<const Application>& app_;

public:
  SolutionPrinter (const Teuchos::RCP<const Application>& app,
                   Teuchos::ParameterList& response_parms)
    : app_(app)
  {
    filename_ = response_parms.get<std::string>("Output File");
  }

  static Teuchos::RCP<SolutionPrinter> create (
    const Teuchos::RCP<const Application>& app,
    Teuchos::ParameterList& response_parms)
  {
    if (response_parms.isType<std::string>("Output File"))
      return Teuchos::rcp(new SolutionPrinter(app, response_parms));
    return Teuchos::null;
  }

  void print (const Tpetra_Vector& g, const Teuchos::Array<Tpetra_GO>& eq_gids) {
    print(Teuchos::arrayView(&g.get1dView()[0], g.getLocalLength()), eq_gids);
  }

private:
  struct Point { ST p[3]; };

  template<typename gid_type>
  void print (const Teuchos::ArrayView<const ST>& g,
              const Teuchos::Array<gid_type>& eq_gids) {
    TEUCHOS_TEST_FOR_EXCEPTION(g.size() != eq_gids.size(), std::logic_error,
                               "g.size() != eq_gids.size()");

    // Get all coordinates.
    std::vector<GO> node_gids;
    std::vector<Point> coords;
    int ndim;
    std::vector<std::size_t> idxs;
    getCoordsOnRank(eq_gids, node_gids, coords, ndim, idxs);

    std::string filename; {
      std::stringstream ss;
      ss << filename_ << "." << app_->getMapT()->getComm()->getRank();
      filename = ss.str();
    }

    std::fstream out;
    out.open(filename.c_str(), std::fstream::out);
    out.precision(15);
    for (std::size_t i = 0; i < node_gids.size(); ++i) {
      // Generally the node gid for the user starts at 1.
      out << std::setw(11) << node_gids[i] + 1;
      out << std::scientific;
      for (int j = 0; j < ndim; ++j)
        out << " " << std::setw(22) << coords[i].p[j];
      out << " " << std::setw(22) << g[idxs[i]] << std::endl;
    }
    out.close();
  }

  // gids is global equation ids.
  template<typename gid_type>
  void getCoordsOnRank (
    const Teuchos::Array<gid_type>& eq_gids, std::vector<GO>& node_gids,
    std::vector<Point>& coords, int& ndim, std::vector<std::size_t>& idxs)
  {
    Teuchos::RCP<Albany::AbstractDiscretization> d = app_->getDiscretization();
    const Teuchos::ArrayRCP<double>& ol_coords = d->getCoordinates();
    Teuchos::RCP<const Tpetra_Map>
      ol_node_map = d->getOverlapNodeMapT(),
      node_map = d->getNodeMapT();
    ndim = d->getNumDim();
    const int neq = d->getNumEq();
    for (std::size_t i = 0; i < eq_gids.size(); ++i) {
      const GO node_gid = eq_gids[i] / neq;
      if ( ! node_map->isNodeGlobalElement(node_gid)) continue;
      idxs.push_back(i);
      node_gids.push_back(node_gid);
      const LO ol_node_lid = ol_node_map->getLocalElement(node_gid);
      coords.push_back(Point());
      for (int j = 0; j < ndim; ++j) {
        // 3 is used regardless of ndim.
        coords.back().p[j] = ol_coords[3*ol_node_lid + j];
      }
    }
  }
};

Albany::SolutionValuesResponseFunction::
SolutionValuesResponseFunction(const Teuchos::RCP<const Application>& app,
                               Teuchos::ParameterList& responseParams) :
  SamplingBasedScalarResponseFunction(app->getComm()),
  app_(app),
  cullingStrategy_(createSolutionCullingStrategy(app, responseParams))
{
  sol_printer_ = SolutionPrinter::create(app_, responseParams);
}

void
Albany::SolutionValuesResponseFunction::
setup()
{
  cullingStrategy_->setupT();
  this->updateSolutionImporterT();
}

unsigned int
Albany::SolutionValuesResponseFunction::
numResponses() const
{
  if (Teuchos::nonnull(solutionImporterT_))
    return solutionImporterT_->getTargetMap()->getNodeNumElements();
  return 0u;
}

void
Albany::SolutionValuesResponseFunction::
evaluateResponse(const double /*current_time*/,
    const Teuchos::RCP<const Thyra_Vector>& x,
    const Teuchos::RCP<const Thyra_Vector>& /*xdot*/,
    const Teuchos::RCP<const Thyra_Vector>& /*xdotdot*/,
		const Teuchos::Array<ParamVec>& /*p*/,
		Tpetra_Vector& gT)
{
  // Until you find out how to do this with Thyra, cast x to Tpetra
  auto xT = Albany::getConstTpetraVector(x);
  this->updateSolutionImporterT();
  this->ImportWithAlternateMapT(solutionImporterT_, *xT, gT, Tpetra::INSERT);
  if (Teuchos::nonnull(sol_printer_))
    sol_printer_->print(gT, cullingStrategy_->selectedGIDsT(app_->getMapT()));
}

void
Albany::SolutionValuesResponseFunction::
evaluateTangent(const double /*alpha*/,
		const double beta,
		const double omega,
		const double /*current_time*/,
		bool /*sum_derivs*/,
    const Teuchos::RCP<const Thyra_Vector>& x,
    const Teuchos::RCP<const Thyra_Vector>& /*xdot*/,
    const Teuchos::RCP<const Thyra_Vector>& /*xdotdot*/,
		const Teuchos::Array<ParamVec>& /*p*/,
		ParamVec* /*deriv_p*/,
    const Teuchos::RCP<const Thyra_MultiVector>& Vx,
    const Teuchos::RCP<const Thyra_MultiVector>& /*Vxdot*/,
    const Teuchos::RCP<const Thyra_MultiVector>& /*Vxdotdot*/,
    const Teuchos::RCP<const Thyra_MultiVector>& /*Vp*/,
		Tpetra_Vector* gT,
		Tpetra_MultiVector* gxT,
		Tpetra_MultiVector* gpT)
{
  this->updateSolutionImporterT();

  if (gT) {
    // Until you find out how to do stuff with Thyra, cast x to Tpetra
    auto xT = Albany::getConstTpetraVector(x);

    this->ImportWithAlternateMapT(solutionImporterT_, *xT, *gT, Tpetra::INSERT);
    if (Teuchos::nonnull(sol_printer_))
      sol_printer_->print(*gT, cullingStrategy_->selectedGIDsT(app_->getMapT()));
  }

  if (gxT) {
    // Until you find out how to do stuff with Thyra, cast Vx to Tpetra
    auto VxT = Albany::getConstTpetraMultiVector(Vx);
    TEUCHOS_TEST_FOR_EXCEPT(VxT.is_null());
    this->ImportWithAlternateMapT(solutionImporterT_, *VxT, gxT, Tpetra::INSERT);
    if (beta != 1.0) {
      gxT->scale(beta);
    }
  }

  if (gpT) {
    gpT->putScalar(0.0);
  }
}

//! Evaluate distributed parameter derivative dg/dp
void
Albany::SolutionValuesResponseFunction::
evaluateDistParamDeriv(
    const double /*current_time*/,
    const Teuchos::RCP<const Thyra_Vector>& /*x*/,
    const Teuchos::RCP<const Thyra_Vector>& /*xdot*/,
    const Teuchos::RCP<const Thyra_Vector>& /*xdotdot*/,
    const Teuchos::Array<ParamVec>& /*param_array*/,
    const std::string& /*dist_param_name*/,
    Tpetra_MultiVector* dg_dpT)
{
  if (dg_dpT) {
    dg_dpT->putScalar(0.0);
  }
}

void
Albany::SolutionValuesResponseFunction::
evaluateGradient(const double /*current_time*/,
    const Teuchos::RCP<const Thyra_Vector>& x,
    const Teuchos::RCP<const Thyra_Vector>& /*xdot*/,
    const Teuchos::RCP<const Thyra_Vector>& /*xdotdot*/,
		const Teuchos::Array<ParamVec>& /*p*/,
		ParamVec* /*deriv_p*/,
		Tpetra_Vector* gT,
		Tpetra_MultiVector* dg_dxT,
		Tpetra_MultiVector* dg_dxdotT,
		Tpetra_MultiVector* dg_dxdotdotT,
		Tpetra_MultiVector* dg_dpT)
{
  this->updateSolutionImporterT();

  if (gT) {
    // Until you find out how to do stuff with Thyra, cast x to Tpetra
    auto xT = Albany::getConstTpetraVector(x);
    this->ImportWithAlternateMapT(solutionImporterT_, *xT, *gT, Tpetra::INSERT);
    if (Teuchos::nonnull(sol_printer_))
      sol_printer_->print(*gT, cullingStrategy_->selectedGIDsT(app_->getMapT()));
  }

  if (dg_dxT) {
    dg_dxT->putScalar(0.0);

    Teuchos::RCP<const Tpetra_Map> replicatedMapT = solutionImporterT_->getTargetMap();
    Teuchos::RCP<const Tpetra_Map> derivMapT = dg_dxT->getMap();
    const int colCount = dg_dxT->getNumVectors();
    for (int icol = 0; icol < colCount; ++icol) {
      const int lid = derivMapT->getLocalElement(replicatedMapT->getGlobalElement(icol));
      if (lid != -1) {
        dg_dxT->replaceLocalValue(lid, icol, 1.0);
      }
    }
  }

  if (dg_dxdotT) {
    dg_dxdotT->putScalar(0.0);
  }
  if (dg_dxdotdotT) {
    dg_dxdotdotT->putScalar(0.0);
  }

  if (dg_dpT) {
    dg_dpT->putScalar(0.0);
  }
}

void
Albany::SolutionValuesResponseFunction::
updateSolutionImporterT()
{
  const Teuchos::RCP<const Tpetra_Map> solutionMapT = app_->getMapT();
  if (Teuchos::is_null(solutionImporterT_) || !solutionMapT->isSameAs(*solutionImporterT_->getSourceMap())) {
    const Teuchos::Array<Tpetra_GO> selectedGIDsT = cullingStrategy_->selectedGIDsT(solutionMapT);
    Teuchos::RCP<const Tpetra_Map> targetMapT = Tpetra::createNonContigMapWithNode<LO, Tpetra_GO, KokkosNode> (selectedGIDsT, solutionMapT->getComm(), solutionMapT->getNode());
    //const Epetra_Map targetMap(-1, selectedGIDs.size(), selectedGIDs.getRawPtr(), 0, solutionMap->Comm());
    solutionImporterT_ = Teuchos::rcp(new Tpetra_Import(solutionMapT, targetMapT));
  }
}

void
Albany::SolutionValuesResponseFunction::
ImportWithAlternateMapT(
    Teuchos::RCP<const Tpetra_Import> importerT,
    const Tpetra_MultiVector& sourceT,
    Tpetra_MultiVector* targetT,
    Tpetra::CombineMode modeT)
{
  Teuchos::RCP<const Tpetra_Map> savedMapT = targetT->getMap();
  targetT->replaceMap(importerT->getTargetMap());
  targetT->doImport(sourceT, *importerT, modeT);
  targetT->replaceMap(savedMapT);
}

void
Albany::SolutionValuesResponseFunction::
ImportWithAlternateMapT(
    Teuchos::RCP<const Tpetra_Import> importerT,
    const Tpetra_Vector& sourceT,
    Tpetra_Vector& targetT,
    Tpetra::CombineMode modeT)
{
  Teuchos::RCP<const Tpetra_Map> savedMapT = targetT.getMap();
  targetT.replaceMap(importerT->getTargetMap());
  targetT.doImport(sourceT, *importerT, modeT);
  targetT.replaceMap(savedMapT);
}
