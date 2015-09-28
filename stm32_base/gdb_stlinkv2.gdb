# Sets up GDB for remote debugging
target extended-remote | openocd -f openocd_stlinkv2.cfg -c "gdb_port pipe; log_output openocd.log"
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
echo === ASL MK2 Setup Done ===\n
echo Use "mon reset init" to reset to initial state.\n
echo Use "load main.ihex" to load up new firmware image.\n

echo \n\n
echo *** Ready Player 1 ***
echo \n\n
