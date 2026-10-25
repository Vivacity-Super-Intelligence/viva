# Viva roadmap

v0.1 establishes a portable firmware core, two RTOS adapters, serial diagnostics,
schema-backed profiles, build automation and physical acceptance evidence.

Next engineering slices, in priority order:

1. Strengthen validation: watchdog starvation/reset-cause tests, power-cycle and
   brownout tests, longer endurance, hardware-in-loop CI with explicit device
   ownership, fuzzing and scheduling stress.
2. Define authenticated host communication and persistent configuration migration,
   integrity checks, reconnect semantics, bounded backpressure and update rollback.
3. Add one onboard IO driver at a time behind HAL with explicit pin maps and
   measured target tests. External actuation requires a separate safety design.
4. Extract a host SDK from the reference protocol client after the interface is
   stable; add device discovery and diagnostics tooling.
5. Build host Viva Runtime and simulation adapters around evidenced firmware
   contracts; only then add Studio workflows and deployment packaging.
6. Evaluate perception, state estimation, planning and world-model interfaces as
   separate Viva Intelligence work. Foundation provides no implementation claims
   for these future capabilities.

Each slice needs a concrete user journey, failure model, resource budget and
observable acceptance criteria before implementation. Release-specific remaining
work is in the physical validation and final engineering reports.
