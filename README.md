## 依赖准备

CentOS一般需要安装EPEL，否则很多包都默认不可用。

```shell
sudo yum install epel-release
```

安装依赖：

```shell
sudo yum install git gcc-c++ make openssl-devel gflags-devel protobuf-devel protobuf-compiler leveldb-devel
```

如果你要在样例中启用cpu/heap的profiler：

## 使用cmake编译

编译brpc

```bash
mkdir build && cd build && cmake -DWITH_RDMA=ON ..
make
```

编译perftest

```shell
cd example/rdma_tcp_perf
mkdir build && cd build && cmake ..
make
```

## 使用rdma传输数据

maybe need `ulimit -n 204800`


```shell
./server -data_size_mb=128 -rdma_memory_pool_initial_size_mb=256 -max_body_size=1073741824
./client -server=127.31.50.183:8002 -rdma_memory_pool_initial_size_mb=256 -max_body_size=1073741824
```

其中server的返回的数据大小是128mb， rdma初始分配内存池是256mb ，单次最大传输数据是1G

client的server可以设置ip地址，server的默认端口是8002

## 使用TCP传输数据

```shell
./server -use_rdma=false -data_size_mb=128 -rdma_memory_pool_initial_size_mb=256 -max_body_size=1073741824
./client -use_rdma=false -server=127.31.50.183:8002 -rdma_memory_pool_initial_size_mb=256 -max_body_size=1073741824
```

只需要加上-use_rdma=false，就会用tcp传输数据了

