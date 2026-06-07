/*
 * FreeRTOSConfig.h — configuracao generica para STM32 Cortex-M + libopencm3.
 *
 * Pontos-chave desta integracao:
 *   1. configCPU_CLOCK_HZ vem do perfil da placa (boards/<placa>.cmake) via a macro
 *      -DCONF_CPU_CLOCK_HZ, injetada pelo CMakeLists. Como o main.c NAO chama
 *      rcc_clock_setup, o nucleo roda no HSI de reset (8 MHz no F1, 16 MHz nos
 *      demais) — e e' esse valor que esta no perfil.
 *   2. O port do FreeRTOS exporta os handlers vPortSVCHandler / xPortPendSVHandler
 *      / xPortSysTickHandler, mas a tabela de vetores do libopencm3 espera os
 *      nomes sv_call_handler / pend_sv_handler / sys_tick_handler. Os #defines no
 *      fim deste arquivo fazem essa ponte (sem isso, o SysTick/PendSV do FreeRTOS
 *      nunca seriam chamados e o scheduler travaria).
 */
#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#ifndef CONF_CPU_CLOCK_HZ
#error "Defina CONF_CPU_CLOCK_HZ no perfil da placa (boards/<placa>.cmake) via CMake."
#endif

/* --- Escalonador e memoria ---------------------------------------------- */
#define configUSE_PREEMPTION                    1
#define configUSE_TIME_SLICING                  1
#define configCPU_CLOCK_HZ                      ( ( unsigned long ) CONF_CPU_CLOCK_HZ )
#define configTICK_RATE_HZ                      ( ( TickType_t ) 1000 )
#define configMAX_PRIORITIES                    ( 5 )
#define configMINIMAL_STACK_SIZE                ( ( unsigned short ) 128 )
#define configTOTAL_HEAP_SIZE                   ( ( size_t ) ( 8 * 1024 ) )
#define configMAX_TASK_NAME_LEN                 ( 16 )
#define configUSE_16_BIT_TICKS                  0
#define configIDLE_SHOULD_YIELD                 1
#define configSUPPORT_DYNAMIC_ALLOCATION        1
#define configSUPPORT_STATIC_ALLOCATION         0

/* --- Recursos opcionais -------------------------------------------------- */
#define configUSE_MUTEXES                       1
#define configUSE_RECURSIVE_MUTEXES             0
#define configUSE_COUNTING_SEMAPHORES           0
#define configUSE_TASK_NOTIFICATIONS            1
#define configQUEUE_REGISTRY_SIZE               0
#define configUSE_TIMERS                        0
#define configUSE_CO_ROUTINES                   0

/* --- Hooks e diagnostico ------------------------------------------------- */
#define configUSE_IDLE_HOOK                     0
#define configUSE_TICK_HOOK                     0
#define configCHECK_FOR_STACK_OVERFLOW          0
#define configUSE_MALLOC_FAILED_HOOK            0

/* --- API incluida na build ----------------------------------------------- */
#define INCLUDE_vTaskPrioritySet                1
#define INCLUDE_uxTaskPriorityGet               1
#define INCLUDE_vTaskDelete                     1
#define INCLUDE_vTaskSuspend                    1
#define INCLUDE_xTaskDelayUntil                 1
#define INCLUDE_vTaskDelay                      1

/* --- Prioridades de interrupcao (Cortex-M, 4 bits no STM32) -------------- */
#define configPRIO_BITS                         4
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY        15
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY   5
#define configKERNEL_INTERRUPT_PRIORITY \
        ( configLIBRARY_LOWEST_INTERRUPT_PRIORITY << ( 8 - configPRIO_BITS ) )
#define configMAX_SYSCALL_INTERRUPT_PRIORITY \
        ( configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << ( 8 - configPRIO_BITS ) )

/* --- Ponte de handlers: nomes do FreeRTOS -> nomes do vetor libopencm3 --- */
#define vPortSVCHandler                         sv_call_handler
#define xPortPendSVHandler                      pend_sv_handler
#define xPortSysTickHandler                     sys_tick_handler

/* --- assert minimo (trava em loop para o debugger pegar) ----------------- */
#define configASSERT( x ) \
        if( ( x ) == 0 ) { taskDISABLE_INTERRUPTS(); for( ;; ); }

#endif /* FREERTOS_CONFIG_H */
