g++ -g -L../../../clapack-3.2.1 -L../../../clapack-3.2.1/f2clibs -L../../../cblas/lib -L../../../blas/lib -L..\..\lib\i686-MINGW32_NT-6.1 -static-libgcc -static-libstdc++ -shared -o bavieca-0014.dll ..\..\obj\i686-MINGW32_NT-6.1\api\BaviecaNET.o ..\..\obj\i686-MINGW32_NT-6.1\api\BaviecaAPI.o ..\..\obj\i686-MINGW32_NT-6.1\api\ConfigurationBavieca.o -lcommon -lole32 -s -Wl,-add-stdcall-alias,--output-def,bavieca-0014.def,--subsystem,windows,--out-implib,bavieca-0014.lib
cp bavieca-0014.dll E:\github\InteractiveBooks\app\aston-wpf\BaviecaDLLTest\shared
cp bavieca-0014.dll E:\github\InteractiveBooks\app\aston-wpf\Deployment
cp bavieca-0014.dll E:\github\InteractiveBooks\app\aston-wpf\Bavieca
cp bavieca-0014.dll E:\github\RAN\RAN