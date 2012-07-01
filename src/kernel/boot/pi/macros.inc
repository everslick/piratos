/* A globally accessible function */
.macro FUNC name
.text
.code 32
.global \name
\name:
.endm

/* Data memory barrier */
/* pass in a spare register */
.macro DMB reg
   mov   \reg, #0
   mcr   p15,0,\reg,c7,c10,5  /* Data memory barrier on ARMv6 */
.endm

/* Data synchronisation barrier */
/* pass in a spare register */
.macro DSB reg
   mov   \reg, #0
   mcr   p15,0,\reg,c7,c10,4  /* Data Synchronisation barrier on ARMv6 */
.endm

