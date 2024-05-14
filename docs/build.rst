=============
Build HCL
=============

This section describes how to build HCL, and what
:ref:`configure time options <configure-options-label>` are available.

There are two build options:

* build HCL with Spack, and 
* build HCL with cmake

----------

-----------------------------------------
Build HCL with Spack
-----------------------------------------


One may install HCL with Spack_.
If you already have Spack, make sure you have the latest release.
If you use a clone of the Spack develop branch, be sure to pull the latest changes.

.. _build-label:

Install Spack
*************
.. code-block:: Bash

    $ git clone https://github.com/spack/spack
    $ # create a packages.yaml specific to your machine
    $ . spack/share/spack/setup-env.sh

Use `Spack's shell support`_ to add Spack to your ``PATH`` and enable use of the
``spack`` command.

Build and Install HCL
*************************

.. code-block:: Bash

    $ spack repo add dependency/hcl/
    $ spack install hcl
    $ spack load hcl

If the most recent changes on the development branch ('develop') of HCL are
desired, then do ``spack install hcl@develop``.

.. attention::

    The initial install could take a while as Spack will install build
    dependencies (autoconf, automake, m4, libtool, and pkg-config) as well as
    any dependencies of dependencies (cmake, perl, etc.) if you don't already
    have these dependencies installed through Spack or haven't told Spack where
    they are locally installed on your system (i.e., through a custom
    packages.yaml_).
    Run ``spack spec -I hcl`` before installing to see what Spack is going
    to do.

----------

-------------------------
Build HCL with CMake
-------------------------

Download the latest HCL release from the Releases_ page or clone the develop
branch ('develop') from the HCL repository
`https://github.com/hariharan-devarajan/hcl <https://github.com/hariharan-devarajan/hcl>`_.


.. code-block:: Bash
    
    cmake . -B build -DCMAKE_INSTALL_PREFIX=<where you want to install GOTCHA>
    cmake --build build
    cmake --install build


Build Options with Cmake
*************************

================================ ======  ===========================================================================
Configuration Option             Type    Description
================================ ======  ===========================================================================
HCL_BUILD_DEPENDENCIES           BOOL    Install dependencies for build. This will not compile HCL. 
                                         You need to rerun with this flag off
HCL_BUILD_WITH_MPI               BOOL    Build with MPI support.
HCL_COMMUNICATION                STRING  Which communication library to use. Supported values are: THALLIUM
HCL_COMMUNICATION_PROTOCOL       STRING  Which protocol to use. Supported Values are: UCX and OFI
HCL_ENABLE_TESTING               BOOL    Enable HCL Test cases.
HCL_LIBDIR_AS_LIB                BOOL    Use lib as library directory else detect it based on architecture.
HCL_LOGGER                       STRING  Enable logger for HCL. Supported values are: NONE and CPP_LOGGER.
HCL_LOGGER_LEVEL                 STRING  Enable logging level for HCL. 
                                         Supported values are: DEBUG, INFO, WARN, and ERROR.
HCL_PROFILER                     STRING  Enable profiler for HCL. Supported values are: NONE and DLIO_PROFILER.
================================ ======  ===========================================================================


-----------

.. explicit external hyperlink targets

.. _Releases: https://github.com/hariharan-devarajan/hcl/releases
.. _Spack: https://github.com/spack/spack
.. _Spack's shell support: https://spack.readthedocs.io/en/latest/getting_started.html#add-spack-to-the-shell
.. _packages.yaml: https://spack.readthedocs.io/en/latest/build_settings.html#external-packages
