========
Overview
========

Most parallel programs use irregular control flow and data structures, which are perfect for one-sided communication paradigms such as MPI or PGAS programming languages. 
However, these environments lack the presence of efficient function-based application libraries that can utilize popular communication fabrics such as TCP, Infinity Band (m), and RDMA over Converged Ethernet (RoCE). 
Additionally, there is a lack of high-performance data structure interfaces. We present Hermes Container Library (HCL), a high-performance distributed data structures library that offers high-level abstractions including hash-maps, sets, and queues. 
HCL uses a RPC over RDMA technology that implements a novel procedural programming paradigm. 
In this paper, we argue a RPC over RDMA technology can serve as a high-performance, flexible, and co-ordination free backend for implementing complex data structures. 
Evaluation results from testing real workloads shows that HCL programs are 2x to 12x faster compared to BCL, a state-of-the-art distributed data structure library.

----------------------------------------------------
Supported Data structors with hash-based distributions
----------------------------------------------------

#. hcl::unordered_map
#. hcl::map
#. hcl::multi map
#. hcl::set

----------------------------------------------------
Supported Data structors with server-specific distribution
----------------------------------------------------

#. hcl::queue
#. hcl::priority_queue
#. hcl::sequencer
#. hcl::vector

--------------------------
Experimenal data structures
--------------------------

#. hcl::concurrent_skiplist
