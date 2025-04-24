# Lệnh chạy chương trình và Test case

Dưới đây là danh sách các test case với mô tả nội dung, và một lệnh duy nhất để chạy toàn bộ các test, lưu output vào thư mục `my_output`:

## Danh sách Test case

- **Test os_0_mlq_paging**  
  Kiểm tra lập lịch MLQ với 2 CPU, 4 tiến trình, và cơ chế phân trang (RAM 1MB, SWAP 16MB).
./os os_0_mlq_paging > my_output/os_0_mlq_paging.txt

- **Test os_1_mlq_paging**  
  Kiểm tra lập lịch MLQ với cấu hình khác, tập trung vào phân trang và quản lý bộ nhớ.
./os os_1_mlq_paging > my_output/os_1_mlq_paging.txt

- **Test os_1_mlq_paging_small_1K**  
  Kiểm tra lập lịch MLQ với bộ nhớ nhỏ (RAM 1KB), tập trung vào cơ chế swap và phân trang.
./os os_1_mlq_paging_small_1K > my_output/os_1_mlq_paging_small_1K.txt

- **Test os_1_mlq_paging_small_4K**  
  Kiểm tra lập lịch MLQ với bộ nhớ nhỏ (RAM 4KB), kiểm tra hiệu suất phân trang.
./os os_1_mlq_paging_small_4K > my_output/os_1_mlq_paging_small_4K.txt 

- **Test os_1_singleCPU_mlq**  
  Kiểm tra lập lịch MLQ với 1 CPU duy nhất, không sử dụng phân trang.
  ./os os_1_singleCPU_mlq > my_output/os_1_singleCPU_mlq.txt

- **Test os_1_singleCPU_mlq_paging**  
  Kiểm tra lập lịch MLQ với 1 CPU duy nhất, có sử dụng cơ chế phân trang.
  ./os os_1_singleCPU_mlq_paging > my_output/os_1_singleCPU_mlq_paging.txt

- **Test os_sc**  
  Kiểm tra lập lịch Shortest Job First (SJF) cơ bản, không sử dụng phân trang.
./os os_sc > my_output/os_sc.txt

- **Test os_syscall**  
  Kiểm tra các syscall cơ bản (như cấp phát, ghi, đọc, giải phóng bộ nhớ) trong hệ điều hành mô phỏng.
  ./os os_syscall > my_output/os_syscall.txt

- **Test os_syscall_list**  
  Kiểm tra danh sách các syscall, tập trung vào việc thực thi tuần tự các lệnh hệ thống.
  ./os os_syscall_list > my_output/os_syscall_list.txt

- **Test sched**  
  Kiểm tra lập lịch cơ bản (có thể là Round Robin hoặc MLQ) với các tiến trình đơn giản.
./os sched > my_output/sched.txt 

- **Test sched_0**  
  Kiểm tra lập lịch với cấu hình cụ thể, tập trung vào tiến trình ưu tiên cao.
./os sched_0 > my_output/sched_0.txt

- **Test sched_1**  
  Kiểm tra lập lịch với cấu hình khác, tập trung vào tiến trình ưu tiên thấp.
  ./os sched_1 > my_output/sched_1.txt

