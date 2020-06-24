# Introduction

A library of motion capture operators for Touch Designer.

These operators triangulate bright features in corresponding camera images (e.g. retroreflective markers).

# Warnings

* You may experience crashes on unload during development, i.e. when loading operators as seperate DLL's (e.g. during development/debugging), note that `shared_ptr` objects are passed between the DLL's and may be invalidated during unload.