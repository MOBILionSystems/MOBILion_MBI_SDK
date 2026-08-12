// MBI Data Access API                                             *
// Copyright 2026 MOBILion Systems, Inc. ALL RIGHTS RESERVED
// MBIFileHDF5Adapter.h - Top-level object initialized and gatheing point of HDF5 operations
// Revised by B. Kalafut, August 2024
// Curt Lindmark 9/1/2021
// For full license terms, see the LICENSE.md file in the root of this repository.

// release-1.12.5.10441
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

#include "MBIConstants.h"
#include "MBIMetadata.h"
#include "MBICalibration.h"
#include "MBIFrame.h"

#include <cstdio>
#include <fstream>
#include <memory>
#include <tuple>

// Do not include H5cpp.h here.
// All methods with signatures that directly reference HDF5 classes or structures should be in H5Bridge,
// which is declared as an opaque pointer here and implemented in the source file.

namespace MBISDK
{

	/// @class MBISDK::MBIFileHDF5Adapter
	/// @brief Top level object for interfacing to the MBI data API.
	/// @author Doug Bodden */
	class MBI_DLLCPP MBIFileHDF5Adapter
	{

	public:
		/// @brief HDF5 Adapter Unexpected error 
		static const int ERR_UNEXPECTED = -1;
		/// @brief HDF5 Adapter Success code 
		static const int ERR_SUCCESS = 0;
		/// @brief HDF5 Adapter Internal error 
		static const int ERR_HDF5_INTERNAL_ERROR = 1;
		/// @brief HDF5 Adapter name size constant 
		static const int NAME_SIZE = 50;
		/// @brief HDF5 Adapter Upper limit for metadata 
		static const int HDF5_METADATA_SIZE_UPPER_LIMIT = 32 * 1024 * 1024; //This is the upper limit of writing as restricted by HDF5.
											                                //It equates to 33,554,432 bytes.

		/// <summary>
		/// Construct an MBIFile for read from a file.
		/// </summary>
		/// <param name="path">Path to the file to load.</param>
		/// <param name="parent">File being opened.</param>
		MBIFileHDF5Adapter(std::shared_ptr<std::string> path, char accessMode, MBIFile *parent);

        // We need to prevent the insertion of a compiler-defined destructor where the opaque pointer to H5Bridge is incomplete
		// In the implementation file MBIFileHDF5Adapter.cpp, we generate the default destructor by
		// MBIFileHDF5Adapter::~MBIFileHDF5Adapter() = default;
		~MBIFileHDF5Adapter();

		/// <summary>
		/// Initialize an existing MBI file.
		/// </summary>
		bool Init();

#ifdef MBI_SDK_INTERNAL
		/// <summary>
		/// Initialize an blank MBI file.
		/// </summary>
		bool Create();
#endif
		/// <summary>
		/// Close the input file and free any associated resources.
		/// </summary>
		void Close();

		/// <summary>
		/// Retrieve the name of the input file (full path).
		/// </summary>
		/// <returns>std::string the path used to open the file.</returns>
		std::shared_ptr<std::string> GetFilename();
		
		/// <summary>
		/// Retrieve a start time for a frame metadata
		/// </summary>
		/// <param name="pFrameMD">Pointer to frame metadata.</param>
		/// <returns>A double from frame metadata
		double GetFrameStartTime(std::shared_ptr<FrameMetadata> pFrameMD);

		/// <summary>
		/// Retrieve a start time for a frame metadata
		/// </summary>
		/// <param name="pFrameMD">Pointer to frame metadata.</param>
		/// <returns>A double from frame metadata
		std::string GetCalibration(std::shared_ptr<FrameMetadata> pFrameMD);

		/// <summary>
		/// Check if the file has MS parameters
		/// </summary>
		bool HasScanDefinitions();

		/// <summary>
		/// Retrieve a list of intensities for a given frame
		/// </summary>
		/// <param name="frameIndex">Frame index.</param>
		/// <returns>A list of intensities for a given frame</returns>
		std::shared_ptr<std::vector<int32_t>> loadSparseSampleIntensities(int frameIndex);

