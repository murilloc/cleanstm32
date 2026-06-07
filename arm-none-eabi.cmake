# Toolchain file: define QUAL FERRAMENTA usar (fixo para toda placa STM32).
# As flags de CPU NAO ficam aqui — ficam no perfil de placa (boards/*.cmake),
# porque mudam por chip. Aqui so' apontamos o compilador.

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

# Caminho do CubeCLT. Se mudar a versao, ajuste aqui (um unico lugar).
# Pode sobrescrever via -DTOOLCHAIN_PREFIX=... na linha de comando.
if(NOT DEFINED TOOLCHAIN_PREFIX)
    set(TOOLCHAIN_PREFIX "C:/ST/STM32CubeCLT_1.21.0/GNU-tools-for-STM32/bin/arm-none-eabi-")
endif()

set(CMAKE_C_COMPILER   ${TOOLCHAIN_PREFIX}gcc.exe)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}g++.exe)
set(CMAKE_ASM_COMPILER ${TOOLCHAIN_PREFIX}gcc.exe)
set(CMAKE_OBJCOPY      ${TOOLCHAIN_PREFIX}objcopy.exe CACHE INTERNAL "")
set(CMAKE_SIZE         ${TOOLCHAIN_PREFIX}size.exe    CACHE INTERNAL "")

# Impede o CMake de tentar LINKAR um executavel no teste do compilador
# (sem isto, ele monta um .exe de Windows e quebra no link — o erro
#  '--major-image-version' que voce ja viu).
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)