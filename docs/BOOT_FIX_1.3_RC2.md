# Wanderer 1.3 RC2 — hardware boot stack fix

RC1 is withdrawn: it crashes on the user's physical FM-1 during startup. Use the separately named RC2 candidate, not RC1.

The crash photo reads vector 00000001, PC 020062AC, return address 02008BC0. In the exact RC1 disassembly, project_read_slot reserves 26,488 bytes of stack before that faulting instruction; the return address is the startup loop scanning saved-project slots. This exceeds the hardware stack region. The emulator did not enforce that hardware guard.

RC2 moves migration/validation workspaces to main-thread-only static RAM and uses pointer-based migration results on the production load path. Explicit no-inline boundaries keep the target compiler from combining large struct-return temporaries. The scanner's compiled frame is now 20 bytes. The conservative audited main/scanner/legacy-migration/phrase chain is 2,916 bytes. Hardware stack guards remain enabled, and the memory map, loader and rescue foundation are unchanged.

45 fresh host/compiler/protocol checks passed with RC2 selected, including legacy import, saving/loading and sound/Print tests. A new actual-target disassembly regression rejects the oversized RC1 scanner frame and bounds the corrected paths. Link limits: application 370,632 / 581,564 bytes; ordinary RAM 57,136 / 98,304; pool 315,392 / 344,064.

Identity remains FM-1_929. Package size 612,196 bytes. SHA-256: a7eb60c3499190e5f5e431a5a22747906526db5e001c306b09112f2fbad2a81d.

This fixes the cause identified by the hardware photo; physical confirmation of RC2 boot is still required. From the USB RESCUE screen, connect USB and use the installer with the RC2 file. Do not clear project storage as a troubleshooting step. The inherited rescue reports a recovery identity, so use an installer supporting this custom firmware's recovery mode.
