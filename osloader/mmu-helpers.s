.text
.org 0x100

.global mmu_enable
mmu_enable_and_jump:
	link %fp, #-4
	pmove %tc, -4(%fp)
	ori.l #0x80000000, -4(%fp)
	pmove -4(%fp), %tc
	move.l #0x0, %sp
	jmp 8(%fp)

.global mmu_set_tc
mmu_set_tc:
	link %fp, #0
	mov.l 8(%fp), %a0
	pmove (%a0), %tc
	unlk %fp
	rts

.global mmu_get_tc
mmu_get_tc:
	link %fp, #0
	mov.l 8(%fp), %a0
	pmove %tc, (%a0)
	unlk %fp
	rts

.global mmu_set_srp
mmu_set_srp:
	link %fp, #0
	mov.l 8(%fp), %a0
	pmove (%a0), %srp
	unlk %fp
	rts

.global mmu_get_srp
mmu_get_srp:
	link %fp, #0
	mov.l 8(%fp), %a0
	pmove %srp, (%a0)
	unlk %fp
	rts

.global mmu_set_crp
mmu_set_crp:
	link %fp, #0
	mov.l 8(%fp), %a0
	pmove (%a0), %crp
	unlk %fp
	rts

.global mmu_get_crp
mmu_get_crp:
	link %fp, #0
	mov.l 8(%fp), %a0
	pmove %crp, (%a0)
	unlk %fp
	rts

.global mmu_set_tt0
mmu_set_tt0:
	link %fp, #0
	mov.l 8(%fp), %a0
	pmove (%a0), %tt0
	unlk %fp
	rts

.global mmu_get_tt0
mmu_get_tt0:
	link %fp, #0
	mov.l 8(%fp), %a0
	pmove %tt0, (%a0)
	unlk %fp
	rts

.global mmu_set_tt1
mmu_set_tt1:
	link %fp, #0
	mov.l 8(%fp), %a0
	pmove (%a0), %tt1
	unlk %fp
	rts

.global mmu_get_tt1
mmu_get_tt1:
	link %fp, #0
	mov.l 8(%fp), %a0
	pmove %tt1, (%a0)
	unlk %fp
	rts
