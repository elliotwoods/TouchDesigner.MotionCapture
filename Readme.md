# Introduction

A library of motion capture operators for Touch Designer.

These operators triangulate bright features in corresponding camera images (e.g. retroreflective markers).

# Warnings

* Do not unload DLL's from the chain unless all downstream OP's have been unloaded first. This is because any OP may contain a `shraed_ptr` to instances created by a previous OP. Unloading the DLL will cause any objects pointed to become invalid.