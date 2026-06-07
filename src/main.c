/*
* Blink multi-placa com libopencm3.
 *
 * Separacao de responsabilidades:
 *   - O PINO do LED (LED_RCC/LED_PORT/LED_PIN) vem do PERFIL da placa
 *     (boards/*.cmake), repassado pelo CMakeLists via -DLED_*. Assim duas
 *     placas da mesma familia (ex.: Black Pill PC13 vs Nucleo-F446 PA5)
 *     convivem sem conflito.
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
    /* API das familias F4/F7/G4 — diferente da F1 (bug silencioso classico de porting) */
    gpio_mode_setup(LED_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LED_PIN);
    gpio_set_output_options(LED_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, LED_PIN);
#endif
}

int main(void)
{
    led_setup();

    while (1) {
        gpio_toggle(LED_PORT, LED_PIN);
        delay(200000);
    }

    return 0;
}