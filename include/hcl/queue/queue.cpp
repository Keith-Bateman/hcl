/*
 * Copyright (C) 2019  Hariharan Devarajan, Keith Bateman
 *
 * This file is part of HCL
 * 
 * HCL is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

template<typename MappedType>
queue<MappedType>::~queue() {
    if (is_server) bip::file_mapping::remove(backed_file.c_str());
}
template<typename MappedType>
queue<MappedType>::queue(std::string name_)
    : num_servers(HCL_CONF->NUM_SERVERS),
      my_server(HCL_CONF->MY_SERVER),
      memory_allocated(HCL_CONF->MEMORY_ALLOCATED),
      is_server(HCL_CONF->IS_SERVER),
      segment(),
      name(name_),
      func_prefix(name_),
      my_queue(),
      server_on_node(HCL_CONF->SERVER_ON_NODE),
      backed_file(HCL_CONF->BACKED_FILE_DIR + PATH_SEPARATOR + name_+"_"+std::to_string(my_server)) {

    AutoTrace trace = AutoTrace("hcl::queue(local)");
    /* create per server name for shared memory. Needed if multiple servers are
       spawned on one node*/
    this->name += "_" + std::to_string(my_server);
    rpc = Singleton<RPCFactory>::GetInstance()->GetRPC(HCL_CONF->RPC_PORT);
    if (is_server) {
        /* Delete existing instance of shared memory space*/
        bip::file_mapping::remove(backed_file.c_str());
        /* allocate new shared memory space */
        segment = bip::managed_mapped_file(bip::create_only, backed_file.c_str(),
                                             memory_allocated);
        ShmemAllocator alloc_inst(segment.get_segment_manager());
        /* Construct queue in the shared memory space. */
        my_queue = segment.construct<Queue>("Queue")(alloc_inst);
        mutex = segment.construct<bip::interprocess_mutex>("mtx")();
        /* Create a RPC server and map the methods to it. */
        switch (HCL_CONF->RPC_IMPLEMENTATION) {
#ifdef HCL_ENABLE_RPCLIB
            case RPCLIB: {
                std::function<bool(MappedType &)> pushFunc(
                    std::bind(&hcl::queue<MappedType>::LocalPush, this,
                              std::placeholders::_1));
                std::function<std::pair<bool, MappedType>(void)> popFunc(std::bind(
                    &hcl::queue<MappedType>::LocalPop, this));
                std::function<size_t(void)> sizeFunc(std::bind(
                    &hcl::queue<MappedType>::LocalSize, this));
                std::function<bool(void)> waitForElementFunc(std::bind(
                    &hcl::queue<MappedType>::LocalWaitForElement, this));
                rpc->bind(func_prefix+"_Push", pushFunc);
                rpc->bind(func_prefix+"_Pop", popFunc);
                rpc->bind(func_prefix+"_WaitForElement", waitForElementFunc);
                rpc->bind(func_prefix+"_Size", sizeFunc);
                break;
            }
#endif
#ifdef HCL_ENABLE_THALLIUM_TCP
            case THALLIUM_TCP:
#endif
#ifdef HCL_ENABLE_THALLIUM_ROCE
            case THALLIUM_ROCE:
#endif
#if defined(HCL_ENABLE_THALLIUM_TCP) || defined(HCL_ENABLE_THALLIUM_ROCE)
            {
                std::function<void(const tl::request &, MappedType &)> pushFunc(
                    std::bind(&hcl::queue<MappedType>::ThalliumLocalPush, this,
                              std::placeholders::_1, std::placeholders::_2));
                std::function<void(const tl::request &)> popFunc(
                    std::bind(&hcl::queue<MappedType>::ThalliumLocalPop, this, std::placeholders::_1));
                std::function<void(const tl::request &)> sizeFunc(
                    std::bind(&hcl::queue<MappedType>::ThalliumLocalSize, this, std::placeholders::_1));
                std::function<void(const tl::request &)> waitForElementFunc(
                    std::bind(&hcl::queue<MappedType>::ThalliumLocalWaitForElement, this,
                              std::placeholders::_1));
                rpc->bind(func_prefix+"_Push", pushFunc);
                rpc->bind(func_prefix+"_Pop", popFunc);
                rpc->bind(func_prefix+"_WaitForElement", waitForElementFunc);
                rpc->bind(func_prefix+"_Size", sizeFunc);
                break;
            }
#endif
            default:
                break;
        }
    } else if (!is_server && server_on_node) {
        /* Map the clients to their respective memory pools */
        segment = bip::managed_mapped_file(bip::open_only, backed_file.c_str());
        std::pair<Queue*, bip::managed_mapped_file::size_type> res;
        res = segment.find<Queue> ("Queue");
        my_queue = res.first;
        std::pair<bip::interprocess_mutex *,
                  bip::managed_mapped_file::size_type> res2;
        res2 = segment.find<bip::interprocess_mutex>("mtx");
        mutex = res2.first;
    }
}

