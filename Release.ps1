cmake -S . -B cmake-build-release -DCMAKE_BUILD_TYPE=Release -DVISERA_APP=AlohaVisera;
cmake --build cmake-build-release --config Release;
.\cmake-build-release\Apps\AlohaVisera\Release\AlohaVisera.exe