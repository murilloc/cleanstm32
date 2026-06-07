# Perfil: Black Pill (STM32F411CE)
# Cortex-M4 com FPU de precisao simples. Cristal HSE de 25 MHz (atencao: difere
# do F103 e do livro, que assume 8 MHz — corrija o clock primeiro ao portar).

set(MCU_FAMILY      STM32F4)
set(CPU_FLAGS       -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard)
set(LIBOPENCM3_TGT  opencm3_stm32f4)        # make TARGETS=stm32/f4
set(LIBOPENCM3_LD   cortex-m-generic.ld)

# FreeRTOS: port do core (Cortex-M4F) e clock real do nucleo.
# Sem rcc_clock_setup, o F411 roda no HSI de reset = 16 MHz.
set(FREERTOS_PORT   ARM_CM4F)
set(CPU_CLOCK_HZ    16000000)

# LED da placa (a API de GPIO e' escolhida pela familia no main.c).
set(LED_RCC   RCC_GPIOC)
set(LED_PORT  GPIOC)
set(LED_PIN   GPIO13)

# F411CE: 512K flash / 128K RAM.
set(FLASH_SIZE  512K)
set(RAM_SIZE    128K)
set(FLASH_ORIGIN 0x08000000)
set(RAM_ORIGIN   0x20000000)