SET(FreeRTOS_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/FreeRTOS-Kernel)
FILE(GLOB FreeRTOS_src ${FreeRTOS_DIR}/*.c)

add_library(FreeRTOS STATIC
    ${FreeRTOS_src}
    ${FreeRTOS_DIR}/portable/GCC/ARM_CM0/port.c
    ${FreeRTOS_DIR}/portable/MemMang/heap_4.c
)

target_include_directories(FreeRTOS PUBLIC 
${FreeRTOS_DIR}/portable/GCC/ARM_CM0/
    ${FreeRTOS_DIR}/include/
)