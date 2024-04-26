-- LMod module file for HCL

-- CMAKE_INSTALL_PREFIX: /usr/workspace/haridev/hcl-2/install-test
-- CMAKE_BUILD_TYPE: Debug
-- C Compiler: /usr/tce/bin/gcc
-- C FLAGS:  -fPIC -Wall -Wextra -pedantic -Wno-unused-parameter -Wno-deprecated-declarations
-- C FLAGS_DEBUG: -g
-- C FLAGS_RELWITHDEBINFO: -O2 -g -DNDEBUG
-- C FLAGS_RELEASE: -O3 -DNDEBUG
-- CXX Compiler: /usr/tce/bin/g++
-- CXX FLAGS:  -fPIC -Wall -Wextra -pedantic -Wno-unused-parameter -Wnon-virtual-dtor -Wno-deprecated-declarations
-- CXX FLAGS_DEBUG: -g
-- CXX FLAGS_RELWITHDEBINFO: -O2 -g -DNDEBUG
-- CXX FLAGS_RELEASE: -O3 -DNDEBUG
-- HCL_GNU_LINUX: TRUE
-- HCL_HAS_DOXYGEN: 
-- HCL_HAS_STD_FILESYSTEM: TRUE
-- HCL_HAS_STD_FSTREAM_FD: TRUE

help(
[[
DYnamic and Asynchronous Data streamliner (HCL) version .
]])

whatis("Package: HCL")
whatis("Version: ")
whatis("Description: DYnamic and Asynchronous Data streamliner (HCL).")
whatis("URL: https://github.com/flux-framework/hcl")
whatis("CMAKE_INSTALL_PREFIX: /usr/workspace/haridev/hcl-2/install-test")
whatis("CMAKE_BUILD_TYPE: Debug")
whatis("C Compiler: /usr/tce/bin/gcc")
whatis("C FLAGS:  -fPIC -Wall -Wextra -pedantic -Wno-unused-parameter -Wno-deprecated-declarations")
whatis("C FLAGS_DEBUG: -g")
whatis("C FLAGS_RELWITHDEBINFO: -O2 -g -DNDEBUG")
whatis("C FLAGS_RELEASE: -O3 -DNDEBUG")
whatis("CXX Compiler: /usr/tce/bin/g++")
whatis("CXX FLAGS:  -fPIC -Wall -Wextra -pedantic -Wno-unused-parameter -Wnon-virtual-dtor -Wno-deprecated-declarations")
whatis("CXX FLAGS_DEBUG: -g")
whatis("CXX FLAGS_RELWITHDEBINFO: -O2 -g -DNDEBUG")
whatis("CXX FLAGS_RELEASE: -O3 -DNDEBUG")
whatis("HCL_GNU_LINUX: TRUE")
whatis("HCL_HAS_DOXYGEN: ")
whatis("HCL_HAS_STD_FILESYSTEM: TRUE")
whatis("HCL_HAS_STD_FSTREAM_FD: TRUE")

prepend_path("PATH","/usr/workspace/haridev/hcl-2/install-test/bin")
prepend_path("LD_LIBRARY_PATH","/usr/workspace/haridev/hcl-2/install-test/lib64")

pushenv("HCL_DIR","/usr/workspace/haridev/hcl-2/install-test/")
