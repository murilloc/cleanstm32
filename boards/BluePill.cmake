# Perfil: Blue Pill (STM32F103C8T6)
# Cortex-M3, SEM FPU (float por software). Cristal HSE de 8 MHz.
# LED onboard em PC13 (ligado em nivel baixo — acende com o pino em 0).

set(MCU_FAMILY      STM32F1)
set(CPU_FLAGS       -mcpu=cortex-m3 -mthumb)   # M3 nao tem FPU: nada de -mfpu/-mfloat-abi
set(LIBOPENCM3_TGT  opencm3_stm32f1)           # make TARGETS=stm32/f1
set(LIBOPENCM3_LD   cortex-m-generic.ld)

# FreeRTOS: port do core (Cortex-M3) e clock real do nucleo.
# Sem rcc_clock_setup, o F103 roda no HSI de reset = 8 MHz.
set(FREERTOS_PORT   ARM_CM3)
set(CPU_CLOCK_HZ    8000000)

# LED da placa (a API de GPIO e' escolhida pela familia no main.c).
set(LED_RCC   RCC_GPIOC)
set(LED_PORT  GPIOC)
set(LED_PIN   GPIO13)

# F103C8: 64K flash / 20K RAM.
set(FLASH_SIZE  64K)
set(RAM_SIZE    20K)
set(FLASH_ORIGIN 0x08000000)
set(RAM_ORIGIN   0x20000000)
