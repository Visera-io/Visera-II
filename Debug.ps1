cmake -S . -B cmake-build-debug -DCMAKE_BUILD_TYPE=Debug -DVISERA_APP=AlohaVisera;
cmake --build cmake-build-debug --config Debug;
.\cmake-build-debug\Apps\AlohaVisera\Debug\AlohaVisera.exe