/**
 * Push the data into the local queue.
 * @param key, the key for put
 * @param data, the value for put
 * @return bool, true if Put was successful else false.
 */
template<typename MappedType>
bool queue<MappedType>::LocalPush(MappedType &data) {
    AutoTrace trace = AutoTrace("hcl::queue::Push(local)", data);
    bip::scoped_lock<bip::interprocess_mutex> lock(*mutex);
    my_queue->push_back(std::move(data));
    return true;
}

/**
 * Push the data into the queue. Uses key to decide the server to hash it to,
 * @param key, the key for put
 * @param data, the value for put
 * @return bool, true if Put was successful else false.
 */
template<typename MappedType>
bool queue<MappedType>::Push(MappedType &data,
                             uint16_t &key_int) {
    if (key_int == my_server && server_on_node) {
        return LocalPush(data);
    } else {
        AutoTrace trace = AutoTrace("hcl::queue::Push(remote)", data,
                                    key_int);
        return RPC_CALL_WRAPPER("_Push", key_int, bool,
                                data);
    }
}

/**
 * Get the local data from the queue.
 * @param key_int, key_int to know which server
 * @return return a pair of bool and Value. If bool is true then data was
 * found and is present in value part else bool is set to false
 */
template<typename MappedType>
std::pair<bool, MappedType>
queue<MappedType>::LocalPop() {
    AutoTrace trace = AutoTrace("hcl::queue::Pop(local)");
    bip::scoped_lock<bip::interprocess_mutex> lock(*mutex);
    if (my_queue->size() > 0) {
        MappedType value = my_queue->front();
        my_queue->pop_front();
        return std::pair<bool, MappedType>(true, value);
    }
    return std::pair<bool, MappedType>(false, MappedType());
}

/**
 * Get the data from the queue. Uses key_int to decide the server to hash it
 * to,
 * @param key_int, key_int to know which server
 * @return return a pair of bool and Value. If bool is true then data was
 * found and is present in value part else bool is set to false
 */
template<typename MappedType>
std::pair<bool, MappedType>
queue<MappedType>::Pop(uint16_t &key_int) {
    if (key_int == my_server && server_on_node) {
        return LocalPop();
    } else {
        AutoTrace trace = AutoTrace("hcl::queue::Pop(remote)",
                                    key_int);
        typedef std::pair<bool, MappedType> ret_type;
        return RPC_CALL_WRAPPER1("_Pop", key_int, ret_type);
    }
}

template<typename MappedType>
bool queue<MappedType>::LocalWaitForElement() {
    AutoTrace trace = AutoTrace("hcl::queue::WaitForElement(local)");
    int count = 0;
    while (my_queue->size() == 0) {
        usleep(10);
        if (count == 0) printf("Server %d, No Events in Queue\n", my_server);
        count++;
    }
    return true;
}

template<typename MappedType>
bool queue<MappedType>::WaitForElement(uint16_t &key_int) {
    if (key_int == my_server && server_on_node) {
        return LocalWaitForElement();
    } else {
        AutoTrace trace = AutoTrace(
            "hcl::queue::WaitForElement(remote)", key_int);
        return RPC_CALL_WRAPPER1("_WaitForElement", key_int, bool);
    }
}

/**
 * Get the size of the local queue.
 * @param key_int, key_int to know which server
 * @return return a size of the queue
 */
template<typename MappedType>
size_t queue<MappedType>::LocalSize() {
    AutoTrace trace = AutoTrace("hcl::queue::Size(local)");
    bip::scoped_lock<bip::interprocess_mutex> lock(*mutex);
    size_t value = my_queue->size();
    return value;
}

/**
 * Get the size of the queue. Uses key_int to decide the server to hash it to,
 * @param key_int, key_int to know which server
 * @return return a size of the queue
 */
template<typename MappedType>
size_t queue<MappedType>::Size(uint16_t &key_int) {
    if (key_int == my_server && server_on_node) {
        return LocalSize();
    } else {
        AutoTrace trace = AutoTrace("hcl::queue::Size(remote)",
                                    key_int);
        return RPC_CALL_WRAPPER1("_Size", key_int, size_t);
    }
}
// template class queue<int>;
