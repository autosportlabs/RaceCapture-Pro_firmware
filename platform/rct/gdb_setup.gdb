# Sets up GDB for remote debugging
target remote | openocd -f board/stm32f3discovery.cfg -c "gdb_port pipe; log_output openocd.log"
mon reset halt

# Matches the output from openocd.  Must be set manually.
set remote hardware-breakpoint-limit 6
set remote hardware-watchpoint-limit 4

# Used to allow us to step without interrupt.
define hook-step
mon cortex_m maskisr on
end
define hookpost-step
mon cortex_m maskisr off
end

# Load up the firmware and go!
load
continue
