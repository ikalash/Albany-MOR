//*****************************************************************//
//    Albany 3.0:  Copyright 2016 Sandia Corporation               //
//    This Software is released under the BSD license detailed     //
//    in the file "license.txt" in the top-level Albany directory  //
//*****************************************************************//

#ifndef AADAPT_ANALYTICFUNCTION_HPP
#define AADAPT_ANALYTICFUNCTION_HPP

#include "Albany_config.h"

#include <string>

// Random and Gaussian number distribution
#include <boost/random.hpp>
#include <boost/random/normal_distribution.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/variate_generator.hpp>

#include "Teuchos_Array.hpp"
#ifdef ALBANY_PAMGEN
#include "RTC_FunctionRTC.hh"
#endif

namespace AAdapt {

// generate seed convenience function
long seedgen(int worksetID);

// Base class for initial condition functions
class AnalyticFunction {
  public:
    virtual ~AnalyticFunction() {}
    virtual void compute(double* x, const double* X) = 0;
};

// Factory method to build functions based on a string name
Teuchos::RCP<AnalyticFunction> createAnalyticFunction(
  std::string name, int neq, int numDim,
  Teuchos::Array<double> data);

// Below is a library of intial condition functions

class ConstantFunction : public AnalyticFunction {
  public:
    ConstantFunction(int neq_, int numDim_, Teuchos::Array<double> data_);
    void compute(double* x, const double* X);
  private:
    int numDim; // size of coordinate vector X
    int neq;    // size of solution vector x
    Teuchos::Array<double> data;
};

class StepX : public AnalyticFunction {
  public:
    StepX(int neq_, int numDim_, Teuchos::Array<double> data_);
    void compute(double* x, const double* X);
  private:
    int numDim; // size of coordinate vector X
    int neq;    // size of solution vector x
    Teuchos::Array<double> data;
};

#ifdef ALBANY_TSUNAMI 
class TsunamiBoussinesq1DSolitaryWave: public AnalyticFunction {
  public:
    TsunamiBoussinesq1DSolitaryWave(int neq_, int numDim_, Teuchos::Array<double> data_);
    void compute(double* x, const double* X);
  private:
    int numDim; // size of coordinate vector X
    int neq;    // size of solution vector x
    Teuchos::Array<double> data;
};
#endif

class TemperatureStep : public AnalyticFunction {
  public:
    TemperatureStep(int neq_, int numDim_, Teuchos::Array<double> data_);
    void compute(double* x, const double* X);
  private:
    int numDim; // size of coordinate vector X
    int neq;    // size of solution vector x
    Teuchos::Array<double> data;
};

class DispConstTemperatureStep : public AnalyticFunction {
  public:
    DispConstTemperatureStep(int neq_, int numDim_, Teuchos::Array<double> data_);
    void compute(double* x, const double* X);
  private:
    int numDim; // size of coordinate vector X
    int neq;    // size of solution vector x
    Teuchos::Array<double> data;
};

class TemperatureLinear : public AnalyticFunction {
  public:
    TemperatureLinear(int neq_, int numDim_, Teuchos::Array<double> data_);
    void compute(double* x, const double* X);
  private:
    int numDim; // size of coordinate vector Y
    int neq;    // size of solution vector
    Teuchos::Array<double> data;
};

class DispConstTemperatureLinear : public AnalyticFunction {
  public:
    DispConstTemperatureLinear(int neq_, int numDim_, Teuchos::Array<double> data_);
    void compute(double* x, const double* X);
  private:
    int numDim; // size of coordinate vector Y
    int neq;    // size of solution vector
    Teuchos::Array<double> data;
};

class ConstantFunctionPerturbed : public AnalyticFunction {
  public:
    ConstantFunctionPerturbed(int neq_, int numDim_, int worksetID,
                              Teuchos::Array<double> const_data_, Teuchos::Array<double> pert_mag_);
    void compute(double* x, const double* X);
  private:
    int numDim; // size of coordinate vector X
    int neq;    // size of solution vector x
    Teuchos::Array<double> data;
    Teuchos::Array<double> pert_mag;

