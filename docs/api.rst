==========
HCL API
==========

This section describes how to use the HCL API in an application.

-----

--------------------------
Include the HCL Header
--------------------------

In C or C++ applications, include ``hcl.h``.

.. code-block:: cpp

    #include <hcl.h>


--------------------------
Configure HCL
--------------------------

HCL data structured need to be configured on clients and servers to select which parts of the application are clients or servers.
These could be ranks of a MPI applications such as rank 0 of every node is a server and others are clients. 
Alternatively, these could be seperate processes or mpi applications such as server processes vs client processes.
Additionally, Servers can also access it as a client.

There are three ways to configure HCL.
1. ENV: Environment variables. This support is currently limited.
2. GLOBAL: Using global configuration manager macro. `HCL_CONF-><VARIABLES>`
3. INIT: During initialization or reinitailization of HCL.
4. DS: Per data structure, HCL configurations can be changed per data structure initiliatization.

Order of applying of configurations are GLOBAL -> ENV -> INIT -> DS


Configuration variables for using configuration manager
*********************************************************

================================ ======  ===========================================================================
Configuration Option             Type    Description
================================ ======  ===========================================================================
RPC_PORT                         INT     Starting port for the server processes. HCL increments this based on server list.
RPC_THREADS                      INT     Number of threads each RPC Server should use.
RPC_IMPLEMENTATION               ENUM    Which implementation of RPC to use. Supported Values are: THALLIUM
URI                              STRING  URI support for HCL. Format is <PROTOCOL>://<DEVICE>/<INTERFACE>
MEMORY_ALLOCATED                 INT     Shared memory to be allocated per datastructure.
IS_SERVER                        BOOL    Is current process a server
MY_SERVER                        INT     Which server index is the current process's server.
NUM_SERVERS                      INT     Total number of server processes in the workload.
SERVER_ON_NODE                   BOOL    Is server collocated with the client. This can be used to have hybrid RPC + Shared memory access model.
SERVER_LIST_PATH                 STRING  List of servers defined for HCL. The format is <hostname>:<number of servers on host>
BACKED_FILE_DIR                  STRING  Where to store the file backed file. Default is /dev/shm. Can be stored on ssd as well.
================================ ======  ===========================================================================

Configuration variables for using environment varibles
*********************************************************
================================ ======  ===========================================================================
Configuration Option             Type    Description
================================ ======  ===========================================================================
HCL_THALLIUM_URI                 STRING  Sets URI for HCL in thallium format. <PROTOCOL>://<DEVICE>/<INTERFACE>
================================ ======  ===========================================================================

--------------------------
Initialize HCL
--------------------------

This is a global initialization step required for HCL datastructures. 
If `needs_initialization` is set to false then the HCL is only reinitialized as needed.
This features is needed to change how HCL interacts and manages the RPC and global variables for data structures.
The `needs_initialization` with `true` should be set once in a process lifetime but can be called with `false` multiple times.

.. code-block:: cpp

    bool needs_initialization = true;
    auto hcl = hcl::HCL::GetInstance(needs_initialization);


--------------------------
Finalize HCL
--------------------------

This is global finalizations to be called only when all data structures are freed or destructed.
This should be called once in a lifetime.

.. code-block:: cpp

    hcl->Finalize();