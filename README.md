# P4EngineAssembler
An assembler tool for a P4 architecture compantible Network Flow Processing engine.

How to make the tool:

    1. make
    2. make DBG_FLAG=-DDEBUG
    3. make SUB_FLAG=-DWITHOUT_SUB_MODULES
    4. make DBG_FLAG=-DDEBUG SUB_FLAG=-DWITHOUT_SUB_MODULES
    5. make clean
    6. make dist-clean

To make with different -D macro, a 'make clean' or 'make dist-clean' must be run in between.