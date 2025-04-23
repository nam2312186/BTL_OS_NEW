#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

int empty(struct queue_t * q) {
        if (q == NULL) return 1;
	return (q->size == 0);
}
//Viết thêm hàm xóa một tiến trình cụ thể trong hàng đợi
void remove_proc_from_queue(struct queue_t *q, struct pcb_t *proc) {
    if (q == NULL || proc == NULL || q->size == 0) return;
    int new_size = 0;

    for (int i = 0; i < q->size; i++) {
        if (q->proc[i] != proc) {
            q->proc[new_size++] = q->proc[i];
        }
    }
    q->size = new_size;
}
void enqueue(struct queue_t * q, struct pcb_t * proc) {
        /* TODO: put a new process to queue [q] */
        if(q==NULL || proc==NULL|| q->size==MAX_QUEUE_SIZE) return;
        q->proc[q->size++]=proc;
}

struct pcb_t * dequeue(struct queue_t * q) {
        /* TODO: return a pcb whose prioprity is the highest
         * in the queue [q] and remember to remove it from q
         * */
        if (empty(q)) return NULL;
#ifdef MLQ_SCHED // Chế độ MLQ
   
    // Lấy tiến trình ở đầu hàng đợi 
    struct pcb_t * proc = q->proc[0];

    // Dịch các tiến trình phía sau lên để lấp chỗ trống
    for (int i = 0; i < q->size - 1; i++) {
        q->proc[i] = q->proc[i+1];
    }

    q->size--;

    return proc;

#else // Chế độ Non-MLQ
//Tìm priority nhỏ nhất (chỉ số của tiến trình có ưu tiên cao nhất)
    int highest_idx = 0;
    for (int i = 1; i < q->size; i++) {
        if (q->proc[i]->priority < q->proc[highest_idx]->priority) {
            highest_idx = i;
        }
    }
    struct pcb_t * proc = q->proc[highest_idx];

    // Dịch các tiến trình phía sau lên để lấp chỗ trống
    for (int i = highest_idx; i < q->size - 1; i++) {
        q->proc[i] = q->proc[i+1];
    }

    q->size--;

    return proc;
#endif
}

