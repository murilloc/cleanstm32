/*
 * Blink multi-placa com libopencm3 + FreeRTOS.
 *
 * Separacao de responsabilidades (igual ao blink bare-metal):
 *   - O PINO do LED (LED_RCC/LED_PORT/LED_PIN) vem do PERFIL da placa
 *     (boards/<placa>.cmake), repassado pelo CMakeLists via -DLED_*.
 *   - A API de GPIO depende so' da FAMILIA (-D${MCU_FAMILY}):
 *       F1            -> gpio_set_mode()
 *       F4/F7/G4 etc. -> gpio_mode_setup() + gpio_set_output_options()
 *
 * Diferenca para a versao anterior: em vez de um while(1) com delay por NOP,
 * criamos UMA tarefa FreeRTOS que pisca o LED com vTaskDelay(). O tempo passa
 * a ser controlado pelo SysTick/escalonador, e o nucleo pode dormir no idle.
 * O mapeamento dos handlers SVC/PendSV/SysTick do FreeRTOS para os nomes do
 * vetor do libopencm3 esta' no config/FreeRTOSConfig.h.
 */

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

#include <FreeRTOS.h>
#include <task.h>

#if !defined(LED_RCC) || !defined(LED_PORT) || !defined(LED_PIN)
  #error "Defina LED_RCC/LED_PORT/LED_PIN no perfil da placa (boards/<placa>.cmake)."
#endif

static void led_setup(void)
{
    rcc_periph_clock_enable(LED_RCC);

#if defined(STM32F1)
    /* API da familia F1 */
    gpio_set_mode(LED_PORT, GPIO_MODE_OUTPUT_2_MHZ,
                  GPIO_CNF_OUTPUT_PUSHPULL, LED_PIN);
#else
    /* API das familias F4/F7/G4 — diferente da F1 (bug silencioso de porting) */
    gpio_mode_setup(LED_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LED_PIN);
    gpio_set_output_options(LED_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, LED_PIN);
#endif
}

/* Tarefa unica: alterna o LED a cada 500 ms cedendo a CPU no vTaskDelay. */
static void blink_task(void *args)
{
    (void)args;

    for (;;) {
        gpio_toggle(LED_PORT, LED_PIN);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

int main(void)
{
    led_setup();

    xTaskCreate(blink_task, "blink", configMINIMAL_STACK_SIZE,
                NULL, tskIDLE_PRIORITY + 1, NULL);

    vTaskStartScheduler();

    /* Nunca deveria chegar aqui: so' retorna se faltar heap para o scheduler. */
    for (;;) {
    }
    return 0;
}
