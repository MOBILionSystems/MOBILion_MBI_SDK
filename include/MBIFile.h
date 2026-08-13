// MBIFile.h - The top-level API object interface to MBI data.
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * MBI Data Access API                                             *
 * Copyright 2026 MOBILion Systems, Inc. ALL RIGHTS RESERVED       *
 * Author: Douglas Bodden                                          *
 * release-1.13.1.11240
 * For full license terms, see the LICENSE.md file in the root of  *
 * this repository.                                                *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#pragma once
#pragma warning(disable : 4996)
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

#include "MBIConstants.h"
#include "MBIMetadata.h"
#include "MBICalibration.h"
#include "MBIFrame.h"

#include <cstdio>
#include <fstream>
#include <memory>
#include "MBIFileHDF5Adapter.h"
#include <set>

namespace MBISDK
{
extern "C"
{
	/*! @class MBISDK::MBIFile
	*	@brief Top level object for interfacing to the MBI data API.
	*	@author Doug Bodden */
	class MBI_DLLCPP MBIFile
	{

	public:
		///<summary>
		///Experiment types represented by a MBI file.
		///</summary>
		enum class ExperimentType
		{
			/// @brief An MS1 experiment.  Note that if collected on MOBIE with a MassHunter method with fragmentation turned on this can be AIF
			MS1,
			/// @brief AIF data collected through EYEON, as e.g. the high-energy file for stiteched (MIFF) MAF
			AIF,
			/// @brief MAF with a single input file
			SIFF_MAF,
			/// @brief MAF stitched from two input files.
			MIFF_MAF
		};


		/// @brief MBIFile unexpected error code
		static const int ERR_UNEXPECTED = -1;
		/// @brief MBIFile Success code
		static const int ERR_SUCCESS = 0;
		/// @brief MBIFile file not found error code
		static const int ERR_FILE_NOT_FOUND = 1;
		/// @brief MBIFile HDF5 file error code
		static const int ERR_HDF5_FILE_ERROR = 2;
		/// @brief MBIFile file not initialized error code
		static const int ERR_FILE_NOT_INITIALIZED = 3;
		/// @brief MBIFile metadata not loaded error code
		static const int ERR_METADATA_NOT_LOADED = 101;
		/// @brief MBIFile frame not loaded error code
		static const int ERR_FRAME_NOT_LOADED = 102;
		/// @brief MBIFile double-load attempt error code
		static const int ERR_DOUBLE_LOAD = 103;
		/// @brief MBIFile bad frame index error code
		static const int ERR_BAD_FRAME_INDEX = 201;
		/// @brief MBIFile bad scan index error code
		static const int ERR_BAD_SCAN_INDEX = 202;
		/// @brief MBIFile item missing error code
		static const int ERR_ITEM_MISSING = 301;
		/// @brief MBIFile operation not supported error code
		static const int ERR_OPERATION_NOT_SUPPORTED = 401;

		// MBISDK-45 - compiler flagged an uninitialized memory use warning for processOffsetLengthStats().  Indeed, if the number of frames retrieved is actually zero,
		// then some of the function variables used to set the MBI object members would be uninitialized.  Created this new error to represent that condition.
		/// @brief MBIFile operation zero frames return error code
		static const int ERR_ZERO_FRAMES = 501;

		/// <summary>
		/// Prepares the MBIFile object for further use by setting internal state variables to defaults. This is the starting point for interacting with an MBI file.
		/// </summary>
		MBIFile();

		/// <summary>
		/// To associate the MBIFile object with an actual file, setting it up for file operations such as reading, processing, or extracting metadata.
		/// </summary>
		/// <param name="path">The file path as a C-style string.</param>
		MBIFile(const char* path);
        
		/// <summary>
		/// To associate the MBIFile object with an actual file, setting it up for file operations such as reading, processing, or extracting metadata.
		/// </summary>
		/// <param name="path">The file path as a C-style string.</param>
		/// <param name="path">'r', 'a', or 'w'</param>
		MBIFile(const char* path, const char mode);

		/// <summary>
		/// To properly close the MBI file and clean up any associated resources to ensure no memory leaks or file handle issues.
		/// </summary>
		void Close();

		/// <summary>
		/// To provide the user with the maximum number of scans available in the specified frame, ensuring valid frame access and processing. 
		/// </summary>
		/// <param name="nFrame">The frame index to check.</param>
		/// <returns>The maximum number of scans in the specified frame. If the file is not initialized, it sets ERR_FILE_NOT_INITIALIZED. If the frame index is invalid, it sets ERR_BAD_FRAME_INDEX. If metadata isn't loaded, it sets ERR_METADATA_NOT_LOADED.</returns>
		int GetMaxScansInFrame(int nFrame);

		/// <summary>
		/// To associate or update the global metadata for the MBIFile, ensuring that necessary file-wide metadata is accessible for further operations.
		/// </summary>
		/// <param name="globalMetadata">Shared pointer to the global metadata object to be set.</param>
		void SetGlobalMetaData(std::shared_ptr<GlobalMetadata> globalMetadata);

		/// <summary>
		/// The MBIFile's mass calibration.
		/// </summary>
		/// <returns></returns>
		MBISDK::TofCalibration GetCalibration();

        /// <summary>
		/// To determine whether or not a file has CCS coefficients written to it.
		//  </summary>
		bool HasCCSCalibration();

		/// <summary>
		/// To obtain the EyeOn CCS calibration data, which is used for analyzing collision cross sections in the data file.
		/// Returns by value for backward compatibility; prefer GetEyeOnCCSCalibrationOwned() for correct
		/// polymorphic dispatch. In 2.0 both methods will be consolidated into a single unique_ptr-returning call.
		/// </summary>
		/// <returns>An object containing CCS calibration data. If the file is not initialized, it sets the error code ERR_FILE_NOT_INITIALIZED. If metadata is not loaded, it sets ERR_METADATA_NOT_LOADED.</returns>
		MBISDK::EyeOnCcsCalibration GetEyeOnCCSCalibration();

		/// <summary>
		/// To obtain the EyeOn CCS calibration data with correct polymorphic type: returns
		/// NonReducedEyeOnCcsCalibration when the stored gas mass is 0, EyeOnCcsCalibration otherwise.
		/// Prefer this over GetEyeOnCCSCalibration() wherever unique_ptr ownership is acceptable.
		/// In 2.0 both methods will be consolidated into this unique_ptr-returning call.
		/// </summary>
		/// <returns>Heap-allocated calibration of the correct concrete type. If the file is not initialized, it sets the error code ERR_FILE_NOT_INITIALIZED. If metadata is not loaded, it sets ERR_METADATA_NOT_LOADED.</returns>
		std::unique_ptr<MBISDK::EyeOnCcsCalibration> GetEyeOnCCSCalibrationOwned();

		/// <summary>
		/// To obtain the CCS calibration data and polynomial coefficients used for CCS calculations and modeling.
		/// </summary>
		/// <returns>Pointer to a vector to store the polynomial coefficients. If the file is not initialized, it sets ERR_FILE_NOT_INITIALIZED. If calibration data is not available or valid, it returns an empty string. </returns>
		std::string GetCCSCalibration(std::vector<double>* coefficients);

		/// @brief
		/// The type of experiment (e.g. MS1, SIFF-MAF) represented by the MBI File.
		/// @return 
		/// An instance of the enum MBIFile::ExperimentType
		ExperimentType GetExperimentType();

		/// <summary>
		/// To provide the total count of frames in the MBI file, which is useful for iterating over frames or performing frame-specific operations.
		/// </summary>
		/// <returns>The number of frames in the file. If the file is not initialized, it sets the error code ERR_FILE_NOT_INITIALIZED. </returns>
		int NumFrames();

		/// <summary>
		/// To provide access to all the frames in the MBI file, allowing for frame-specific operations and analysis.
		/// </summary>
		/// <returns>A shared pointer to a vector of Frame pointers, all loaded, or a NULL ptr if frameIndex is invalid</returns>
		std::shared_ptr<std::vector<std::shared_ptr<Frame>>> GetFrames();

		/// <summary>
		/// To access a specific frame in the MBI file based on its index, allowing for frame-specific operations and analysis.
		/// </summary>
		/// <param name="frameIndex">1-based index into the Frame collection.</param>
		/// <returns>shared_ptr to a loaded Frame w/ the specified index. If the file is not initialized, it sets ERR_FILE_NOT_INITIALIZED. If the frame index is invalid, it sets ERR_BAD_FRAME_INDEX.</returns>
		std::shared_ptr<Frame> GetFrame(int frameIndex);

		/// <summary>
		/// To access detailed metadata about a specific frame, such as properties or additional information about the frame.
		/// </summary>
		/// <param name="frameIndex">1-based index into the frame collection.</param>
		/// <returns>shared_ptr to the frame's metadata object. If the file is not initialized, it sets ERR_FILE_NOT_INITIALIZED. If the frame index is invalid, it sets ERR_BAD_FRAME_INDEX.</returns>
		std::shared_ptr<FrameMetadata> GetFrameMetadata(int frameIndex);

		/// <summary>
		/// Returns the time of acquisition (UTC) as a string.
		/// </summary>
		std::string GetAcquisitionTimestamp();

		/// <summary>
		/// Retrieves a vector of retention times, loaded at file opening.
		/// </summary>
		/// <returns>shared_ptr to the vector of retention times.</returns>
		const std::shared_ptr<std::vector<double>> GetRetentionTimes();

        /// <summary>
        /// Retrieves a vector of MS levels, loaded at file opening.
        /// </summary>
        /// <returns>shared_ptr to the vector of MS levels.</returns>
        const std::shared_ptr<std::vector<int>> GetFrameMSLevels();

        /// @brief  Returns a vector of ion polarities (per frame, zero-based)
        const std::shared_ptr<std::vector<IonPolarity>>GetFrameIonPolarities();

        /// @brief Retrieve the ion polarity for the whole file
        const IonPolarity GetIonPolarity();

        /// <summary>
        /// Retrieves a vector of MS levels, loaded at file opening.
        /// </summary>
        /// <returns>shared_ptr to the vector of collision energies.</returns>
        const std::shared_ptr<std::vector<double>> GetFrameCollisionEnergies();

        /// <summary>
        /// Retrieves a vector of each frame's upper mass/charge bound, loaded at file opening.
        /// </summary>
        const std::shared_ptr<std::vector<double>> GetUpperMZBounds();

        /// <summary>
        /// Retrieves a vector of the number of scans per frame, loaded at opening.
        /// </summary>
        /// <returns>shared_ptr to the vector of MS levels.</returns>
        const std::shared_ptr<std::vector<int>> GetScanCounts();

        /// <summary>
        /// Computes and returns the averaged frm-dt-period from the frames' metadata.
        /// </summary>
        const double GetAverageIMSamplingPeriod();

        /// <summary>
        /// Determines and returns the length (in ms scans) of the longest frame.
        /// </summary>
        const int GetMaxNumScansPerFrame();

		/// @brief Return the number of scan definitions in the file.
		const size_t NumScanDefinitions();


		/// <summary>
		/// Deep-load a frame's data. If you only want to deepload frames based on metadata criteria, you may use this to avoid calling GetFrames() to load them all.
		/// </summary>
		/// <param name="frameIndex">The 1-based index into the frame collection. If the file is not initialized, it sets ERR_FILE_NOT_INITIALIZED. If the frame index is invalid, it sets ERR_BAD_FRAME_INDEX.</param>
		void LoadFrameData(int frameIndex);

		/// <summary>
		/// To remove the frame's data from memory, freeing resources and ensuring efficient memory management after processing. If the file is not initialized, it sets ERR_FILE_NOT_INITIALIZED. If the frame index is invalid, it sets ERR_BAD_FRAME_INDEX.
		[[deprecated]]
		/// </summary>
		void UnloadFrame(int frameIndex);

		/// <summary>
		/// Totally remove a frame from the file object's cache, without mutating any existing instances.
        /// If the file is not initialized, it sets ERR_FILE_NOT_INITIALIZED. If the frame index is invalid, it sets ERR_BAD_FRAME_INDEX.
		/// </summary>
		void UncacheFrame(int frameIndex);

		/// <summary>
		/// Return the max scan samples for the experiment.
		[[deprecated]]
		/// </summary>
		/// <returns>The max number of samples in a scan in the file.</returns>
		/// This method will be removed in a future version of MBI SDK.
		int GetMaxPointsInScan();

		/// <summary>
		/// Retrieve the name of the input file (full path).
		/// </summary>
		/// <returns>std::string the path used to open the file.</returns>
		std::shared_ptr<std::string> GetFilename();

		/// <summary>
		/// Set the name of the input file (full path).
		/// </summary>
		/// <param name="path">Path to the file to load.</param>
		void SetFilename(const char* path);

		/// <summary>
		/// Set the access mode of an input file.
		/// </summary>
		/// <param name="accessMode">'r', 'a', 'w'</param>
		void SetAccessMode(const char accessMode);


		/// <summary>
		/// To access the Retention Time TIC value and its corresponding timestamp for a specific frame.
		/// </summary>
		/// <returns>A pair containing the Retention Time TIC value (double) and timestamp (int64_t). If the file is not initialized, it sets ERR_FILE_NOT_INITIALIZED. If the frame index is invalid, it sets ERR_BAD_FRAME_INDEX.</returns>.
		std::pair<double, int64_t> GetRtTic(int frameIndex);

		/// <summary>
		/// Evaluate the frame index being passed in to ensure it is valid
		/// </summary>
		/// <returns>Returns true if the frame index is valid. If the file is not initialized, the function sets ERR_FILE_NOT_INITIALIZED. If the frame index is out of range, it sets ERR_BAD_FRAME_INDEX and returns false.
		bool CheckFrameIndex(int nFrameIndex);

		/// <summary>
		/// To access the start time of a specific frame.
		/// </summary>
		/// <returns>The start time (double) of the frame. Unit: seconds.
		double GetFrameStartTime(std::shared_ptr<FrameMetadata> pFrameMD);

		/// <summary>
		/// To load the Arrival Time TIC data for a specific frame.
		/// </summary>
		/// <param name="frameIndex">The 1-based index of the frame desired.</param>
		/// <returns>A vector of arrival time intensity counts per bin in the frame. If the file is not initialized, it sets ERR_FILE_NOT_INITIALIZED. If the frame index is invalid, it sets ERR_BAD_FRAME_INDEX.</returns>
		std::shared_ptr <std::vector<int64_t>> loadAtTic(int frameIndex);

		/// <summary>
		/// Initialize the MBIFile object to ensure that the object is properly initialized and ready for use before any further operations or methods are called. If initialization fails due to any reason (e.g., missing resources, incorrect configuration, or failed setup), the function returns false and sets an appropriate error code that can be checked using methods GetErrorCode().
		/// </summary>
		bool Init();

#ifdef MBI_SDK_INTERNAL
		/// <summary>
		/// Create a new the MBIFile file.
		/// </summary>
		bool Create();

		/// <summary>
		/// SaveChanges to the file.
		/// </summary>
		void SaveChanges();

		/// <summary>
		/// SaveChanges to a new file.
		/// </summary>
		void SaveChangesAs(int new_frames = 0);

        /// <summary>
		/// For incremental saving of frames to a file.
		/// </summary>
		void AppendFrame(std::shared_ptr<Frame> inboundFrame);

        /// <summary>
        /// Write cached global-metadata and chromatogram to a file written incrementally using AppendFrame.
        /// </summary>
        void WriteFinalData();

#endif
		/// <summary>
		/// Return the error code from the last method called.
		/// </summary>
		/// <returns>The error code from the last method call.</returns>
		int GetErrorCode();

		/// <summary>
		/// Return an error message from the last method call.
		/// </summary>
		/// <returns>The error message from the last method call.</returns>
		std::string GetErrorMessage();

		/// <summary>
		/// Retrieve the digitizer sample rate from global meta data
		/// </summary>
		/// <returns>A double, the sample rate from the file's global metadata
		double GetSampleRate();

		/// <summary>
		/// Retrieve the number of frames present in the MBI file from global meta data
		/// </summary>
		/// <returns>An int, the number of frames from the file's global metadata. If the file is not initialized or there is no valid data, it return 0 and set an error code ERR_FILE_NOT_INITIALIZED.
		int GetNumFrames();

		/// <summary>
		/// Return the version string of the MBI-SDK library.
		/// </summary>
		/// <returns></returns>
		std::string GetVersion();

		/// <summary>
		/// To access the global metadata, which typically contains essential file-wide information like configuration, calibration, or descriptive data.
		/// </summary>
		/// <returns>If the object is not initialized or if the metadata is unavailable, it may return a nullptr and set an error code like ERR_METADATA_NOT_LOADED.</returns>
		std::shared_ptr<GlobalMetadata> GetGlobalMetaData();

		/// <summary>
		/// Return the member variable value representing the initialzation status of the class.
		/// Many functions of the class rely on data being properly initialized.  By checking this status first, the library
		/// can be protected against attempted usage in an uninitialized condition
		/// </summary>
		/// <returns>The value stored in the member variable, true or false.</returns>
		bool IsInitialized();

		/// <summary>
		/// Return the lowest offset length statistics from all frames in the data file.
		/// This method will be removed from a future version of MBI SDK
		/// </summary>
		/// <returns>The lowest value computed from all frames.</returns>
		[[deprecated]]
		int64_t GetToFOffset();

		/// <summary>
		/// Return the length of offset length statistics from all frames in the data file.
		/// This method will be removed in a future version of MBI SDK.
		/// </summary>
		/// <returns>The length of all value computed from all frames.</returns>
		[[deprecated]]
		int64_t GetToFLength();

		/// <summary>
		/// Return the lowest arrival bin offset statistics from all frames in the data file.
		/// This method will be removed from a future version of MBI SDK
		/// </summary>
		/// <returns>The lowest value computed from all frames.</returns>
		[[deprecated]]
		size_t GetArrivalBinOffset();

		/// <summary>
		/// Return the length of arrival bin offset statistics from all frames in the data file.
		/// This method will be removed from a future version of MBI SDK
		/// </summary>
		/// <returns>The length of all value computed from all frames.</returns>
		[[deprecated]]
		size_t GetArrivalBinLength();

#ifdef MBI_SDK_INTERNAL
		/// <summary>
		/// A single gathering point for all metadata writing
		/// </summary>
		void WriteGenericMetaData(std::string key, std::string input);

		/// <summary>
		/// Calls the set of functions that represent "frame related data", allowing for future expansion should extra tables be added
		/// </summary>
		void SaveIntensityDataToFile(int frame_index);

		/// <summary>
		/// Saves AT_TIC data for a specific frame
		/// </summary>
		void SaveATTicData(int frame_index);

		/// <summary>
		/// Saves DATA_COUNTS data for a specific frame
		/// </summary>
		void SaveDataCounts(int frame_index);

		/// <summary>
		/// Saves DATA_POSITIONS data for a specific frame
		/// </summary>
		void SaveDataPositions(int frame_index);

		/// <summary>
		/// Saves INDEX_COUNTS data for a specific frame
		/// </summary>
		void SaveIndexCounts(int frame_index);

		/// <summary>
		/// Saves INDEX_POSITIONS data for a specific frame
		/// </summary>
		void SaveIndexPositions(int frame_index);

		/// <summary>
		/// Saves TRIGGER_TIMESTAMPS data for a specific frame
		/// </summary>
		void SaveTriggerTimeStamps(int frame_index);

		/// <summary>
		/// Saves Total Ion Count (TIC) data for a specific frame
		/// </summary>
		void SaveRtTicData();
#endif

		/// <summary>
		/// Return specific Total Ion Count (TIC) data from a specific frame
		/// </summary>
		/// <returns>Returns the Total Ion Count (TIC) value for the specified frame in int64_t </returns>
		int64_t GetSpecificRtTicValue(int nFrameIndex);

		/// <summary>
		/// List of RT Tic values
		/// </summary>
		/// <returns>List of int64_t values</returns>
		std::shared_ptr<std::vector<std::pair<double, int64_t>>> GetRtTicList();

		/// <summary>
		/// Per-frame total ion current as a contiguous vector of int64_t,
		/// indexed 0-based by internal frame index. Populated by loadRtTic.
		/// </summary>
		std::shared_ptr<std::vector<int64_t>> GetFrameTIC();

#ifdef MBI_SDK_INTERNAL
		/// <summary>
		/// Set specific Total Ion Count (TIC) data from a specific frame
		/// </summary>
		void SetSpecificRtTicValue(int frame_index, int64_t input_rt_tic);

		/// <summary>
		/// Add a single frame to the internal frame collection
		/// </summary>
		void AddFrame(std::shared_ptr<MBISDK::Frame> frame_ptr);

		/// <summary>
		/// Add a single frame metadata to the internal frame collection
		/// </summary>
		void AddFrameMetaData(std::shared_ptr<MBISDK::FrameMetadata> frame_meta_data_ptr);

		/// <summary>
		/// Calls the set of functions that represent "frame related data", allowing for future expansion should extra tables be added
		/// </summary>
		void WriteNewIntensityDataToFile(int frame_index);

		/// <summary>
		/// Writes New Total Ion Count (TIC) data to a file
		/// </summary>
		void WriteNewRtTicData();

		/// <summary>
		/// Writes New AT_TIC data for a specific frame
		/// </summary>
		void WriteNewATTicData(int frame_index);

		/// <summary>
		/// List of RT Tic values
		/// </summary>
		void SetRtTicList(std::shared_ptr<std::vector<std::pair<double, int64_t>>> input_rt_tic_list);

		/// <summary>
		/// Add input value to end of internal RTTic list
		/// </summary>
		void AppendRtTicItem(int64_t input_rt_tic, double input_start_time);

		/// <summary>
		/// Writes New DATA_COUNTS data for a specific frame
		/// </summary>
		void WriteNewDataCounts(int frame_index);

		/// <summary>
		/// Writes New DATA_POSITIONS data for a specific frame
		/// </summary>
		void WriteNewDataPositions(int frame_index);

		/// <summary>
		/// Writes New INDEX_COUNTS data for a specific frame
		/// </summary>
		void WriteNewIndexCounts(int frame_index);

		/// <summary>
		/// Writes New INDEX_POSITIONS data for a specific frame
		/// </summary>
		void WriteNewIndexPositions(int frame_index);

		/// <summary>
		/// Writes New TRIGGER_TIMESTAMPS data for a specific frame
		/// </summary>
		void WriteNewTriggerTimeStamps(int frame_index);

		/// <summary>
		/// Update changes made to frame metadata for this frame into the file's frame_metadata_collection
		/// </summary>
		void UpdateFrameMetaData(int frame_index, std::shared_ptr<MBISDK::FrameMetadata> frame_meta_data_ptr);

		/// <summary>
		/// Calculate the Total Ion Count (TIC) value for a given frame and frame index
		/// </summary>
		/// <returns>recalculated rt tic value</returns>
		int64_t RecalculateRTTicForFrame(std::shared_ptr<MBISDK::Frame> frame_ptr);

		/// <summary>
		/// Write slope/intercept and optional residual-fit terms as the ToF (mass) calibration
		/// on every frame without loading scan data.
		/// residualFit may be nullptr if there are no correction terms.
		/// </summary>
		void SetTofCalibrationAllFrames(double slope, double intercept,
		                                const double* residualFit, size_t residualFitCount);

		/// <summary>
		/// Convenience overload; delegates to the pointer-based overload.
		/// </summary>
		void SetTofCalibrationAllFrames(double slope, double intercept,
		                                const std::vector<double>& residualFit = std::vector<double>());
#endif
		/// <summary>
		/// Retrieves single metadata item
		/// </summary>
		/// <returns>String of metadata item</returns>
		std::string getMetaDataItem(std::string key);

		/// <summary>
		/// Retrieves single metadata item
		/// </summary>
		/// <returns>Int of metadata item</returns>
		int getMetaDataItemInt(std::string key);

		/// <summary>
		/// Retrieves single metadata item
		/// </summary>
		/// <returns>Double of metadata item</returns>
		double getMetaDataItemDouble(std::string key);

		/// <summary>
		/// Set the external error message
		/// </summary>
		void setExternalErrorMessage(std::string input_str);

		/// <summary>
		/// Retrieves external error message
		/// </summary>
		/// <returns>external error message</returns>
		std::string getExternalErrorMessage();

		bool HasScanDefinitions();

		double GetIsolationStart(size_t frameIndex, double t);
		double GetIsolationEnd(size_t frameIndex, double t);

	private:
		enum class CollectionMode
		{
            UNKNOWN,
			NONE,
			SIFF,
			MIFF
		};
		
		void setErrorCode(int error_code);
		MBIFileHDF5Adapter * getFileAbstract();
		void loadAllFrameMetadata();
		std::shared_ptr<FrameMetadata> loadFrameMetadata(int frameIndex);
		void loadRtTic();
		void loadMSLevelsFromScanDefinitions();
		void LoadFrameSummaryInformation();
		void processOffsetLengthStats();

		std::shared_ptr<Frame> GetNonLoadedFrame(int frameIndex);

		int MSLevel();

		CollectionMode AcqCollectionMode();
		ExperimentType MIFFFileCollectionMode();


	private:
		
	    char accessMode;
	
	    bool is_initialized;
		bool processed_offset_length_stats;
		int error_code;
		std::string external_error_message;
		std::shared_ptr < std::map<std::string, std::string>> map_meta_all;
		// file-oriented members
		std::shared_ptr<std::string> input_file_path;

		// Abstract data members
		// Frame collection:
		std::shared_ptr <std::vector<std::shared_ptr<Frame>>> frame_collection;
		std::shared_ptr <std::vector<std::shared_ptr<FrameMetadata>>> frame_metadata_collection;

        int num_frames; ///< Number of frames present in the file.
       	int total_scans; ///< Number of scans in the file.
		int max_scans; ///< Maximum number of scans in a frame in the file.
		int max_samples; ///< Maximum number of samples in a frame in the file.
		int total_samples; ///< Number of samples in the file.

		std::shared_ptr<GlobalMetadata> global_metadata;

	    // Acquisition metadata
        IonPolarity ion_polarity;
		int ms_level; /// MS1 or MS2
		CollectionMode collection_mode; // SIFF or MIFF; corresponds to acq-collection-mode in metadata
		std::shared_ptr<std::vector<double>> retention_times;
		std::shared_ptr<std::vector<int>> frame_ms_levels;
		std::shared_ptr<std::vector<double>> frame_collision_energies;
        std::shared_ptr<std::vector<IonPolarity>> frame_ion_polarities;

		// Frame-level internal implementation stuff

		std::shared_ptr<std::vector<std::pair<double, int64_t>>> rt_tic_collection;
		std::shared_ptr<std::vector<int64_t>> frame_tic;
		std::shared_ptr<std::vector<int>> scanCounts;
		std::shared_ptr<std::vector<double>> mzUpperBounds;
		std::string oob_error_message;
		bool has_rt_tic;

		int64_t tof_offset;
		int64_t tof_length;
		size_t scan_offset;
		size_t scan_length;

		MBIFileHDF5Adapter * mbifile_abstract;
	};
}
}
