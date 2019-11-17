.thumb
	b .main
.fat:
	push {lr}
	cmp r0, #0
	
	bne .prox
	
	mov r1, #1
	b .fim_fat
.prox:
	sub r0, r0, #1
	bl .fat
	add r0, r0, #1
	
	mul r1, r1, r0	
	
.fim_fat:
	bkpt #0x10
	pop {pc}

.main:
	mov r0, #5 //fat de 5
	bl .fat		