    // random number generator convenience function
    double udrand(double lo, double hi);

};

class ConstantFunctionGaussianPerturbed : public AnalyticFunction {
  public:
    ConstantFunctionGaussianPerturbed(int neq_, int numDim_, int worksetID,
                                      Teuchos::Array<double> const_data_, Teuchos::Array<double> pert_mag_);
    void compute(double* x, const double* X);
  private:
    int numDim; // size of coordinate vector X
    int neq;    // size of solution vector x
    Teuchos::Array<double> data;
    Teuchos::Array<double> pert_mag;

    boost::mt19937 rng;
    Teuchos::Array<Teuchos::RCP<boost::normal_distribution<double> > > nd;
    Teuchos::Array < Teuchos::RCP < boost::variate_generator < boost::mt19937&,
            boost::normal_distribution<double> > > > var_nor;

};

class GaussSin : public AnalyticFunction {
  public:
    GaussSin(int neq_, int numDim_, Teuchos::Array<double> data_);
    void compute(double* x, const double* X);
  private:
    int numDim; // size of coordinate vector X
    int neq;    // size of solution vector x
    Teuchos::Array<double> data;
};

class GaussCos : public AnalyticFunction {
  public:
    GaussCos(int neq_, int numDim_, Teuchos::Array<double> data_);
    void compute(double* x, const double* X);
  private:
    int numDim; // size of coordinate vector X
    int neq;    // size of solution vector x
    Teuchos::Array<double> data;
};

class LinearY : public AnalyticFunction {
  public:
    LinearY(int neq_, int numDim_, Teuchos::Array<double> data_);
    void compute(double* x, const double* X);
  private:
    int numDim; // size of coordinate vector X
    int neq;    // size of solution vector x
    Teuchos::Array<double> data;
};

class Linear : public AnalyticFunction {
  public:
    Linear(int neq_, int numDim_, Teuchos::Array<double> data_);
    void compute(double* x, const double* X);
  private:
    int numDim; // size of coordinate vector X
    int neq;    // size of solution vector x
    Teuchos::Array<double> data;
};

class ConstantBox : public AnalyticFunction {
  public:
  ConstantBox(int neq_, int numDim_, Teuchos::Array<double> data_);
    void compute(double* x, const double* X);
  private:
    int numDim; // size of coordinate vector X
    int neq;    // size of solution vector x
    Teuchos::Array<double> data;
};

class AboutZ : public AnalyticFunction {
  public:
    AboutZ(int neq_, int numDim_, Teuchos::Array<double> data_);
    void compute(double* x, const double* X);
  private:
    int numDim; // size of coordinate vector X
    int neq;    // size of solution vector x
    Teuchos::Array<double> data;
};

class RadialZ : public AnalyticFunction {
  public:
    RadialZ(int neq_, int numDim_, Teuchos::Array<double> data_);
    void compute(double* x, const double* X);
  private:
    int numDim; // size of coordinate vector X
    int neq;    // size of solution vector x
    Teuchos::Array<double> data;
};

class AboutLinearZ : public AnalyticFunction {
  public:
    AboutLinearZ(int neq_, int numDim_, Teuchos::Array<double> data_);
    void compute(double* x, const double* X);
  private:
    int numDim; // size of coordinate vector X
    int neq;    // size of solution vector x
    Teuchos::Array<double> data;
};

class GaussianZ : public AnalyticFunction {
  public:
  GaussianZ(int neq_, int numDim_, Teuchos::Array<double> data_);
    void compute(double* x, const double* X);
  private:
    int numDim; // size of coordinate vector X
    int neq;    // size of solution vector x
    Teuchos::Array<double> data;
};

class Circle : public AnalyticFunction {
  public:
    Circle(int neq_, int numDim_, Teuchos::Array<double> data_);
    void compute(double* x, const double* X);
  private:
    int numDim; // size of coordinate vector X
    int neq;    // size of solution vector x
    Teuchos::Array<double> data;
};

class GaussianPress : public AnalyticFunction {
  public:
    GaussianPress(int neq_, int numDim_, Teuchos::Array<double> data_);
    void compute(double* x, const double* X);
  private:
    int numDim; // size of coordinate vector X
    int neq;    // size of solution vector x
    Teuchos::Array<double> data;
};

class SinCos : public AnalyticFunction {
  public:
    SinCos(int neq_, int numDim_, Teuchos::Array<double> data_);
    void compute(double* x, const double* X);
  private:
    int numDim; // size of coordinate vector X
    int neq;    // size of solution vector x
    Teuchos::Array<double> data;
};

class SinScalar : public AnalyticFunction {
  public:
    SinScalar(int neq_, int numDim_, Teuchos::Array<double> data_);
    void compute(double* x, const double* X);
  private:
    int numDim; // size of coordinate vector X
    int neq;    // size of solution vector x
    Teuchos::Array<double> data;
};

class TaylorGreenVortex : public AnalyticFunction {
  public:
    TaylorGreenVortex(int neq_, int numDim_, Teuchos::Array<double> data_);
    void compute(double* x, const double* X);
  private:
    int numDim; // size of coordinate vector X
    int neq;    // size of solution vector x
    Teuchos::Array<double> data;
};

class AcousticWave : public AnalyticFunction {
  public:
    AcousticWave(int neq_, int numDim_, Teuchos::Array<double> data_);
    void compute(double* x, const double* X);
  private:
    int numDim; // size of coordinate vector X
    int neq;    // size of solution vector x
    Teuchos::Array<double> data;
};

class AerasScharDensity : public AnalyticFunction {
  public:
    AerasScharDensity(int neq_, int numDim_, Teuchos::Array<double> data_);
    void compute(double* x, const double* X);
  private:
    int numDim; // size of coordinate vector X
    int neq;    // size of solution vector x
    Teuchos::Array<double> data;
};

class AerasXScalarAdvection : public AnalyticFunction {
  public:
    AerasXScalarAdvection(int neq_, int numDim_, Teuchos::Array<double> data_);
    void compute(double* x, const double* X);
  private:
    int numDim; // size of coordinate vector X
    int neq;    // size of solution vector x
    Teuchos::Array<double> data;
};


class AerasHydrostaticBaroclinicInstabilities : public AnalyticFunction {
  public:
    AerasHydrostaticBaroclinicInstabilities(int neq_, int numDim_, Teuchos::Array<double> data_);
    void compute(double* x, const double* X);
  private:
    const int numDim; // size of coordinate vector X
    const int neq;    // size of solution vector x
    Teuchos::Array<double> data;
    bool printedHybrid;
};

class AerasHydrostaticPureAdvection1 : public AnalyticFunction {
  public:
    AerasHydrostaticPureAdvection1(int neq_, int numDim_, Teuchos::Array<double> data_);
    void compute(double* x, const double* X);
  private:
    const int numDim; // size of coordinate vector X
    const int neq;    // size of solution vector x
    Teuchos::Array<double> data;
};

class AerasHydrostatic3dDeformationalFlow : public AnalyticFunction {
  public:
    AerasHydrostatic3dDeformationalFlow(int neq_, int numDim_, Teuchos::Array<double> data_);
    void compute(double* x, const double* X);
  private:
    const int numDim; // size of coordinate vector X
    const int neq;    // size of solution vector x
    Teuchos::Array<double> data;
};

class AerasXZHydrostatic : public AnalyticFunction {
  public:
    AerasXZHydrostatic(int neq_, int numDim_, Teuchos::Array<double> data_);
    void compute(double* x, const double* X);
  private:
    const int numDim; // size of coordinate vector X
    const int neq;    // size of solution vector x
    Teuchos::Array<double> data;
};

class AerasXZHydrostaticGaussianBall : public AnalyticFunction {
  public:
    AerasXZHydrostaticGaussianBall(int neq_, int numDim_, Teuchos::Array<double> data_);
    void compute(double* x, const double* X);
  private:
    const int numDim; // size of coordinate vector X
    const int neq;    // size of solution vector x
    Teuchos::Array<double> data;
};

class AerasXZHydrostaticGaussianBallInShear : public AnalyticFunction {
  public:
    AerasXZHydrostaticGaussianBallInShear(int neq_, int numDim_, Teuchos::Array<double> data_);
    void compute(double* x, const double* X);
  private:
    const int numDim; // size of coordinate vector X
    const int neq;    // size of solution vector x
    Teuchos::Array<double> data;
};

class AerasXZHydrostaticGaussianVelocityBubble : public AnalyticFunction {
  public:
    AerasXZHydrostaticGaussianVelocityBubble(int neq_, int numDim_, Teuchos::Array<double> data_);
    void compute(double* x, const double* X);
  private:
    const int numDim; // size of coordinate vector X
    const int neq;    // size of solution vector x
    Teuchos::Array<double> data;
};

class AerasXZHydrostaticCloud : public AnalyticFunction {
  public:
    AerasXZHydrostaticCloud(int neq_, int numDim_, Teuchos::Array<double> data_);
    void compute(double* x, const double* X);
  private:
    const int numDim; // size of coordinate vector X
    const int neq;    // size of solution vector x
    Teuchos::Array<double> data;
};

class AerasXZHydrostaticMountain : public AnalyticFunction {
  public:
    AerasXZHydrostaticMountain(int neq_, int numDim_, Teuchos::Array<double> data_);
    void compute(double* x, const double* X);
  private:
    const int numDim; // size of coordinate vector X
    const int neq;    // size of solution vector x
    Teuchos::Array<double> data;
};

class AerasHydrostatic : public AnalyticFunction {
  public:
    AerasHydrostatic(int neq_, int numDim_, Teuchos::Array<double> data_);
    void compute(double* x, const double* X);
  private:
    int numDim; // size of coordinate vector X
    int neq;    // size of solution vector x
    Teuchos::Array<double> data;
};

class AerasRestingHydrostatic : public AnalyticFunction {
  public:
    AerasRestingHydrostatic(int neq_, int numDim_, Teuchos::Array<double> data_);
    void compute(double* x, const double* X);
  private:
    const int numDim; // size of coordinate vector X
    const int neq;    // size of solution vector x
    Teuchos::Array<double> data;
};

class AerasHeaviside : public AnalyticFunction {
  public:
    AerasHeaviside(int neq_, int numDim_, Teuchos::Array<double> data_);
    void compute(double* x, const double* X);
  private:
    int numDim; // size of coordinate vector X
    int neq;    // size of solution vector x
    Teuchos::Array<double> data;
};

class AerasCosineBell : public AnalyticFunction {
  public:
    AerasCosineBell(int neq_, int spatialDim_, Teuchos::Array<double> data_);
    void compute(double* x, const double* X);
  private:
    int spatialDim; // size of coordinate vector X
    int neq;    // size of solution vector x
    Teuchos::Array<double> data;
};

class AerasSlottedCylinder : public AnalyticFunction {
  public:
    AerasSlottedCylinder(int neq_, int spatialDim_, Teuchos::Array<double> data_);
    void compute(double* x, const double* X);
  private:
    int spatialDim; // size of coordinate vector X
    int neq;    // size of solution vector x
    Teuchos::Array<double> data;
};

class AerasScalarCosineBell : public AnalyticFunction {
  public:
    AerasScalarCosineBell(int neq_, int spatialDim_, Teuchos::Array<double> data_);
    void compute(double* x, const double* X);
  private:
    int spatialDim; // size of coordinate vector X
    int neq;    // size of solution vector x
    Teuchos::Array<double> data;
};

class AerasZonalFlow : public AnalyticFunction {
  public:
     AerasZonalFlow(int neq_, int spatialDim_, Teuchos::Array<double> data_);
    void compute(double* x, const double* X);
  private:
    int spatialDim; // size of coordinate vector X
    int neq;    // size of solution vector x

