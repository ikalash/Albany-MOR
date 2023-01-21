//*****************************************************************//
//    Albany 3.0:  Copyright 2016 Sandia Corporation               //
//    This Software is released under the BSD license detailed     //
//    in the file "license.txt" in the top-level Albany directory  //
//*****************************************************************//
#ifndef MOR_MATRIXMARKETMVOUTPUTFILE_HPP
#define MOR_MATRIXMARKETMVOUTPUTFILE_HPP

#include "MOR_MultiVectorOutputFile.hpp"

namespace MOR {

class MatrixMarketMVOutputFile : public MultiVectorOutputFile {
public:
  explicit MatrixMarketMVOutputFile(const std::string &path);

  virtual void write(const Epetra_MultiVector &mv); // overriden
};

} // namespace MOR

#endif /* MOR_MATRIXMARKETMVOUTPUTFILE_HPP */
