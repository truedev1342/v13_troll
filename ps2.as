; file ps2.as

DELAY:
	sub r0, #1
	bne DELAY
END:
	rts
