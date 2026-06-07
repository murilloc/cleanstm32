# Perfil: NUCLEO-144 STM32F767ZI
# Cortex-M7 com FPU de precisao dupla. Tres LEDs: LD1 verde (PB0),
# LD2 azul (PB7), LD3 vermelho (PB14). Usamos LD1 (PB0).

set(MCU_FAMILY      STM32F7)
set(CPU_FLAGS       -mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard)
set(LIBOPENCM3_TGT  opencm3_stm32f7)           # make TARGETS=stm32/f7
set(LIBOPENCM3_LD   cortex-m-generic.ld)

# FreeRTOS: port do core (Cortex-M7 r0p1) e clock real do nucleo.
# Sem rcc_clock_setup, o F767 roda no HSI de reset = 16 MHz.
set(FREERTOS_PORT   ARM_CM7/r0p1)
set(CPU_CLOCK_HZ    16000000)

# LED da placa (LD1 verde).
set(LED_RCC   RCC_GPIOB)
set(LED_PORT  GPIOB)
set(LED_PIN   GPIO0)

# F767ZI: 2 MB flash / 512K RAM (DTCM+SRAM contiguos a partir de 0x20000000).
set(FLASH_SIZE  2048K)
set(RAM_SIZE    512K)
set(FLASH_ORIGIN 0x08000000)
set(RAM_ORIGIN   0x20000000)
