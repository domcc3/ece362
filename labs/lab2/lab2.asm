;***********************************************************************
; ECE 362 - Experiment 2 - Fall 2015
;***********************************************************************
;
; Completed by: < Dominic Celiano >
;               < 0617-C >
;               < Lab Division 9 >
;
;
; Academic Honesty Statement:  In signing this statement, I hereby certify
; that I am the individual who created this HCS12 source file and that I have
; not copied the work of any other student (past or present) while completing it.
; I understand that if I fail to honor this agreement, I will receive a grade of
; ZERO and be subject to possible disciplinary action.
;
;
; Signature: ________________________________________________   Date: ____________
;
; NOTE: The printed hard copy of this file you submit for evaluation must be signed
;       in order to receive credit.
;
;*******************************************************************************
; Step 4-A: 
;
; Write a program that loads two unsigned 16-bit numbers from the memory location
; "ops", divides the first number by the second, then stores the quotient 
; in the memory location "quot" and the remainder in memory location 
; "remain". Note that the space for the operands and results has 
; already been allocated for you below.  Each has a label associated with 
; the memory location.  You can use the labels "ops", "quot", and "remain"
; when writing your code, and the assembler will convert it to the 
; appropriate memory address.
;
; Note that the instructions for the next step begin at memory location
; $0820.  In the debugger, check the assembly window to make sure your
; code for step 4a doesn't overlap into step 4b (address $0820)
;
;*******************************************************************************

      	org	$0800 ; start code here
step4a  
; Put your code here
        ldd ops       ; first number
        ldx (ops + 2) ; second number
        idiv          ; (X) <- (D) / (X), (D) <- Remainder (first number / second number)
        stx quot      ; store (X) at quot
        std remain    ; store (D) at remain

	      stop	;  breakpoint here

ops    	rmb	4
quot  	rmb	2
remain	rmb	2
	

;*******************************************************************************
; Step 4-B:
;
; Write a program that tests whether the unsigned value contained 
; in the A register is higher than value stored at the memory location 
; "tval".  If it is, the program sets the variable "higher" to $FF,
; and if not, the program sets the variable "higher" to $00. 
;
;*******************************************************************************

        org	$0820  ; start code here
step4b
 
; Put your code here	

        cmpa tval    ; compare accumulator A value with (tval)
        bhi higher_branch  ; branch if (A) higher than (tval)
        ; if no branch was taken, the code below is executed
        ldaa #$00    ; set "higher" to $00
        staa higher
        bra end_if 

higher_branch
        ldaa #$FF  ; set "higher" to $FF
        staa higher
        
end_if  
      	stop	       ; breakpoint here

tval	fcb	100    ;note: this is 100 in base 10
higher	rmb	1


;*******************************************************************************
; Step 4-C:
;
; Write a program that performs the addition of the two 
; 3-byte numbers located at the memory location "adds" and stores
; the result in the memory location "sum".  
;
; After execution, the values in the registers must be the same 
; as before execution. 
; 
; Therefore, you should push any registers used in the program at the 
; beginning of the program, and then pull (pop) them off at the end
; of the program.  
; 
; NOTE: The operands are stored MSB to LSB.
;
;*******************************************************************************

      	org	$0840 ; start code here
step4c
 
; Put your code here
        ; push the original register values onto the stack
        psha
        pshd
        
        ;load, perform addition, and store
        
        ldd (adds + 1) ;last 2 bytes
        addd (adds + 4) ; add the last two bytes of each number together
        std (sum + 1) ;store d to last 2 bytes of "sum"
        	
        ldaa adds ; first byte
        adca (adds + 3) ; add the first byte of each number together
        staa sum ; store first byte of "sum"
        
        
        
        
        
        ; popping (pulling) off the original values back into the registers
        puld
        psha


	      stop	; use this location to set breakpoint 
        
	      org	$0870
adds	  rmb	6	; Addends 
sum	    rmb	3	; Sum

 
;*******************************************************************************
; Step 4-D: 
;
; Write a program that will transfer a specified number of bytes of data 
; from one memory location (source) to another memory location (destination).   
; Assume that the source address is contained in the Y register, the 
; destination address is contained in the X register, and the A register 
; contains the number of bytes of data to be transferred.  The X, Y and A 
; registers should return with their original values, and the other registers 
; should be unchanged.
;
; Note: For this program, you should use a FOR loop. The basic 
; structure of a FOR loop is:
;
;	loop	check counter
;		branch out of loop if done (here, to label "done")
;	 	perform action
;		branch back to "loop" 
;	done	next instruction
;
; Note: When testing this program, make sure that you are not transferring 
; data to memory locations where your program is located!!!  Check your 
; assembled listing file to see where your programs are located.  
;
;*******************************************************************************

	      org	$0890  ; start code here
step4d
 
; Put your code here
        
for_start
        psha
        pshy
        pshx
        adda #$01 ; add 1 to a so it is correct number of iterations
        
for_loop
        ;loops one time for each byte
        dbeq a, for_exit ; decrement a, then exit the loop if a is 0
        movb 1,y+,1,x+ ; move the value of y to x and increment each (movb = move byte)
        bra for_loop  ; go back through the loop
        
for_exit
        pulx
        puly
        pula
        	

	      stop	; use this location to set breakpoint	

;***********************************************************************
; Step 4-E:
;
; Write a program that determines how many bits of the number 
; passed in the A register are 1's.  The A register should return its original 
; value, and the number of 1 bits should be returned in the B register.  
;
; Note: For this program, you should use a DO loop. The basic 
; structure of a DO loop is:
;
;		initialize counter
;	loop 	perform action
;		update and check counter
;		branch back to "loop" if not done
;
; You will need to maintain three pieces of data for this program:
;    (a) initial value (in A register)
;    (b) number of 1 bits (returned in B register)
;    (c) counter for the loop
;
; Since we only have two accumulators available in the HC12, you will either
; need to use an index register, a local variable (stored in memory), or 
; the stack to implement this.  A memory location with the label "count"
; has been reserved below if you would like to use it. 
;
;***********************************************************************

      	org	$0920  ; start code here
step4e
 
; Put your code here	

do_start
        psha
        ldab #$0 ;initialize count as 0
        ldx #$8  ;number of iterations to do - should be length of a (8 bits)
        
do_loop
        lsla ; push the left-most bit into the carry flag - this also 'increments' what we need to do with a
        adcb #$0 ; add C + 0 + B
        dbne x,do_loop
              
do_exit
        pula       
        
	      stop	; breakpoint here


;***********************************************************************
; ECE 362 - Experiment 2 - Fall 2014
;***********************************************************************