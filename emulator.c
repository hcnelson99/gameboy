#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include <readline/readline.h>
#include <readline/history.h>

#define u8 uint8_t
#define i8 int8_t
#define u16 uint16_t

typedef struct CPU {
    u8 a;
    u8 f;
    u8 b;
    u8 c;
    u8 d;
    u8 e;
    u8 h;
    u8 l;
    u16 pc;
    u16 sp;
    u8 memory[0xFFFF];
} CPU;

u8 boot_rom[] = {
  0x31, 0xfe, 0xff, 0xaf, 0x21, 0xff, 0x9f, 0x32, // 0x00
  0xcb, 0x7c, 0x20, 0xfb, 0x21, 0x26, 0xff, 0x0e, // 0x08
  0x11, 0x3e, 0x80, 0x32, 0xe2, 0x0c, 0x3e, 0xf3, // 0x10
  0xe2, 0x32, 0x3e, 0x77, 0x77, 0x3e, 0xfc, 0xe0, // 0x18
  0x47, 0x11, 0x04, 0x01, 0x21, 0x10, 0x80, 0x1a, // 0x20
  0xcd, 0x95, 0x00, 0xcd, 0x96, 0x00, 0x13, 0x7b, // 0x28
  0xfe, 0x34, 0x20, 0xf3, 0x11, 0xd8, 0x00, 0x06, // 0x30
  0x08, 0x1a, 0x13, 0x22, 0x23, 0x05, 0x20, 0xf9, // 0x38
  0x3e, 0x19, 0xea, 0x10, 0x99, 0x21, 0x2f, 0x99, // 0x40
  0x0e, 0x0c, 0x3d, 0x28, 0x08, 0x32, 0x0d, 0x20, // 0x48
  0xf9, 0x2e, 0x0f, 0x18, 0xf3, 0x67, 0x3e, 0x64, // 0x50
  0x57, 0xe0, 0x42, 0x3e, 0x91, 0xe0, 0x40, 0x04, // 0x58
  0x1e, 0x02, 0x0e, 0x0c, 0xf0, 0x44, 0xfe, 0x90, // 0x60
  0x20, 0xfa, 0x0d, 0x20, 0xf7, 0x1d, 0x20, 0xf2, // 0x68
  0x0e, 0x13, 0x24, 0x7c, 0x1e, 0x83, 0xfe, 0x62, // 0x70
  0x28, 0x06, 0x1e, 0xc1, 0xfe, 0x64, 0x20, 0x06, // 0x78
  0x7b, 0xe2, 0x0c, 0x3e, 0x87, 0xe2, 0xf0, 0x42, // 0x80
  0x90, 0xe0, 0x42, 0x15, 0x20, 0xd2, 0x05, 0x20, // 0x88
  0x4f, 0x16, 0x20, 0x18, 0xcb, 0x4f, 0x06, 0x04, // 0x90
  0xc5, 0xcb, 0x11, 0x17, 0xc1, 0xcb, 0x11, 0x17, // 0x98
  0x05, 0x20, 0xf5, 0x22, 0x23, 0x22, 0x23, 0xc9, // 0xa0
  0xce, 0xed, 0x66, 0x66, 0xcc, 0x0d, 0x00, 0x0b, // 0xa8
  0x03, 0x73, 0x00, 0x83, 0x00, 0x0c, 0x00, 0x0d, // 0xb0
  0x00, 0x08, 0x11, 0x1f, 0x88, 0x89, 0x00, 0x0e, // 0xb8
  0xdc, 0xcc, 0x6e, 0xe6, 0xdd, 0xdd, 0xd9, 0x99, // 0xc0
  0xbb, 0xbb, 0x67, 0x63, 0x6e, 0x0e, 0xec, 0xcc, // 0xc8
  0xdd, 0xdc, 0x99, 0x9f, 0xbb, 0xb9, 0x33, 0x3e, // 0xd0
  0x3c, 0x42, 0xb9, 0xa5, 0xb9, 0xa5, 0x42, 0x3c, // 0xd8
  0x21, 0x04, 0x01, 0x11, 0xa8, 0x00, 0x1a, 0x13, // 0xe0
  0xbe, 0x20, 0xfe, 0x23, 0x7d, 0xfe, 0x34, 0x20, // 0xe8
  0xf5, 0x06, 0x19, 0x78, 0x86, 0x23, 0x05, 0x20, // 0xf0
  0xfb, 0x86, 0x20, 0xfe, 0x3e, 0x01, 0xe0, 0x50  // 0xf8
}; 
u16 boot_rom_len = 256; 

