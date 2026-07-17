set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_VERSION 1)
cmake_minimum_required(VERSION 3.22)
# 设置目标处理器架构,芯片型号,和目标板简称
set(CMAKE_SYSTEM_PROCESSOR cortex-m0plus)
set(Target_Board MSPM0xx)
set(Board_Name mspm0g3507)

if(NOT DEFINED ARM_GCC_TOOLCHAIN_ROOT_DIR)
    if(DEFINED ENV{ARM_GCC_ROOT} AND NOT "$ENV{ARM_GCC_ROOT}" STREQUAL "")
        file(TO_CMAKE_PATH "$ENV{ARM_GCC_ROOT}" ARM_GCC_TOOLCHAIN_ROOT_DIR)
    else()
        set(ARM_GCC_TOOLCHAIN_ROOT_DIR "E:/DevEnv/10 2021.10")
    endif()
endif()

# 指定交叉编译工具链arm-none-eabi-gcc
set(CMAKE_C_COMPILER "${ARM_GCC_TOOLCHAIN_ROOT_DIR}/bin/arm-none-eabi-gcc.exe")
set(CMAKE_CXX_COMPILER "${ARM_GCC_TOOLCHAIN_ROOT_DIR}/bin/arm-none-eabi-g++.exe")
set(CMAKE_ASM_COMPILER "${ARM_GCC_TOOLCHAIN_ROOT_DIR}/bin/arm-none-eabi-gcc.exe")
set(CMAKE_AR "${ARM_GCC_TOOLCHAIN_ROOT_DIR}/bin/arm-none-eabi-ar.exe")
set(CMAKE_LINKER "${ARM_GCC_TOOLCHAIN_ROOT_DIR}/bin/arm-none-eabi-ld.exe")
set(CMAKE_OBJCOPY "${ARM_GCC_TOOLCHAIN_ROOT_DIR}/bin/arm-none-eabi-objcopy.exe")
set(CMAKE_OBJDUMP "${ARM_GCC_TOOLCHAIN_ROOT_DIR}/bin/arm-none-eabi-objdump.exe")
set(CMAKE_SIZE "${ARM_GCC_TOOLCHAIN_ROOT_DIR}/bin/arm-none-eabi-size.exe")
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# 项目名称
project(MSPM0 C CXX ASM)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)
# 添加关于硬件浮点的支持的相关编译选项
message(STATUS "${Target_Board}")
add_compile_options(-mfloat-abi=soft)
add_link_options(-mfloat-abi=soft)

# 指定芯片型号的相关编译选项
add_compile_options(-mcpu=${CMAKE_SYSTEM_PROCESSOR} -mthumb)
add_link_options(-mcpu=${CMAKE_SYSTEM_PROCESSOR} -mthumb)

add_compile_options(-ffunction-sections -fdata-sections -fno-common -fmessage-length=0)
add_link_options(-specs=nosys.specs -lc -lm)


# 进行汇编文件的预编译
add_compile_options($<$<COMPILE_LANGUAGE:ASM>:-x$<SEMICOLON>assembler-with-cpp>)
# 提供编译选项,根据不同的编译类型进行不同的优化,常见的编译类型有Debug,Release,RelWithDebInfo,MinSizeRel
if ("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
    message(STATUS "Maximum optimization for speed")
    add_compile_options(-Ofast)
elseif ("${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo")
    message(STATUS "Maximum optimization for speed, debug info included")
    add_compile_options(-Ofast -g)
elseif ("${CMAKE_BUILD_TYPE}" STREQUAL "MinSizeRel")
    message(STATUS "Maximum optimization for size")
    add_compile_options(-Os)
else ()
    message(STATUS "Minimal optimization, debug info included")
    add_compile_options(-Og -g)
endif ()


# 添加链接脚本
set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/linker/mspm0g3507.ld)

# 添加链接选项,生成map文件
add_link_options(-Wl,-gc-sections,--print-memory-usage,-Map=${PROJECT_BINARY_DIR}/${PROJECT_NAME}.map)
add_link_options(-T ${LINKER_SCRIPT})


# 可选编译开关：TI Drivers / 逐飞库 / FreeRTOS
option(USE_TI_DRIVERS     "编译 TI Drivers 源码" OFF)
option(USE_ZF_LIB         "编译逐飞库 (zf) 源码" OFF)
option(USE_FREERTOS       "编译 FreeRTOS 源码"   OFF)

if(USE_FREERTOS)
    set(LINK_HEAP_SIZE 0x0)
    set(LINK_STACK_SIZE 0x200)
else()
    set(LINK_HEAP_SIZE 0x800)
    set(LINK_STACK_SIZE 0x800)
endif()

