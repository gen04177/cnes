#include "ccpu.h"
#include <stddef.h>

#define   CPU_READ8(c, addr)           ((c)->read8((addr)))
#define   CPU_WRITE8(c, addr, v)       ((c)->write8((addr), (v)))
#define   CPU_READ16(c, addr)          (CPU_READ8((c), (addr)) | ((uint16_t)CPU_READ8((c), (addr)+1))<<8)
#define   CPU_READ16BUG(c, addr)       (CPU_READ8((c), (addr)) | (uint16_t)(CPU_READ8((c), ((addr)&0xFF00)|((uint16_t)(((uint8_t)(addr))+1)))<<8))

#define CROSSPAGE(a, b) ((a&0xFF00)!=(b&0xFF00))

void iptr(cpu_context *c, uint16_t ivector);

// interrupt types
enum {
    iptNone = 0,
    iptNMI,
    iptIRQ,
};

// addressing modes
enum {
    modeAbsolute = 1,
    modeAbsoluteX,
    modeAbsoluteY,
    modeAccumulator,
    modeImmediate,
    modeImplied,
    modeIndexedIndirect,
    modeIndirect,
    modeIndirectIndexed,
    modeRelative,
    modeZeroPage,
    modeZeroPageX,
    modeZeroPageY,
};

//opcodes
void adc(cpu_context *c, stepInfo *info);
void and(cpu_context *c, stepInfo *info);
void asl(cpu_context *c, stepInfo *info);
void bcc(cpu_context *c, stepInfo *info);
void bcs(cpu_context *c, stepInfo *info);
void beq(cpu_context *c, stepInfo *info);
void bit(cpu_context *c, stepInfo *info);
void bmi(cpu_context *c, stepInfo *info);
void bne(cpu_context *c, stepInfo *info);
void bpl(cpu_context *c, stepInfo *info);
void brk(cpu_context *c, stepInfo *info);
void bvc(cpu_context *c, stepInfo *info);
void bvs(cpu_context *c, stepInfo *info);
void clc(cpu_context *c, stepInfo *info);
void cld(cpu_context *c, stepInfo *info);
void cli(cpu_context *c, stepInfo *info);
void clv(cpu_context *c, stepInfo *info);
void cmp(cpu_context *c, stepInfo *info);
void cpx(cpu_context *c, stepInfo *info);
void cpy(cpu_context *c, stepInfo *info);
void dec(cpu_context *c, stepInfo *info);
void dex(cpu_context *c, stepInfo *info);
void dey(cpu_context *c, stepInfo *info);
void eor(cpu_context *c, stepInfo *info);
void inc(cpu_context *c, stepInfo *info);
void inx(cpu_context *c, stepInfo *info);
void iny(cpu_context *c, stepInfo *info);
void jmp(cpu_context *c, stepInfo *info);
void jsr(cpu_context *c, stepInfo *info);
void lda(cpu_context *c, stepInfo *info);
void ldx(cpu_context *c, stepInfo *info);
void ldy(cpu_context *c, stepInfo *info);
void lsr(cpu_context *c, stepInfo *info);
void nop(cpu_context *c, stepInfo *info);
void ora(cpu_context *c, stepInfo *info);
void pha(cpu_context *c, stepInfo *info);
void php(cpu_context *c, stepInfo *info);
void pla(cpu_context *c, stepInfo *info);
void plp(cpu_context *c, stepInfo *info);
void rol(cpu_context *c, stepInfo *info);
void ror(cpu_context *c, stepInfo *info);
void rti(cpu_context *c, stepInfo *info);
void rts(cpu_context *c, stepInfo *info);
void sbc(cpu_context *c, stepInfo *info);
void sec(cpu_context *c, stepInfo *info);
void sed(cpu_context *c, stepInfo *info);
void sei(cpu_context *c, stepInfo *info);
void sta(cpu_context *c, stepInfo *info);
void stx(cpu_context *c, stepInfo *info);
void sty(cpu_context *c, stepInfo *info);
void tax(cpu_context *c, stepInfo *info);
void tay(cpu_context *c, stepInfo *info);
void tsx(cpu_context *c, stepInfo *info);
void txa(cpu_context *c, stepInfo *info);
void txs(cpu_context *c, stepInfo *info);
void tya(cpu_context *c, stepInfo *info);

//illegal opcodes
void ahx(cpu_context *c, stepInfo *info) {}
void alr(cpu_context *c, stepInfo *info) {}
void anc(cpu_context *c, stepInfo *info) {}
void arr(cpu_context *c, stepInfo *info) {}
void axs(cpu_context *c, stepInfo *info) {}
void dcp(cpu_context *c, stepInfo *info) {}
void isc(cpu_context *c, stepInfo *info) {}
void kil(cpu_context *c, stepInfo *info) {}
void las(cpu_context *c, stepInfo *info) {}
void lax(cpu_context *c, stepInfo *info) {}
void rla(cpu_context *c, stepInfo *info) {}
void rra(cpu_context *c, stepInfo *info) {}
void sax(cpu_context *c, stepInfo *info) {}
void shx(cpu_context *c, stepInfo *info) {}
void shy(cpu_context *c, stepInfo *info) {}
void slo(cpu_context *c, stepInfo *info) {}
void sre(cpu_context *c, stepInfo *info) {}
void tas(cpu_context *c, stepInfo *info) {}
void xaa(cpu_context *c, stepInfo *info) {}

