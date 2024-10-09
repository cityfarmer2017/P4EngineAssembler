# P4EngineAssembler
An assembler tool for a P4 architecture compantible Network Flow Processing engine.

How to make the tool (-D macros are not necessary exactly same sequence):

1. to make P4eAsm with both sub modules of table generating & preprocessing, but without DEBUG info output.

    **make**

2. to make P4eAsm with both sub modules of table generating & preprocessing, and with DEBUG info output.

    **make DBG_FLAG=-DDEBUG**

3. to make P4eAsm with only sub modules of preprocessing, but without DEBUG info output.

    **make TBL_FLAG=-DNO_TBL_PROC**

4. to make P4eAsm with only sub modules of preprocessing, and with DEBUG info output.

    **make DBG_FLAG=-DDEBUG TBL_FLAG=-DNO_TBL_PROC**

5. to make P4eAsm with only sub modules of table generating, but without DEBUG info output.

    **make PRE_FLAG=-DNO_PRE_PROC**

6. to make P4eAsm with only sub modules of table generating, and with DEBUG info output.

    **make DBG_FLAG=-DDEBUG PRE_FLAG=-DNO_PRE_PROC**

7. to clean bin and obj files

    **make clean**

8. to clean everthing

    **make dist-clean**

To make with different -D macro, a 'make clean' or 'make dist-clean' must be run in between.

Or you can run ***build.sh*** to build all release versions of P4eAsm (non-DEBUG).