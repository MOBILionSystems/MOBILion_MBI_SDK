# MOBILion MBI SDK

C++ SDK for reading MOBILion's MBI file format — the ion mobility / mass spectrometry data files
produced by the MOBIE® EyeOn acquisition system.

Release `1.13.1`

Principal maintainer: Bennett Kalafut &lt;bennett.kalafut@mobilionsystems.com&gt;

The SDK exposes metadata, raw and calibrated mass spectral data, ion mobility (arrival time) data,
TOF mass calibrations, and CCS calibrations. MBI files are HDF5-based; the SDK handles HDF5 access
internally, so no separate HDF5 dependency is required.

---

## Contents

| Path | Description |
| --- | --- |
| `include/` | Public C++ headers |
| `lib/win-x64/MBI_SDK.dll` | Runtime library (Windows x64) |
| `lib/win-x64/MBI_SDK.lib` | Import library for linking |
| `lib/win-x64/swig-python/` | SWIG-generated Python bindings (`mbisdk.py`, `_mbisdk.pyd`) |
| `lib/linux-x64/` | Linux x86-64 packages (`.deb` and `.rpm`) |
| `doc/html/index.html` | Full Doxygen API reference |
| `LICENSE.md` | MOBILion Software Use Agreement |
| `COPYING.txt` | HDF5 license (third-party component) |

### Headers

| Header | Contents |
| --- | --- |
| `MBIFile.h` | `MBIFile` — top-level file object. Include this; it pulls in the rest. |
| `MBIFrame.h` | `Frame`, `MassSpectrum` — frame data access |
| `MBISparse.h` | `COOArray<T>`, `CSRArray<T>`, `CSRIMMSSpectrum<T>`, `COOIMMSSpectrum<T>` sparse containers |
| `MBICalibration.h` | `TofCalibration`, `EyeOnCcsCalibration`, and related calibration classes |
| `MBIConstants.h` | Metadata key names (`MBIAttr::GlobalKey`, etc.) and enums |
| `MBIMetadata.h` | `Metadata`, `GlobalMetadata`, `FrameMetadata` |
| `MBIScanDefinition.h` | `ScanDefinition` — CE ramps and isolation windows. Groundwork for future DIA support; not required for current data. |
| `MBIFileHDF5Adapter.h` | Low-level HDF5 adapter (not normally called directly) |

Full API documentation is in `doc/html/index.html`.

---

## Requirements

**Windows**

- Windows x64
- MSVC toolset v143 (Visual Studio 2022) or compatible
- C++14 or later

**Linux**

- x86-64, glibc-based distribution
- GCC with C++17 support (the shared library is built with `-std=c++17`)

---

## Setting up a project (Windows)

1. Copy `include/` into your project (or add it to your include path).
2. Add `MBI_SDK.lib` to your linker inputs, and `lib/win-x64` to your library directories.
3. Copy `MBI_SDK.dll` next to your executable (or somewhere on `PATH`) so it can be found at runtime.

Visual Studio project settings (x64 configuration):

- **C/C++ → General → Additional Include Directories:** `include;`
- **Linker → General → Additional Library Directories:** path to `lib\win-x64`
- **Linker → Input → Additional Dependencies:** `MBI_SDK.lib`

Command line equivalent:

```
cl /std:c++14 /EHsc /I include myprogram.cpp /link /LIBPATH:lib\win-x64 MBI_SDK.lib
```

Do **not** define `MBI_EXPORTS` — that macro is for building the SDK itself. Client code picks up
`__declspec(dllimport)` automatically.

---

## Setting up a project (Linux)

Linux x86-64 packages are in `lib/linux-x64/`, as both a Debian and an RPM package.

> **Version note.** These packages are the **1.13.1** release. They are labelled 1.13.0 — in the
> filename, in the package metadata, and in the `libmbisdk.so.1.13.0` soname file — because the
> Linux packaging job did not pick up the release version stamp. The binaries themselves are 1.13.1
> and expose the full 1.13.1 API. Correctly versioned packages will follow in the next build.

Install:

```bash
sudo dpkg -i mbisdk-dev_1.13.0_amd64.deb     # Debian / Ubuntu
sudo rpm  -i mbisdk-dev-1.13.0-1.x86_64.rpm  # RHEL / Fedora / SUSE
```

Installation prompts for acceptance of the license agreement. For unattended installs, accept it up
front:

```bash
sudo MBISDK_ACCEPT_LICENSE=yes dpkg -i mbisdk-dev_1.13.0_amd64.deb
```

The package installs:

