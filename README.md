# P4EngineAssembler
An assembler tool for a P4 architecture compantible Network Flow Processing engine.

How to make the tool:

    1. make
    2. make DBG_FLAG=-DDEBUG
    3. make SUB_FLAG=-DWITH_SUB_MODULES
    4. make DBG_FLAG=-DDEBUG SUB_FLAG=-DWITH_SUB_MODULES
    5. make clean
    6. make dist-clean

Two makes with different -D macros must be itercepted by a 'make clean' or 'make dist-clean'.