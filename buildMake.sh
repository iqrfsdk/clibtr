project=clibtr

#expected build dir structure
buildexp=build/Unix_Makefiles

currentdir=$PWD
builddir=./${buildexp}

mkdir -p ${builddir}

#get path to cutils libs
cutils=../cutils/${buildexp}
pushd ${cutils}
cutils=$PWD
popd

#get path to clibcdc libs
clibcdc=../clibcdc/${buildexp}
pushd ${clibcdc}
clibcdc=$PWD
popd

#get path to clibspi libs
clibspi=../clibspi/${buildexp}
pushd ${clibspi}
clibspi=$PWD
popd

#launch cmake to generate build environment
pushd ${builddir}
cmake -G "Unix Makefiles" -Dcutils_DIR:PATH=${cutils}  -Dclibcdc_DIR:PATH=${clibcdc} -Dclibspi_DIR:PATH=${clibspi} ${currentdir}
popd

#build from generated build environment
cmake --build ${builddir}