| Path | Contents |
| --- | --- |
| `/usr/include/mbisdk/` | Public headers |
| `/usr/lib/libmbisdk.so` | The SDK, with `libmbisdk.so.1` soname |
| `/usr/lib/libhdf5.so.200`, `libhdf5_cpp.so.200`, `libtbb.so.2` | Bundled dependencies |
| `/usr/share/doc/mbisdk-dev/` | `LICENSE.txt` and `COPYING.txt` |

`ldconfig` runs on install, so no further linker configuration is needed.

Compiling against it:

```bash
g++ -std=c++17 -DLINUX=1 -I/usr/include/mbisdk myprogram.cpp -lmbisdk -o myprogram
```

**`-DLINUX=1` is required.** The headers select their export macro on it:

```cpp
#ifdef LINUX
#define MBI_DLLCPP
#else
#define MBI_DLLCPP __declspec(dllimport)
#endif
```

Without it the headers fall through to the MSVC `__declspec` path and will not compile under GCC.

The API is identical to Windows — everything in this README applies unchanged, apart from the
Python bindings, which are currently shipped for Windows only.

---

## Quick start

```cpp
#include <iostream>
#include "MBIFile.h"

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cout << "Usage: readmbi <file.mbi>" << std::endl;
        return 1;
    }

    // Open and initialize.  Construction alone does not read the file.
    MBISDK::MBIFile f = MBISDK::MBIFile(argv[1]);
    f.Init();

    // MBIFile does not throw on a missing or bad file -- always check the error code.
    if (f.GetErrorCode() == MBISDK::MBIFile::ERR_FILE_NOT_FOUND)
    {
        std::cout << "File not found." << std::endl;
        return 1;
    }
    else if (f.GetErrorCode() != MBISDK::MBIFile::ERR_SUCCESS)
    {
        std::cout << "Error at file open.  Code: " << f.GetErrorCode()
                  << " " << f.GetErrorMessage() << std::endl;
        return 1;
    }

    std::cout << "SDK version: " << f.GetVersion() << std::endl;
    std::cout << "Number of frames: " << f.GetNumFrames() << std::endl;

    f.Close();
    return 0;
}
```

---

## Data model

An MBI file is a sequence of **frames**. Each frame is one point in retention time, and holds a
**sparse 2-D array** of ion counts indexed by (ion mobility arrival-time bin, TOF sample index).
A single row of that array — one arrival-time bin — is a mass spectrum, referred to as a *scan*.

- Frame indices run from `0` to `GetNumFrames() - 1`.
- The data is genuinely sparse: most cells are zero, and many rows are entirely empty. The SDK
  stores and returns it in sparse form, and nothing is decompressed into a dense array unless you
  explicitly ask for it.
- TOF sample indices are converted to m/z with a `TofCalibration`; arrival-time bins are converted
  to drift time (and optionally CCS) with the frame's arrival-bin accessors and the CCS calibration.

**Read whole frames as sparse arrays.** `GetFrameDataAsCSRArray()` /
`GetFrameDataAsCOOArray()` are the primary access path and the one to reach for by default. They
pull the frame in a single call, in the layout the file already uses, and hand you something that
maps directly onto SciPy-style sparse matrices, GPU kernels, or your own vectorized code. Scan-level
accessors exist for cases where you truly want one arrival-time bin, but looping them over a frame
does far more work than the array call and is the most common performance mistake made with this
SDK.

---

## Common tasks

### Reading a frame as a sparse array

Rows are arrival-time bins, columns are TOF sample indices, values are `int32_t` ion counts.

Compressed sparse row — compact, and the right choice for elementwise arithmetic, row slicing, or iteration of mass spectra in order:

```cpp
std::shared_ptr<MBISDK::Frame> frame = f.GetFrame(frameIndex);

MBISDK::CSRArray<int32_t> csr = frame->GetFrameDataAsCSRArray();

std::cout << csr.nnz << " nonzero points in a "
          << csr.nRows << " x " << csr.nColumns << " array" << std::endl;

// Row (arrival-time bin) r spans [indptr[r], indptr[r+1]) in data/indices.
for (size_t r = 0; r + 1 < csr.indptr.size(); ++r)
{
    for (size_t k = csr.indptr[r]; k < csr.indptr[r + 1]; ++k)
    {
        size_t  tofIndex  = csr.indices[k];
        int32_t intensity = csr.data[k];
        // ...
    }
}

int32_t frameTotal = csr.Sum();
```

Coordinate form — a flat triplet list, convenient for export, filtering, or feeding peak pickers:

