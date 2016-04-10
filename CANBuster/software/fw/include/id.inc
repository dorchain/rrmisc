;------------------------------------------------------------------------------
;
; CANBuster ID routines
;
; Copyright (c) 2016 Gerhard Bertelsmann
;
;------------------------------------------------------------------------------
;
; This file is part of the CANBuster project.
;
; CANBuster is licensed under the CC BY-NC-SA 4.0.
;
; See file /LICENSE for details.
; 
;------------------------------------------------------------------------------

                LIST
                RADIX   DEC


;------------------------------------------------------------------------------
; Generic Flash routines
;------------------------------------------------------------------------------

EEPROM_REFRESH
                CLRF    EEADR		; Start at address 0
                CLRF    EEADRH
                BCF     EECON1, CFGS	; Set for memory
                BCF     EECON1, EEPGD	; Set for Data EEPROM
                BCF     INTCON, GIE	; Disable interrupts
                BSF     EECON1, WREN	; Enable writes

EEPROM_REFRESH_LOOP	; Loop to refresh array
                BSF     EECON1, RD	; Read current address
                MOVLW   55h
                MOVWF   EECON2
                MOVLW   0AAh
                MOVWF   EECON2
                BSF     EECON1, WR	; Set WR bit to begin write
                BTFSC   EECON1, WR	; Wait for write to complete
                BRA $-2
                INCFSZ  EEADR, F	; Increment address
                BRA     EEPROM_REFRESH_LOOP	; Not zero, do it again
                INCFSZ  EEADRH, F	; Increment the high address
                BRA     EEPROM_REFRESH_LOOP	; Not zero, do it again
                BCF     EECON1, WREN	; Disable writes
                BSF     INTCON, GIE	; Enable interrupts
                RETURN


PROGRAM_MEMORY
                BSF     EECON1, EEPGD	; point to Flash program memory
                BCF     EECON1, CFGS	; access Flash program memory
                BSF     EECON1, WREN	; enable write to memory
                BCF     INTCON, GIE	; disable interrupts

                MOVLW   0x55
                MOVWF   EECON2
                MOVLW   0xAA
                MOVWF   EECON2
                BSF     EECON1, WR	; start program (CPU stall)
                BSF     INTCON, GIE	; enable interrupts
                BCF     EECON1, WREN	; Disable writes on write complete (EEIF set)
                RETURN

;------------------------------------------------------------------------------
; Location ID routines
;------------------------------------------------------------------------------
;------------------------------------------------------------------------------
; Location ID
; 0x200000	CAN HASH High
; 0x200001	CAN HASH Low
;------------------------------------------------------------------------------

SETLOCIDP
                MOVWF   TBLPTRL
                CLRF    TBLPTRH
                MOVLW   20h
                MOVWF   TBLPTRU
                RETURN

READUID
                ; read CAN hash ID - stored at 200000h + 200001h
                MOVLW	0
                RCALL   SETLOCIDP
                TBLRD*+
                MOVFF   TABLAT,ID_L
                TBLRD*
                MOVFF   TABLAT,ID_H
                RETURN

SETUID
                ; set CAN hash ID - stored at 200000h + 200001h
                MOVLW	0
                RCALL   SETLOCIDP
                MOVF    ID_H,W
                TBLWT*+
                MOVF    ID_L,W
                TBLWT*
                RETURN

;------------------------------------------------------------------------------
; Set CAN Hash accroding to Maerklin
;------------------------------------------------------------------------------
READCANHASH
                RCALL   READUID
                MOVLW   'C'
                XORWF   ID_H,W
                MOVWF   CANHASH_HIGH
                ; shift left 3
                SWAPF	CANHASH_HIGH,F
                RRNCF	CANHASH_HIGH,F
                ; set to lower bits
                BSF     CANHASH_HIGH,0
                BSF     CANHASH_HIGH,1

                MOVLW   'B'
                XORWF   ID_L,W
                MOVWF   CANHASH_LOW
                BTFSS   ID_L,7
                BSF     CANHASH_HIGH,2
                BCF     CANHASH_LOW,7
                RETURN

;
; vim: set shiftwidth=4 tabstop=4 softtabstop=4 expandtab
;