#define rom_len 0x8000
u8 rom[rom_len];

void init_cpu(CPU *cpu) {
    cpu->a = 0;
    cpu->f = 0;
    cpu->b = 0;
    cpu->c = 0;
    cpu->d = 0;
    cpu->e = 0;
    cpu->h = 0;
    cpu->l = 0;
    cpu->pc = 0;
    cpu->sp = 0;
    memset(cpu->memory, 0, 0xFFFF);
    memcpy(cpu->memory, rom, rom_len);
    memcpy(cpu->memory, boot_rom, boot_rom_len);
}

const int Z_INDEX = 7;
const int N_INDEX = 6;
const int H_INDEX = 5;
const int C_INDEX = 4;

const u8 Z_MASK = 1 << Z_INDEX;
const u8 N_MASK = 1 << N_INDEX;
const u8 H_MASK = 1 << H_INDEX;
const u8 C_MASK = 1 << C_INDEX;

u16 make_u16(u8 hi, u8 lo) {
    return (hi << 8) | lo;
}

i8 parse_i8(CPU *cpu) {
    i8 ret = (i8) cpu->memory[cpu->pc];
    cpu->pc += 1;
    return ret;
}

u8 parse_u8(CPU *cpu) {
    u16 ret = cpu->memory[cpu->pc];
    cpu->pc += 1;
    return ret;
}

u16 parse_u16(CPU *cpu) {
    u16 ret = make_u16(cpu->memory[cpu->pc + 1], cpu->memory[cpu->pc]);
    cpu->pc += 2;
    return ret;
}

u8 hi(u16 val) {
    return val >> 8;
}

u8 lo(u16 val) {
    return val & 0xFF;
}

void set_bc(CPU *cpu, u16 val) {
    cpu->b = hi(val);
    cpu->c = lo(val);
}

void set_de(CPU *cpu, u16 val) {
    cpu->d = hi(val);
    cpu->e = lo(val);
}

void set_hl(CPU *cpu, u16 val) {
    cpu->h = hi(val);
    cpu->l = lo(val);
}

void set_af(CPU *cpu, u16 val) {
    cpu->a = hi(val);
    cpu->f = lo(val) & 0xF0;
}

u16 af(CPU *cpu) {
    return make_u16(cpu->a, cpu->f);
}

u16 hl(CPU *cpu) {
    return make_u16(cpu->h, cpu->l);
}

u16 bc(CPU *cpu) {
    return make_u16(cpu->b, cpu->c);
}

u16 de(CPU *cpu) {
    return make_u16(cpu->d, cpu->e);
}

void set_dereference_bc(CPU *cpu, u8 val) {
    cpu->memory[bc(cpu)] = val;
}

void set_dereference_de(CPU *cpu, u8 val) {
    cpu->memory[de(cpu)] = val;
}

void set_dereference_hl(CPU *cpu, u8 val) {
    cpu->memory[hl(cpu)] = val;
}

u8 *reference_hl(CPU *cpu) {
    return &cpu->memory[hl(cpu)];
}

u8 dereference_bc(CPU *cpu) {
    return cpu->memory[bc(cpu)];
}

u8 dereference_de(CPU *cpu) {
    return cpu->memory[de(cpu)];
}

u8 dereference_hl(CPU *cpu) {
    return cpu->memory[hl(cpu)];
}

bool z(CPU *cpu) {
    assert((cpu->f & 0xF) == 0);
    return (cpu->f & Z_MASK) >> Z_INDEX;
}

bool n(CPU *cpu) {
    assert((cpu->f & 0xF) == 0);
    return (cpu->f & N_MASK) >> N_INDEX;
}

bool h(CPU *cpu) {
    assert((cpu->f & 0xF) == 0);
    return (cpu->f & H_MASK) >> H_INDEX;
}

bool c(CPU *cpu) {
    assert((cpu->f & 0xF) == 0);
    return (cpu->f & C_MASK) >> C_INDEX;
}

void set_z(CPU *cpu, bool val) {
    cpu->f = (cpu->f & ~Z_MASK) | (val << Z_INDEX);
    assert((cpu->f & 0xF) == 0);
}

