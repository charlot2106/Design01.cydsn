/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include "project.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "params.h"
#include "queue.h"

SemaphoreHandle_t semaph;
int compteur = 1;
task_params_t task_A = {
    .delay = 1000,
    .message = "Tache A en cours\n\r"
};
task_params_t task_B = {
    .delay = 999,
    .message = "Tache B en cours\n\r"
};

void taskLED(void *pvParameters)
{
    for (;;)
    {
        vTaskDelay(pdMS_TO_TICKS(500));
        Cy_GPIO_Write(P1_1_PORT, P1_1_NUM, !Cy_GPIO_Read(P1_1_PORT, P1_1_NUM));
    }
}  

void bouton_isr(void)
{
    xSemaphoreGiveFromISR(semaph, NULL);
    Cy_GPIO_ClearInterrupt(P0_4_PORT, P0_4_NUM);
    NVIC_ClearPendingIRQ(bouton_isr_cfg.intrSrc);
}

void bouton_task(void *pvParameters)
{
    for (;;)
    {
        vTaskDelay(pdMS_TO_TICKS(20));
        if (xSemaphoreTake(semaph,0) == true)
        {
            if (compteur%2 !=0)
                UART_PutString("\n Bouton appuye");
            else
                UART_PutString("\n Bouton relache");
            compteur++;
        }
    }
}

void print_loop(task_params_t *params)
{
    for (;;)
    {
        vTaskDelay(params->delay);
        UART_PutString(params->message);
    }
}
int main(void)
{
    __enable_irq(); /* Enable global interrupts. */
    
    UART_Start();
    semaph = xSemaphoreCreateBinary();

    /* Place your initialization/startup code here (e.g. MyInst_Start()) */
    xTaskCreate(taskLED, //Pointeur de la fonction qui implémente la tâche
                "LED", //Nom de la tâche
                200, //Stack Depth
                NULL, //Pas de task parameter
                3, //Priorité 3
                NULL); //Pas de task handle
    xTaskCreate(bouton_task, "Bouton", 100, NULL, 3, NULL);
    xTaskCreate(print_loop, "task A", configMINIMAL_STACK_SIZE, (void *) &task_A, 1, NULL);
    xTaskCreate(print_loop, "task B", configMINIMAL_STACK_SIZE, (void *) &task_B, 1, NULL);
                
    Cy_SysInt_Init(&bouton_isr_cfg, bouton_isr);
    NVIC_EnableIRQ(bouton_isr_cfg.intrSrc);
    vTaskStartScheduler();
    
    
    for(;;)
    {
    }
}

/* [] END OF FILE */
