# RDMA test

./server -data_size_mb=128 -rdma_memory_pool_initial_size_mb=256 -max_body_size=1073741824
./client -server=127.31.50.183:8002 -rdma_memory_pool_initial_size_mb=256 -max_body_size=1073741824

# TCP test

./server -use_rdma=false -data_size_mb=128 -rdma_memory_pool_initial_size_mb=256 -max_body_size=1073741824
./client -use_rdma=false -server=127.31.50.183:8002 -rdma_memory_pool_initial_size_mb=256 -max_body_size=1073741824
