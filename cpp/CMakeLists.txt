set(CMAKE_CXX_STANDARD 14)

include_directories(${Project_SOURCE_DIR}/rabbitmq-c/include/)

find_library(RABBITMQ_LIB NAMES rabbitmq.4.lib PATHS ${Project_SOURCE_DIR}/rabbitmq-c/lib/)
message(STATUS "Library path RABBITMQ_LIB is " ${RABBITMQ_LIB})

add_executable(send ./HelloWorld/send.cpp)
target_link_libraries(send ${RABBITMQ_LIB})

add_executable(receive ./HelloWorld/receive.cpp)
target_link_libraries(receive ${RABBITMQ_LIB})

add_executable(new_task ./WorkQueues/new_task.cpp)
target_link_libraries(new_task ${RABBITMQ_LIB})

add_executable(worker ./WorkQueues/worker.cpp)
target_link_libraries(worker ${RABBITMQ_LIB})

add_executable(emit_log ./PublishSubscribe/emit_log.cpp)
target_link_libraries(emit_log ${RABBITMQ_LIB})

add_executable(receive_logs ./PublishSubscribe/receive_logs.cpp)
target_link_libraries(receive_logs ${RABBITMQ_LIB})

add_executable(emit_log_direct ./Routing/emit_log_direct.cpp)
target_link_libraries(emit_log_direct ${RABBITMQ_LIB})

add_executable(receive_logs_direct ./Routing/receive_logs_direct.cpp)
target_link_libraries(receive_logs_direct ${RABBITMQ_LIB})

add_executable(emit_log_topic ./Topic/emit_log_topic.cpp)
target_link_libraries(emit_log_topic ${RABBITMQ_LIB})

add_executable(receive_logs_topic ./Topic/receive_logs_topic.cpp)
target_link_libraries(receive_logs_topic ${RABBITMQ_LIB})

add_executable(rpc_server ./RPC/rpc_server.cpp)
target_link_libraries(rpc_server ${RABBITMQ_LIB})

add_executable(rpc_client ./RPC/rpc_client.cpp)
target_link_libraries(rpc_client ${RABBITMQ_LIB})

add_executable(publisher_confirms publisher_confirms.cpp)
target_link_libraries(publisher_confirms ${RABBITMQ_LIB})