add_link_options(
    -Wl,--defsym=__heap_size__=${LINK_HEAP_SIZE}
    -Wl,--defsym=__stack_size__=${LINK_STACK_SIZE}
)

# 递归收集内核、驱动和用户代码
file(
    GLOB_RECURSE PROJECT_SOURCE_FILES CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/source/core/startup_mspm0g350x_gcc.c
    ${CMAKE_SOURCE_DIR}/source/core/ti/driverlib/*.c
    ${CMAKE_SOURCE_DIR}/user/*.c)

# 排除 rtos 目录（后续按开关单独添加）
list(FILTER PROJECT_SOURCE_FILES EXCLUDE REGEX ".*/rtos/.*")

# 根据开关决定是否收集 TI Drivers 源码
if(USE_TI_DRIVERS)
    file(GLOB_RECURSE TI_DRIVERS_SOURCE_FILES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/source/core/ti/drivers/*.c)
    list(APPEND PROJECT_SOURCE_FILES ${TI_DRIVERS_SOURCE_FILES})
    message(STATUS "TI Drivers: 已启用")
else()
    message(STATUS "TI Drivers: 已禁用")
endif()

# 根据开关决定是否收集逐飞库源码
if(USE_ZF_LIB)
    file(GLOB_RECURSE ZF_SOURCE_FILES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/source/core/zf/*.c)
    list(APPEND PROJECT_SOURCE_FILES ${ZF_SOURCE_FILES})
    message(STATUS "逐飞库 (zf): 已启用")
else()
    message(STATUS "逐飞库 (zf): 已禁用")
endif()

# 根据开关决定是否收集 FreeRTOS 源码
if(USE_FREERTOS)
    set(FREERTOS_SOURCE_FILES
        ${CMAKE_SOURCE_DIR}/source/rtos/croutine.c
        ${CMAKE_SOURCE_DIR}/source/rtos/event_groups.c
        ${CMAKE_SOURCE_DIR}/source/rtos/heap_4.c
        ${CMAKE_SOURCE_DIR}/source/rtos/list.c
        ${CMAKE_SOURCE_DIR}/source/rtos/port.c
        ${CMAKE_SOURCE_DIR}/source/rtos/portasm.c
        ${CMAKE_SOURCE_DIR}/source/rtos/queue.c
        ${CMAKE_SOURCE_DIR}/source/rtos/stream_buffer.c
        ${CMAKE_SOURCE_DIR}/source/rtos/tasks.c
        ${CMAKE_SOURCE_DIR}/source/rtos/timers.c
        ${CMAKE_SOURCE_DIR}/user/rtos/freertos_hooks.c
    )
    list(APPEND PROJECT_SOURCE_FILES ${FREERTOS_SOURCE_FILES})
    message(STATUS "FreeRTOS: 已启用（内核、端口与用户钩子）")
else()
    message(STATUS "FreeRTOS: 已禁用")
endif()

list(SORT PROJECT_SOURCE_FILES)

# 生成可执行文件
add_executable(${PROJECT_NAME}.elf ti_msp_dl_config.c ${PROJECT_SOURCE_FILES} ${LINKER_SCRIPT})

set(COMMON_INCLUDE_DIRECTORIES
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/include/core
        ${CMAKE_SOURCE_DIR}/include/core/ti/driverlib
        ${CMAKE_SOURCE_DIR}/include/core/third_party/CMSIS/Core/Include
        ${CMAKE_SOURCE_DIR}/user
        ${CMAKE_SOURCE_DIR}/user/uart
        ${CMAKE_SOURCE_DIR}/user/imu601
)

# 根据开关添加对应的头文件目录
if(USE_ZF_LIB)
    list(APPEND COMMON_INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR}/include/core/zf)
endif()

if(USE_FREERTOS)
    list(APPEND COMMON_INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR}/include/rtos)
    list(APPEND COMMON_INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR}/user/rtos)
endif()

include_directories(${CMAKE_SOURCE_DIR})

function(apply_common_target_settings target_name)
    target_include_directories(${target_name} PUBLIC ${COMMON_INCLUDE_DIRECTORIES})
endfunction()

apply_common_target_settings(${PROJECT_NAME}.elf)

# 编译宏定义
add_definitions(
        -D__MSPM0G3507__  # 定义芯片型号
)

# 定义其他类型的可执行文件
set(HEX_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.hex)
set(BIN_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.bin)
# 生成hex和bin文件
add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -Oihex $<TARGET_FILE:${PROJECT_NAME}.elf> ${HEX_FILE}
        COMMAND ${CMAKE_OBJCOPY} -Obinary $<TARGET_FILE:${PROJECT_NAME}.elf> ${BIN_FILE}
        COMMENT "Building ${HEX_FILE}
Building ${BIN_FILE}")