//instruction table
static const cpu_instruction instructions[256] = {
    {brk, "BRK", modeImplied, 1, 0, 0},             // 00
    {ora, "ORA", modeIndexedIndirect, 2, 6, 0},     // 01
    {kil, "KIL", modeImplied, 0, 2, 0},             // 02
    {slo, "SLO", modeIndexedIndirect, 2, 8, 0},     // 03
    {nop, "NOP", modeZeroPage, 2, 3, 0},            // 04
    {ora, "ORA", modeZeroPage, 2, 3, 0},            // 05
    {asl, "ASL", modeZeroPage, 2, 5, 0},            // 06
    {slo, "SLO", modeZeroPage, 2, 5, 0},            // 07
    {php, "PHP", modeImplied, 1, 3, 0},             // 08
    {ora, "ORA", modeImmediate, 2, 2, 0},           // 09
    {asl, "ASL", modeAccumulator, 1, 2, 0},         // 0A
    {anc, "ANC", modeImmediate, 2, 2, 0},           // 0B
    {nop, "NOP", modeAbsolute, 3, 4, 0},            // 0C
    {ora, "ORA", modeAbsolute, 3, 4, 0},            // 0D
    {asl, "ASL", modeAbsolute, 3, 6, 0},            // 0E
    {slo, "SLO", modeAbsolute, 3, 6, 0},            // 0F
    {bpl, "BPL", modeRelative, 2, 2, 1},            // 10
    {ora, "ORA", modeIndirectIndexed, 2, 5, 1},     // 11
    {kil, "KIL", modeImplied, 0, 2, 0},             // 12
    {slo, "SLO", modeIndirectIndexed, 2, 8, 0},     // 13
    {nop, "NOP", modeZeroPageX, 2, 4, 0},           // 14
    {ora, "ORA", modeZeroPageX, 2, 4, 0},           // 15
    {asl, "ASL", modeZeroPageX, 2, 6, 0},           // 16
    {slo, "SLO", modeZeroPageX, 2, 6, 0},           // 17
    {clc, "CLC", modeImplied, 1, 2, 0},             // 18
    {ora, "ORA", modeAbsoluteY, 3, 4, 1},           // 19
    {nop, "NOP", modeImplied, 1, 2, 0},             // 1A
    {slo, "SLO", modeAbsoluteY, 3, 7, 0},           // 1B
    {nop, "NOP", modeAbsoluteX, 3, 4, 1},           // 1C
    {ora, "ORA", modeAbsoluteX, 3, 4, 1},           // 1D
    {asl, "ASL", modeAbsoluteX, 3, 7, 0},           // 1E
    {slo, "SLO", modeAbsoluteX, 3, 7, 0},           // 1F
    {jsr, "JSR", modeAbsolute, 3, 6, 0},            // 20
    {and, "AND", modeIndexedIndirect, 2, 6, 0},     // 21
    {kil, "KIL", modeImplied, 0, 2, 0},             // 22
    {rla, "RLA", modeIndexedIndirect, 2, 8, 0},     // 23
    {bit, "BIT", modeZeroPage, 2, 3, 0},            // 24
    {and, "AND", modeZeroPage, 2, 3, 0},            // 25
    {rol, "ROL", modeZeroPage, 2, 5, 0},            // 26
    {rla, "RLA", modeZeroPage, 2, 5, 0},            // 27
    {plp, "PLP", modeImplied, 1, 4, 0},             // 28
    {and, "AND", modeImmediate, 2, 2, 0},           // 29
    {rol, "ROL", modeAccumulator, 1, 2, 0},         // 2A
    {anc, "ANC", modeImmediate, 2, 2, 0},           // 2B
    {bit, "BIT", modeAbsolute, 3, 4, 0},            // 2C
    {and, "AND", modeAbsolute, 3, 4, 0},            // 2D
    {rol, "ROL", modeAbsolute, 3, 6, 0},            // 2E
    {rla, "RLA", modeAbsolute, 3, 6, 0},            // 2F
    {bmi, "BMI", modeRelative, 2, 2, 1},            // 30
    {and, "AND", modeIndirectIndexed, 2, 5, 1},     // 31
    {kil, "KIL", modeImplied, 0, 2, 0},             // 32
    {rla, "RLA", modeIndirectIndexed, 2, 8, 0},     // 33
    {nop, "NOP", modeZeroPageX, 2, 4, 0},           // 34
    {and, "AND", modeZeroPageX, 2, 4, 0},           // 35
    {rol, "ROL", modeZeroPageX, 2, 6, 0},           // 36
    {rla, "RLA", modeZeroPageX, 2, 6, 0},           // 37
    {sec, "SEC", modeImplied, 1, 2, 0},             // 38
    {and, "AND", modeAbsoluteY, 3, 4, 1},           // 39
    {nop, "NOP", modeImplied, 1, 2, 0},             // 3A
    {rla, "RLA", modeAbsoluteY, 3, 7, 0},           // 3B
    {nop, "NOP", modeAbsoluteX, 3, 4, 1},           // 3C
    {and, "AND", modeAbsoluteX, 3, 4, 1},           // 3D
    {rol, "ROL", modeAbsoluteX, 3, 7, 0},           // 3E
    {rla, "RLA", modeAbsoluteX, 3, 7, 0},           // 3F
    {rti, "RTI", modeImplied, 1, 6, 0},             // 40
    {eor, "EOR", modeIndexedIndirect, 2, 6, 0},     // 41
    {kil, "KIL", modeImplied, 0, 2, 0},             // 42
    {sre, "SRE", modeIndexedIndirect, 2, 8, 0},     // 43
    {nop, "NOP", modeZeroPage, 2, 3, 0},            // 44
    {eor, "EOR", modeZeroPage, 2, 3, 0},            // 45
    {lsr, "LSR", modeZeroPage, 2, 5, 0},            // 46
    {sre, "SRE", modeZeroPage, 2, 5, 0},            // 47
    {pha, "PHA", modeImplied, 1, 3, 0},             // 48
    {eor, "EOR", modeImmediate, 2, 2, 0},           // 49
    {lsr, "LSR", modeAccumulator, 1, 2, 0},         // 4A
    {alr, "ALR", modeImmediate, 2, 2, 0},           // 4B
    {jmp, "JMP", modeAbsolute, 3, 3, 0},            // 4C
    {eor, "EOR", modeAbsolute, 3, 4, 0},            // 4D
    {lsr, "LSR", modeAbsolute, 3, 6, 0},            // 4E
    {sre, "SRE", modeAbsolute, 3, 6, 0},            // 4F
    {bvc, "BVC", modeRelative, 2, 2, 1},            // 50
    {eor, "EOR", modeIndirectIndexed, 2, 5, 1},     // 51
    {kil, "KIL", modeImplied, 0, 2, 0},             // 52
    {sre, "SRE", modeIndirectIndexed, 2, 8, 0},     // 53
    {nop, "NOP", modeZeroPageX, 2, 4, 0},           // 54
    {eor, "EOR", modeZeroPageX, 2, 4, 0},           // 55
    {lsr, "LSR", modeZeroPageX, 2, 6, 0},           // 56
    {sre, "SRE", modeZeroPageX, 2, 6, 0},           // 57
    {cli, "CLI", modeImplied, 1, 2, 0},             // 58
    {eor, "EOR", modeAbsoluteY, 3, 4, 1},           // 59
    {nop, "NOP", modeImplied, 1, 2, 0},             // 5A
    {sre, "SRE", modeAbsoluteY, 3, 7, 0},           // 5B
    {nop, "NOP", modeAbsoluteX, 3, 4, 1},           // 5C
    {eor, "EOR", modeAbsoluteX, 3, 4, 1},           // 5D
    {lsr, "LSR", modeAbsoluteX, 3, 7, 0},           // 5E
    {sre, "SRE", modeAbsoluteX, 3, 7, 0},           // 5F
    {rts, "RTS", modeImplied, 1, 6, 0},             // 60
    {adc, "ADC", modeIndexedIndirect, 2, 6, 0},     // 61
    {kil, "KIL", modeImplied, 0, 2, 0},             // 62
    {rra, "RRA", modeIndexedIndirect, 2, 8, 0},     // 63
    {nop, "NOP", modeZeroPage, 2, 3, 0},            // 64
    {adc, "ADC", modeZeroPage, 2, 3, 0},            // 65
    {ror, "ROR", modeZeroPage, 2, 5, 0},            // 66
    {rra, "RRA", modeZeroPage, 2, 5, 0},            // 67
    {pla, "PLA", modeImplied, 1, 4, 0},             // 68
    {adc, "ADC", modeImmediate, 2, 2, 0},           // 69
    {ror, "ROR", modeAccumulator, 1, 2, 0},         // 6A
    {arr, "ARR", modeImmediate, 2, 2, 0},           // 6B
    {jmp, "JMP", modeIndirect, 3, 5, 0},            // 6C
    {adc, "ADC", modeAbsolute, 3, 4, 0},            // 6D
    {ror, "ROR", modeAbsolute, 3, 6, 0},            // 6E
    {rra, "RRA", modeAbsolute, 3, 6, 0},            // 6F
    {bvs, "BVS", modeRelative, 2, 2, 1},            // 70
    {adc, "ADC", modeIndirectIndexed, 2, 5, 1},     // 71
    {kil, "KIL", modeImplied, 0, 2, 0},             // 72
    {rra, "RRA", modeIndirectIndexed, 2, 8, 0},     // 73
    {nop, "NOP", modeZeroPageX, 2, 4, 0},           // 74
    {adc, "ADC", modeZeroPageX, 2, 4, 0},           // 75
    {ror, "ROR", modeZeroPageX, 2, 6, 0},           // 76
    {rra, "RRA", modeZeroPageX, 2, 6, 0},           // 77
    {sei, "SEI", modeImplied, 1, 2, 0},             // 78
    {adc, "ADC", modeAbsoluteY, 3, 4, 1},           // 79
    {nop, "NOP", modeImplied, 1, 2, 0},             // 7A
    {rra, "RRA", modeAbsoluteY, 3, 7, 0},           // 7B
    {nop, "NOP", modeAbsoluteX, 3, 4, 1},           // 7C
    {adc, "ADC", modeAbsoluteX, 3, 4, 1},           // 7D
    {ror, "ROR", modeAbsoluteX, 3, 7, 0},           // 7E
    {rra, "RRA", modeAbsoluteX, 3, 7, 0},           // 7F
    {nop, "NOP", modeImmediate, 2, 2, 0},           // 80
    {sta, "STA", modeIndexedIndirect, 2, 6, 0},     // 81
    {nop, "NOP", modeImmediate, 2, 2, 0},           // 82
    {sax, "SAX", modeIndexedIndirect, 2, 6, 0},     // 83
    {sty, "STY", modeZeroPage, 2, 3, 0},            // 84
    {sta, "STA", modeZeroPage, 2, 3, 0},            // 85
    {stx, "STX", modeZeroPage, 2, 3, 0},            // 86
    {sax, "SAX", modeZeroPage, 2, 3, 0},            // 87
    {dey, "DEY", modeImplied, 1, 2, 0},             // 88
    {nop, "NOP", modeImmediate, 2, 2, 0},           // 89
    {txa, "TXA", modeImplied, 1, 2, 0},             // 8A
    {xaa, "XAA", modeImmediate, 2, 2, 0},           // 8B
    {sty, "STY", modeAbsolute, 3, 4, 0},            // 8C
    {sta, "STA", modeAbsolute, 3, 4, 0},            // 8D
    {stx, "STX", modeAbsolute, 3, 4, 0},            // 8E
    {sax, "SAX", modeAbsolute, 3, 4, 0},            // 8F
    {bcc, "BCC", modeRelative, 2, 2, 1},            // 90
    {sta, "STA", modeIndirectIndexed, 2, 6, 0},     // 91
    {kil, "KIL", modeImplied, 0, 2, 0},             // 92
    {ahx, "AHX", modeIndirectIndexed, 2, 6, 0},     // 93
    {sty, "STY", modeZeroPageX, 2, 4, 0},           // 94
    {sta, "STA", modeZeroPageX, 2, 4, 0},           // 95
    {stx, "STX", modeZeroPageY, 2, 4, 0},           // 96
    {sax, "SAX", modeZeroPageY, 2, 4, 0},           // 97
    {tya, "TYA", modeImplied, 1, 2, 0},             // 98
    {sta, "STA", modeAbsoluteY, 3, 5, 0},           // 99
    {txs, "TXS", modeImplied, 1, 2, 0},             // 9A
    {tas, "TAS", modeAbsoluteY, 3, 5, 0},           // 9B
    {shy, "SHY", modeAbsoluteX, 3, 5, 0},           // 9C
    {sta, "STA", modeAbsoluteX, 3, 5, 0},           // 9D
    {shx, "SHX", modeAbsoluteY, 3, 5, 0},           // 9E
    {ahx, "AHX", modeAbsoluteY, 3, 5, 0},           // 9F
    {ldy, "LDY", modeImmediate, 2, 2, 0},           // A0
    {lda, "LDA", modeIndexedIndirect, 2, 6, 0},     // A1
    {ldx, "LDX", modeImmediate, 2, 2, 0},           // A2
    {lax, "LAX", modeIndexedIndirect, 2, 6, 0},     // A3
    {ldy, "LDY", modeZeroPage, 2, 3, 0},            // A4
    {lda, "LDA", modeZeroPage, 2, 3, 0},            // A5
    {ldx, "LDX", modeZeroPage, 2, 3, 0},            // A6
    {lax, "LAX", modeZeroPage, 2, 3, 0},            // A7
    {tay, "TAY", modeImplied, 1, 2, 0},             // A8
    {lda, "LDA", modeImmediate, 2, 2, 0},           // A9
    {tax, "TAX", modeImplied, 1, 2, 0},             // AA
    {lax, "LAX", modeImmediate, 2, 2, 0},           // AB
    {ldy, "LDY", modeAbsolute, 3, 4, 0},            // AC
    {lda, "LDA", modeAbsolute, 3, 4, 0},            // AD
    {ldx, "LDX", modeAbsolute, 3, 4, 0},            // AE
    {lax, "LAX", modeAbsolute, 3, 4, 0},            // AF
    {bcs, "BCS", modeRelative, 2, 2, 1},            // B0
    {lda, "LDA", modeIndirectIndexed, 2, 5, 1},     // B1
    {kil, "KIL", modeImplied, 0, 2, 0},             // B2
    {lax, "LAX", modeIndirectIndexed, 2, 5, 1},     // B3
    {ldy, "LDY", modeZeroPageX, 2, 4, 0},           // B4
    {lda, "LDA", modeZeroPageX, 2, 4, 0},           // B5
    {ldx, "LDX", modeZeroPageY, 2, 4, 0},           // B6
    {lax, "LAX", modeZeroPageY, 2, 4, 0},           // B7
    {clv, "CLV", modeImplied, 1, 2, 0},             // B8
    {lda, "LDA", modeAbsoluteY, 3, 4, 1},           // B9
    {tsx, "TSX", modeImplied, 1, 2, 0},             // BA
    {las, "LAS", modeAbsoluteY, 3, 4, 1},           // BB
    {ldy, "LDY", modeAbsoluteX, 3, 4, 1},           // BC
    {lda, "LDA", modeAbsoluteX, 3, 4, 1},           // BD
    {ldx, "LDX", modeAbsoluteY, 3, 4, 1},           // BE
    {lax, "LAX", modeAbsoluteY, 3, 4, 1},           // BF
    {cpy, "CPY", modeImmediate, 2, 2, 0},           // C0
    {cmp, "CMP", modeIndexedIndirect, 2, 6, 0},     // C1
    {nop, "NOP", modeImmediate, 2, 2, 0},           // C2
    {dcp, "DCP", modeIndexedIndirect, 2, 8, 0},     // C3
    {cpy, "CPY", modeZeroPage, 2, 3, 0},            // C4
    {cmp, "CMP", modeZeroPage, 2, 3, 0},            // C5
    {dec, "DEC", modeZeroPage, 2, 5, 0},            // C6
    {dcp, "DCP", modeZeroPage, 2, 5, 0},            // C7
    {iny, "INY", modeImplied, 1, 2, 0},             // C8
    {cmp, "CMP", modeImmediate, 2, 2, 0},           // C9
    {dex, "DEX", modeImplied, 1, 2, 0},             // CA
    {axs, "AXS", modeImmediate, 2, 2, 0},           // CB
    {cpy, "CPY", modeAbsolute, 3, 4, 0},            // CC
    {cmp, "CMP", modeAbsolute, 3, 4, 0},            // CD
    {dec, "DEC", modeAbsolute, 3, 6, 0},            // CE
    {dcp, "DCP", modeAbsolute, 3, 6, 0},            // CF
    {bne, "BNE", modeRelative, 2, 2, 1},            // D0
    {cmp, "CMP", modeIndirectIndexed, 2, 5, 1},     // D1
    {kil, "KIL", modeImplied, 0, 2, 0},             // D2
    {dcp, "DCP", modeIndirectIndexed, 2, 8, 0},     // D3
    {nop, "NOP", modeZeroPageX, 2, 4, 0},           // D4
    {cmp, "CMP", modeZeroPageX, 2, 4, 0},           // D5
    {dec, "DEC", modeZeroPageX, 2, 6, 0},           // D6
    {dcp, "DCP", modeZeroPageX, 2, 6, 0},           // D7
    {cld, "CLD", modeImplied, 1, 2, 0},             // D8
    {cmp, "CMP", modeAbsoluteY, 3, 4, 1},           // D9
    {nop, "NOP", modeImplied, 1, 2, 0},             // DA
    {dcp, "DCP", modeAbsoluteY, 3, 7, 0},           // DB
    {nop, "NOP", modeAbsoluteX, 3, 4, 1},           // DC
    {cmp, "CMP", modeAbsoluteX, 3, 4, 1},           // DD
    {dec, "DEC", modeAbsoluteX, 3, 7, 0},           // DE
    {dcp, "DCP", modeAbsoluteX, 3, 7, 0},           // DF
    {cpx, "CPX", modeImmediate, 2, 2, 0},           // E0
    {sbc, "SBC", modeIndexedIndirect, 2, 6, 0},     // E1
    {nop, "NOP", modeImmediate, 2, 2, 0},           // E2
    {isc, "ISC", modeIndexedIndirect, 2, 8, 0},     // E3
    {cpx, "CPX", modeZeroPage, 2, 3, 0},            // E4
    {sbc, "SBC", modeZeroPage, 2, 3, 0},            // E5
    {inc, "INC", modeZeroPage, 2, 5, 0},            // E6
    {isc, "ISC", modeZeroPage, 2, 5, 0},            // E7
    {inx, "INX", modeImplied, 1, 2, 0},             // E8
    {sbc, "SBC", modeImmediate, 2, 2, 0},           // E9
    {nop, "NOP", modeImplied, 1, 2, 0},             // EA
    {sbc, "SBC", modeImmediate, 2, 2, 0},           // EB
    {cpx, "CPX", modeAbsolute, 3, 4, 0},            // EC
    {sbc, "SBC", modeAbsolute, 3, 4, 0},            // ED
    {inc, "INC", modeAbsolute, 3, 6, 0},            // EE
    {isc, "ISC", modeAbsolute, 3, 6, 0},            // EF
    {beq, "BEQ", modeRelative, 2, 2, 1},            // F0
    {sbc, "SBC", modeIndirectIndexed, 2, 5, 1},     // F1
    {kil, "KIL", modeImplied, 0, 2, 0},             // F2
    {isc, "ISC", modeIndirectIndexed, 2, 8, 0},     // F3
    {nop, "NOP", modeZeroPageX, 2, 4, 0},           // F4
    {sbc, "SBC", modeZeroPageX, 2, 4, 0},           // F5
    {inc, "INC", modeZeroPageX, 2, 6, 0},           // F6
    {isc, "ISC", modeZeroPageX, 2, 6, 0},           // F7
    {sed, "SED", modeImplied, 1, 2, 0},             // F8
    {sbc, "SBC", modeAbsoluteY, 3, 4, 1},           // F9
    {nop, "NOP", modeImplied, 1, 2, 0},             // FA
    {isc, "ISC", modeAbsoluteY, 3, 7, 0},           // FB
    {nop, "NOP", modeAbsoluteX, 3, 4, 1},           // FC
    {sbc, "SBC", modeAbsoluteX, 3, 4, 1},           // FD
    {inc, "INC", modeAbsoluteX, 3, 7, 0},           // FE
    {isc, "ISC", modeAbsoluteX, 3, 7, 0}            // FF
};