void set_n(CPU *cpu, bool val) {
    cpu->f = (cpu->f & ~N_MASK) | (val << N_INDEX);
    assert((cpu->f & 0xF) == 0);
}

void set_h(CPU *cpu, bool val) {
    cpu->f = (cpu->f & ~H_MASK) | (val << H_INDEX);
    assert((cpu->f & 0xF) == 0);
}

void set_c(CPU *cpu, bool val) {
    cpu->f = (cpu->f & ~C_MASK) | (val << C_INDEX);
    assert((cpu->f & 0xF) == 0);
}

void xor(CPU *cpu, u8 val) {
    cpu->a ^= val;
    if (cpu->a == 0) {
        set_z(cpu, 1);
    } else {
        set_z(cpu, 0);
    }
}

void bit(CPU *cpu, int n, u8 val) {
    u8 res = cpu->h & (1 << n);
    set_z(cpu, res == 0);
    set_n(cpu, 0);
    set_h(cpu, 1);
}

void inc(CPU *cpu, u8 *loc) {
    u8 half_carry = ((*loc & 0xf) + 1) & 0x10;
    *loc += 1;
    set_z(cpu, *loc == 0);
    set_n(cpu, 0);
    set_h(cpu, half_carry != 0);
}

void dec(CPU *cpu, u8 *loc) {
    u8 half_carry = ((*loc & 0xf) - 1) & 0x10;
    *loc -= 1;
    set_z(cpu, *loc == 0);
    set_n(cpu, 1);
    set_h(cpu, half_carry != 0);
}

void push(CPU *cpu, u16 val) {
    cpu->memory[cpu->sp] = lo(val);
    cpu->memory[cpu->sp + 1] = hi(val);
    cpu->sp -= 2;
}

u16 pop(CPU *cpu) {
    cpu->sp += 2;
    u16 ret = make_u16(cpu->memory[cpu->sp + 1], cpu->memory[cpu->sp]);
    return ret;
}

void rla(CPU *cpu) {
    u8 carry = cpu->a & 0x80;
    cpu->a <<= 1;
    cpu->a |= c(cpu);
    set_z(cpu, 0);
    set_n(cpu, 0);
    set_h(cpu, 0);
    set_c(cpu, carry != 0);
}

void rl(CPU *cpu, u8 *loc) {
    u8 carry = *loc & 0x80;
    *loc <<= 1;
    *loc |= c(cpu);
    set_z(cpu, *loc == 0);
    set_n(cpu, 0);
    set_h(cpu, 0);
    set_c(cpu, carry != 0);
}

void dump_regs(CPU *cpu) {
    printf("PC = %04x\n", cpu->pc);
    printf("SP = %04x\n", cpu->sp);
    printf("A = %02x\n", cpu->a);
    printf("B = %02x\n", cpu->b);
    printf("C = %02x\n", cpu->c);
    printf("D = %02x\n", cpu->d);
    printf("E = %02x\n", cpu->e);
    printf("H = %02x\n", cpu->h);
    printf("L = %02x\n", cpu->l);
    printf("F = %02x\n", cpu->f);
    printf("[");
    printf(z(cpu) ? "Z" : "-");
    printf(n(cpu) ? "N" : "-");
    printf(h(cpu) ? "H" : "-");
    printf(c(cpu) ? "C" : "-");
    printf("]\n");
}

void cb_prefix(CPU *cpu) {
    u8 byte = cpu->memory[cpu->pc];
    printf("  %02x\n", byte);
    cpu->pc += 1;
    switch (byte) {
        case 0x10: {
            rl(cpu, &cpu->b);
            break;
        }
        case 0x11: {
            rl(cpu, &cpu->c);
            break;
        }
        case 0x12: {
            rl(cpu, &cpu->d);
            break;
        }
        case 0x13: {
            rl(cpu, &cpu->e);
            break;
        }
        case 0x14: {
            rl(cpu, &cpu->h);
            break;
        }
        case 0x15: {
            rl(cpu, &cpu->l);
            break;
        }
        case 0x16: {
            rl(cpu, reference_hl(cpu));
            break;
        }
        case 0x17: {
            rl(cpu, &cpu->a);
            break;
        }
        case 0x78: {
            bit(cpu, 7, cpu->b);
            break;
        }
        case 0x79: {
            bit(cpu, 7, cpu->c);
            break;
        }
        case 0x7a: {
            bit(cpu, 7, cpu->d);
            break;
        }
        case 0x7b: {
            bit(cpu, 7, cpu->e);
            break;
        }
        case 0x7c: {
            bit(cpu, 7, cpu->h);
            break;
        }
        case 0x7d: {
            bit(cpu, 7, cpu->l);
            break;
        }
        case 0x7e: {
            bit(cpu, 7, dereference_hl(cpu));
            break;
        }
        case 0x7f: {
            bit(cpu, 7, cpu->a);
            break;
        }
        default: {
            printf("cb prefix not yet implemented: 0x%02x\n", byte);
            exit(1);
        }
    }
}

