
#include "queue.h"
#include "sched.h"
#include <pthread.h>

#include <stdlib.h>
#include <stdio.h>
static struct queue_t ready_queue;
static struct queue_t run_queue;
static pthread_mutex_t queue_lock;

static struct queue_t running_list;
#ifdef MLQ_SCHED
static struct queue_t mlq_ready_queue[MAX_PRIO];
static int slot[MAX_PRIO];
#endif

int queue_empty(void) {
#ifdef MLQ_SCHED
	unsigned long prio;
	for (prio = 0; prio < MAX_PRIO; prio++)
		if(!empty(&mlq_ready_queue[prio])) 
			return -1;
#endif
	return (empty(&ready_queue) && empty(&run_queue));
}

void init_scheduler(void) {
#ifdef MLQ_SCHED
    int i ;

	for (i = 0; i < MAX_PRIO; i ++) {
		mlq_ready_queue[i].size = 0;
		slot[i] = MAX_PRIO - i; 
	}
#endif
	ready_queue.size = 0;
	run_queue.size = 0;
	pthread_mutex_init(&queue_lock, NULL);
}

#ifdef MLQ_SCHED
/* 
 *  Stateful design for routine calling
 *  based on the priority and our MLQ policy
 *  We implement stateful here using transition technique
 *  State representation   prio = 0 .. MAX_PRIO, curr_slot = 0..(MAX_PRIO - prio)
 */
struct pcb_t * get_mlq_proc(void) {
	struct pcb_t * proc = NULL;
	/*TODO: get a process from PRIORITY [ready_queue].
	 * Remember to use lock to protect the queue.
	 * */
	//khóa mutex để bảo vệ hàng đợi
	pthread_mutex_lock(&queue_lock);
	//kiểm tra xem hàng đợi đã được khởi tạo chưa
	int priority;
    for (priority = 0; priority < MAX_PRIO; priority++) {
        if (mlq_ready_queue == NULL) {
            pthread_mutex_unlock(&queue_lock);
            printf("Lỗi: mlq_ready_queue chưa được khởi tạo!\n");
            return NULL;
        }
    }
	//duyệt qua các mức ưu tiên để tìm tiến trình
	for (priority = 0; priority < MAX_PRIO; priority++) {
		if (!empty(&mlq_ready_queue[priority])) {
			if (slot[priority] > 0) {
				proc = dequeue(&mlq_ready_queue[priority]);
				slot[priority]--; 
				break;
			}
		}
	}
	// Nếu không tìm thấy tiến trình và vẫn còn tiến trình trong hàng đợi, reset slot
	if (proc == NULL && queue_empty()!=-1) {
		for (priority = 0; priority < MAX_PRIO; priority++) {
			slot[priority] = MAX_PRIO - priority;
		}
	}

	//mở khóa mutex để cho phép các luồng khác truy cập hàng đợi
	pthread_mutex_unlock(&queue_lock);
	return proc;	
}

void put_mlq_proc(struct pcb_t * proc) {
	pthread_mutex_lock(&queue_lock);
	enqueue(&mlq_ready_queue[proc->prio], proc);
	pthread_mutex_unlock(&queue_lock);
}

void add_mlq_proc(struct pcb_t * proc) {
	pthread_mutex_lock(&queue_lock);
	enqueue(&mlq_ready_queue[proc->prio], proc);
	pthread_mutex_unlock(&queue_lock);	
}

struct pcb_t * get_proc(void) {
	return get_mlq_proc();
}

void put_proc(struct pcb_t * proc) {
	if (proc == NULL) {
        printf("Lỗi: Tiến trình là NULL trong put_proc!\n");
        return;
    }

	proc->ready_queue = &ready_queue;
	proc->mlq_ready_queue = mlq_ready_queue;
	proc->running_list = & running_list;

	/* TODO: put running proc to running_list */
	pthread_mutex_lock(&queue_lock);
	if (proc->running_list == NULL) {
        pthread_mutex_unlock(&queue_lock);
        printf("Lỗi: running_list chưa được khởi tạo trong put_proc!\n");
        return;
    }
	enqueue(proc->running_list, proc);
	pthread_mutex_unlock(&queue_lock);

	return put_mlq_proc(proc);
}

void add_proc(struct pcb_t * proc) {
	if (proc == NULL) {
        printf("Lỗi: Tiến trình là NULL trong add_proc!\n");
        return;
    }

	proc->ready_queue = &ready_queue;
	proc->mlq_ready_queue = mlq_ready_queue;
	proc->running_list = & running_list;

	/* TODO: put running proc to running_list */
	pthread_mutex_lock(&queue_lock);
	if (proc->running_list == NULL) {
		pthread_mutex_unlock(&queue_lock);
		printf("Lỗi: running_list chưa được khởi tạo trong add_proc!\n");
		return;
	}
	enqueue(proc->running_list, proc);
	pthread_mutex_unlock(&queue_lock);

	return add_mlq_proc(proc);
}
#else
struct pcb_t * get_proc(void) {
	struct pcb_t * proc = NULL;
	/*TODO: get a process from [ready_queue].
	 * Remember to use lock to protect the queue.
	 * */

	pthread_mutex_lock(&queue_lock);
	proc = dequeue(&ready_queue);
	pthread_mutex_unlock(&queue_lock);

	return proc;
}

void put_proc(struct pcb_t * proc) {
	if (proc == NULL) {
		printf("Lỗi: Tiến trình là NULL trong put_proc!\n");
		return;
	}
	proc->ready_queue = &ready_queue;
	proc->running_list = & running_list;

	/* TODO: put running proc to running_list */
	pthread_mutex_lock(&queue_lock);
	if (proc->running_list == NULL) {
		pthread_mutex_unlock(&queue_lock);
		printf("Lỗi: running_list chưa được khởi tạo trong put_proc!\n");
		return;
	}
	enqueue(proc->running_list, proc);
	pthread_mutex_unlock(&queue_lock);

	pthread_mutex_lock(&queue_lock);
	enqueue(&run_queue, proc);
	pthread_mutex_unlock(&queue_lock);
}

void add_proc(struct pcb_t * proc) {
	if (proc == NULL) {
		printf("Lỗi: Tiến trình là NULL trong add_proc!\n");
		return;
	}
	proc->ready_queue = &ready_queue;
	proc->running_list = & running_list;

	/* TODO: put running proc to running_list */
	pthread_mutex_lock(&queue_lock);
	if (proc->running_list == NULL) {
		pthread_mutex_unlock(&queue_lock);
		printf("Lỗi: running_list chưa được khởi tạo trong add_proc!\n");
		return;
	}
	enqueue(proc->running_list, proc);
	pthread_mutex_unlock(&queue_lock);

	pthread_mutex_lock(&queue_lock);
	enqueue(&ready_queue, proc);
	pthread_mutex_unlock(&queue_lock);	
}
#endif


