;*******************************************
;** Project 		LCD demonstrator 
;** Target 			ATmega8535 on STK200
;** Program			Richard_LCD_INC.asm	(INCLUDE file)
;** Author			Richard Anthony
;** Date			23rd August 2008 	

;** Function		An include file which provides subroutines for
;**					configuring the LCD and displaying data onto it.

;** Ports	  		Uses the custom LCD interface connection on the STK 200

;** 				Uses port A for Data and Command values (8 bits, Output)
;**					The LCD 'device Busy' flag also uses bit 7 of port A 
;**					So sometimes this bit has to be set for Input

;**					Uses port C for control (register select 'RS' bit 6 Output)
;**											(device enable 'ENABLE' bit 7 Output)

;**					Uses port D for control (write 'WR' bit 6 Output) 
;**											(read 'RD' bit 7 Output) 

;** This file extends the basic LCD library routines supplied by Kanda
;** These routines are I/O mapped, i.e. do not use memory mapping		

;**	Command register values

;** bit  3 = (value 1) Display On/Off control mode
;** bit  2 = Display on(1) / off(0)
;** bit  1 = Cursor on(1) / off(0)
;** bit  0 = Cursor position on(1) / off(0)
;*******************************************


;Subroutines:
;lcd_wait ; Check if the LCD device is busy, if so wait.
;lcd_WriteFunctionCommand ; Output the function command in LCDreg to the LCD:	
;lcd_ReadFunctionCommand ; Read the function command from the LCD into LCDreg	
;lcd_Clear; Clear the LCD display and set the cursor to the 'home' position	
;lcd_StandardMode ; Set LCD to 8-bit-mode, display window freeze and auto cursor increment	
;lcd_SetCursor ; Set cursor to a specific display position identified in LCDreg
;lcd_WriteChar ; Write the char in LCDreg at current cursor position and increment cursor	
;lcd_on: ; Set LCD display on, cursor on and blink on
;lcd_CursorOn ; Set Cursor on
;lcd_CursorOff; Set Cursor Off	
;lcd_DisplayOn; Set Display on	
;lcd_DisplayOff; Set Display off	
;lcd_CursorPositionOff ; Set Cursor Position Indicator Off	
;lcd_BarGraph ; Display 2-bar bar graph on LCD display

	.equ RD=7
	.equ WR=6
	.equ RS=6
	.equ BF=7
	.equ ENABLE=7


; Check if the LCD device is busy, if so wait.
lcd_Wait:	
	push LCDreg 		; Save LCDreg register on stack
	cbi PORTC, ENABLE  	; Set enable low
    cbi PORTC, RS  		; Set RS low (command register) so can read busy flag		
	cbi PORTD, RD  		; Set RD pin low for read operation
	sbi PORTD, WR  		; Set WR pin high for read
	cbi DDRA, BF   		; High bit of port 7 is Busy Flag (BF), set to input

lcd_wt1:
	sbi PORTC, ENABLE   ; Set ENABLE high
	nop
	nop
	sbis PINA, BF		; Skip if Busy Flag is high
	rjmp restore		; Exit if RS is low
	nop					; wait (timing)!
	nop
    cbi PORTC, ENABLE  	; Set ENABLE low
	nop
	nop
    rjmp lcd_wt1

restore:
    cbi PORTC, ENABLE  	; Set ENABLE low
	cbi PORTD, WR  		; Set WR read/write low for write 
	sbi PORTD, RD  		; Set RD pin high (for enable circuit)
	sbi DDRA, BF  		; Restore bit 7 of port A to output (for Data use)
	pop	LCDreg 			; Restore LCDreg register from Stack
	ret

; Output the function command in LCDreg to the LCD
lcd_WriteFunctionCommand:	
	rcall lcd_Wait 		; Wait if the LCD device is busy
	cbi PORTC, RS   	; Set RS low for command
	sbi PORTC, ENABLE   ; Set ENABLE pin high
	out PORTA, LCDreg	; Send function command to command register, via port A
	cbi PORTC, ENABLE   ; Set ENABLE Low
	ret

; Read the function command from the LCD into LCDreg
lcd_ReadFunctionCommand:	
	rcall lcd_Wait 		; Wait if the LCD device is busy
	cbi PORTC, RS   	; set RS low for command
	sbi PORTC, ENABLE   ; Set ENABLE pin high
	in LCDreg, PORTA	; Read the command register via port A
	cbi PORTC, ENABLE   ; Set ENABLE low
	ret

; Clear the LCD display and set the cursor to the 'home' position
lcd_Clear:	
	push LCDreg 					; Save LCDreg register on stack
	ldi	LCDreg,0x01 				; The 'clear; command
	rcall lcd_WriteFunctionCommand 	; Output the command to the LCD
	pop	LCDreg 						; Restore LCDreg register from Stack
	ret

