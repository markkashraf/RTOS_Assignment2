#include <stdint.h>
#include "scheduler.h"
#include <stdlib.h>

Task_t * volatile OS_curr; 
Task_t * volatile OS_next; 

void OS_init(void) {
    *(uint32_t volatile *)0xE000ED20 |= (0xFFU << 16); // PendSV Lowest Priority
}

void OS_sched(void) {
    /* OS_next = ... */
    Task_t const *next = OS_next; /* volatile to temporary */
    if (next != OS_curr) {
        *(uint32_t volatile *)0xE000ED04 = (1U << 28);
    }
}

void create_task(
    Task_t *me,
    TaskHandler handler,
    void *stkSto, uint32_t stack_size)
{
    /* user shouldn't care about memory allignment */
    uint32_t *sp = (uint32_t *)((((uint32_t)stkSto + stack_size) / 8) * 8);
    uint32_t *stk_limit;
    *(--sp) = (1U << 24);  /* xPSR */
    *(--sp) = (uint32_t)handler; /* PC */
    *(--sp) = 0x000000EEU; /* LR  */
    *(--sp) = 0x00000012U; /* R12 */
    *(--sp) = 0x00000003U; /* R3  */
    *(--sp) = 0x00000002U; /* R2  */
    *(--sp) = 0x00000001U; /* R1  */
    *(--sp) = 0x00000000U; /* R0  */
    *(--sp) = 0x00000011U; /* R11 */
    *(--sp) = 0x00000010U; /* R10 */
    *(--sp) = 0x00000009U; /* R9 */
    *(--sp) = 0x00000008U; /* R8 */
    *(--sp) = 0x00000007U; /* R7 */
    *(--sp) = 0x00000006U; /* R6 */
    *(--sp) = 0x00000005U; /* R5 */
    *(--sp) = 0x00000004U; /* R4 */

    /* save the top of the stack in the thread's attibute */
    me->sp = sp;
    /* set a PID for this task */
    me->TID = rand()%1000;

    /* round up the bottom of the stack to the 8-byte boundary */
    stk_limit = (uint32_t *)(((((uint32_t)stkSto - 1U) / 8) + 1U) * 8);

}

__attribute__ ((naked))
void PendSV_Handler(void) {
__asm volatile (
    /* __disable_irq(); */
    "  CPSID         I                 \n"

    /* if (OS_curr != (OSThread *)0) { */
    "  LDR           r1,=OS_curr       \n"
    "  LDR           r1,[r1,#0x00]     \n"
    "  CBZ           r1,PendSV_restore \n"

    /*   push registers r4-r11 on the stack */
    "  PUSH          {r4-r11}          \n"

    /*   OS_curr->sp = sp; */
    "  LDR           r1,=OS_curr       \n"
    "  LDR           r1,[r1,#0x00]     \n"
    "  STR           sp,[r1,#0x00]     \n"
    /* } */

    "PendSV_restore:                   \n"
    /* sp = OS_next->sp; */
    "  LDR           r1,=OS_next       \n"
    "  LDR           r1,[r1,#0x00]     \n"
    "  LDR           sp,[r1,#0x00]     \n"

    /* OS_curr = OS_next; */
    "  LDR           r1,=OS_next       \n"
    "  LDR           r1,[r1,#0x00]     \n"
    "  LDR           r2,=OS_curr       \n"
    "  STR           r1,[r2,#0x00]     \n"

    /* pop registers r4-r11 */
    "  POP           {r4-r11}          \n"

    /* __enable_irq(); */
    "  CPSIE         I                 \n"

    /* return to the next thread */
    "  BX            lr                \n"
    );
}