void addBranchCycles(cpu_context *c, stepInfo* info)
{
    CROSSPAGE(info->pc, info->address)?(c->cycles+=2):(c->cycles+=1);
}

void branch(cpu_context *c, stepInfo* info, char value)
{
    if (value) { c->PC = info->address; addBranchCycles(c, info); }
}

void setC(cpu_context *c, char value)
{
    value?(c->P|=P_C):(c->P&=~P_C);
}

void setV(cpu_context *c, char value)
{
    value?(c->P|=P_V):(c->P&=~P_V);
}

void setZ(cpu_context *c, uint8_t value)
{
    value?(c->P&=~P_Z):(c->P|=P_Z);
}

void setN(cpu_context *c, uint8_t value)
{
    (value&0x80)?(c->P|=P_N):(c->P&=~P_N);
}

void setZN(cpu_context *c, uint8_t value)
{
    setZ(c, value); setN(c, value);
}

void compare(cpu_context *c, uint8_t a, uint8_t b)
{
    setZN(c, a - b); setC(c, a >= b);
}

void push(cpu_context *c, uint8_t value)
{
    CPU_WRITE8(c, 0x100|(uint16_t)(c->SP), value); c->SP--;
}

uint8_t pull(cpu_context *c)
{
    c->SP++; return CPU_READ8(c, 0x100|(uint16_t)(c->SP));
}

