#include "malloc.h"
#include "utils.h"
#include "scheduler.h"

#define THREAD_STACK_SIZE 512
#define MAX_THREADS 10

typedef struct Task {
	unsigned int *sp;
} Task;

Task tasks[MAX_THREADS] = {0};

// FORWARD DECLARATIONS
void schedule_task(int id, void* stack_addr, void (*task_func)());

__attribute__((naked)) void PendSV_Handler(void) {
	__asm volatile (
        "MRS R0, PSP \n"             // Read PSP
        "CMP R0, #0 \n"              // Is it 0? (First run?)
        "BEQ restore_context \n"     // If 0, SKIP saving!

        // --- SAVE CONTEXT (Only if PSP != 0) ---
        "STMDB R0!, {R4-R11} \n"     // Save R4-R11

        "restore_context: \n"
        "PUSH {LR} \n"               // We still need to call schedule_next for the first task!
        "BL schedule_next \n"        // This R0 has PSP which will pass to Schedule Next
        "POP {LR} \n"

		"ORR LR, LR, #4\n" // Ensure LR has returns to Thread PSP

        "LDMIA R0!, {R4-R11} \n"     // Restore R4-R11
        "MSR PSP, R0 \n"             // Update PSP to the new stack
        "BX LR \n"                   // Return
    );
}

void task_return_trap(void) {
    usart_print("CRITICAL: A Task returned! Freezing.");
    while(1); // Trap the CPU here
}

int count = 0;
int amount_tasks = 0;
void* schedule_next(void* old_sp) {
	count++;
	int index = count % amount_tasks;
	if (old_sp) {
		if (index > 0)
			tasks[index - 1].sp = old_sp;
		else
		 	tasks[amount_tasks - 1].sp = old_sp;
	}
	return tasks[index].sp;
}

int task_id = 0;
void create_thread(void (*task_func)()) {
	if (task_id >= MAX_THREADS) {
		usart_print("Can't allocate more threads. MAX_THREADS reached");
		return;
	}
	void *stack = mmalloc(256);
	usart_print("Adding task to queue\n");
	schedule_task(task_id++, stack, task_func);
}

void schedule_task(int id, void* stack_addr, void (*task_func)()) {
	unsigned int *s_ptr = stack_addr + THREAD_STACK_SIZE;
	s_ptr--;
	*(s_ptr--) = 0x01000000; //xPSR
	*(s_ptr--) = (unsigned int)task_func; //PC
	*(s_ptr--) = (unsigned int)task_return_trap; //LR stores the return address 
	*(s_ptr--) = 0; //R12
	*(s_ptr--) = 0; //R3
	*(s_ptr--) = 0; //R2
	*(s_ptr--) = 0; //R1
	*(s_ptr--) = 0; //R0
	*(s_ptr--) = 0; //R11
	*(s_ptr--) = 0; //R10
	*(s_ptr--) = 0; //R9
	*(s_ptr--) = 0; //R8
	*(s_ptr--) = 0; //R7
	*(s_ptr--) = 0; //R6
	*(s_ptr--) = 0; //R5
	*(s_ptr) = 0; //R4 
	tasks[id] = (Task){.sp=s_ptr};
	amount_tasks++;
}
