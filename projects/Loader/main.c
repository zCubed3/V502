#include <V502/6502_vm.h>

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

int main() {
    v502_6502vm_createinfo_t createinfo;
    createinfo.hunk_size = 0xFFFF + 1;
    createinfo.feature_set = V502_FEATURESET_MOS6502;

    v502_6502vm_t* vm = v502_create_vm(&createinfo);

    // TODO: Make this better
    FILE* bin_file = fopen("./test.bin", "rb");

    assert(bin_file != NULL);

    fseek(bin_file, 0, SEEK_END);

    size_t len = ftell(bin_file);
    fseek(bin_file, 0, SEEK_SET);
    fread(vm->hunk, len, 1, bin_file);

    v502_reset_vm(vm);

    while (v502_cycle_vm(vm)) {
        printf("0x%x\n", vm->hunk[0x00]);
    }
}