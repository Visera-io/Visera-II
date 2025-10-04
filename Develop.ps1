cmake -S . -B cmake-build-develop -DCMAKE_BUILD_TYPE=Develop -DVISERA_APP=AlohaVisera;
cmake --build cmake-build-develop --config Develop;
.\cmake-build-develop\Apps\AlohaVisera\Develop\AlohaVisera.exe