cmake_minimum_required(VERSION 3.20)

# ============================================================================
# Template libopencm3 multi-placa.
# Uso: passe -DBOARD=<placa>, onde <placa> e' o nome de um arquivo em boards/.
#   Placas prontas: BluePill, BlackPill, NucleoF446, NucleoF767, NucleoG474
# Exemplo no CLion: CMake options -> -DBOARD=BlackPill
# ============================================================================

# --- 1. Seletor de placa --------------------------------------------------
if(NOT DEFINED BOARD)
    set(BOARD BluePill)   # padrao se nada for passado
    message(STATUS "BOARD nao definido, usando padrao: ${BOARD}")
endif()

set(BOARD_FILE ${CMAKE_SOURCE_DIR}/boards/${BOARD}.cmake)
if(NOT EXISTS ${BOARD_FILE})
    message(FATAL_ERROR "Placa '${BOARD}' nao encontrada. Veja a pasta boards/.")
endif()

# Carrega o perfil: define MCU_FAMILY, CPU_FLAGS, LIBOPENCM3_TGT,
# LIBOPENCM3_LD, FLASH_SIZE, RAM_SIZE, FLASH_ORIGIN, RAM_ORIGIN.
include(${BOARD_FILE})
message(STATUS "Placa: ${BOARD}  Familia: ${MCU_FAMILY}  Flags: ${CPU_FLAGS}")

project(stm32_app C ASM)
set(CMAKE_C_STANDARD 11)

# --- 2. Caminhos do libopencm3 -------------------------------------------
set(LIBOPENCM3_DIR ${CMAKE_SOURCE_DIR}/libopencm3)
set(LIBOPENCM3_INC ${LIBOPENCM3_DIR}/include)
set(LIBOPENCM3_LIB ${LIBOPENCM3_DIR}/lib)

# --- 3. Linker script gerado a partir do perfil ---------------------------
# Em vez de manter um .ld por placa, geramos o mapa de memoria a partir das
# variaveis do perfil e damos INCLUDE no script generico do libopencm3.
set(GEN_LD ${CMAKE_BINARY_DIR}/memory.ld)
file(WRITE  ${GEN_LD} "MEMORY\n{\n")
file(APPEND ${GEN_LD} "  rom (rx)  : ORIGIN = ${FLASH_ORIGIN}, LENGTH = ${FLASH_SIZE}\n")
file(APPEND ${GEN_LD} "  ram (rwx) : ORIGIN = ${RAM_ORIGIN}, LENGTH = ${RAM_SIZE}\n")
file(APPEND ${GEN_LD} "}\n")
file(APPEND ${GEN_LD} "INCLUDE ${LIBOPENCM3_LD}\n")

# --- 4. Flags (CPU_FLAGS vem do perfil; usar em COMPILE e LINK) -----------
add_compile_options(
    ${CPU_FLAGS}
    -D${MCU_FAMILY}
    -DLED_RCC=${LED_RCC}            # pino do LED vem do perfil da placa
    -DLED_PORT=${LED_PORT}
    -DLED_PIN=${LED_PIN}
    -Os -g3 -Wall -Wextra
    -fno-common -ffunction-sections -fdata-sections
)
add_link_options(
    ${CPU_FLAGS}                     # MESMAS flags no link (FPU/float-abi)
    --static -nostartfiles
    -T${GEN_LD}
    -Wl,--gc-sections
    -L${LIBOPENCM3_LIB}
    -specs=nosys.specs
)

# --- 4b. FreeRTOS ---------------------------------------------------------
# Port escolhido pelo perfil da placa (FREERTOS_PORT), conforme o core:
#   Cortex-M3 -> ARM_CM3 | Cortex-M4F -> ARM_CM4F | Cortex-M7 -> ARM_CM7/r0p1
# CPU_CLOCK_HZ tambem vem do perfil (clock real do nucleo no HSI de reset).
if(NOT DEFINED FREERTOS_PORT)
    message(FATAL_ERROR "FREERTOS_PORT nao definido em boards/${BOARD}.cmake.")
endif()
if(NOT DEFINED CPU_CLOCK_HZ)
    message(FATAL_ERROR "CPU_CLOCK_HZ nao definido em boards/${BOARD}.cmake.")
endif()

set(FREERTOS_DIR      ${CMAKE_SOURCE_DIR}/freertos)
set(FREERTOS_PORT_DIR ${FREERTOS_DIR}/portable/GCC/${FREERTOS_PORT})

set(FREERTOS_SRC
    ${FREERTOS_DIR}/list.c
    ${FREERTOS_DIR}/queue.c
    ${FREERTOS_DIR}/tasks.c
    ${FREERTOS_DIR}/timers.c
    ${FREERTOS_DIR}/event_groups.c
    ${FREERTOS_DIR}/stream_buffer.c
    ${FREERTOS_PORT_DIR}/port.c
    ${FREERTOS_DIR}/portable/MemMang/heap_4.c
)

# Clock real (HSI de reset) -> usado pelo FreeRTOSConfig.h (configCPU_CLOCK_HZ).
add_compile_definitions(CONF_CPU_CLOCK_HZ=${CPU_CLOCK_HZ})

include_directories(
    ${LIBOPENCM3_INC}
    ${FREERTOS_DIR}/include
    ${FREERTOS_PORT_DIR}
    ${CMAKE_SOURCE_DIR}/config        # onde mora o FreeRTOSConfig.h
)

# --- 5. Alvo --------------------------------------------------------------
add_executable(${PROJECT_NAME}.elf src/main.c ${FREERTOS_SRC})
target_link_libraries(${PROJECT_NAME}.elf ${LIBOPENCM3_TGT})

add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex   $<TARGET_FILE:${PROJECT_NAME}.elf> ${PROJECT_NAME}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${PROJECT_NAME}.elf> ${PROJECT_NAME}.bin
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${PROJECT_NAME}.elf>
    COMMENT "Gerando .hex/.bin e tamanho do firmware"
)

# --- 6. Gravar na placa via ST-LINK --------------------------------------
# Alvo opcional. Uso: cmake --build build --target flash
# (ou selecione o alvo 'flash' no menu de Run do CLion).
find_program(STM32_PROGRAMMER_CLI
    NAMES STM32_Programmer_CLI
    PATHS "C:/ST/STM32CubeCLT_1.21.0/STM32CubeProgrammer/bin"
    DOC "STM32CubeProgrammer CLI")

if(STM32_PROGRAMMER_CLI)
    add_custom_target(flash
        COMMAND ${STM32_PROGRAMMER_CLI} -c port=SWD freq=4000 -w $<TARGET_FILE:${PROJECT_NAME}.elf> -v -rst
        DEPENDS ${PROJECT_NAME}.elf
        USES_TERMINAL
        COMMENT "Gravando ${PROJECT_NAME}.elf na placa via ST-LINK (SWD)")
else()
    message(STATUS "STM32_Programmer_CLI nao encontrado: alvo 'flash' indisponivel.")
endif()