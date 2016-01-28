# Sets up GDB for remote debugging
target extended-remote | openocd -c "log_output openocd.log; gdb_port pipe" -f sam7s_gdb.cfg

file ../main.elf

# Matches the output from openocd.  Must be set manually.
set remote hardware-breakpoint-limit 2
set remote hardware-watchpoint-limit 2

monitor soft_reset_halt

echo \n
echo === ASL MK1 Setup Done ===\n
echo Use "mon soft_reset_halt" to reset to initial state.\n
echo NOTE: Loading of image from GDB not currently supported.\n
echo NOTE: Openocd and Sam7s don't play well.  Expect issues.\n
echo       You need to power-cycle MK1 after every debug attempt.\n
echo       Otherwise it simply will not work.  Sorry :(.\n
echo       Type "continue" to begin execution.\n
echo \n\n
echo *** Ready Player 1 ***
echo \n\n