```cpp
MBISDK::COOArray<int32_t> coo = frame->GetFrameDataAsCOOArray();

for (size_t i = 0; i < coo.nnz; ++i)
{
    size_t  arrivalBin = coo.rowIndices[i];
    size_t  tofIndex   = coo.columnIndices[i];
    int32_t intensity  = coo.data[i];
    // ...
}
```

Both `CSRArray<T>` and `COOArray<T>` carry `nRows`, `nColumns`, `nnz`, `isZeroPadded`, and a `Sum()`
convenience method. Each getter has an overload taking `bool padWithZeroes` if you need explicit
zeros retained in the returned structure.

To get the frame with a calibrated m/z axis attached, use `GetFrameIMMSSpectrumAsCSR()` or
`GetFrameIMMSSpectrumAsCOO()`. These return `CSRIMMSSpectrum<int32_t>` / `COOIMMSSpectrum<int32_t>`,
which are the corresponding sparse arrays plus an `mz` vector:

```cpp
MBISDK::CSRIMMSSpectrum<int32_t> immsCsr = frame->GetFrameIMMSSpectrumAsCSR();
// immsCsr.data / .indices / .indptr as above, plus immsCsr.mz

MBISDK::COOIMMSSpectrum<int32_t> immsCoo = frame->GetFrameIMMSSpectrumAsCOO();
// immsCoo.data / .rowIndices / .columnIndices as above, plus immsCoo.mz
```

If you would rather write into your own buffers than take the returned structure, use
`GetFrameDataAsCSRComponents(&data, &indices, &indptr)`.

### Scan-level access

Use these when you want one specific arrival-time bin — not as a way to traverse a frame.

```cpp
// Which rows are non-empty
std::vector<size_t> nonZeroScans = frame->GetNonZeroScanIndices();

// One scan as a MassSpectrum (indices, mz, intensities, nnz)
MBISDK::MassSpectrum spectrum = frame->GetMassSpectrum(nonZeroScans[0]);

// Or into your own vectors
std::vector<double> mzs;
std::vector<size_t> intensity;
frame->GetScanDataMzIndexedSparse(nonZeroScans[0], &mzs, &intensity);
```

TOF-indexed (`GetScanDataToFIndexedSparse`) and dense (`GetScanDataMzIndexedDense`,
`GetScanDataToFIndexedDense`) variants are available. `GetScanSummationToFIndexedDense()` sums a
range of scans without materializing them individually.

### Mass calibration

```cpp
MBISDK::TofCalibration tofCal = f.GetCalibration();

size_t index = tofCal.MzToIndex(622.0);        // m/z -> TOF sample index
double mz    = tofCal.IndexToMz(index + 1000); // TOF sample index -> m/z

// Vectorized form for a whole scan
std::vector<double> mzAxis = tofCal.IndexToMz(spectrum.indices);
```

`Frame::GetCalibration()` returns the calibration in effect for that frame, which is what you want
for files where the calibration varies.

### CCS calibration

```cpp
if (f.HasCCSCalibration())
{
    // Prefer the Owned variant: it returns the correct concrete type
    // (NonReducedEyeOnCcsCalibration when the stored gas mass is 0).
    std::unique_ptr<MBISDK::EyeOnCcsCalibration> ccsCal = f.GetEyeOnCCSCalibrationOwned();

    double ccs   = ccsCal->ArrivalTimeToCCS(arrivalTimeMs, ionMz);       // assumes Z = 1
    double ccs3p = ccsCal->ArrivalTimeToCCS(arrivalTimeMs, ionMz, 3);    // charge state 3

    double at = ccsCal->CCSToArrivalTime(ccs, ionMz);                    // inverse
}
```

Notes:

- CCS coefficients (`GetCCSCoefficients()` / `SetCCSCoefficients()`) are ordered lowest-order first,
  with the constant term at index 0.
- `GetEyeOnCCSCalibration()` (returning by value) is retained for backward compatibility but slices
  derived types; `GetEyeOnCCSCalibrationOwned()` is the correct choice for new code. The two will be
  consolidated in 2.0.
- The `ArrivalTimeToCCS(int frame_index)` overload is deprecated — it hard-couples the calibration to
  an open parent `MBIFile`. That responsibility moves to `Frame` in 2.0.

### Metadata

Global metadata is addressed by key; the key names are constants in `MBIAttr::GlobalKey`.

```cpp
std::string rawCcsCal = f.getMetaDataItem(MBISDK::MBIAttr::GlobalKey::CAL_CCS);
int    someInt    = f.getMetaDataItemInt("key-name");
double someDouble = f.getMetaDataItemDouble("key-name");

std::shared_ptr<MBISDK::GlobalMetadata> global = f.GetGlobalMetaData();
const std::map<std::string, std::string>& all = global->ReadAll();
```