		/// <summary>
		/// Retrieve a list of samples for a given frame
		/// </summary>
		/// <param name="frameIndex">Frame index.</param>
		/// <returns>A list of samples for a given frame</returns>
		std::shared_ptr<std::vector<std::pair<int64_t, int64_t>>> loadScanSampleIndexPairs(int frameIndex);

		/// <summary>
		/// Retrieve a list of sample offsets for a given frame
		/// </summary>
		/// <param name="frameIndex">Frame index.</param>
		/// <returns>A list of sample offsets for a given frame</returns>
		std::shared_ptr<std::vector<int64_t>> loadScanSampleOffsets(int frameIndex);

		/// <summary>
		/// Retrieve a list of scan index pair offsets for a given frame
		/// </summary>
		/// <param name="frameIndex">Frame index.</param>
		/// <returns>A list of scan index pair offsets for a given frame</returns>
		std::shared_ptr<std::vector<int64_t>> loadScanIndexPairOffsets(int frameIndex);

		/// <summary>
		/// Retrieve a list of scan index pair offsets for a given frame
		/// </summary>
		/// <param name="frameIndex">Frame index.</param>
		/// <returns>A list of scan index pair offsets for a given frame</returns>
		std::shared_ptr<std::vector<double>> LoadTriggerTimeStamps(int frameIndex);

		// Frame-level internal implementation stuff
		/// <summary>
		/// Retrieve a list of frame metadata for a given frame
		/// </summary>
		/// <param name="frameIndex">Frame index.</param>
		/// <param name="metadataMap">Metadata map.</param>
		/// <returns>A list of frame metadata for a given frame</returns>
		std::shared_ptr<FrameMetadata> loadFrameMetadata(int frameIndex, std::map<std::string, std::string> &metadataMap);

		/// @brief Retrieve the scan definition (MS parameters) for a given frame.
		/// @return An instance of ScanDefinition containing CEs, times, and isolation windows.
		ScanDefinition LoadScanDefinition(size_t frameIndex);

		/// @brief Retrieve a shared pointer to the scan definition (MS parameters) for a given frame.
		/// @return An instance of shared pointer to ScanDefinition containing CEs, times, and isolation windows.
		//
		// This constructs a new shared pointer, not a copy of the old one.
		std::shared_ptr<ScanDefinition> LoadScanDefinitionSP(size_t frameIndex);
		
		/// @brief Retrieve the number of explicit scan definitions present in the file.
		size_t NumScanDefinitions();

		/// <summary>
		/// Retrieve a list of retention time data for a given file
		/// </summary>
		/// <param name="rtTic">Pointer to vector for rttic data.</param>
		/// <returns>A list of retention time data for a given file</returns>
		bool LoadRtTic(std::vector<int64_t> *rtTic);

		/// <summary>
		/// Retrieve a list of arrival time data for a given file
		/// </summary>
		/// <param name="nFrameIndex">Frame index.</param>
		/// <param name="nSize">Size of at_tic list.</param>
		/// <returns>A list of arrival time data for a given file</returns>
		std::shared_ptr <std::vector<int64_t>> LoadAtTic(int nFrameIndex, int nSize);

		/// <summary>
		/// Loads global metadata information to map
		/// </summary>
		void LoadGlobalMetadataToMap(std::map<std::string, std::string>& mapMetaAll);

		/// <summary>
		/// Loads frame metadata information for a given frame to map
		/// </summary>
		/// <param name="frameIndex">Frame index.</param>
		/// <param name="mapMetaAll">Metadata map.</param>
		void LoadFrameMetadataToMap(int frameIndex, std::map<std::string, std::string>& mapMetaAll);

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

#ifdef MBI_SDK_INTERNAL
		/// <summary>
		/// Saves global metadata information to file
		/// </summary>
		/// <param name="global_meta_data_ptr">Pointer to global metadata.</param>
		/// <returns></returns>
		void SaveGlobalMetadataToFile(std::shared_ptr<GlobalMetadata> global_meta_data_ptr);

		/// <summary>
		/// Saves global metadata information to file
		/// </summary>
		/// <param name="global_meta_data_ptr">Pointer to global metadata.</param>
		/// <returns></returns>
		void SaveGlobalMetadataToMap(std::shared_ptr<GlobalMetadata> global_meta_data_ptr);