; Set the LCD to 8-bit-mode, display window freeze and
; automatic cursor increment (standard mode)
lcd_StandardMode:	
	push LCDreg 					; Save LCDreg register on stack
	ldi	LCDreg,0b00111000 			; Command for 8-Bit-transfer
	rcall lcd_WriteFunctionCommand 	; Output the command to the LCD
	ldi	LCDreg,0b00000110 			; Command for Increment, display freeze
	rcall lcd_WriteFunctionCommand 	; Output the command to the LCD
	ldi	LCDreg,0b00010000 			; Command for Cursor move, not shift
	rcall lcd_WriteFunctionCommand 	; Output the command to the LCD
	pop	LCDreg 						; Command for Restore LCDreg register from stack
	ret

; Set cursor on the LCD to a specific display position (identified in LCDreg)
lcd_SetCursor:	
	push LCDreg 					; Save LCDreg register on stack (save cursor position)
	ori	LCDreg,0x80 				; Set bit 7 of the position
	rcall lcd_WriteFunctionCommand 	; Output the command to the LCD
	pop	LCDreg 						; Restore LCDreg register from Stack
	ret

; Write the character in LCDreg to the display at the current cursor
; position (the position is incremented after write)
lcd_WriteChar:	
	rcall lcd_Wait 		; Wait if the LCD device is busy
	sbi PORTC, RS   	; Set RS high (for data)
	sbi PORTC, ENABLE   ; Set ENABLE high
	out PORTA, LCDreg  	; Write data character to LCD
	cbi PORTC, ENABLE   ; Set Enable low
	ret
 
; Set LCD display on, cursor on and blink on
lcd_on:	
	push LCDreg 					; Save LCDreg register on stack (save cursor position)
	ldi	LCDreg,0b00001111 			; Combined command byte
	rcall lcd_WriteFunctionCommand 	; Output the command to the LCD
	pop	LCDreg 						; Restore LCDreg register from Stack
	ret

; Set Cursor on
lcd_CursorOn:	
	push LCDreg 					; Save LCDreg register on stack (save cursor position)
	rcall lcd_ReadFunctionCommand	; Input the command register value into the LCDreg
	ori LCDreg, 0b00000010			; Command value to set cursor on
	rcall lcd_WriteFunctionCommand 	; Output the command to the LCD
	pop	LCDreg 						; Restore LCDreg register from Stack
	ret

; Set Cursor Off
lcd_CursorOff:	
	push LCDreg 					; Save LCDreg register on stack
	rcall lcd_ReadFunctionCommand	; Input the command register value into the LCDreg
	andi LCDreg, 0b11111101			; Command value to set cursor off
	rcall lcd_WriteFunctionCommand 	; Output the command to the LCD
	pop	LCDreg 						; Restore LCDreg register from Stack
	ret

; Set Display on
lcd_DisplayOn:	
	push LCDreg 					; Save LCDreg register on stack
	rcall lcd_ReadFunctionCommand	; Input the command register value into the LCDreg
	ori LCDreg, 0b00000100			; Command value to set display on
	rcall lcd_WriteFunctionCommand 	; Output the command to the LCD
	pop	LCDreg 						; Restore LCDreg register from Stack
	ret

; Set Display off
lcd_DisplayOff:	
	push LCDreg 					; Save LCDreg register on stack
	rcall lcd_ReadFunctionCommand	; Input the command register value into the LCDreg
	andi LCDreg, 0b11111011			; Command value to set display off
	rcall lcd_WriteFunctionCommand 	; Output the command to the LCD
	pop	LCDreg 						; Restore LCDreg register from Stack
	ret

; Set Cursor Position Indicator Off
lcd_CursorPositionOff:	
	push LCDreg 					; Save LCDreg register on stack
	ldi	LCDreg,0b00001100 			; Command value to set cursor position indicator off
	rcall lcd_WriteFunctionCommand 	; Output the command to the LCD
	pop	LCDreg 						; Restore LCDreg register from Stack
	ret

; Display 2-bar bar graph on LCD display
lcd_BarGraph:
	;The bar length is given by the values in BarGraph1 and BarGraph2
	rcall lcd_CursorOff	
	rcall lcd_CursorPositionOff	
	rcall lcd_Clear
	ldi	LCDreg,0x00			; Set cursor position to line 1, col 0
	rcall lcd_SetCursor   
	
ElementRow1loop:
	tst	BarGraph1 			; Check for 0 bar height
	breq	SetRow2Cursor
	ldi LCDreg,0x11			; The best character code for the bar element
	rcall lcd_WriteChar
	dec BarGraph1
	rjmp ElementRow1loop

SetRow2Cursor:
	ldi	LCDreg,0x40			; Set cursor position to line 2, col 0
	rcall lcd_SetCursor   

ElementRow2loop:
	tst	BarGraph2 			; Check for 0 bar height
	breq	EndGraph
	ldi LCDreg,0x11			; The best character code for the bar element
	rcall lcd_WriteChar
	dec BarGraph2
	rjmp ElementRow2loop

EndGraph:	
	ret




