# Guia: Ambiente de Desenvolvimento STM32 (libopencm3 + CMake + CLion)

Guia passo a passo para montar, do zero, um ambiente de desenvolvimento
bare-metal para placas STM32 no **Windows**, usando:

- **STM32CubeCLT** (toolchain ARM GCC, CMake, Ninja, ST-LINK GDB server, CubeProgrammer CLI)
- **libopencm3** (biblioteca de periféricos, alternativa leve à HAL/CubeMX)
- **CLion** como IDE (build, flash e debug com breakpoints)

O projeto é **multi-placa**: um único código-fonte serve várias placas; troca-se
de placa apenas passando `-DBOARD=<placa>`, sem editar código.

> A [Parte B — Problemas Encontrados e Soluções](#parte-b--problemas-encontrados-e-soluções)
> reúne todos os erros que apareceram durante a montagem e como cada um foi resolvido.
> Vale a leitura mesmo que tudo funcione de primeira.

---

## Sumário

- [Pré-requisitos](#pré-requisitos)
- [Placas suportadas](#placas-suportadas)
- [Estrutura final do projeto](#estrutura-final-do-projeto)
- [Parte A — Passo a passo](#parte-a--passo-a-passo)
  - [A.1 Instalar o STM32CubeCLT](#a1-instalar-o-stm32cubeclt)
  - [A.2 Criar a estrutura de pastas](#a2-criar-a-estrutura-de-pastas)
  - [A.3 Toolchain file (`arm-none-eabi.cmake`)](#a3-toolchain-file-arm-none-eabicmake)
  - [A.4 `CMakeLists.txt`](#a4-cmakeliststxt)
  - [A.5 Perfis de placa (`boards/*.cmake`)](#a5-perfis-de-placa-boardscmake)
  - [A.6 Código da aplicação (`src/main.c`)](#a6-código-da-aplicação-srcmainc)
  - [A.7 Baixar e compilar a libopencm3](#a7-baixar-e-compilar-a-libopencm3)
  - [A.8 Configurar o CLion](#a8-configurar-o-clion)
  - [A.9 Compilar (build)](#a9-compilar-build)
  - [A.10 Gravar na placa (flash)](#a10-gravar-na-placa-flash)
  - [A.11 Depurar (debug com breakpoints)](#a11-depurar-debug-com-breakpoints)
- [Parte B — Problemas encontrados e soluções](#parte-b--problemas-encontrados-e-soluções)
- [Apêndice — Referência rápida](#apêndice--referência-rápida)

---

## Pré-requisitos

**Hardware**
- Uma placa STM32 (ex.: Blue Pill STM32F103C8).
- Uma sonda **ST-LINK V2** (avulsa, para Blue Pill/Black Pill) ou o ST-LINK
  embutido (placas Nucleo).

**Software**
- Windows 10/11.
- **STM32CubeCLT** (testado com a versão **1.21.0**, instalado em `C:\ST\STM32CubeCLT_1.21.0`).
- **CLion** (testado com a versão **2026.1.2**).
- **Git** (Git for Windows).
- Opcional: **STM32CubeIDE** instalado — traz um **OpenOCD** que serve de
  alternativa de debug para sondas clone (ver Parte B, Problema 11).

Os caminhos neste guia assumem CubeCLT `1.21.0`. Se a sua versão for outra,
ajuste o número da pasta nos comandos e arquivos.

---

## Placas suportadas

| `-DBOARD=` | MCU | Núcleo | LED de usuário | Flash / RAM | Alvo libopencm3 |
|---|---|---|---|---|---|
| `BluePill`   | STM32F103C8 | Cortex-M3        | PC13 | 64K / 20K   | `opencm3_stm32f1` |
| `BlackPill`  | STM32F411CE | Cortex-M4F       | PC13 | 512K / 128K | `opencm3_stm32f4` |
| `NucleoF446` | STM32F446RE | Cortex-M4F       | PA5 (LD2) | 512K / 128K | `opencm3_stm32f4` |
| `NucleoF767` | STM32F767ZI | Cortex-M7 (FPU dupla) | PB0 (LD1) | 2M / 512K | `opencm3_stm32f7` |
| `NucleoG474` | STM32G474RE | Cortex-M4F       | PA5 (LD2) | 512K / 128K | `opencm3_stm32g4` |

---

## Estrutura final do projeto

```
cleanstm32/
├── arm-none-eabi.cmake      # toolchain (ferramentas; igual para toda placa)
├── CMakeLists.txt           # seletor de placa + build (não muda por placa)
├── boards/                  # 1 perfil por placa (CPU, família, memória, LED)
│   ├── BluePill.cmake
│   ├── BlackPill.cmake
│   ├── NucleoF446.cmake
│   ├── NucleoF767.cmake
│   └── NucleoG474.cmake
├── src/
│   └── main.c               # aplicação (blink); LED vem do perfil
└── libopencm3/              # clonada e compilada (passo A.7)
```

**Princípio de organização:**
- **Toolchain** (`arm-none-eabi.cmake`) → *quais ferramentas* usar. Igual para toda placa.
- **Perfil** (`boards/*.cmake`) → *o que muda por placa*: família, flags de CPU,
  memória e pino do LED.
- **`CMakeLists.txt`** → cola tudo; não precisa ser editado ao trocar de placa.

---

## Parte A — Passo a passo

### A.1 Instalar o STM32CubeCLT

Baixe e instale o STM32CubeCLT (site da ST). Ele entrega, num só pacote:

| Ferramenta | Caminho (CubeCLT 1.21.0) |
|---|---|
| ARM GCC | `C:\ST\STM32CubeCLT_1.21.0\GNU-tools-for-STM32\bin\arm-none-eabi-gcc.exe` |
| GDB | `...\GNU-tools-for-STM32\bin\arm-none-eabi-gdb.exe` |
| Ninja | `C:\ST\STM32CubeCLT_1.21.0\Ninja\bin\ninja.exe` |
| CubeProgrammer CLI | `...\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe` |
| ST-LINK GDB server | `...\STLink-gdb-server\bin\ST-LINK_gdbserver.exe` |
| Atualizador de firmware ST-LINK | `C:\ST\STM32CubeCLT_1.21.0\STLinkUpgrade.bat` |

Confirme no terminal (PowerShell):

```powershell
arm-none-eabi-gcc --version
git --version
```

### A.2 Criar a estrutura de pastas

Na raiz do projeto:

```powershell
New-Item -ItemType Directory boards
New-Item -ItemType Directory src
```

### A.3 Toolchain file (`arm-none-eabi.cmake`)

Diz ao CMake para compilar para ARM Cortex-M (cross-compile), não para o PC.

```cmake
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
# (sem isto, ele monta um .exe de Windows e quebra no link).
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
```

> **Atenção a parênteses:** cada `set(...)` precisa fechar o `)`. Um `)` esquecido
> funde dois comandos e gera erros confusos (ver Parte B, Problema 1).

### A.4 `CMakeLists.txt`

> **O nome do arquivo precisa ser exatamente `CMakeLists.txt`** — com **`s`** e com
> esse "case". `CMakeList.txt` (sem o `s`) é silenciosamente ignorado pelo CMake
> (ver Parte B, Problema 3).

```cmake
cmake_minimum_required(VERSION 3.20)

# ============================================================================
# Template libopencm3 multi-placa.
# Uso: passe -DBOARD=<placa>, onde <placa> e' o nome de um arquivo em boards/.
#   Placas prontas: BluePill, BlackPill, NucleoF446, NucleoF767, NucleoG474
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

# Carrega o perfil: define MCU_FAMILY, CPU_FLAGS, LIBOPENCM3_TGT, LIBOPENCM3_LD,
# LED_RCC/LED_PORT/LED_PIN, FLASH_SIZE, RAM_SIZE, FLASH_ORIGIN, RAM_ORIGIN.
include(${BOARD_FILE})
message(STATUS "Placa: ${BOARD}  Familia: ${MCU_FAMILY}  Flags: ${CPU_FLAGS}")

project(stm32_app C ASM)
set(CMAKE_C_STANDARD 11)

# --- 2. Caminhos do libopencm3 -------------------------------------------
set(LIBOPENCM3_DIR ${CMAKE_SOURCE_DIR}/libopencm3)
set(LIBOPENCM3_INC ${LIBOPENCM3_DIR}/include)
set(LIBOPENCM3_LIB ${LIBOPENCM3_DIR}/lib)

# --- 3. Linker script gerado a partir do perfil ---------------------------
# Geramos o mapa de memoria a partir das variaveis do perfil e damos INCLUDE
# no script generico do libopencm3 (cortex-m-generic.ld).
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

include_directories(${LIBOPENCM3_INC})

# --- 5. Alvo --------------------------------------------------------------
add_executable(${PROJECT_NAME}.elf src/main.c)
target_link_libraries(${PROJECT_NAME}.elf ${LIBOPENCM3_TGT})

add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex   $<TARGET_FILE:${PROJECT_NAME}.elf> ${PROJECT_NAME}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${PROJECT_NAME}.elf> ${PROJECT_NAME}.bin
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${PROJECT_NAME}.elf>
    COMMENT "Gerando .hex/.bin e tamanho do firmware"
)

# --- 6. Gravar na placa via ST-LINK --------------------------------------
# Alvo opcional. Uso: cmake --build build --target flash
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
```

### A.5 Perfis de placa (`boards/*.cmake`)

Cada perfil define o que muda por placa. Crie um arquivo por placa.

**`boards/BluePill.cmake`**
```cmake
# Blue Pill (STM32F103C8T6) — Cortex-M3, SEM FPU. LED em PC13.
set(MCU_FAMILY      STM32F1)
set(CPU_FLAGS       -mcpu=cortex-m3 -mthumb)   # M3 nao tem FPU
set(LIBOPENCM3_TGT  opencm3_stm32f1)
set(LIBOPENCM3_LD   cortex-m-generic.ld)
set(LED_RCC   RCC_GPIOC)
set(LED_PORT  GPIOC)
set(LED_PIN   GPIO13)
set(FLASH_SIZE  64K)
set(RAM_SIZE    20K)
set(FLASH_ORIGIN 0x08000000)
set(RAM_ORIGIN   0x20000000)
```

**`boards/BlackPill.cmake`**
```cmake
# Black Pill (STM32F411CE) — Cortex-M4F. LED em PC13.
set(MCU_FAMILY      STM32F4)
set(CPU_FLAGS       -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard)
set(LIBOPENCM3_TGT  opencm3_stm32f4)
set(LIBOPENCM3_LD   cortex-m-generic.ld)
set(LED_RCC   RCC_GPIOC)
set(LED_PORT  GPIOC)
set(LED_PIN   GPIO13)
set(FLASH_SIZE  512K)
set(RAM_SIZE    128K)
set(FLASH_ORIGIN 0x08000000)
set(RAM_ORIGIN   0x20000000)
```

**`boards/NucleoF446.cmake`**
```cmake
# NUCLEO-64 STM32F446RE — Cortex-M4F. LED LD2 em PA5.
set(MCU_FAMILY      STM32F4)
set(CPU_FLAGS       -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard)
set(LIBOPENCM3_TGT  opencm3_stm32f4)
set(LIBOPENCM3_LD   cortex-m-generic.ld)
set(LED_RCC   RCC_GPIOA)
set(LED_PORT  GPIOA)
set(LED_PIN   GPIO5)
set(FLASH_SIZE  512K)
set(RAM_SIZE    128K)
set(FLASH_ORIGIN 0x08000000)
set(RAM_ORIGIN   0x20000000)
```

**`boards/NucleoF767.cmake`**
```cmake
# NUCLEO-144 STM32F767ZI — Cortex-M7 (FPU dupla). LD1 verde em PB0.
set(MCU_FAMILY      STM32F7)
set(CPU_FLAGS       -mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard)
set(LIBOPENCM3_TGT  opencm3_stm32f7)
set(LIBOPENCM3_LD   cortex-m-generic.ld)
set(LED_RCC   RCC_GPIOB)
set(LED_PORT  GPIOB)
set(LED_PIN   GPIO0)
set(FLASH_SIZE  2048K)
set(RAM_SIZE    512K)
set(FLASH_ORIGIN 0x08000000)
set(RAM_ORIGIN   0x20000000)
```

**`boards/NucleoG474.cmake`**
```cmake
# NUCLEO-64 STM32G474RE — Cortex-M4F. LED LD2 em PA5.
set(MCU_FAMILY      STM32G4)
set(CPU_FLAGS       -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard)
set(LIBOPENCM3_TGT  opencm3_stm32g4)
set(LIBOPENCM3_LD   cortex-m-generic.ld)
set(LED_RCC   RCC_GPIOA)
set(LED_PORT  GPIOA)
set(LED_PIN   GPIO5)
set(FLASH_SIZE  512K)
set(RAM_SIZE    128K)
set(FLASH_ORIGIN 0x08000000)
set(RAM_ORIGIN   0x20000000)
```

> **Por que o LED vem do perfil, e não do `main.c`?** Duas placas da mesma família
> (Black Pill e Nucleo-F446, ambas `STM32F4`) têm o LED em pinos diferentes
> (PC13 vs PA5). Selecionar o pino pela família entraria em conflito. Ver Parte B,
> Problema 9.

### A.6 Código da aplicação (`src/main.c`)

Blink que se adapta: o **pino** vem do perfil (via `-D`); a **API de GPIO** é
escolhida pela **família** (F1 usa `gpio_set_mode`; F4/F7/G4 usam `gpio_mode_setup`).

```c
/*
 * Blink multi-placa com libopencm3.
 *   - O PINO do LED (LED_RCC/LED_PORT/LED_PIN) vem do PERFIL da placa
 *     (boards/*.cmake), repassado pelo CMakeLists via -DLED_*.
 *   - A API de GPIO depende so' da FAMILIA (-D${MCU_FAMILY}):
 *       F1            -> gpio_set_mode()
 *       F4/F7/G4 etc. -> gpio_mode_setup() + gpio_set_output_options()
 */

#include <stdint.h>

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

#if !defined(LED_RCC) || !defined(LED_PORT) || !defined(LED_PIN)
  #error "Defina LED_RCC/LED_PORT/LED_PIN no perfil da placa (boards/*.cmake)."
#endif

static void delay(volatile uint32_t count)
{
    while (count--) {
        __asm__("nop");
    }
}

static void led_setup(void)
{
    rcc_periph_clock_enable(LED_RCC);

#if defined(STM32F1)
    /* API da familia F1 */
    gpio_set_mode(LED_PORT, GPIO_MODE_OUTPUT_2_MHZ,
                  GPIO_CNF_OUTPUT_PUSHPULL, LED_PIN);
#else
    /* API das familias F4/F7/G4 */
    gpio_mode_setup(LED_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LED_PIN);
    gpio_set_output_options(LED_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, LED_PIN);
#endif
}

int main(void)
{
    led_setup();

    while (1) {
        gpio_toggle(LED_PORT, LED_PIN);
        delay(200000);   // menor = pisca mais rapido
    }

    return 0;
}
```

> O `#include <stdint.h>` é necessário por causa de `uint32_t`. Sem ele, o código
> só compila "por acidente" via include transitivo (ver Parte B, Problema 2).

### A.7 Baixar e compilar a libopencm3

Na raiz do projeto, clone a biblioteca:

```powershell
git clone https://github.com/libopencm3/libopencm3.git
```

> **Se der erro de certificado SSL** (`unable to get local issuer certificate`),
> use o backend de certificados do Windows:
> ```powershell
> git -c http.sslBackend=schannel clone https://github.com/libopencm3/libopencm3.git
> ```
> (ver Parte B, Problema 7).

Compile **apenas as famílias que você vai usar** (cada `TARGETS` gera um `.a`):

```powershell
cd libopencm3
# Blue Pill=f1, Black Pill/Nucleo-F446=f4, Nucleo-F767=f7, Nucleo-G474=g4
make TARGETS="stm32/f1 stm32/f4 stm32/f7 stm32/g4"
cd ..
```

Ao terminar, confirme os artefatos (ex.: F1):

```
libopencm3/lib/libopencm3_stm32f1.a     <- biblioteca compilada
libopencm3/lib/cortex-m-generic.ld      <- linker script generico
```

> **Importante:** a libopencm3 atual **não** gera mais um `.ld` por família
> (ex.: `libopencm3_stm32f4.ld`). Existe um único `cortex-m-generic.ld`, e o
> projeto fornece o bloco `MEMORY` — exatamente o que o `CMakeLists.txt` faz no
> passo 3 (ver Parte B, Problema 8).

### A.8 Configurar o CLion

**a) CMake options** — `File → Settings → Build, Execution, Deployment → CMake`.

No perfil (ex.: *Debug*), no campo **CMake options**:

```
-DCMAKE_TOOLCHAIN_FILE=arm-none-eabi.cmake -DBOARD=BluePill
```

- **Generator**: *Let CMake decide* ou **Ninja**.
- Para trocar de placa, mude `-DBOARD=`. Dica: crie **um perfil CMake por placa**
  (botão `+`), cada um com seu `-DBOARD=`, e troque pelo seletor de perfis.

**b) Toolchain** — `Settings → Build, Execution, Deployment → Toolchains`.
O CLion precisa de CMake + build tool + debugger. Os compiladores C/C++ são
sobrescritos pelo toolchain file, então o bundled já serve. Para o debug, o GDB
é informado na própria configuração de debug (passo A.11).

> O editor pode mostrar erros falsos (`#error`, "Cannot resolve symbol") **antes**
> de o perfil CMake com os `-D` ser carregado. Recarregue o projeto CMake. O build
> real é a fonte da verdade (ver Parte B, Problema 10).

### A.9 Compilar (build)

Equivalente em linha de comando (útil para validar fora do CLion):

```powershell
$NINJA = "C:/ST/STM32CubeCLT_1.21.0/Ninja/bin/ninja.exe"
cmake -S . -B build -G Ninja `
  -DCMAKE_MAKE_PROGRAM="$NINJA" `
  -DCMAKE_TOOLCHAIN_FILE="$PWD/arm-none-eabi.cmake" `
  -DBOARD=BluePill
cmake --build build
```

No fim, o `size` imprime `text/data/bss`, e são gerados `stm32_app.elf`,
`stm32_app.hex` e `stm32_app.bin`.

> **Não use o gerador "MinGW Makefiles"** se não tiver `mingw32-make`: dá
> `CMAKE_MAKE_PROGRAM is not set`. Use **Ninja** (vem no CubeCLT). Ver Parte B,
> Problema 6.

### A.10 Gravar na placa (flash)

Com a placa conectada via ST-LINK. **Pela IDE:** selecione o alvo **`flash`** no
seletor de configurações e clique no **martelo de build** (`Ctrl+F9`).

**Por linha de comando:**
```powershell
cmake --build build --target flash
```

Que executa, por baixo:
```
STM32_Programmer_CLI -c port=SWD freq=4000 -w stm32_app.elf -v -rst
```

Saída esperada: `File download complete` → `Download verified successfully` →
`Software reset is performed`. O LED deve começar a piscar.

> **Se a conexão falhar** numa Blue Pill, tente conectar "sob reset":
> ```powershell
> STM32_Programmer_CLI -c port=SWD mode=UR -w build/stm32_app.elf -v -rst
> ```

### A.11 Depurar (debug com breakpoints)

`Run → Edit Configurations → + → Embedded GDB Server`:

| Campo | Valor |
|---|---|
| **Name** | `BluePill Debug` |
| **Target / Executable** | `stm32_app.elf` |
| **Download executable** | *Always* |
| **'target remote' args** | `tcp:localhost:61234` |
| **GDB Server** | `C:\ST\STM32CubeCLT_1.21.0\STLink-gdb-server\bin\ST-LINK_gdbserver.exe` |
| **GDB Server args** | `-p 61234 --swd -cp "C:\ST\STM32CubeCLT_1.21.0\STM32CubeProgrammer\bin"` |
| **GDB** | `C:\ST\STM32CubeCLT_1.21.0\GNU-tools-for-STM32\bin\arm-none-eabi-gdb.exe` |

Selecione `BluePill Debug` e clique **Debug (🐞)**. Ele grava o `.elf`, sobe o
GDB server e para no início do `main`. A partir daí: breakpoints, inspeção de
variáveis, Step Over (F8), Continue (F9).

> **Se aparecer `ST-LINK firmware upgrade required` (exit code 6)**, é preciso
> atualizar o firmware da sonda ST-LINK. Ver Parte B, Problema 11 — é o passo mais
> "manual" de toda a montagem.

---

## Parte B — Problemas encontrados e soluções

Lista de todos os problemas reais enfrentados ao montar este ambiente, em ordem
aproximada de aparição, com a causa e a correção adotada.

### Problema 1 — Parêntese faltando no toolchain file
- **Sintoma:** erros confusos de CMake ao processar `arm-none-eabi.cmake`.
- **Causa:** a linha de `set(CMAKE_ASM_COMPILER ...)` estava **sem o `)` de
  fechamento**, fundindo-a com o `set()` seguinte. O CMake passou a interpretar
  duas variáveis como uma só.
- **Solução:** fechar o `)`. Aproveitou-se para refatorar os caminhos usando uma
  única variável `TOOLCHAIN_PREFIX` (um só lugar para ajustar a versão do CubeCLT).

### Problema 2 — `uint32_t` sem `#include <stdint.h>`
- **Sintoma:** nenhum erro imediato (compilava), mas dependência frágil.
- **Causa:** `main.c` usava `uint32_t` sem incluir `<stdint.h>`; só funcionava
  porque o header da libopencm3 puxava o tipo **transitivamente**.
- **Solução:** adicionar `#include <stdint.h>` explicitamente no topo. Declarar a
  dependência em vez de depender da ordem de includes.

### Problema 3 — Nome do arquivo: `CMakeList.txt`
- **Sintoma:** o CMake "não encontrava" o projeto / ignorava o arquivo.
- **Causa:** o arquivo se chamava `CMakeList.txt` (**faltava o `s`**). O CMake só
  reconhece **`CMakeLists.txt`**.
- **Solução:** renomear para `CMakeLists.txt`.

### Problema 4 — Caminho do fonte: `src/main.c` vs `main.c`
- **Sintoma:** o `add_executable` referenciava `src/main.c`, mas o arquivo estava
  na raiz.
- **Causa:** descompasso entre a estrutura esperada e a real.
- **Solução:** padronizar na estrutura `src/` — criar a pasta `src/` e mover o
  `main.c` para `src/main.c`, mantendo `add_executable(... src/main.c)`.

### Problema 5 — Pastas `boards/` e `libopencm3/` ausentes
- **Sintoma:** `FATAL_ERROR: Placa '...' nao encontrada` e, depois,
  `Cannot find directory 'libopencm3'` / símbolos não resolvidos.
- **Causa:** o `CMakeLists.txt` dependia de coisas que ainda não existiam no
  projeto.
- **Solução:** criar os perfis em `boards/` (passo A.5) e clonar/compilar a
  libopencm3 (passo A.7).

### Problema 6 — Toolchain file não é aplicado sozinho
- **Sintoma:** CMake tentava usar o compilador do PC; nada de ARM funcionava.
- **Causa:** o `arm-none-eabi.cmake` não é incluído pelo `CMakeLists.txt`; precisa
  ser informado ao CMake.
- **Solução:** passar `-DCMAKE_TOOLCHAIN_FILE=arm-none-eabi.cmake` nas **CMake
  options** do CLion (ou na linha de comando).

### Problema 7 — `git clone` falha com erro de certificado SSL
- **Sintoma:** `fatal: unable to access ... SSL certificate problem: unable to get
  local issuer certificate`.
- **Causa:** o Git não encontrou o bundle de CAs no ambiente em uso.
- **Solução:** usar o backend de certificados do Windows:
  `git -c http.sslBackend=schannel clone <url>`.

### Problema 8 — Linker script da família não existe mais
- **Sintoma:** o perfil apontava `LIBOPENCM3_LD libopencm3_stm32f4.ld`, mas esse
  arquivo não era gerado pela libopencm3.
- **Causa:** versões atuais da libopencm3 **não geram mais** um `.ld` por família.
  Agora há um único `cortex-m-generic.ld`, e o projeto deve fornecer o bloco
  `MEMORY` e dar `INCLUDE` nele.
- **Solução:** nos perfis, usar `set(LIBOPENCM3_LD cortex-m-generic.ld)`. O
  `CMakeLists.txt` gera `memory.ld` com as regiões `rom`/`ram` (nomes que o script
  genérico espera) e faz `INCLUDE cortex-m-generic.ld`.

### Problema 9 — Conflito de LED entre placas da mesma família
- **Sintoma:** com a seleção do LED por família, duas placas `STM32F4` (Black Pill
  PC13 e Nucleo-F446 PA5) não podiam coexistir; e a família `STM32F7` nem tinha
  branch no `main.c` (caía no `#error`).
- **Causa:** o pino do LED estava amarrado à *família*, mas o pino é uma
  característica da *placa*.
- **Solução:** mover a definição do LED para o **perfil da placa**
  (`LED_RCC/LED_PORT/LED_PIN`), repassada via `-D` pelo `CMakeLists.txt`. O
  `main.c` passou a usar o LED do perfil e a escolher só a **API de GPIO** pela
  família (incluindo suporte a F7).

### Problema 10 — Erros falsos no editor do CLion
- **Sintoma:** `#error directive`, `Cannot resolve symbol 'gpio_...'`, "unused
  include" no editor, mesmo com o build funcionando.
- **Causa:** o analisador (clangd) ainda não tinha os `-D` (família, LED) nem os
  include paths da libopencm3, porque o perfil CMake não havia sido carregado.
- **Solução:** recarregar o projeto CMake com o `-DBOARD=...` correto. Tratar o
  **build real** como fonte da verdade, não o índice do editor.

### Problema 11 — Debug: "ST-LINK firmware upgrade required" (exit code 6)
Este foi o problema com mais etapas. Foi resolvido em três fases:

1. **Erro inicial:** ao iniciar o Debug, o GDB server abortava com
   `ST-LINK firmware upgrade required ... GDB Server stopped, exit code 6`.
   - **Causa:** o ST-LINK GDB server exige um firmware de sonda mais novo que o
     instalado (`V2J37S7`). Curiosamente, o `STM32_Programmer_CLI` (usado para
     gravar) é mais tolerante e funcionava — ou seja, **o flash não ficou
     bloqueado**, só o debug.
   - **Solução:** atualizar o firmware da sonda com o utilitário
     `C:\ST\STM32CubeCLT_1.21.0\STLinkUpgrade.bat`.

2. **Atualizador em linha de comando falhou:** o modo `-update` (headless) parou
   com `ST-Link is not in the DFU mode. Please restart it.`, e na janela gráfica o
   botão **"Upgrade" aparecia desabilitado**.
   - **Causa:** a sonda precisa entrar em **modo DFU/update** antes da escrita —
     passo que a linha de comando não dispara sozinha.
   - **Solução:** usar a **janela gráfica** do utilitário e, crucialmente,
     **desconectar e reconectar** o cabo USB do ST-LINK. Após reconectar, a sonda
     enumerou no modo certo, o botão "Upgrade" habilitou, e a atualização concluiu
     (`V2J37S7` → `V2J47S7`).

3. **Estado transitório pós-upgrade:** logo após atualizar, a sonda aparecia com
   número de série inválido (um ID de dispositivo do Windows), firmware em branco,
   e a conexão SWD dava `DEV_CONNECT_ERR`.
   - **Causa:** a sonda re-enumerou sem publicar os descritores USB corretos.
   - **Solução:** **desconectar e reconectar fisicamente** o cabo USB mais uma
     vez. Depois disso, tudo normalizou — número de série correto, firmware
     `V2J47S7`, e a conexão SWD passou a ler o alvo (Device ID `0x410`,
     *STM32F103 Medium-density*). O Debug pela IDE passou a funcionar.

   > **Lição:** sempre que mexer no firmware do ST-LINK, faça um ciclo de
   > **unplug/replug** do USB antes de testar. E lembre que **flash e debug usam
   > caminhos diferentes**: o flash (CubeProgrammer) pode funcionar mesmo com a
   > sonda exigindo upgrade para o debug (GDB server).

### Alternativa de debug — OpenOCD (para sondas clone)
- **Contexto:** ST-LINK V2 **clones** (comuns em kits de Blue Pill) às vezes são
  recusados pelo atualizador oficial da ST. No caso deste guia a sonda era
  genuína (só precisou reconectar), mas se o upgrade for impossível:
- **Solução:** usar **OpenOCD** em vez do ST-LINK GDB server — ele aceita firmwares
  antigos e clones. Um OpenOCD costuma vir instalado com o **STM32CubeIDE**
  (ex.: `C:\ST\STM32CubeIDE_*\...\tools\bin\openocd.exe`). Configura-se uma
  *Embedded GDB Server* no CLion apontando para o `openocd.exe` com
  `-f interface/stlink.cfg -f target/stm32f1x.cfg` (ajuste o target por família).

---

## Apêndice — Referência rápida

### Comandos do dia a dia (PowerShell, na raiz do projeto)

```powershell
# Configurar (1a vez ou ao trocar de placa)
cmake -S . -B build -G Ninja `
  -DCMAKE_MAKE_PROGRAM="C:/ST/STM32CubeCLT_1.21.0/Ninja/bin/ninja.exe" `
  -DCMAKE_TOOLCHAIN_FILE="$PWD/arm-none-eabi.cmake" -DBOARD=BluePill

# Compilar
cmake --build build

# Gravar
cmake --build build --target flash

# Listar sondas ST-LINK e firmware
& "C:/ST/STM32CubeCLT_1.21.0/STM32CubeProgrammer/bin/STM32_Programmer_CLI.exe" -l st-link

# Testar conexao SWD (so leitura)
& "C:/ST/STM32CubeCLT_1.21.0/STM32CubeProgrammer/bin/STM32_Programmer_CLI.exe" -c port=SWD mode=normal
```

### Fluxo no CLion

| Quero... | Faço... |
|---|---|
| Trocar de placa | Mudar `-DBOARD=` no perfil CMake (ou trocar de perfil) |
| Só gravar | Selecionar alvo `flash` + martelo (`Ctrl+F9`) |
| Gravar e depurar | Config `Embedded GDB Server` + Debug (🐞) |
| Piscar mais rápido/devagar | Ajustar `delay(...)` no `src/main.c` |

### Adicionar uma placa nova

1. Copie um arquivo de `boards/` e ajuste: `MCU_FAMILY`, `CPU_FLAGS`,
   `LIBOPENCM3_TGT`, o LED (`LED_RCC/PORT/PIN`) e o mapa de memória
   (`FLASH_*`, `RAM_*`).
2. Compile a família correspondente: `make TARGETS=stm32/<familia>` na libopencm3.
3. Se for uma família nova (ex.: F7), garanta que o `main.c` trata a API dela
   (F1 vs. demais).
4. Use `-DBOARD=<NomeDoArquivo>`.

### Tabela de flags de CPU por núcleo

| Núcleo | Flags |
|---|---|
| Cortex-M3 (F1) | `-mcpu=cortex-m3 -mthumb` |
| Cortex-M4F (F4/G4) | `-mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard` |
| Cortex-M7 (F7) | `-mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard` |

---

*Ambiente validado com STM32CubeCLT 1.21.0, CLion 2026.1.2, libopencm3 (clone
atual) e Blue Pill STM32F103C8 + ST-LINK V2 (firmware V2J47S7).*