void push16(cpu_context *c, uint16_t value)
{
    push(c, (uint8_t)(value >> 8)); push(c, (uint8_t)(value & 0xFF));
}

uint16_t pull16(cpu_context *c)
{
    return pull(c) | (uint16_t)pull(c) << 8;
}

// ADC - Add with Carry
void adc(cpu_context *c, stepInfo* info)
{
    uint8_t a = c->A, b = CPU_READ8(c, info->address), cy = (c->P & P_C) >> 0;
    setZN(c, c->A = a + b + cy);
    setC(c, (int)(a)+(int)(b)+(int)(cy) > 0xFF);
    setV(c, (((a^b)&0x80) == 0) && (((a^c->A)&0x80) != 0));
}

// AND - Logical AND
void and(cpu_context *c, stepInfo* info)
{
    setZN(c, c->A &= CPU_READ8(c, info->address));
}

// ASL - Arithmetic Shift Left
void asl(cpu_context *c, stepInfo* info)
{
    if (info->mode == modeAccumulator) {
        setC(c, ((c->A >> 7) & 1));
        setZN(c, c->A <<= 1);
    } else {
        uint8_t value = CPU_READ8(c, info->address);
        setC(c, ((value >> 7) & 1));
        setZN(c, value <<= 1);
        CPU_WRITE8(c, info->address, value);
    }
}

