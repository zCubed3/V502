#include <V502/components/mos6502.hpp>
#include <V502/components/memory.hpp>

int main() {
    V502::MOS6502* cpu = new V502::MOS6502();

    V502::Memory* sys_memory = new V502::Memory(255);
    cpu->system_memory = sys_memory;

    V502::Memory* prog_memory = new V502::Memory(255);
    cpu->program_memory = prog_memory;

    // Debug program
    // TODO: Remove me and add binary loading

    // ADC #$01
    (*prog_memory)[0] = 0x69;
    (*prog_memory)[1] = 0x01;

    // STA $00)
    (*prog_memory)[2] = 0x85;
    (*prog_memory)[3] = 0x00;

    //JMP $0000
    (*prog_memory)[4] = 0x4C;
    (*prog_memory)[5] = 0x00;
    (*prog_memory)[6] = 0x00;

    while (1) {
        cpu->Cycle();
        printf("%x\n", sys_memory->at(0));
    }

    return 0;
}