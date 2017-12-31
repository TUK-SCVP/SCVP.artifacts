addi x5, x0, 0
addi x6, x0, 9


#t1
addi x7, x0, 1
#t2
addi x8, x0, 1
#next
addi x9, x0, 1

#        next = t1 + t2;
#        t1 = t2;
#        t2 = next;

loop:
    add x9, x8, x7
    add x7, x0, x8
    add x8, x0, x9

    addi x5, x5, 1
    bne x5, x6, loop



