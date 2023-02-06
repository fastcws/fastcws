mkdir build install output
cmake -B build -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -DCMAKE_INSTALL_PREFIX=fastcws-windows-%BUILD_TYPE%-%GITHUB_SHA:~0,7%
cmake --build build --config %BUILD_TYPE%
cmake --install build --config %BUILD_TYPE%
copy LICENSE fastcws-windows-%BUILD_TYPE%-%GITHUB_SHA:~0,7%\
7z a -tzip output\fastcws-windows-%BUILD_TYPE%-%GITHUB_SHA:~0,7%.zip fastcws-windows-%BUILD_TYPE%-%GITHUB_SHA:~0,7%

