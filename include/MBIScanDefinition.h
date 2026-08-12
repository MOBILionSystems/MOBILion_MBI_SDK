// MBIScanDefinition.h - Classes for summarizing an IM-MS scan definition
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * MBI Data Access API                                             *
 * Copyright 2026 MOBILion Systems, Inc. ALL RIGHTS RESERVED       *
 * Author: Bennett Kalafut                                         *
 * release-1.12.5.10441
 * For full license terms, see the LICENSE.md file in the root of  *
 * this repository.                                                *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#pragma once
#include <memory>
#include <vector>
#include <stdexcept>
#ifndef MBI_DLLCPP
#ifdef LINUX
#define MBI_DLLCPP
#else
#ifdef SWIG_WIN
#define MBI_DLLCPP
#else
#ifdef MBI_EXPORTS
#define MBI_DLLCPP __declspec(dllexport)
#else
#define MBI_DLLCPP __declspec(dllimport)
#endif
#endif
#endif
#endif

namespace MBISDK {

    class ScanDefinition; // forward declaration

    template <typename Tx, typename Ty>
    class PiecewiseFunction
    {
        friend class ScanDefinition;
        
        public:
            PiecewiseFunction(const std::vector<Tx>& x, const std::vector<Ty>& y) //Inputs should be sorted.  We will not check at initialization.
            {
                xTable = x;
                yTable = y;
                n = x.size();
                if (y.size() != n)
                {
                    throw std::runtime_error("x and y table sizes must match.");
                }
                xMin = x[0];
                xMax = x[n];

            }
            
            PiecewiseFunction(std::vector<Tx>&& x, std::vector<Ty>&& y)
            {
                xTable = std::move(x);
                yTable = std::move(y);
                n = xTable.size();
                if (yTable.size() != n)
                {
                    throw std::runtime_error("x and y table sizes must match.");
                }
                xMin = xTable[0];
                xMax = xTable[n];
            }
            virtual Ty Evaluate(const Tx x) = 0; //SWIG doesn't handle "virtual" correctly
            //Ty Evaluate(const Tx x){throw std::runtime_error("Not implemented in base class.");}

            Ty operator()(const Tx x){return Evaluate(x);} //This won't work until we can use "virtual"

            

        protected:
            std::vector<Tx> xTable;
            std::vector<Ty> yTable;
            Ty xMin;
            Ty xMax;
            size_t n;
    };

    template <typename Tx, typename Ty>
    class StepwiseFunction : public PiecewiseFunction<Tx, Ty>
    {
        using PiecewiseFunction<Tx,Ty>::PiecewiseFunction;

        public:
            Ty Evaluate(const Tx x)
            {
                if (x < this->xMin) //No support for leftward extrapolation.
                {
                    throw std::domain_error("Input value is outside defined range for a stepwise function.");
                }
                for (size_t i = 0; i < this->n; i++)
                {
                    if (x < this->xTable[i])
                    {
                        return this->yTable[i-1];
                    }
                }
                return this->yTable[this->n - 1];
            }
    };

    template <typename Tx, typename Ty>
    class PiecewiseLinearFunction : public PiecewiseFunction <Tx, Ty>
    {
        public:
            PiecewiseLinearFunction(const std::vector<Tx>& x, const std::vector<Ty>& y) : PiecewiseFunction<Tx,Ty>(x,y)
            {
                if (this->n < 2)
                {
                    throw std::runtime_error("A piecewise linear function must be defined by at least two points.");
                }
                SetSlopesAndIntercepts();
            }

            PiecewiseLinearFunction(std::vector<Tx>&& x, std::vector<Ty>&& y) : PiecewiseFunction<Tx,Ty>(std::move(x), std::move(y))
            {
                if (this->n < 2)
                {
                    throw std::runtime_error("A piecewise linear function must be defined by at least two points.");
                }
                SetSlopesAndIntercepts();
            }
            
            Ty Evaluate(const Tx x)
            {
                if ((x < this->xMin) || (x < this->xMax))
                {
                    throw std::domain_error("Input value is outside defined range for a piecewise linear function.");
                }

                size_t intervalIndex;
                for (intervalIndex = 0; intervalIndex < (this->n - 1); intervalIndex++)
                {
                    if (x <= this->xTable[intervalIndex+1])
                        break;
                }    
                return intercepts[intervalIndex] + slopes[intervalIndex] * x;
            }
        
        private:

            std::vector<Ty> slopes = std::vector<Ty>(0);
            std::vector<Ty> intercepts = std::vector<Ty>(0);
            void SetSlopesAndIntercepts()
            {
                for (size_t i = 0; i < (this->n - 1) ; i++) // loop over intervals
                {
                    Tx x1 = this->xTable[i];
                    Tx x2 = this->xTable[i+1];
                    Ty y1 = this->yTable[i];
                    Ty y2 = this->yTable[i+1];

                    Ty slope = (y2-y1)/(x2-x1);
                    slopes.push_back(slope);
                    intercepts.push_back(y1 - (Ty)(slope * x1));
                }

            }
    };

extern "C" {

            

    /*! @class MBISDK::ScanDefinition
     *  @brief Configuration of an IM-MS scan frame, including tables of isolation masses and system voltages as relevant
     *	
     *  Mass isolation windows and collision energy ramps are stored as piecewise link tables.  
     *  The location (in time) of the joints and the endpoints are given.
     *  This is similar to the specification of an LC gradient on most systems.
     * 
     *  This can also be used to specify an *effective* isolation window in postprocessing, 
     *  to help informatics tools understand MAF.
     */
    class MBI_DLLCPP ScanDefinition
    {
        public:
            
            enum class MBI_DLLCPP InterpolationMode {
                STEPWISE,
                LINEAR
            };

            ~ScanDefinition();

            
            // Allow partial construction by move of elements.
            void SetIsolationWindows(std::vector<size_t>&& indices, std::vector<double>&& windowStarts, std::vector<double>&& windowEnds, ScanDefinition::InterpolationMode isolationWindowInterpolation);
            void SetCEs(std::vector<size_t>&& indices, std::vector<double>&& ces, ScanDefinition::InterpolationMode ceInterpolation);

            bool HasCE() {return hasCE;};
            bool HasIsolationWindows() {return hasIsolationWindows;};

            double CE(size_t index){return ceTable->Evaluate(index);}
            double IsolationWindowStart(size_t index){return isolationWindowStart->Evaluate(index);}
            double IsolationWindowEnd(size_t index){return isolationWindowEnd->Evaluate(index);}

            int MSLevel(){return msLevel;} //MS Level set at load time.

            bool IsPartitionable();

            /// @brief Returns a vector of the indices of the beginning of regions of different collision energy or mass isolation.
            std::vector<size_t> Partition();

            /// @brief Returns a shared pointer to a vector of the indices of the beginning of regions of different collision energy or mass isolation.
            std::shared_ptr<std::vector<size_t>> PartitionSP();
               

            

        private:
            
            InterpolationMode massWindowInterpolation;
            InterpolationMode ceInterpolation;
            PiecewiseFunction<size_t, double> * ceTable; // Must be a pointer since this late-binding 
            PiecewiseFunction<size_t, double> * isolationWindowStart;
            PiecewiseFunction<size_t, double> * isolationWindowEnd;
            bool hasCE = false; //private because we do not want external routines to toggle them.
            bool hasIsolationWindows = false;
            size_t nPoints = 0;
            int msLevel = -1; // Setting to nonsense at first.
            void SetMSLevel(std::vector<double> ces); // set based on CEs.
            void ComputePartition(std::vector<size_t> * output_vector);
            
    };

}

}
