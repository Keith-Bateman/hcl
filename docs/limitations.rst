=============
Limitations
=============

Character arrays have a stack-size limit, this is system-dependent, but typically 1-2 MiB. You can make custom CharStructs with character arrays that store a lot of data, but never more than stack size.

In order to alleviate this, the intended solution in HCL is to utilize boost shared memory stack allocation. See example:

.. code-block:: cpp

    typedef boost::interprocess::allocator<char, boost::interprocess::managed_mapped_file::segment_manager> CharAllocator;
    typedef bip::basic_string<char, std::char_traits<char>, CharAllocator> MappedUnitString;
    hcl::unordered_map<struct KeyType, std::string, std::hash<KeyType>, CharAllocator, MappedUnitString> *hcl_string_client;


However, boost shared memory allocators also have limitations. The limitation is nearly 128 MiB, you can push to 127.9 MiB, but not all the way up to 128 (more detail at `issue 22
<https://github.com/hariharan-devarajan/hcl/issues/22>`_). We recommend not going over 64 MiB for boost shared memory strings, and also note that performance with this allocation is far more variable than stack allocation.
