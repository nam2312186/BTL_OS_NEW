/*
 * Copyright (C) 2025 pdnguyen of HCMC University of Technology VNU-HCM
 */

/* Sierra release
 * Source Code License Grant: The authors hereby grant to Licensee
 * personal permission to use and modify the Licensed Source Code
 * for the sole purpose of studying while attending the course CO2018.
 */

#include "common.h"
#include "syscall.h"
#include "stdio.h"
#include "libmem.h"
#include "queue.h"
#include <string.h>   // Cho memset, strcmp
#include <stdlib.h>   // Cho free
#include <stdio.h>    // Cho printf
int get_process_name(struct pcb_t *caller, uint32_t memrg, char *proc_name, int max_len) {
    uint32_t data;
    int i = 0;
    if (!caller || !proc_name || max_len <= 0)
        return -1;
    memset(proc_name, 0, max_len);
    while (i < max_len - 1) {  
        libread(caller, memrg, i, &data);
        if (data == -1 || data == 0) {  
            proc_name[i] = '\0';
            return 0;
        }
        proc_name[i] = (char)data;
        i++;
    }
    proc_name[max_len - 1] = '\0';
    return 0;
}
int free_process(struct pcb_t *pcb) {
    if (pcb==NULL ) return -1;

#ifdef MM_PAGING
    if (pcb->mm) {
        struct mm_struct *mm = pcb->mm;

        if (mm->pgd) {
            free(mm->pgd);
        }

        // Giải phóng danh sách các vùng nhớ ảo
        struct vm_area_struct *area = mm->mmap;
        while (area) {
            struct vm_rg_struct *rg = area->vm_freerg_list;

            while (rg) {
                struct vm_rg_struct *next_rg = rg->rg_next;
                free(rg);
                rg = next_rg;
            }

            struct vm_area_struct *next_area = area->vm_next;
            free(area);
            area = next_area;
        }

        struct pgn_t *pgn = mm->fifo_pgn;
        while (pgn) {
            struct pgn_t *next_pgn = pgn->pg_next;
            free(pgn);
            pgn = next_pgn;
        }

        free(mm);
    }
#endif

    if (pcb->page_table !=NULL) {
        free(pcb->page_table);
        pcb->page_table=NULL;
    }

    if (pcb->code) {
        if (pcb->code->text) {
            free(pcb->code->text);
        }
        free(pcb->code);
        pcb->code=NULL;
    }
    //free(pcb);
    return 0;
}

int __sys_killall(struct pcb_t *caller, struct sc_regs* regs)
{
    char proc_name[100];
    // uint32_t data;

    //hardcode for demo only
    uint32_t memrg = regs->a1;
    
    if (get_process_name(caller, memrg, proc_name, sizeof(proc_name))==-1) return -1;
    
    printf("The procname retrieved from memregionid %d is \"%s\"\n", memrg, proc_name);

    struct pcb_t *pcb;

    char name_pcb[100];
    int i;
    for(i=0; i< caller->running_list->size;i++) {
        pcb = caller->running_list->proc[i];
        if (get_process_name(pcb, memrg, name_pcb, sizeof(name_pcb))==-1) {
            printf("%d no name", pcb->pid);
            continue;
        }
        if (strcmp(name_pcb, proc_name) == 0 ) {
            printf("Killing process %s (pid: %d)\n", name_pcb, pcb->pid);
            if(free_process(pcb)==-1) continue;
            remove_proc_from_queue(caller->ready_queue, pcb);
            remove_proc_from_queue(caller->running_list, pcb);
        }
    }
#ifdef MLQ_SCHED
    for(i=0; i< caller->ready_queue->size;i++) {
        pcb = caller->ready_queue->proc[i];
        if (get_process_name(pcb, memrg, name_pcb, sizeof(name_pcb))==-1) {
            printf("%d no name",pcb->pid);
            continue;
        }
        if (strcmp(name_pcb, proc_name )==0 ) {
            printf("Killing process %s (pid: %d)\n", name_pcb, pcb->pid);
            if(free_process(pcb)==-1) continue;
            remove_proc_from_queue(caller->ready_queue, pcb);
            remove_proc_from_queue(caller->running_list, pcb);
        }
    }
#endif
    return 0; 
}
