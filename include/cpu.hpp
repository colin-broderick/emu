#ifndef CPU_H
#define CPU_H

#include "semaphore.h"

class Memory;

class CPU
{
    const static char INSTR_LDA = 0x01;
    const static char INSTR_LDR1 = 0x03;
    const static char INSTR_LDR2 = 0x04;
    const static char INSTR_MOV = 0x02;
    const static char INSTR_ADD_R1_R2 = 0x05;

    private:


    public:
        char SP;
        char IP;
        char A;
        char R1;
        char R2;
        char R3;
        
        CPU();
        void run(Memory& memory, Semaphore& sem);
        char get_instruction(Memory& memory);
};

std::ostream& operator<<(std::ostream& stream, const CPU& cpu);

#endif