// BCC - Branch if Carry Clear
void bcc(cpu_context *c, stepInfo* info)
{
    branch(c, info, !(c->P&P_C));
}

// BCS - Branch if Carry Set
void bcs(cpu_context *c, stepInfo* info)
{
    branch(c, info, c->P & P_C);
}

// BEQ - Branch if Equal
void beq(cpu_context *c, stepInfo* info)
{
    branch(c, info, c->P & P_Z);
}

// BIT - Bit Test
void bit(cpu_context *c, stepInfo* info)
{
    uint8_t value = CPU_READ8(c, info->address);
    setV(c, ((value >> 6) & 1));
    setZ(c, value & c->A);
    setN(c, value);
}

// BMI - Branch if Minus
void bmi(cpu_context *c, stepInfo* info)
{
    branch(c, info, c->P & P_N);
}

// BNE - Branch if Not Equal
void bne(cpu_context *c, stepInfo* info)
{
    branch(c, info, !(c->P & P_Z));
}

// BPL - Branch if Positive
void bpl(cpu_context *c, stepInfo* info)
{
    branch(c, info, !(c->P & P_N));
}

// BRK - Force Interrupt
void brk(cpu_context *c, stepInfo* info)
{
    iptr(c, 0xFFFE);
}

// BVC - Branch if Overflow Clear
void bvc(cpu_context *c, stepInfo* info)
{
    branch(c, info, !(c->P & P_V));
}