void breakpoint() { }

CPU cpu;

int main() {
    FILE *f = fopen("tetris.gb", "r");
    assert(fread(rom, 1, rom_len, f) == rom_len);
    assert(fclose(f) == 0);

    bool stopped = false;

    init_cpu(&cpu);
    while (true) {
        u8 byte = cpu.memory[cpu.pc];
        dump_regs(&cpu);
        /* if (cpu.pc == 0x9c) stopped = true; */
        if (stopped) {
            char *prompt = readline("> ");
            if (!prompt) exit(0);
            if (strcmp(prompt, "c") == 0)
                stopped = false;
        }
        printf("Running %02x [%04x]\n", byte, cpu.pc);
        cpu.pc += 1;
        switch (byte) {
            case 0x01: {
                u16 arg = parse_u16(&cpu);
                set_bc(&cpu, arg);
                break;
            }
            case 0x02: {
                set_dereference_bc(&cpu, cpu.a);
                break;
            }
            case 0x03: {
                set_bc(&cpu, bc(&cpu) + 1);
                break;
            }
            case 0x04: {
                inc(&cpu, &cpu.b);
                break;
            }
            case 0x05: {
                dec(&cpu, &cpu.b);
                break;
            }
            case 0x06: {
                u8 arg = parse_u8(&cpu);
                cpu.b = arg;
                break;
            }
            case 0x0a: {
                cpu.a = dereference_bc(&cpu);
                break;
            }
            case 0x0b: {
                set_bc(&cpu, bc(&cpu) - 1);
                break;
            }
            case 0x0c: {
                inc(&cpu, &cpu.c);
                break;
            }
            case 0x0d: {
                dec(&cpu, &cpu.c);
                break;
            }
            case 0x0e: {
                u8 arg = parse_u8(&cpu);
                cpu.c = arg;
                break;
            }
            case 0x11: {
                u16 arg = parse_u16(&cpu);
                set_de(&cpu, arg);
                break;
            }
            case 0x12: {
                set_dereference_de(&cpu, cpu.a);
                break;
            }
            case 0x13: {
                set_de(&cpu, de(&cpu) + 1);
                break;
            }
            case 0x14: {
                inc(&cpu, &cpu.d);
                break;
            }
            case 0x15: {
                dec(&cpu, &cpu.d);
                break;
            }
            case 0x16: {
                u8 arg = parse_u8(&cpu);
                cpu.d = arg;
                break;
            }
            case 0x17: {
                rla(&cpu);
                break;
            }
            case 0x1a: {
                cpu.a = dereference_de(&cpu);
                break;
            }
            case 0x1b: {
                set_de(&cpu, de(&cpu) - 1);
                break;
            }
            case 0x1c: {
                inc(&cpu, &cpu.e);
                break;
            }
            case 0x1d: {
                dec(&cpu, &cpu.e);
                break;
            }
            case 0x1e: {
                u8 arg = parse_u8(&cpu);
                cpu.e = arg;
                break;
            }
            case 0x20: {
                i8 arg = parse_i8(&cpu);
                if (!z(&cpu)) {
                    cpu.pc += arg;
                }
                break;
            }
            case 0x21: {
                u16 arg = parse_u16(&cpu);
                set_hl(&cpu, arg);
                break;
            }
            case 0x22: {
                set_dereference_hl(&cpu, cpu.a);
                set_hl(&cpu, hl(&cpu) + 1);
                break;
            }
            case 0x23: {
                set_hl(&cpu, hl(&cpu) + 1);
                break;
            }
            case 0x24: {
                inc(&cpu, &cpu.h);
                break;
            }
            case 0x25: {
                dec(&cpu, &cpu.h);
                break;
            }
            case 0x26: {
                u8 arg = parse_u8(&cpu);
                cpu.h = arg;
                break;
            }
            case 0x2a: {
                cpu.a = dereference_hl(&cpu);
                set_hl(&cpu, hl(&cpu) + 1);
                break;
            }
            case 0x2b: {
                set_hl(&cpu, hl(&cpu) - 1);
                break;
            }
            case 0x2c: {
                inc(&cpu, &cpu.l);
                break;
            }
            case 0x2d: {
                dec(&cpu, &cpu.l);
                break;
            }
            case 0x2e: {
                u8 arg = parse_u8(&cpu);
                cpu.l = arg;
                break;
            }
            case 0x31: {
                u16 arg = parse_u16(&cpu);
                cpu.sp = arg;
                break;
            }
            case 0x32: {
                set_hl(&cpu, hl(&cpu) - 1);
                set_dereference_hl(&cpu, cpu.a);
                break;
            }
            case 0x33: {
                cpu.sp += 1;
                break;
            }
            case 0x34: {
                inc(&cpu, reference_hl(&cpu));
                break;
            }
            case 0x35: {
                dec(&cpu, reference_hl(&cpu));
                break;
            }
            case 0x36: {
                u8 arg = parse_u8(&cpu);
                set_dereference_hl(&cpu, arg);
                break;
            }
            case 0x3a: {
                set_hl(&cpu, hl(&cpu) - 1);
                cpu.a = dereference_hl(&cpu);
                break;
            }
            case 0x3b: {
                cpu.sp -= 1;
                break;
            }
            case 0x3c: {
                inc(&cpu, &cpu.a);
                break;
            }
            case 0x3d: {
                dec(&cpu, &cpu.a);
                break;
            }
            case 0x3e: {
                u8 arg = parse_u8(&cpu);
                cpu.a = arg;
                break;
            }
            case 0x40: {
                cpu.b = cpu.b;
                break;
            }
            case 0x41: {
                cpu.b = cpu.c;
                break;
            }
            case 0x42: {
                cpu.b = cpu.d;
                break;
            }
            case 0x43: {
                cpu.b = cpu.e;
                break;
            }
            case 0x44: {
                cpu.b = cpu.h;
                break;
            }
            case 0x45: {
                cpu.b = cpu.l;
                break;
            }
            case 0x46: {
                cpu.b = dereference_hl(&cpu);
                break;
            }
            case 0x47: {
                cpu.b = cpu.a;
                break;
            }
            case 0x48: {
                cpu.c = cpu.b;
                break;
            }
            case 0x49: {
                cpu.c = cpu.c;
                break;
            }
            case 0x4a: {
                cpu.c = cpu.d;
                break;
            }
            case 0x4b: {
                cpu.c = cpu.e;
                break;
            }
            case 0x4c: {
                cpu.c = cpu.h;
                break;
            }
            case 0x4d: {
                cpu.c = cpu.l;
                break;
            }
            case 0x4e: {
                cpu.c = dereference_hl(&cpu);
                break;
            }
            case 0x4f: {
                cpu.c = cpu.a;
                break;
            }
            case 0x50: {
                cpu.d = cpu.b;
                break;
            }
            case 0x51: {
                cpu.d = cpu.c;
                break;
            }
            case 0x52: {
                cpu.d = cpu.d;
                break;
            }
            case 0x53: {
                cpu.d = cpu.e;
                break;
            }
            case 0x54: {
                cpu.d = cpu.h;
                break;
            }
            case 0x55: {
                cpu.d = cpu.l;
                break;
            }
            case 0x56: {
                cpu.d = dereference_hl(&cpu);
                break;
            }
            case 0x57: {
                cpu.d = cpu.a;
                break;
            }
            case 0x58: {
                cpu.e = cpu.b;
                break;
            }
            case 0x59: {
                cpu.e = cpu.c;
                break;
            }
            case 0x5a: {
                cpu.e = cpu.d;
                break;
            }
            case 0x5b: {
                cpu.e = cpu.e;
                break;
            }
            case 0x5c: {
                cpu.e = cpu.h;
                break;
            }
            case 0x5d: {
                cpu.e = cpu.l;
                break;
            }
            case 0x5e: {
                cpu.e = dereference_hl(&cpu);
                break;
            }
            case 0x5f: {
                cpu.e = cpu.a;
                break;
            }
            case 0x60: {
                cpu.h = cpu.b;
                break;
            }
            case 0x61: {
                cpu.h = cpu.c;
                break;
            }
            case 0x62: {
                cpu.h = cpu.d;
                break;
            }
            case 0x63: {
                cpu.h = cpu.e;
                break;
            }
            case 0x64: {
                cpu.h = cpu.h;
                break;
            }
            case 0x65: {
                cpu.h = cpu.l;
                break;
            }
            case 0x66: {
                cpu.h = dereference_hl(&cpu);
                break;
            }
            case 0x67: {
                cpu.h = cpu.a;
                break;
            }
            case 0x68: {
                cpu.l = cpu.b;
                break;
            }
            case 0x69: {
                cpu.l = cpu.c;
                break;
            }
            case 0x6a: {
                cpu.l = cpu.d;
                break;
            }
            case 0x6b: {
                cpu.l = cpu.e;
                break;
            }
            case 0x6c: {
                cpu.l = cpu.h;
                break;
            }
            case 0x6d: {
                cpu.l = cpu.l;
                break;
            }
            case 0x6e: {
                cpu.l = dereference_hl(&cpu);
                break;
            }
            case 0x6f: {
                cpu.l = cpu.a;
                break;
            }
            case 0x70: {
                set_dereference_hl(&cpu, cpu.b);
            }
            case 0x71: {
                set_dereference_hl(&cpu, cpu.c);
            }
            case 0x72: {
                set_dereference_hl(&cpu, cpu.d);
            }
            case 0x73: {
                set_dereference_hl(&cpu, cpu.e);
            }
            case 0x74: {
                set_dereference_hl(&cpu, cpu.h);
            }
            case 0x75: {
                set_dereference_hl(&cpu, cpu.l);
            }
            case 0x77: {
                set_dereference_hl(&cpu, cpu.a);
            }
            case 0xa8: {
                xor(&cpu, cpu.b);
                break;
            }
            case 0xa9: {
                xor(&cpu, cpu.c);
                break;
            }
            case 0xaa: {
                xor(&cpu, cpu.d);
                break;
            }
            case 0xab: {
                xor(&cpu, cpu.e);
                break;
            }
            case 0xac: {
                xor(&cpu, cpu.h);
                break;
            }
            case 0xad: {
                xor(&cpu, cpu.l);
                break;
            }
            case 0xae: {
                xor(&cpu, dereference_hl(&cpu));
                break;
            }
            case 0xaf: {
                xor(&cpu, cpu.a);
                break;
            }
            case 0xc5: {
                push(&cpu, bc(&cpu));
                break;
            }
            case 0xc1: {
                set_bc(&cpu, pop(&cpu));
                break;
            }
            case 0xcb: {
                cb_prefix(&cpu);
                break;
            }
            case 0xcd: {
                u16 arg = parse_u16(&cpu);
                push(&cpu, cpu.pc);
                cpu.pc = arg;
                break;
            }
            case 0xd1: {
                set_de(&cpu, pop(&cpu));
                break;
            }
            case 0xd5: {
                push(&cpu, de(&cpu));
                break;
            }
            case 0xe0: {
                u8 arg = parse_u8(&cpu);
                cpu.memory[0xff00 + arg] = cpu.a;
                break;
            }
            case 0xe1: {
                set_hl(&cpu, pop(&cpu));
                break;
            }
            case 0xe2: {
                cpu.memory[0xff00 + cpu.c] = cpu.a;
                break;
            }
            case 0xe5: {
                push(&cpu, hl(&cpu));
                break;
            }
            case 0xf0: {
                u8 arg = parse_u8(&cpu);
                cpu.a = cpu.memory[0xff00 + arg];
                break;
            }
            case 0xf1: {
                set_af(&cpu, pop(&cpu));
                break;
            }
            case 0xf2: {
                cpu.a = cpu.memory[0xff00 + cpu.c];
                break;
            }
            case 0xf5: {
                push(&cpu, af(&cpu));
                break;
            }
            default: {
                printf("Not yet implemented: 0x%02x\n", byte);
                exit(1);
            }
        }
    }
}
