sys_exit:       equ             60
sys_write:      equ             1
sys_exit_error: equ             1
sys_stdout:     equ             1
sys_errout:     equ             2

                section         .text
                global          _start

buf_size:       equ             4096
_start:
        ; rbx will contains answer
                xor             ebx, ebx
                sub             rsp, buf_size
                mov             rsi, rsp

        ; flag - prev_symb was white space
                mov             r10b, 1
read_again:
        ; xor give a zero in a rax - sys_read
                xor             rax, rax
                xor             rdi, rdi
                mov             rdx, buf_size
                syscall

                test            rax, rax
                jz              quit
                js              read_error

                xor             ecx, ecx

check_char:
        ; r8b - cur_symb 
                mov             r8b, byte[rsi + rcx]

        ; check if symbol is space (dec code 32) 
                cmp             r8b, 0x20
                je              check_last

        ; check if symbol is another whitespace symbol (9-13) 
                sub             r8b, 9
                cmp             r8b, 4
                ja              inc_ans
check_last:
        ; check prev_whitespace flag
                mov             r10b, 1
                jmp             skip
inc_ans:
        ; if r10b (flag) is zero, then rbx' = rbx, else +1
                add             rbx, r10
                xor             r10b, r10b
skip:
        ; inc rcx and check if rcx == rax
                inc             rcx
                cmp             rcx, rax
                je              read_again

                jmp             check_char

quit:
                mov             rax, rbx
                call            print_int

                mov             rax, sys_exit
        ; sys_exit with no error, code - 0
                xor             rdi, rdi
                syscall

        ; rax - number to print
print_int:
                mov             rsi, rsp
        ; reserve memory in stack, our number in redzone
                sub             rsp, 32

                mov             rbx, 10

                dec             rsi
                mov             byte [rsi], 0x0a

next_char:
                xor             rdx, rdx
                div             rbx
                add             dl, '0'
                dec             rsi
                mov             [rsi], dl
                test            rax, rax
                jnz             next_char

        ; r9 - stack start pointer
                mov             r9, rsp
                add             r9, 32

                mov             rax, sys_write
                mov             rdi, sys_stdout
                mov             rdx, r9
                sub             rdx, rsi
                syscall

        ; clear stack
                mov             rsp, r9
                ret

read_error:
                mov             rax, sys_write
                mov             rdi, sys_errout
                mov             rsi, read_error_msg
                mov             rdx, read_error_len
                syscall

                mov             rax, sys_exit
                mov             rdi, sys_exit_error
                syscall

                section         .rodata

read_error_msg: db              "Read failure", 0x0a
read_error_len: equ             $ - read_error_msg
