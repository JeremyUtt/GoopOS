// https://stackoverflow.com/questions/37618111/keyboard-irq-within-an-x86-kernel
#include <stdint.h>

#include <libIDT.hpp>
#include <libIO.hpp>
#include <printf.hpp>
#include <utils.hpp>
enum ICW1 {
    ICW1_ICW4      = 0x01,   // Indicates that ICW4 will be present
    ICW1_SINGLE    = 0x02,   // Single (cascade) mode
    ICW1_INTERVAL4 = 0x04,   // Call address interval 4 (8)
    ICW1_LEVEL     = 0x08,   // Level triggered (edge) mode
    ICW1_INIT      = 0x10    // Initialization - required!
};

enum ICW4 {
    ICW4_8086      = 0x01,   // 8086/88 (MCS-80/85) mode
    ICW4_AUTO      = 0x02,   // Auto (normal) EOI
    ICW4_BUF_SLAVE = 0x08,   // Buffered mode/slave
    ICW4_BUF_MASTER= 0x0C,   // Buffered mode/master
    ICW4_SFNM      = 0x10    // Special fully nested (not)
};

IdtEntry idtTable[IDT_SIZE];
IdtPointer idtPtr;

void loadIdtEntry(int32_t isrNumber, uint32_t baseAddress, uint16_t gdtSegment, uint8_t flags) {
    idtTable[isrNumber].offsetLower = baseAddress & 0xFFFF;
    idtTable[isrNumber].offsetHigher = (baseAddress >> 16) & 0xFFFF;
    idtTable[isrNumber].GdtSegment = gdtSegment;
    idtTable[isrNumber].flags = flags;
    idtTable[isrNumber].zero = 0;
}

static void initIdtPtr() {
    idtPtr.limit = (sizeof(IdtEntry) * IDT_SIZE) - 1;
    idtPtr.base = idtTable;
}

static void initPic() {
    // TODO: mabye add io_wait() between all calls

    /* ICW1 - begin initialization */
    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);

    /* ICW2 - remap offset address of idt_table */
    /*
     * In x86 protected mode, we have to remap the PICs beyond 0x20 because
     * Intel have designated the first 32 interrupts as "reserved" for cpu
     * exceptions
     */
    outb(PIC1_DATA, 0x20);
    outb(PIC2_DATA, 0x28);

    /* ICW3 - setup cascading */
    // TODO: this differs from osdev wiki
    outb(PIC1_DATA, 0x00);
    outb(PIC2_DATA, 0x00);

    /* ICW4 - environment info */
    outb(PIC1_DATA, ICW4_8086);
    outb(PIC2_DATA, ICW4_8086);
    /* Initialization finished */

    /* mask interrupts */
    outb(PIC1_DATA, 0xff);
    outb(PIC2_DATA, 0xff);
}

void idtInit() {
    initPic();
    initIdtPtr();
    loadIdt(&idtPtr);
}

void printIdtEntry(uint8_t i) {
    fprintf(Serial, "IDT Entry: 0x%X\n", i);
    fprintf(Serial, "\tOffset: 0x%X\n", (idtTable[i].offsetHigher << 16) + idtTable[i].offsetLower);
    fprintf(Serial, "\tGdt Segment: %d\n", idtTable[i].GdtSegment);
    fprintf(Serial, "\tFlags: 0b%d\n", idtTable[i].flags);
}
