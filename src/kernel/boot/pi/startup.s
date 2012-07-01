.section .reset, "ax"
.global  __reset
.code 32

/* initial, unpatchable vector table */
__reset:
   ldr   pc, reset_handler_address
   ldr   pc, undef_handler_address
   ldr   pc, svc_handler_address
   ldr   pc, prefetch_abort_handler_address
   ldr   pc, data_abort_handler_address
_loop:   b  .
   ldr   pc, irq_handler_address
   ldr   pc, fiq_handler_address

reset_handler_address:           .word _reset
undef_handler_address:           .word __undef
svc_handler_address:             .word _svc
prefetch_abort_handler_address:  .word __prefetch_abort
data_abort_handler_address:      .word __data_abort
unused:                          .word _loop
irq_handler_address:             .word _irq
fiq_handler_address:             .word _fiq

.global reset_handler_address
.global undef_handler_address
.global svc_handler_address
.global prefetch_abort_handler_address
.global data_abort_handler_address
.global irq_handler_address
.global fiq_handler_address

.weak __undef
.set __undef, _loop
.weak __prefetch_abort
.set __prefetch_abort, _loop
.weak __data_abort
.set __data_abort, _loop

.asciz "(c) 2011 Simon Stapleton <simon.stapleton@gmail.com>"
.align

