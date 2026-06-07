# Template libopencm3 + FreeRTOS multi-placa (STM32)

Um único projeto que serve várias placas STM32, com o blink rodando como uma
**task do FreeRTOS** sobre a libopencm3. Troca-se de placa passando
`-DBOARD=<nome>`, **sem editar código** — inclusive o port do FreeRTOS (Cortex-M3
/ M4F / M7) é escolhido pelo perfil da placa. Build com CMake + Ninja, toolchain
ARM GCC do STM32CubeCLT, flash e debug pelo CLion.

> 📖 **Guia completo, passo a passo (do zero ao debug) + seção de problemas e
> soluções:** [`GUIA_AMBIENTE_STM32.md`](GUIA_AMBIENTE_STM32.md).
> Este README é só o resumo.

## Placas suportadas

| `-DBOARD=` | MCU | Núcleo | Port FreeRTOS | LED | Flash / RAM |
|---|---|---|---|---|---|
| `BluePill`   | STM32F103C8 | Cortex-M3        | `ARM_CM3`      | PC13 | 64K / 20K   |
| `BlackPill`  | STM32F411CE | Cortex-M4F       | `ARM_CM4F`     | PC13 | 512K / 128K |
| `NucleoF446` | STM32F446RE | Cortex-M4F       | `ARM_CM4F`     | PA5 (LD2) | 512K / 128K |
| `NucleoF767` | STM32F767ZI | Cortex-M7 (FPU dupla) | `ARM_CM7/r0p1` | PB0 (LD1) | 2M / 512K |
| `NucleoG474` | STM32G474RE | Cortex-M4F       | `ARM_CM4F`     | PA5 (LD2) | 512K / 128K |

## Estrutura

```
cleanstm32/
  arm-none-eabi.cmake     toolchain (ferramentas; não muda por placa)
  CMakeLists.txt          seletor de placa + build (não muda por placa)
  boards/<placa>.cmake    perfil: CPU, família, memória, LED, port FreeRTOS (1 por placa)
  config/FreeRTOSConfig.h configuração do FreeRTOS (genérica p/ Cortex-M)
  src/main.c              aplicação (blink em task FreeRTOS); LED vem do perfil
  libopencm3/             biblioteca de periféricos (você clona e compila — ver abaixo)
  freertos/               kernel FreeRTOS (você clona — ver abaixo)
```

Princípio: **toolchain** = quais ferramentas usar; **perfil** = o que muda por
placa (família, flags de CPU, memória, pino do LED, port do FreeRTOS);
**CMakeLists** cola tudo.

## 1. Baixar e compilar a libopencm3 (uma vez por família usada)

```powershell
git clone https://github.com/libopencm3/libopencm3.git
cd libopencm3
make TARGETS="stm32/f1 stm32/f4 stm32/f7 stm32/g4"   # só as famílias que usar
cd ..
```

> Se o `git clone` falhar com erro de certificado SSL, use o backend do Windows:
> `git -c http.sslBackend=schannel clone <url>`.

## 1b. Baixar o FreeRTOS (uma vez)

Na raiz do projeto, clone o kernel na pasta `freertos/`:

```powershell
git clone --branch V11.1.0 --depth 1 https://github.com/FreeRTOS/FreeRTOS-Kernel.git freertos
```

Não é preciso compilar à parte: o `CMakeLists.txt` puxa os fontes do kernel, o
**port do core** (`portable/GCC/<port>`, escolhido pelo perfil) e o alocador
`heap_4.c` direto para o build do firmware.

> **Como libopencm3 e FreeRTOS convivem:** a tabela de vetores do libopencm3 espera
> os handlers `sv_call_handler` / `pend_sv_handler` / `sys_tick_handler`; o port do
> FreeRTOS exporta `vPortSVCHandler` / `xPortPendSVHandler` / `xPortSysTickHandler`.
> O `config/FreeRTOSConfig.h` faz a ponte com `#define` — sem isso, SysTick/PendSV
> nunca seriam chamados e o scheduler travaria.

## 2. Configurar no CLion

`Settings → Build, Execution, Deployment → CMake`. No campo **CMake options**:

```
-DCMAKE_TOOLCHAIN_FILE=arm-none-eabi.cmake -DBOARD=BluePill
```

Troque `BluePill` por `BlackPill`, `NucleoF446`, `NucleoF767` ou `NucleoG474`.
Dica: crie um perfil CMake por placa e troque pelo seletor de perfis.

Se o CubeCLT estiver em outra versão/caminho, edite `arm-none-eabi.cmake` ou
passe `-DTOOLCHAIN_PREFIX=C:/.../arm-none-eabi-` nas CMake options.

## 3. Build

Selecione o perfil e rode Build. No fim, o `size` imprime `text/data/bss`, e são
gerados `stm32_app.elf`, `.hex` e `.bin`.

Em linha de comando:
```powershell
cmake -S . -B build -G Ninja `
  -DCMAKE_MAKE_PROGRAM="C:/ST/STM32CubeCLT_1.21.0/Ninja/bin/ninja.exe" `
  -DCMAKE_TOOLCHAIN_FILE="$PWD/arm-none-eabi.cmake" -DBOARD=BluePill
cmake --build build
```

## 4. Gravar (flash)

Com a placa no ST-LINK. Pela IDE: selecione o alvo **`flash`** e clique no martelo
(`Ctrl+F9`). Por linha de comando:

```powershell
cmake --build build --target flash
```

## 5. Depurar

Configuração **Embedded GDB Server** no CLion (ST-LINK GDB server do CubeCLT) →
Debug (🐞). Campos detalhados no [guia](GUIA_AMBIENTE_STM32.md#a11-depurar-debug-com-breakpoints).

> Se o debug acusar `ST-LINK firmware upgrade required`, atualize o firmware da
> sonda com `STLinkUpgrade.bat` (passo detalhado no guia, Parte B, Problema 11).

## Adicionar uma placa nova

Copie um arquivo de `boards/`, ajuste `MCU_FAMILY`, `CPU_FLAGS`, `LIBOPENCM3_TGT`,
o LED (`LED_RCC/PORT/PIN`), o mapa de memória, o `FREERTOS_PORT` (conforme o core)
e o `CPU_CLOCK_HZ` (clock real do núcleo). Compile a família na libopencm3
(`make TARGETS=stm32/<familia>`). Pronto — `-DBOARD=SuaNova`.
