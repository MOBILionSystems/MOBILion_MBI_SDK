# MBISDK_demo

A worked example of reading an MBI file with the MOBILion MBI SDK.

## What it demonstrates

- Opening a file and checking the error code (`MBIFile` reports failures by code, not exception)
- Reading global metadata and the CCS calibration string
- Listing the non-empty scans in a frame
- Pulling a whole frame as a `CSRArray<int32_t>` and as a `COOArray<int32_t>` — the primary
  data access path
- Exporting a frame to `COO_test.csv` in coordinate (row, column, intensity) form
- Round-tripping m/z through `TofCalibration::MzToIndex` / `IndexToMz`
- Retrieving a single scan as a `MassSpectrum`

## Building

Open `MBISDK_demo.sln` in Visual Studio 2022 and build the **x64** configuration. The project
already points at the SDK in this repository:

- Include directories: `..\..\include`
- Library directories: `..\..\lib\win-x64`
- Linker input: `MBI_SDK.lib`

A post-build step copies `MBI_SDK.dll` next to the executable, so it runs without further setup.

Only the x64 configurations are usable — the SDK ships as a 64-bit library.

## Running

```
MBISDK_demo.exe <path-to-file.mbi> <frame-index>
```

For example:

```
MBISDK_demo.exe sample.mbi 1
```

Frame indices are **1-based**: the valid range is `1` to `GetNumFrames()` inclusive. The program
writes `COO_test.csv` into the working directory.

No sample `.mbi` file is included; supply one of your own.