		/// <summary>
		/// Saves global metadata information to file
		/// </summary>
		/// <param name="frame_meta_data_ptr">Pointer to frame metadata.</param>
		/// <returns></returns>
		void SaveFrameMetadataToFile(int frame_index, std::shared_ptr<FrameMetadata> frame_meta_data_ptr);

		/// <summary>
		/// Saves global metadata information to file
		/// </summary>
		/// <param name="metadata_str">Metadata name.</param>
		/// <param name="frame_meta_data_ptr">Pointer to frame metadata.</param>
		/// <returns></returns>
		void SaveFrameMetadataToMap(std::string metadata_str, std::shared_ptr<FrameMetadata> frame_meta_data_ptr);

		/// <summary>
		/// Saves RT_TIC data information to file
		/// </summary>
		/// <param name="input_rt_tic_list">Rt_tic list.</param>
		/// <returns></returns>
		void SaveRtTic(std::shared_ptr<std::vector<int64_t>> input_rt_tic_list);

		/// <summary>
		/// Saves AT_TIC data information for a given frame to file
		/// </summary>
		/// <param name="input_at_tic_list">At_tic list.</param>
		/// <returns></returns>
		void SaveAtTic(int frame_index, std::shared_ptr<std::vector<int64_t>> input_at_tic_list);

		/// <summary>
		/// Saves DATA_COUNTS data information for a given frame to file
		/// </summary>
		/// <param name="frame_index">Frame index.</param>
		/// <param name="input_data_counts_list">Intensity list.</param>
		/// <returns></returns>
		void SaveDataCounts(int frame_index, std::shared_ptr<std::vector<int32_t>> input_data_counts_list);

		/// <summary>
		/// Saves DATA_POSITIONS data information for a given frame to file
		/// </summary>
		/// <param name="frame_index">Frame index.</param>
		/// <param name="input_data_positions_list">Gates list.</param>
		/// <returns></returns>
		void SaveDataPositions(int frame_index, std::shared_ptr<std::vector<std::pair<int64_t, int64_t>>> input_data_positions_list);

		/// <summary>
		/// Saves INDEX_COUNTS data information for a given frame to file
		/// </summary>
		/// <param name="frame_index">Frame index.</param>
		/// <param name="input_index_counts_list">Intensity offset list.</param>
		/// <returns></returns>
		void SaveIndexCounts(int frame_index, std::shared_ptr<std::vector<int64_t>> input_index_counts_list);

		/// <summary>
		/// Saves INDEX_POSITIONS data information for a given frame to file
		/// </summary>
		/// <param name="frame_index">Frame index.</param>
		/// <param name="input_index_positions_list">Gates offset list.</param>
		/// <returns></returns>
		void SaveIndexPositions(int frame_index, std::shared_ptr<std::vector<int64_t>> input_index_positions_list);

		/// <summary>
		/// Saves TRIGGER_TIMESTAMPS data information for a given frame to file
		/// </summary>
		/// <param name="frame_index">Frame index.</param>
		/// <param name="input_trigger_timestamps_list">Trigger timestamps list.</param>
		/// <returns></returns>
		void SaveTriggerTimeStamps(int frame_index, std::shared_ptr<std::vector<double>> input_trigger_timestamps_list);

		/// <summary>
		/// Creates data groups for saving new MBI files
		/// </summary>
		/// <returns></returns>
		void CreateGlobalGroups();

		/// <summary>
		/// Creates data groups for saving new MBI files
		/// </summary>
		/// <param name="input_frame_max">Frame maximum.</param>
		/// <returns></returns>
		void CreateFrameGroups(int input_frame_max);

		/// <summary>
		/// Creates a data group for incremental saving of a frame
		/// </summary>
		/// <param name="frame_index">The index of the new frame to be saved.</param>
		/// <returns></returns>
		void CreateFrameGroup(int frame_index);

		/// <summary>
		/// Creates data sets for saving new MBI files
		/// </summary>
		/// <returns></returns>
		void CreateFrameDataSets();