// BVS - Branch if Overflow Set
void bvs(cpu_context *c, stepInfo* info)
{
    branch(c, info, c->P & P_V);
}

// CLC - Clear Carry Flag
void clc(cpu_context *c, stepInfo* info)
{
    c->P &= ~P_C;
}

// CLD - Clear Decimal Mode
void cld(cpu_context *c, stepInfo* info)
{
    c->P &= ~P_D;
}

// CLI - Clear Interrupt Disable
void cli(cpu_context *c, stepInfo* info)
{
    c->P &= ~P_I;
}

// CLV - Clear Overflow Flag
void clv(cpu_context *c, stepInfo* info)
{
    c->P &= ~P_V;
}

// CMP - Compare
void cmp(cpu_context *c, stepInfo* info)
{
    compare(c, c->A, CPU_READ8(c, info->address));
}

// CPX - Compare X Register
void cpx(cpu_context *c, stepInfo* info)
{
    compare(c, c->X, CPU_READ8(c, info->address));
}

// CPY - Compare Y Register
void cpy(cpu_context *c, stepInfo* info)
{
    compare(c, c->Y, CPU_READ8(c, info->address));
}

// DEC - Decrement Memory
void dec(cpu_context *c, stepInfo* info)
{
    uint8_t value = CPU_READ8(c, info->address) - 1;
    CPU_WRITE8(c, info->address, value);
    setZN(c, value);
}

