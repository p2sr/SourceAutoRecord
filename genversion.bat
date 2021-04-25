for /f "tokens=* usebackq" %%x in (`git describe --tags`) do (set version=%%x)
echo #define SAR_VERSION "%version%">Version.hpp
