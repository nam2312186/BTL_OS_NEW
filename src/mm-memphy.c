// #ifdef MM_PAGING
/*
 * PAGING based Memory Management
 * Memory physical module mm/mm-memphy.c
 */

#include "mm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 *  MEMPHY_mv_csr - move MEMPHY cursor
 *  @mp: memphy struct
 *  @offset: offset
 */
int MEMPHY_mv_csr(struct memphy_struct *mp, int offset)
{
   if (mp == NULL || offset < 0 || offset >= mp->maxsz) {
      return -1; 
  }

   int numstep = 0;

   mp->cursor = 0;
   while (numstep < offset && numstep < mp->maxsz)
   {
      /* Traverse sequentially */
      mp->cursor = (mp->cursor + 1) % mp->maxsz;
      numstep++;
   }

   return 0;
}

/*
 *  MEMPHY_seq_read - read MEMPHY device
 *  @mp: memphy struct
 *  @addr: address
 *  @value: obtained value
 */
int MEMPHY_seq_read(struct memphy_struct *mp, int addr, BYTE *value)
{
   if (mp == NULL || value ==NULL || addr < 0 || addr >= mp->maxsz)
      return -1;

   // if (!mp->rdmflg)
   //    return -1; /* Not compatible mode for sequential read */
   if (mp->rdmflg)
         return -1; /* Not compatible mode for sequential read */

   // MEMPHY_mv_csr(mp, addr);
   if (MEMPHY_mv_csr(mp, addr) != 0) {
      return -1; // Di chuyển con trỏ thất bại
   }

   *value = (BYTE)mp->storage[addr];

   return 0;
}

/*
 *  MEMPHY_read read MEMPHY device
 *  @mp: memphy struct
 *  @addr: address
 *  @value: obtained value
 */
int MEMPHY_read(struct memphy_struct *mp, int addr, BYTE *value)
{
   if (mp == NULL || value == NULL || addr < 0 || addr >= mp->maxsz)
      return -1;

   if (mp->rdmflg)
      *value = mp->storage[addr];
   else /* Sequential access device */
      return MEMPHY_seq_read(mp, addr, value);

   return 0;
}

/*
 *  MEMPHY_seq_write - write MEMPHY device
 *  @mp: memphy struct
 *  @addr: address
 *  @data: written data
 */
int MEMPHY_seq_write(struct memphy_struct *mp, int addr, BYTE value)
{

   if (mp == NULL || addr < 0 || addr >= mp->maxsz)
      return -1;

   if (mp->rdmflg) //fixed
      return -1; /* Not compatible mode for sequential read */

   // MEMPHY_mv_csr(mp, addr);
   if (MEMPHY_mv_csr(mp, addr) != 0) {
      return -1; // Di chuyển con trỏ thất bại
   }
   mp->storage[addr] = value;

   return 0;
}

/*
 *  MEMPHY_write-write MEMPHY device
 *  @mp: memphy struct
 *  @addr: address
 *  @data: written data
 */
int MEMPHY_write(struct memphy_struct *mp, int addr, BYTE data)
{
   if (mp == NULL || addr < 0 || addr >= mp->maxsz)
      return -1;

   if (mp->rdmflg)
      mp->storage[addr] = data;
   else /* Sequential access device */
      return MEMPHY_seq_write(mp, addr, data);

   return 0;
}

/*
 *  MEMPHY_format-format MEMPHY device
 *  @mp: memphy struct
 */
int MEMPHY_format(struct memphy_struct *mp, int pagesz)
{
   /* This setting come with fixed constant PAGESZ */
   if (mp == NULL || pagesz <= 0)
      return -1;// Không hợp lệ
   
   int numfp = mp->maxsz / pagesz;
   struct framephy_struct *newfst, *fst;
   int iter = 0;

   if (numfp <= 0)
      return -1;

   /* Init head of free framephy list */
   fst = malloc(sizeof(struct framephy_struct));
   if (fst == NULL) {
      return -1; // Không thể cấp phát bộ nhớ
   }
   fst->fpn = iter;
   mp->free_fp_list = fst;

   /* We have list with first element, fill in the rest num-1 element member*/
   for (iter = 1; iter < numfp; iter++)
   {
      newfst = malloc(sizeof(struct framephy_struct));
      newfst->fpn = iter;
      newfst->fp_next = NULL;
      fst->fp_next = newfst;
      fst = newfst;
   }

   return 0;
}

