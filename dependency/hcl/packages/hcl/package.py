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
#     spack install rpclib
#
# You can edit this file again by typing:
#
#     spack edit rpclib
#
# See the Spack documentation for more information on packaging.
# ----------------------------------------------------------------------------

from spack import *


class Hcl(CMakePackage):
    url = "https://github.com/HDFGroup/hcl/tarball/master"
    git = "https://github.com/HDFGroup/hcl.git"

    version('develop', branch='dev')
    variant(
        "thallium",
        default=True,
        description="Enable Thallium communication",
    )
    variant(
        "rpclib",
        default=True,
        description="Enable RPC lib communication",
    )

    variant(
        "ucx",
        default=True,
        description="Enable UCX communication",
    )
    variant(
        "tcp",
        default=True,
        description="Enable TCP communication protocol",
    )
    variant(
        "verbs",
        default=True,
        description="Enable VERBS communication protocol",
    )
    conflicts("+rpclib",
     when="+verbs",
     msg="RPC lib only supports tcp protocol")
    depends_on("mpi")
    depends_on('rpclib@2.2.1:', when='+rpclib')
    depends_on('mochi-thallium~cereal@0.11.3', when='+thallium')
    depends_on('ucx@1.13.1:', when='+ucx')
    depends_on('boost@1.71.0:')

    def cmake_args(self):
        spec = self.spec
        args = ['-DCMAKE_INSTALL_PREFIX={}'.format(self.prefix)]
        communication="THALLIUM"
        protocol="TCP"
        if '+rpclib' in spec:
            communication="RPCLIB"
        elif '+ucx' in spec:
            communication="UCX"
        elif '+thallium' in spec:
            communication="THALLIUM"
        
        if '+tcp' in spec:
            communication="TCP"
        elif '+verbs' in spec:
            communication="VERBS"

        args.append(f"-DHCL_COMMUNICATION={communication}")
        args.append(f"-DHCL_COMMUNICATION_PROTOCOL={protocol}")               
        return args