#ifndef SCHEDULER_H_
#define SCHEDULER_H_

/* Thread Control Block (TCB) */
typedef struct {
    void *sp; /* stack pointer */
    int TID; /*Task ID*/
} Task_t;

typedef void (*TaskHandler)();

void OS_init(void);

/* this function must be called with interrupts DISABLED */
void OS_sched(void);

void create_task(
    Task_t *me,
    TaskHandler threadHandler,
    void *stkSto, uint32_t stkSize);

#endif /* SCHEDULER_H_ */

