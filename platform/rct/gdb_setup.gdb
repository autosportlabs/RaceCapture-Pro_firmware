# Sets up GDB for remote debugging
target remote | openocd -c "log_output openocd.log; gdb_port pipe" \
       -f openocd_debug.cfg
file main.elf

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

echo \n
echo === ASL RCT Setup Done ===\n
echo Use "mon reset init" to reset to initial state.\n
echo Use "load main.elf" to load up new firmware image.\n

echo \n\n
echo *** Ready Player 1 ***
echo \n\n