    Teuchos::Array<double> data;
};
class AerasTC5Init : public AnalyticFunction {
  public:
     AerasTC5Init(int neq_, int spatialDim_, Teuchos::Array<double> data_);
    void compute(double* x, const double* X);

  private:
    int spatialDim; // size of coordinate vector X
    int neq;    // size of solution vector x

    Teuchos::Array<double> data;
};

//------------------------------------------------------------------------------

class AerasTC3Init : public AnalyticFunction {
    public:
        AerasTC3Init(int neq_, int spatialDim_, Teuchos::Array<double> data_);
        void compute(double* x, const double* X);

    private:
        int spatialDim; // size of coordinate vector X
        int neq;        // size of solution vector x

        Teuchos::Array<double> data;

    private:
        double bx(const double x); //an indicator function

        double ucomponent(const double lon); //unrotated u-comp in TC3

        //obtains rotated lon lat for TC3.
        void rotate(const double lon, const double lat, const double alpha, double& rotlon, double& rotlat);

        double earthRadius; //Earth radius
        double testDuration; // =12 days, in seconds
        double myPi; // a local copy of pi
        double u0; //a u-comp velocity multiplier, based on Williamson1992
        double Omega;
        double gravity;
        double h0g;

        double xe;
        double thetae;
        double thetab;
};

//------------------------------------------------------------------------------

class AerasTCGalewskyInit : public AnalyticFunction {
    public:
        AerasTCGalewskyInit(int neq_, int spatialDim_, Teuchos::Array<double> data_);
        void compute(double* x, const double* X);

