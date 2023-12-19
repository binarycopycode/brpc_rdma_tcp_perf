// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.


#include <fstream>
#include <filesystem>

#include <gflags/gflags.h>
#include "butil/atomicops.h"
#include "butil/logging.h"
#include "butil/time.h"
#include "butil/iobuf.h"
#include "brpc/server.h"
#include "bvar/variable.h"
#include "brpc/rdma/block_pool.h"
#include "brpc/rdma/rdma_endpoint.h"
#include "brpc/rdma/rdma_helper.h"
#include "transfile.pb.h"

#ifdef BRPC_WITH_RDMA

DEFINE_int32(port, 8002, "TCP Port of this server");
DEFINE_bool(use_rdma, true, "Use RDMA or not");
DEFINE_int32(data_size_mb, 64, "transfile data size in perf test");

char* data_buf;
size_t data_size_bytes;
uint32_t lkey;

int gen_data()
{
    data_size_bytes = FLAGS_data_size_mb * (long long)1024 * 1024;
    // Allocate memory
    data_buf = (char*)malloc(data_size_bytes);

    if(!data_buf)
    {
        LOG(ERROR) << "malloc data_buf failed"; 
        return -1;
    }

    memset(data_buf, 0x3f, data_size_bytes);

    if(FLAGS_use_rdma)
    {
        uint32_t lkey = brpc::rdma::RegisterMemoryForRdma(data_buf, data_size_bytes);
        if(lkey == 0)
        {
            LOG(ERROR) << "rdma::registerMemoryForRdma failed";  
            free(data_buf); 
            return -1;
        }
    }

    return 0;
}

int rec_data()
{
    free(data_buf);
    return 0;
}

static void MockFree(void* buf) { }

namespace transfile {
class TransFileServiceImpl : public TransFileService {
public:
    TransFileServiceImpl() {}
    ~TransFileServiceImpl() {}

    void TransFile(google::protobuf::RpcController* cntl_base,
              const fileRequest* request,
              fileResponse* response,
              google::protobuf::Closure* done) {
        brpc::ClosureGuard done_guard(done);
        brpc::Controller* cntl =
                static_cast<brpc::Controller*>(cntl_base);


        response->set_message("Succ");
        if(FLAGS_use_rdma)
        {
            cntl->response_attachment().append_user_data_with_meta(
                data_buf,
                data_size_bytes,
                MockFree,
                lkey
            );
        }
        else
        {
            cntl->response_attachment().append_user_data(
                data_buf,
                data_size_bytes,
                MockFree
            );
        }
        LOG(INFO) << "Received request Succ response attachment";         
    }
};
}

int main(int argc, char* argv[]) {
    GFLAGS_NS::ParseCommandLineFlags(&argc, &argv, true);

    // Initialize RDMA environment in advance.
    if (FLAGS_use_rdma) 
    {
        brpc::rdma::GlobalRdmaInitializeOrDie();
    }

    if(gen_data() < 0) 
    {
        LOG(ERROR) << "Fail to read data";
        return -1;
    }

    brpc::Server server;
    transfile::TransFileServiceImpl trans_file_service_impl;

    if (server.AddService(&trans_file_service_impl, 
                          brpc::SERVER_DOESNT_OWN_SERVICE) != 0) {
        LOG(ERROR) << "Fail to add service";
        return -1;
    }

    brpc::ServerOptions options;
    options.use_rdma = FLAGS_use_rdma;
    if (server.Start(FLAGS_port, &options) != 0) {
        LOG(ERROR) << "Fail to start transfileServer";
        return -1;
    }

    server.RunUntilAskedToQuit();

    rec_data();
    return 0;
}

#else


int main(int argc, char* argv[]) {
    LOG(ERROR) << " brpc is not compiled with rdma. To enable it, please refer to https://github.com/apache/brpc/blob/master/docs/en/rdma.md";
    return 0;
}

#endif
