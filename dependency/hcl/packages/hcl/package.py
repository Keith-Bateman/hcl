# Copyright 2013-2020 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)

# ----------------------------------------------------------------------------
# If you submit this package back to Spack as a pull request,
# please first remove this boilerplate and all FIXME comments.
#
# This is a template package file for Spack.  We've put "FIXME"
# next to all the things you'll want to change. Once you've handled
# them, you can save this file and test your package like this:
#
#     spack install hcl
#
# You can edit this file again by typing:
#
#     spack edit hcl
#
# See the Spack documentation for more information on packaging.
# ----------------------------------------------------------------------------

from spack import *


class Hcl(CMakePackage):
    url = "https://github.com/HDFGroup/hcl/tarball/master"
    git = "https://github.com/HDFGroup/hcl.git"

    version('develop', branch='develop')
    version('0.0.1', tag='0.0.1', commit='4647b14a11cd0650eb26c0eafcc22968b54c4f53')
    version('0.0.2', tag='0.0.2', commit='cc9ced060536ccab32dfc53cff83ac8a09c120f5')

    variant("thallium", default=True, description="Enable thallium based RPC communication")
    variant("ofi", default=False, description="Enable Verbs protocol")
    variant("ucx", default=False, description="Enable UCX protocol")
    variant("cpp-logger", default=False, description="Enable CPP Logger based logging")
    variant("verbose", default=False, description="Enable verbose logging")
    variant("debug", default=False, description="Enable debug logging")
    variant("dlp", default=False, description="Enable DLIO Profiler")
    
    depends_on('mpi')
    depends_on('mochi-thallium~cereal@0.11.3:', when='+thallium')
    depends_on('mercury@2.3.1+ofi', when='+thallium+ofi')
    depends_on('mercury@2.3.1+ucx', when='+thallium+ucx')
    depends_on("libfabric fabrics=rxm,sockets,tcp", when="^mercury@2:+ofi")
    depends_on('boost@1.71.0:')
    depends_on('ucx@1.13.1:', when='+ucx')
    depends_on('cpp-logger@0.0.3:', when='+cpp-logger')
    depends_on('py-dlio-profiler-py@0.0.4:', when='+dlp')
    
    def cmake_args(self):
        spec = self.spec
        args = ['-DCMAKE_INSTALL_PREFIX={}'.format(self.prefix)]
        if self.spec.satisfies("+thallium"):
            args.append("-DHCL_COMMUNICATION=THALLIUM")
        elif self.spec.satisfies("+ofi"):
            args.append("-DHCL_COMMUNICATION_PROTOCOL=OFI")
        elif self.spec.satisfies("+ucx"):
            args.append("-DHCL_COMMUNICATION_PROTOCOL=UCX")   
        if self.spec.satisfies("+cpp-logger"):
            args.append("-DHCL_LOGGING=CPP_LOGGER")   
        if self.spec.satisfies("+debug"):
            args.append("-DHCL_LOG_LEVEL=DEBUG")
        elif self.spec.satisfies("+verbose"):
            args.append("-DHCL_LOG_LEVEL=INFO")
        else:
            args.append("-DHCL_LOG_LEVEL=WARN")     
        if self.spec.satisfies("+dlp"):
            args.append("-DHCL_PROFILER=DLIO_PROFILER")

        return args