    private:
        int spatialDim; // size of coordinate vector X
        int neq;    // size of solution vector x

        Teuchos::Array<double> data;

    private:

        double ucomponent(const double lon); //
        double hperturb(const double lon, const double lat); //

        double earthRadius; //Earth radius
        double gravity; // a loc copy of gravity
        double myPi; // a local copy of pi
        double Omega;

        double phi0;
        double phi1;
        double umax;
        double en;

        double h0;
        double phi2;
        double al;
        double beta;
        double hhat;
};

//-------------------------------------------------------------------

class AerasTC4Init : public AnalyticFunction {
  public:
    AerasTC4Init(int neq_, int spatialDim_, Teuchos::Array<double> data_);
    void compute(double* x, const double* X);

  private:
    int spatialDim; // size of coordinate vector X
    int neq;    // size of solution vector x

    Teuchos::Array<double> data;

  private:

    double earthRadius; //Earth radius
    double myPi; // a local copy of pi
    double Omega;
    double gravity;

    double su0;
    double phi0;
    double rlon0;
    double rlat0;

    double alfa; //spelling is correct
    double sigma;
    double npwr;

    double phicon(const double lat);
    double bubfnc(const double lat);
    double dbubf(const double lat);

};

//----------------------------------------------------------------------------

class AerasPlanarCosineBell : public AnalyticFunction {
  public:
    AerasPlanarCosineBell(int neq_, int numDim_, Teuchos::Array<double> data_);
    void compute(double* x, const double* X);
  private:
    int numDim; // size of coordinate vector X
    int neq;    // size of solution vector x
    Teuchos::Array<double> data;
};

//----------------------------------------------------------------------------

class AerasRossbyHaurwitzWave : public AnalyticFunction {
  public:
    AerasRossbyHaurwitzWave(int neq_, int spatialDim_, Teuchos::Array<double> data_);
    void compute(double* x, const double* X);
  private:
    int spatialDim; // size of coordinate vector X
    int neq;    // size of solution vector x

    Teuchos::Array<double> data;
};

//----------------------------------------------------------------------------

class ExpressionParser : public AnalyticFunction {
  public:
    ExpressionParser(int neq_, int spatialDim_, std::string expressionX_, std::string expressionY_, std::string expressionZ_);
    void compute(double* x, const double* X);
  private:
    int spatialDim; // size of coordinate vector X
    int neq;    // size of solution vector x
    std::string expressionX;
    std::string expressionY;
    std::string expressionZ;
#ifdef ALBANY_PAMGEN
    PG_RuntimeCompiler::Function rtcFunctionX;
    PG_RuntimeCompiler::Function rtcFunctionY;
    PG_RuntimeCompiler::Function rtcFunctionZ;
#endif
};

}

#endif