Per-frame metadata is reached with `f.GetFrameMetadata(frameIndex)` or
`frame->getFrameMetaDataItem(key)`.

### File-level summaries

These avoid loading frame data when you only need an overview:

```cpp
f.GetAcquisitionTimestamp();
f.GetExperimentType();          // MS1, AIF, SIFF_MAF, MIFF_MAF
f.GetIonPolarity();
f.GetRetentionTimes();          // shared_ptr<vector<double>>
f.GetFrameMSLevels();
f.GetFrameCollisionEnergies();
f.GetScanCounts();
f.GetFrameTIC();                // per-frame total ion current, shared_ptr<vector<int64_t>>
f.GetAverageIMSamplingPeriod();
f.GetMaxNumScansPerFrame();
```

### Fragmentation and scan definitions

Collision energy for a fragmentation frame:

```cpp
if (frame->IsCollisionEnergyValid())
{
    std::cout << "CE: " << frame->GetCollisionEnergy() << std::endl;
}
```

### Memory management

Frame data is cached once loaded. For long files, release frames you are finished with:

```cpp
f.UnloadFrame(frameIndex);   // free the frame's data
f.UncacheFrame(frameIndex);  // drop the cached frame object
```

Call `f.Close()` when done.

---

## Python bindings

SWIG-generated Python bindings ship for Windows in `lib/win-x64/swig-python/`, as `mbisdk.py` and
`_mbisdk.pyd`.
Put that directory on `sys.path` (with `MBI_SDK.dll` findable) and the C++ API is available
essentially unchanged:

```python
import mbisdk

f = mbisdk.MBIFile("example.mbi")
f.Init()
if f.GetErrorCode() != 0:
    raise RuntimeError(f.GetErrorMessage())

print(f.GetVersion(), f.GetNumFrames())

frame = f.GetFrame(0)
csr = frame.GetFrameDataAsCSRArray()
coo = frame.GetFrameDataAsCOOArray()

f.Close()
```

The same guidance applies: pull whole frames as sparse arrays rather than iterating scans. The
returned index and data vectors are SWIG wrappers over `std::vector`, so convert them to NumPy
arrays once (`numpy.asarray(...)`) rather than indexing them element-by-element in Python.

---

## Error handling

`MBIFile` reports failures through error codes rather than exceptions. Check `GetErrorCode()` after
`Init()` and after any operation that may fail; `GetErrorMessage()` gives a description.

| Constant | Value | Meaning |
| --- | --- | --- |
| `ERR_UNEXPECTED` | -1 | Unexpected error |
| `ERR_SUCCESS` | 0 | Success |
| `ERR_FILE_NOT_FOUND` | 1 | File not found |
| `ERR_HDF5_FILE_ERROR` | 2 | HDF5 read error |
| `ERR_FILE_NOT_INITIALIZED` | 3 | `Init()` not called or failed |
| `ERR_METADATA_NOT_LOADED` | 101 | Metadata not loaded |
| `ERR_FRAME_NOT_LOADED` | 102 | Frame data not loaded |
| `ERR_DOUBLE_LOAD` | 103 | Frame loaded twice |
| `ERR_BAD_FRAME_INDEX` | 201 | Frame index out of range |
| `ERR_BAD_SCAN_INDEX` | 202 | Scan index out of range |
| `ERR_ITEM_MISSING` | 301 | Requested item not present |
| `ERR_OPERATION_NOT_SUPPORTED` | 401 | Operation not supported |
| `ERR_ZERO_FRAMES` | 501 | File contains no frames |

---

## Further documentation

The complete API reference, generated with Doxygen, is in [`doc/html/index.html`](doc/html/index.html).

A worked demonstration program — opening a file, pulling frames as CSR and COO arrays, exercising the
mass calibration, and exporting a frame to CSV in coordinate format — is available in the
`MBISDK_demo` example project.

---

## Maintainer

Principal maintainer: **Bennett Kalafut** — <bennett.kalafut@mobilionsystems.com>

Note that MBI has no obligation to support the SDK under the terms of the license below; direct
questions and bug reports here on a best-effort basis.

---

## License

Use of this SDK is governed by the MOBILion Software Use Agreement in [LICENSE.md](LICENSE.md).
Notably, the SDK is provided for reading MBI data files; redistribution is limited to object-code
form for non-commercial third-party use, and reverse engineering is prohibited. Read the agreement
in full before distributing anything built against these libraries.

Third-party components: this SDK incorporates HDF5, whose license terms are in
[COPYING.txt](COPYING.txt).

Copyright © 2026 MOBILion Systems, Inc. All rights reserved.
