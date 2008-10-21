rmdir /S /Q doc\uapi_cpp
del doxygen.log

mkdir doc
mkdir doc\uapi_cpp

doxygen uapi.doxygen
