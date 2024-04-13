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
    
    depends_on('mpi')
    depends_on('mochi-thallium~cereal@0.11.3:', when='+thallium')
    depends_on('mercury@2.3.1+ofi', when='+thallium+ofi')
    depends_on('mercury@2.3.1+ucx', when='+thallium+ucx')
    depends_on("libfabric fabrics=rxm,sockets,tcp", when="^mercury@2:+ofi")
    depends_on('boost@1.71.0:')
    depends_on('ucx@1.13.1:', when='+ucx')

    def cmake_args(self):
        spec = self.spec
        args = ['-DCMAKE_INSTALL_PREFIX={}'.format(self.prefix)]
        if self.spec.satisfies("+thallium"):
            args.append("-DHCL_COMMUNICATION=THALLIUM")
        elif self.spec.satisfies("+ofi"):
            args.append("-DHCL_COMMUNICATION_PROTOCOL=OFI")
        elif self.spec.satisfies("+ucx"):
            args.append("-DHCL_COMMUNICATION_PROTOCOL=UCX")                
        return args