int MEMPHY_get_freefp(struct memphy_struct *mp, int *retfpn)
{
   struct framephy_struct *fp = mp->free_fp_list;

   if (fp == NULL)
      return -1;

   *retfpn = fp->fpn;
   mp->free_fp_list = fp->fp_next;

   /* MEMPHY is iteratively used up until its exhausted
    * No garbage collector acting then it not been released
    */
   free(fp);

   return 0;
}
/* THÊM
 * SWAPMEM_try_get_freefp - Tìm khung trống trong các thiết bị swap
 * @proc: Tiến trình gọi hàm
 * @retfpn: Số khung trống trả về
 * @mswp_id: ID của thiết bị swap được chọn
 */
int SWAPMEM_try_get_freefp(struct pcb_t *proc, int *retfpn, int *mswp_id ){
  if(proc == NULL || retfpn == NULL || mswp_id == NULL) {
      return -1; // Không hợp lệ
  }
  
   int i = 0;
   int st_id = proc->active_mswp_id;
   while (i < PAGING_MAX_MMSWP) {
       int cur_id = (st_id + i) % PAGING_MAX_MMSWP;
       struct memphy_struct *cur_swap = proc->mswp[cur_id];
       if (cur_swap !=NULL && cur_swap->maxsz>0 && MEMPHY_get_freefp(cur_swap, retfpn) == 0) {
           proc->active_mswp_id = cur_id;
           proc->active_mswp = cur_swap;
           *mswp_id=cur_id;
           return 0;
       }
       i++;
   }
   return -1;
}


int MEMPHY_dump(struct memphy_struct *mp)
{
  /*TODO dump memphy contnt mp->storage
   *     for tracing the memory content
   */
  if (mp == NULL || mp->storage == NULL|| mp->maxsz<=0) 
  {
   return -1; // Không có bộ nhớ vật lý nào được cấp phát
  }

  printf("================================================================\n");
  printf("===== PHYSICAL MEMORY DUMP =====\n");
  for (int i = 0; i < mp->maxsz; ++i)
  {
     if (mp->storage[i] != 0)
     {
        printf("BYTE %08x: %d\n", i, mp->storage[i]);
     }
  }
  printf("===== PHYSICAL MEMORY END-DUMP =====\n");
  
 
  return 0;
}

int MEMPHY_put_freefp(struct memphy_struct *mp, int fpn)
{
   if (mp == NULL || fpn < 0)
      return -1; // Không hợp lệ
      
   struct framephy_struct *fp = mp->free_fp_list;
   struct framephy_struct *newnode = malloc(sizeof(struct framephy_struct));
   if (newnode == NULL)
   {
      return -1; // Không thể cấp phát bộ nhớ
   }

   /* Create new node with value fpn */
   newnode->fpn = fpn;
   newnode->fp_next = fp;
   mp->free_fp_list = newnode;

   return 0;
}

/*
 *  Init MEMPHY struct
 */
int init_memphy(struct memphy_struct *mp, int max_size, int randomflg)
{
   if (mp == NULL || max_size <= 0)
      return -1; // Không hợp lệ
   
   mp->storage = (BYTE *)malloc(max_size * sizeof(BYTE));
   if (mp->storage == NULL)
   {
      return -1; // Không thể cấp phát bộ nhớ
   }
   mp->maxsz = max_size;
   memset(mp->storage, 0, max_size * sizeof(BYTE));

   MEMPHY_format(mp, PAGING_PAGESZ);

   mp->rdmflg = (randomflg != 0) ? 1 : 0;

   if (!mp->rdmflg) /* Not Ramdom acess device, then it serial device*/
      mp->cursor = 0;

   return 0;
}

// #endif
