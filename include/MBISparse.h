// MBISparse.h - Sparse matrix codes
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * MBI Data Access API                                             *
 * Copyright 2026 MOBILion Systems, Inc. ALL RIGHTS RESERVED       *
 * Author: Bennett Kalafut                                         *
 * release-1.12.5.10441
 * For full license terms, see the LICENSE.md file in the root of  *
 * this repository.                                                *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#pragma once

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

#include <vector>

namespace MBISDK
{

    /*! @struct MBISDK::COOArray
	 *  @brief A structure holding a coordinate object array
	 */
	template<typename var>
	struct COOArray
	{
		std::vector<var> data;
		std::vector<size_t> rowIndices;
		std::vector<size_t> columnIndices;
		size_t nRows;
		size_t nColumns;
		size_t nnz;
		bool isZeroPadded = false;

		var Sum()
		{
			var accumulator = 0;
			for (auto x = data.begin(); x != data.end(); ++x)
			{
				accumulator += *x;
			}
			return accumulator;
		}
	};

    /*! @struct MBISDK::CSRArray
	 *  @brief A structure for an aray in the Compressed Sparse Row format
	 */
	template<typename var>
	struct CSRArray
	{
		std::vector<var> data;
		std::vector<size_t> indices;
		std::vector<size_t> indptr;
		size_t nRows;
		size_t nColumns;
		size_t nnz;
		bool isZeroPadded = false;

        CSRArray<var>() = default;

        CSRArray<var>(std::vector<var>&& dataIn, std::vector<size_t>&& indicesIn, std::vector<size_t>&& indptrIn, size_t nRowsIn, size_t nColumnsIn, size_t nnzIn, bool isZeroPaddedIn)
            : data(std::move(dataIn)),
            indices(std::move(indicesIn)),
            indptr(std::move(indptrIn)),
            nRows(nRowsIn),
            nColumns(nColumnsIn),
            nnz(nnzIn),
            isZeroPadded(isZeroPaddedIn)
        {}

        CSRArray<var>(std::vector<var>& dataIn, std::vector<size_t>& indicesIn, std::vector<size_t>& indptrIn, size_t nRowsIn, size_t nColumnsIn, size_t nnzIn, bool isZeroPaddedIn)
            : data(dataIn),
            indices(indicesIn),
            indptr(indptrIn),
            nRows(nRowsIn),
            nColumns(nColumnsIn),
            nnz(nnzIn),
            isZeroPadded(isZeroPaddedIn)
        {}

		var Sum()
		{
			var accumulator = 0;
			for (auto x = data.begin(); x != data.end(); ++x)
			{
				accumulator += *x;
			}
			return accumulator;
		}
	};

    template<typename T>
	struct CSRIMMSSpectrum : public CSRArray<T>
	{
	    std::vector<double> mz;
         
		/// @brief A move construtor taking a csrArray as input.  The m/z array will be left blank.
		/// @param csrArray 
		explicit CSRIMMSSpectrum<T>(CSRArray<T>&& csrArray)
		{
            this->nnz = csrArray.nnz; // The "this" pointer always implicitly resolves.
            this->nRows = csrArray.nRows;
            this->nColumns = csrArray.nColumns;
			this->isZeroPadded = csrArray.isZeroPadded;
            this->data = std::move(csrArray.data);
            this->indices = std::move(csrArray.indices);
            this->indptr = std::move(csrArray.indptr);
        }

		explicit CSRIMMSSpectrum<T>(const CSRArray<T>& csrArray)
		{
            this->nnz = csrArray.nnz; // The "this" pointer always implicitly resolves.
            this->nRows = csrArray.nRows;
            this->nColumns = csrArray.nColumns;
			this->isZeroPadded = csrArray.isZeroPadded;
            this->data = csrArray.data; // lvalue assignment of vector is a full copy, incl. deep copy of the data array
            this->indices = csrArray.indices;
            this->indptr = csrArray.indptr;
        }

	};
}