// DEX - Decrement X Register
void dex(cpu_context *c, stepInfo* info)
{
    setZN(c, --c->X);
}

// DEY - Decrement Y Register
void dey(cpu_context *c, stepInfo* info)
{
    setZN(c, --c->Y);
}

// EOR - Exclusive OR
void eor(cpu_context *c, stepInfo* info)
{
    setZN(c, c->A ^= CPU_READ8(c, info->address));
}

// INC - Increment Memory
void inc(cpu_context *c, stepInfo* info)
{
    uint8_t value = CPU_READ8(c, info->address) + 1;
    CPU_WRITE8(c, info->address, value);
    setZN(c, value);
}

// INX - Increment X Register
void inx(cpu_context *c, stepInfo* info)
{
    setZN(c, ++c->X);
}

// INY - Increment Y Register
void iny(cpu_context *c, stepInfo* info)
{
    setZN(c, ++c->Y);
}

// JMP - Jump
void jmp(cpu_context *c, stepInfo* info)
{
    c->PC = info->address;
}

// JSR - Jump to Subroutine
void jsr(cpu_context *c, stepInfo* info)
{
    push16(c, c->PC - 1);
    jmp(c, info);
}

// LDA - Load Accumulator
void lda(cpu_context *c, stepInfo* info)
{
    setZN(c, c->A = CPU_READ8(c, info->address));
}

// LDX - Load X Register
void ldx(cpu_context *c, stepInfo* info)
{
    setZN(c, c->X = CPU_READ8(c, info->address));
}

// LDY - Load Y Register
void ldy(cpu_context *c, stepInfo* info)
{
    setZN(c, c->Y = CPU_READ8(c, info->address));
}

// LSR - Logical Shift Right
void lsr(cpu_context *c, stepInfo* info)
{
    if (info->mode == modeAccumulator) {
        setC(c, (c->A & 0x1));
        setZN(c, c->A >>= 1);
    } else {
        uint8_t value = CPU_READ8(c, info->address);
        setC(c, (value & 0x1));
        setZN(c, value >>= 1);
        CPU_WRITE8(c, info->address, value);
    }
}

// NOP - No Operation
void nop(cpu_context *c, stepInfo* info)
{
    // No operation
}

// ORA - Logical Inclusive OR
void ora(cpu_context *c, stepInfo* info)
{
    setZN(c, c->A |= CPU_READ8(c, info->address));
}

// PHA - Push Accumulator
void pha(cpu_context *c, stepInfo* info)
{
    push(c, c->A);
}

// PHP - Push Processor Status
void php(cpu_context *c, stepInfo* info)
{
    push(c, c->P | P_U);
}

// PLA - Pull Accumulator
void pla(cpu_context *c, stepInfo* info)
{
    setZN(c, c->A = pull(c));
}

// PLP - Pull Processor Status
void plp(cpu_context *c, stepInfo* info)
{
    c->P = (pull(c) & 0xEF) | 0x20;
}

// ROL - Rotate Left
void rol(cpu_context *c, stepInfo* info)
{
    if (info->mode == modeAccumulator) {
        uint8_t cy = (c->P & P_C) >> 0;
        setC(c, ((c->A >> 7) & 1));
        c->A = (c->A << 1) | cy;
        setZN(c, c->A);
    } else {
        uint8_t cy = (c->P & P_C) >> 0;
        uint8_t value = CPU_READ8(c, info->address);
        setC(c, ((value >> 7) & 1));
        value = (value << 1) | cy;
        CPU_WRITE8(c, info->address, value);
        setZN(c, value);
    }
}

// ROR - Rotate Right
void ror(cpu_context *c, stepInfo* info)
{
    if (info->mode == modeAccumulator) {
        uint8_t cy = (c->P & P_C) >> 0;
        setC(c, (c->A & 1));
        c->A = (c->A >> 1) | (cy << 7);
        setZN(c, c->A);
    } else {
        uint8_t cy = (c->P & P_C) >> 0;
        uint8_t value = CPU_READ8(c, info->address);
        setC(c, (value & 1));
        value = (value >> 1) | (cy << 7);
        CPU_WRITE8(c, info->address, value);
        setZN(c, value);
    }
}

// RTI - Return from Interrupt
void rti(cpu_context *c, stepInfo* info)
{
    c->P = (pull(c) & 0xEF) | 0x20;
    c->PC = pull16(c);
}

// RTS - Return from Subroutine
void rts(cpu_context *c, stepInfo* info)
{
    c->PC = pull16(c) + 1;
}

// SBC - Subtract with Carry
void sbc(cpu_context *c, stepInfo* info)
{
    uint8_t a = c->A, b = CPU_READ8(c, info->address), cy = (c->P & P_C) >> 0;
    setZN(c, c->A = a - b - (1 - cy));
    setC(c, (int)(a) - (int)(b) - (int)(1 - cy) >= 0);
    setV(c, ((a ^ b) & 0x80) != 0 && ((a ^ c->A) & 0x80) != 0);
}

