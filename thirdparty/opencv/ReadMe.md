# Building OpenCV

- Disable tests
- Disable perf tests
- Disable cuDNN
- Enable CUDA
- Enable World
- Add extra modules path (opencv_contrib)
- Build python can be disabled


When running, some CUDA DLL's are required.
Currently we manually add these to our target directory from https://github.com/cherry-imaging/cuda (This is a security risk)
They are added to ../cuda