//*****************************************************************//
//    Albany 3.0:  Copyright 2016 Sandia Corporation               //
//    This Software is released under the BSD license detailed     //
//    in the file "license.txt" in the top-level Albany directory  //
//*****************************************************************//

#ifndef ALBANY_ORDINARYSTKFIELDCONT_HPP
#define ALBANY_ORDINARYSTKFIELDCONT_HPP

#include "Albany_GenericSTKFieldContainer.hpp"

namespace Albany {

template<bool Interleaved>

class OrdinarySTKFieldContainer : public GenericSTKFieldContainer<Interleaved> {

  public:

    OrdinarySTKFieldContainer(const Teuchos::RCP<Teuchos::ParameterList>& params_,
                              const Teuchos::RCP<stk::mesh::MetaData>& metaData_,
                              const Teuchos::RCP<stk::mesh::BulkData>& bulkData_,
                              const int neq_,
                              const AbstractFieldContainer::FieldContainerRequirements& req,
                              const int numDim_,
                              const Teuchos::RCP<Albany::StateInfoStruct>& sis);

    ~OrdinarySTKFieldContainer();

    bool hasResidualField(){ return (residual_field != NULL); }
    bool hasSphereVolumeField(){ return buildSphereVolume; }
    bool hasLatticeOrientationField(){ return false; }

    Teuchos::Array<AbstractSTKFieldContainer::VectorFieldType*> getSolutionFieldArray() {return solution_field; }

    AbstractSTKFieldContainer::VectorFieldType* getSolutionField(){ return solution_field[0]; };
   
#if defined(ALBANY_LCM)  
    AbstractSTKFieldContainer::VectorFieldType* getResidualField(){ return residual_field; };
#endif
#if defined(ALBANY_DTK) 
    Teuchos::Array<AbstractSTKFieldContainer::VectorFieldType*> getSolutionFieldDTKArray(){ return solution_field_dtk; };

    AbstractSTKFieldContainer::VectorFieldType* getSolutionFieldDTK(){ return solution_field_dtk[0]; };
#endif

#if defined(ALBANY_EPETRA)
    void fillSolnVector(Epetra_Vector& soln, stk::mesh::Selector& sel, const Teuchos::RCP<const Epetra_Map>& node_map);
#endif
    void fillSolnVectorT(Tpetra_Vector& solnT, stk::mesh::Selector& sel, const Teuchos::RCP<const Tpetra_Map>& node_mapT);

    void fillSolnMultiVector(Tpetra_MultiVector& solnT, stk::mesh::Selector& sel, const Teuchos::RCP<const Tpetra_Map>& node_mapT);

#if defined(ALBANY_EPETRA)
    void fillVector(Epetra_Vector& field_vector, const std::string& field_name, stk::mesh::Selector& field_selection,
                    const Teuchos::RCP<const Epetra_Map>& field_node_map, const NodalDOFManager& nodalDofManager);

    void saveVector(const Epetra_Vector& field_vector, const std::string& field_name, stk::mesh::Selector& field_selection,
                        const Teuchos::RCP<const Epetra_Map>& field_node_map, const NodalDOFManager& nodalDofManager);

    void saveSolnVector(const Epetra_Vector& soln, stk::mesh::Selector& sel, const Teuchos::RCP<const Epetra_Map>& node_map);
#endif

    //Tpetra version of above
    void fillVectorT(Tpetra_Vector& field_vector, const std::string& field_name, stk::mesh::Selector& field_selection,
                    const Teuchos::RCP<const Tpetra_Map>& field_node_map, const NodalDOFManager& nodalDofManager);

    void saveVectorT(const Tpetra_Vector& field_vector, const std::string& field_name, stk::mesh::Selector& field_selection,
                        const Teuchos::RCP<const Tpetra_Map>& field_node_map, const NodalDOFManager& nodalDofManager);


    void saveSolnVectorT(const Tpetra_Vector& solnT, stk::mesh::Selector& sel, 
                         const Teuchos::RCP<const Tpetra_Map>& node_mapT);
    void saveSolnVectorT(const Tpetra_Vector& solnT, const Tpetra_Vector& soln_dotT,
                         stk::mesh::Selector& sel, const Teuchos::RCP<const Tpetra_Map>& node_mapT);
    void saveSolnVectorT(const Tpetra_Vector& solnT, const Tpetra_Vector& soln_dotT,
                         const Tpetra_Vector& soln_dotdotT, 
                         stk::mesh::Selector& sel, const Teuchos::RCP<const Tpetra_Map>& node_mapT);

    void saveSolnMultiVector(const Tpetra_MultiVector& solnT, stk::mesh::Selector& sel, const Teuchos::RCP<const Tpetra_Map>& node_mapT);

#if defined(ALBANY_EPETRA)
    void saveResVector(const Epetra_Vector& res, stk::mesh::Selector& sel, const Teuchos::RCP<const Epetra_Map>& node_map);
#endif

    void saveResVectorT(const Tpetra_Vector& res, stk::mesh::Selector& sel, const Teuchos::RCP<const Tpetra_Map>& node_map);

    void transferSolutionToCoords();

  private:

    void initializeSTKAdaptation();

    bool buildSphereVolume;

    Teuchos::Array<AbstractSTKFieldContainer::VectorFieldType*> solution_field;
    Teuchos::Array<AbstractSTKFieldContainer::VectorFieldType*> solution_field_dtk;
    AbstractSTKFieldContainer::VectorFieldType* residual_field;

};

} // namespace Albany



// Define macro for explicit template instantiation
#define ORDINARYSTKFIELDCONTAINER_INSTANTIATE_TEMPLATE_CLASS_NONINTERLEAVED(name) \
  template class name<false>;
#define ORDINARYSTKFIELDCONTAINER_INSTANTIATE_TEMPLATE_CLASS_INTERLEAVED(name) \
  template class name<true>;

#define ORDINARYSTKFIELDCONTAINER_INSTANTIATE_TEMPLATE_CLASS(name) \
  ORDINARYSTKFIELDCONTAINER_INSTANTIATE_TEMPLATE_CLASS_NONINTERLEAVED(name) \
  ORDINARYSTKFIELDCONTAINER_INSTANTIATE_TEMPLATE_CLASS_INTERLEAVED(name)


#endif