// SEC - Set Carry Flag
void sec(cpu_context *c, stepInfo* info)
{
    c->P |= P_C;
}

// SED - Set Decimal Flag
void sed(cpu_context *c, stepInfo* info)
{
    c->P |= P_D;
}

// SEI - Set Interrupt Disable
void sei(cpu_context *c, stepInfo* info)
{
    c->P |= P_I;
}

// STA - Store Accumulator
void sta(cpu_context *c, stepInfo* info)
{
    CPU_WRITE8(c, info->address, c->A);
}

// STX - Store X Register
void stx(cpu_context *c, stepInfo* info)
{
    CPU_WRITE8(c, info->address, c->X);
}

// STY - Store Y Register
void sty(cpu_context *c, stepInfo* info)
{
    CPU_WRITE8(c, info->address, c->Y);
}

// TAX - Transfer Accumulator to X
void tax(cpu_context *c, stepInfo* info)
{
    setZN(c, c->X = c->A);
}

// TAY - Transfer Accumulator to Y
void tay(cpu_context *c, stepInfo* info)
{
    setZN(c, c->Y = c->A);
}

// TSX - Transfer Stack Pointer to X
void tsx(cpu_context *c, stepInfo* info)
{
    setZN(c, c->X = c->SP);
}

// TXA - Transfer X to Accumulator
void txa(cpu_context *c, stepInfo* info)
{
    setZN(c, c->A = c->X);
}

// TXS - Transfer X to Stack Pointer
void txs(cpu_context *c, stepInfo* info)
{
    c->SP = c->X;
}

// TYA - Transfer Y to Accumulator
void tya(cpu_context *c, stepInfo* info)
{
    setZN(c, c->A = c->Y);
}


void bke(cpu_context *c, stepInfo* info)
{
    iptr(c, 0xFFFE);
}

void iptt(cpu_context *c, uint8_t type)
{
    if (type == iptNMI) { c->ipttype = type; }
    if (type == iptIRQ && !(c->P&P_I)) { c->ipttype = iptIRQ; }
}

void iptr(cpu_context *c, uint16_t ivector)
{
    stepInfo info;
    push16(c, c->PC);
    php(c, &info);
    sei(c, &info);
    c->PC = CPU_READ16(c, ivector);
    c->cycles += 7;
}

void reset(cpu_context* c)
{
    c->PC = CPU_READ16(c, 0xFFFC); c->SP = 0xFD; c->P = 0x24;
}


uint8_t cpu_step_i(cpu_context *c)
{
    if (c->stall > 0) { c->stall--; return 1; }
    uint64_t cycles = c->cycles;
    c->ipttype == iptNMI ? (iptr(c, 0xFFFA)) : (c->ipttype == iptIRQ ? (iptr(c, 0xFFFE)) : (0));
    c->ipttype = iptNone;
    uint8_t opcode = CPU_READ8(c, c->PC);
    const cpu_instruction *inst = &instructions[opcode];
    uint16_t address = 0, pageCrossed = 0;
    stepInfo info;
    switch (inst->mode) {
        case modeAbsolute:
            address = CPU_READ16(c, c->PC + 1); break;
        case modeAbsoluteX: {
            address = CPU_READ16(c, c->PC + 1) + c->X;
            pageCrossed = CROSSPAGE(address - c->X, address);
            break;
        }
        case modeAbsoluteY: {
            address = CPU_READ16(c, c->PC + 1) + c->Y;
            pageCrossed = CROSSPAGE(address - c->Y, address);
            break;
        }
        case modeAccumulator:
            address = 0; break;
        case modeImmediate:
            address = c->PC + 1; break;
        case modeImplied:
            address = 0; break;
        case modeIndexedIndirect:
            address = CPU_READ16BUG(c, CPU_READ8(c, c->PC + 1) + c->X); break;
        case modeIndirect:
            address = CPU_READ16BUG(c, CPU_READ16(c, c->PC + 1)); break;
        case modeIndirectIndexed: {
            address = CPU_READ16BUG(c, CPU_READ8(c, c->PC + 1)) + c->Y;
            pageCrossed = CROSSPAGE(address - c->Y, address);
            break;
        }
        case modeRelative: {
            address = c->PC + 2;
            uint16_t offset = CPU_READ8(c, c->PC + 1);
            (offset < 0x80) ? (address += offset) : (address += offset - 0x100);
            break;
        }
        case modeZeroPage:
            address = CPU_READ8(c, c->PC + 1); break;
        case modeZeroPageX:
            address = CPU_READ8(c, c->PC + 1) + c->X; break;
        case modeZeroPageY:
            address = CPU_READ8(c, c->PC + 1) + c->Y; break;
    }
    c->PC += inst->size;
    c->cycles += pageCrossed ? inst->cycles + inst->pageCycles : inst->cycles;
    info.address = address;
    info.pc = c->PC;
    info.mode = inst->mode;
    inst->execute(c, &info);
    return (c->cycles - cycles);
}


void cpu_init(cpu_context *c)
{
    reset(c);
}

uint8_t cpu_step(cpu_context *c)
{
    return cpu_step_i(c);
}

void cpu_nmi(cpu_context *c)
{
    iptt(c, iptNMI);
}

void cpu_irq(cpu_context *c)
{
    iptt(c, iptIRQ);
}