		/// <summary>
		/// Writes globalmetadata to empty file
		/// </summary>
		/// <param name="global_meta_data_ptr">Pointer to global metadata.</param>
		/// <returns></returns>
		void WriteNewGlobalMetadataToMap(std::shared_ptr<GlobalMetadata> global_meta_data_ptr);

		/// <summary>
		/// Writes new frame metadata information to file
		/// </summary>
		/// <param name="frame_meta_data_ptr">Pointer to frame metadata.</param>
		/// <param name="frame_index">Frame index.</param>
		/// <returns></returns>
		void WriteNewFrameMetadataToFile(std::shared_ptr<FrameMetadata> frame_meta_data_ptr, int frame_index);

		/// <summary>
		/// Writes new frame metadata information to file
		/// </summary>
		/// <param name="metadata_str">Metadata string.</param>
		/// <param name="frame_meta_data_ptr">Pointer to frame metadata.</param>
		/// <returns></returns>
		void WriteNewFrameMetadataToMap(std::string metadata_str, std::shared_ptr<FrameMetadata> frame_meta_data_ptr);

		/// <summary>
		/// Writes new RT_TIC data information to file
		/// </summary>
		/// <param name="input_rt_tic_list">Rt_tic list.</param>
		/// <returns></returns>
		void WriteNewRtTic(std::shared_ptr<std::vector<int64_t>> input_rt_tic_list);

		/// <summary>
		/// Writes ATTic data to empty file
		/// </summary>
		/// <param name="frame_index">Frame index.</param>
		/// <param name="input_at_tic_list">At_tic list.</param>
		/// <returns></returns>
		void WriteNewATTic(int frame_index, std::shared_ptr<std::vector<int64_t>> input_at_tic_list);

		/// <summary>
		/// Writes DataCounts data to empty file
		/// </summary>
		/// <param name="frame_index">Frame index.</param>
		/// <param name="input_data_counts_list">Intensities list.</param>
		/// <returns></returns>
		void WriteNewDataCounts(int frame_index, std::shared_ptr<std::vector<int32_t>> input_data_counts_list);

		/// <summary>
		/// Writes DataPositions data to empty file
		/// </summary>
		/// <param name="frame_index">Frame index.</param>
		/// <param name="input_data_positions_list">Gates list.</param>
		/// <returns></returns>
		void WriteNewDataPositions(int frame_index, std::shared_ptr<std::vector<std::pair<int64_t, int64_t>>> input_data_positions_list);

		/// <summary>
		/// Writes IndexCount data to empty file
		/// </summary>
		/// <param name="frame_index">Frame index.</param>
		/// <param name="input_index_counts_list">Intensity offset list.</param>
		/// <returns></returns>
		void WriteNewIndexCounts(int frame_index, std::shared_ptr<std::vector<int64_t>> input_index_counts_list);

		/// <summary>
		/// Writes IndexPosition data to empty file
		/// </summary>
		/// <param name="frame_index">Frame index.</param>
		/// <param name="input_index_positions_list">Gates offset list.</param>
		/// <returns></returns>
		void WriteNewIndexPositions(int frame_index, std::shared_ptr<std::vector<int64_t>> input_index_positions_list);

		/// <summary>
		/// Writes TriggerTimeStamps data to empty file
		/// </summary>
		/// <param name="frame_index">Frame index.</param>
		/// <param name="input_trigger_timestamps_list">Trigger timestamp list.</param>
		/// <returns></returns>
		void WriteNewTriggerTimeStamps(int frame_index, std::shared_ptr<std::vector<double>> input_trigger_timestamps_list);

#endif
	private:

        // Thin HDF5 wrapper
		class H5Bridge;

        // Opaque pointer to HDF5 wrapper
	    std::unique_ptr<H5Bridge> pH5Bridge;

		MBIFile* parent_file;
		// file-oriented members
		std::shared_ptr<std::string> input_file_path;
		char accessMode;

		void setErrorCode(int local_error_code);
		std::string error_message;
		int error_code;
		bool has_rt_tic;
		
		std::vector<size_t> scanDefinitionMapping; 
		size_t numFrames;
	};
}
