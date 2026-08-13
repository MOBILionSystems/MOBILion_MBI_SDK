#include <iostream>
#include <fstream>
#include "MBIFile.h"

int main(int argc, char *argv[])
{
    std::cout << "Mobilion SDK data access demonstration." << std::endl;
    std::cout << "Exports frame data." << std::endl;

    if (argc < 3)
    {
        std::cout << "Nothing to do.";
        return 1;
    }
    
    char * fileName = argv[1];
    int frameIndex = std::stoi(argv[2]);

    //std::string testFile="2024-03-05 13.30.16-20240305MJ_NISTmAb_2pmolCE25_03_SIFF25_0.mbi";

    // Open a file 
	MBISDK::MBIFile f = MBISDK::MBIFile(fileName);
	f.Init();
    
    // MBIFile does not throw an error if the file is not found.
    if (f.GetErrorCode() == 1)
    {
        std::cout << "File not found." << std::endl;
        return 0;
    } else if (f.GetErrorCode() != 0)
    {
        std::cout << "Unanticipated error at file open.  Code: " << f.GetErrorCode()  << std::endl;
        return 0;
    }
    
    std::cout << "SDK version:" << f.GetVersion() <<std::endl;

    // Read a simple property
    std::cout << "Number of frames: " << f.GetNumFrames() << std::endl;
    std::cout << "Requested frame index:" << frameIndex << std::endl;

    // Frame indices are 1-based: valid range is 1 .. GetNumFrames() inclusive.
    // GetFrame() returns a null pointer outside that range.
    if (frameIndex < 1 || frameIndex > f.GetNumFrames())
    {
        std::cout << "Frame index out of bounds.  Valid range is 1 to "
                  << f.GetNumFrames() << "." << std::endl;
        return 0;
    }


    // MBI CCS calibration?
    std::cout << "Read in the CCS calibration." << std::endl;
    std::string rawCalibration = f.getMetaDataItem(MBISDK::MBIAttr::GlobalKey::CAL_CCS);
    std::cout << "CCS Calibration string has length " << rawCalibration.length() << std::endl;
    // Nothing here in the current file.

    
    // Read in a frame and a scan:
	std::shared_ptr<MBISDK::Frame> frm1 = f.GetFrame(frameIndex);
    if (frm1->IsCollisionEnergyValid())
    {
        std::cout << "Frame " << frameIndex << " was collected with collision energy " << frm1->GetCollisionEnergy() << std::endl;
    }
    std::vector<size_t> nonZeroScans = frm1->GetNonZeroScanIndices();
    std::cout << "Frame " << frameIndex << " has " << nonZeroScans.size() << " nonzero scans." << std::endl;
    if (nonZeroScans.size() == 0)
    {
        std::cout << "Frame " << frameIndex << " has no nonempty scans." << std::endl;
        return 0;
    }


    std::cout << "Number of nonempty scans is " << nonZeroScans.size() << std::endl;
    std::vector<size_t>* intensity = new std::vector<size_t>();
    std::vector<double>* mzs = new std::vector<double>();
    bool scan = frm1->GetScanDataMzIndexedSparse(nonZeroScans[0], mzs, intensity);
    std::cout << "This has " << mzs->size() << " data entries. " << std::endl;
    for (int i = 0; i < mzs-> size(); i++)
    {
        std::cout << "Entry " << i << " mass " << (*mzs)[i] << " counts " << (*intensity)[i] << std::endl;
    }

    //Test drive the mass calibration
    MBISDK::TofCalibration tofCal = f.GetCalibration();
    double testMz = 622.0;
    size_t indexAtTestValue= tofCal.MzToIndex(testMz);
    std::cout << "Index nearest to mass " << testMz << " is " << indexAtTestValue << std::endl;

    std::cout << std::endl;
    
    size_t indexIncrement = 1000;
    size_t higherIndex = indexAtTestValue + indexIncrement;
    double massAtHigherIndex = tofCal.IndexToMz(higherIndex);
    std::cout << indexIncrement << " TOF MS samples past this corresponds to m/z = " << massAtHigherIndex << "." << std::endl;


    std::cout << "********************************************************************************" << std::endl;
    std::cout << std::endl;
    
    std::cout << "Access as a CSR Array" << std::endl;

    MBISDK::CSRArray<int32_t> frameAsCSR = frm1->GetFrameDataAsCSRArray();

    std::cout << "data has " << frameAsCSR.data.size() << " entries." << std::endl;
    std::cout << "indices has " << frameAsCSR.indices.size() << " entries." << std::endl;
    std::cout << "indptr has " << frameAsCSR.indptr.size() << " entries." << std::endl;
    
    std::cout << std::endl;
    std::cout << "Access as a CSR Array" << std::endl;
    MBISDK::COOArray<int32_t> frameAsCOO = frm1->GetFrameDataAsCOOArray();

    std::cout << "data has " << frameAsCOO.data.size() << " entries." << std::endl;
    std::cout << "rowIndices has " << frameAsCOO.rowIndices.size() << " entries." << std::endl;
    std::cout << "columnIndices has " << frameAsCOO.columnIndices.size() << " entries." << std::endl;

    std::cout << std::endl;
    std::cout << "Writing to COO_test.csv" << std::endl;

    std::ofstream outfile;
    outfile.open("COO_test.csv");
    for (size_t i=0; i<frameAsCOO.nnz; ++i)
    {
        outfile << frameAsCOO.rowIndices[i] << ",";
        outfile << frameAsCOO.columnIndices[i] << ",";
        outfile << frameAsCOO.data[i] << std::endl;
    }

    std::cout << "*********************************************************************************************" << std::endl;
    std::cout << std::endl;

    std::cout << "Getting a MassSpectrum object, for the first nonzero scan" << std::endl;

    MBISDK::MassSpectrum firstnonzero = frm1->GetMassSpectrum(nonZeroScans[0]);

    std::cout << "This has " << firstnonzero.indices.size() << " entries, at indices " << std::endl;
    for (int i = 0; i < firstnonzero.indices.size(); ++i)
    {
        std::cout << firstnonzero.indices[i] << " ";
    }
    std::cout << std::endl;
    std::cout << "*********************************************************************************************" << std::endl;
    
   
    f.Close();
}