# Introduction

A library of motion capture operators for Touch Designer.

These operators triangulate bright features in corresponding camera images (e.g. retroreflective markers).

# Installation

## Requirements

* CUDA 10.1 runtime : https://developer.nvidia.com/cuda-10.1-download-archive-update2?target_os=Windows&target_arch=x86_64
* Ximea SDK : https://www.ximea.com/support/wiki/apis/XIMEA_Windows_Software_Package

# Warnings

* You may experience crashes on unload during development, i.e. when loading operators as seperate DLL's (e.g. during development/debugging), note that `shared_ptr` objects are passed between the DLL's and may be invalidated during unload.
