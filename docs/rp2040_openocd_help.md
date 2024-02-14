```
adapter
      adapter command group (command valid any time)
  adapter assert |deassert [srst|trst [assert|deassert srst|trst]]
        Controls SRST and TRST lines.
  adapter deassert |assert [srst|trst [deassert|assert srst|trst]]
        Controls SRST and TRST lines.
  adapter driver driver_name
        Select a debug adapter driver (configuration command)
  adapter list
        List all built-in debug adapter drivers (command valid any time)
  adapter name
        Returns the name of the currently selected adapter (driver)
        (command valid any time)
  adapter speed [khz]
        With an argument, change to the specified maximum jtag speed.  For
        JTAG, 0 KHz signifies adaptive clocking. With or without argument,
        display current setting. (command valid any time)
  adapter srst
        srst adapter command group (command valid any time)
    adapter srst delay [milliseconds]
          delay after deasserting SRST in ms (command valid any time)
    adapter srst pulse_width [milliseconds]
          SRST assertion pulse width in ms (command valid any time)
  adapter transports transport ... 
        Declare transports the adapter supports. (configuration command)
  adapter usb
        usb adapter command group (command valid any time)
    adapter usb location [<bus>-port[.port]...]
          display or set the USB bus location of the USB device
          (configuration command)
add_help_text command_name helptext_string
      Add new command help text; Command can be multiple tokens. (command
      valid any time)
add_script_search_dir <directory>
      dir to search for config files and scripts (command valid any time)
add_usage_text command_name usage_string
      Add new command usage text; command can be multiple tokens. (command
      valid any time)
arm
      ARM command group (command valid any time)
  arm core_state ['arm'|'thumb']
        display/change ARM core state
  arm disassemble address [count ['thumb']]
        disassemble instructions 
  arm mcr cpnum op1 CRn CRm op2 value
        write coprocessor register
  arm mrc cpnum op1 CRn CRm op2
        read coprocessor register
  arm reg
        display ARM core registers
  arm semihosting ['enable'|'disable']
        activate support for semihosting operations
  arm semihosting_cmdline arguments
        command line arguments to be passed to program
  arm semihosting_fileio ['enable'|'disable']
        activate support for semihosting fileio operations
  arm semihosting_resexit ['enable'|'disable']
        activate support for semihosting resumable exit
array2mem arrayname bitwidth address count
      convert a TCL array to memory locations and write the 8/16/32 bit
      values
bindto [name]
      Specify address by name on which to listen for incoming TCP/IP
      connections (configuration command)
bp [<address> [<asid>] <length> ['hw'|'hw_ctx']]
      list or set hardware or software breakpoint
capture command
      Capture progress output and return as tcl return value. If the
      progress output was empty, return tcl return value. (command valid
      any time)
cmsis-dap <cmd>
      perform CMSIS-DAP management (command valid any time)
  cmsis-dap cmd
        issue cmsis-dap command
  cmsis-dap info
        show cmsis-dap info
cmsis_dap_backend (auto | usb_bulk | hid)
      set the communication backend to use (USB bulk or HID).
      (configuration command)
cmsis_dap_serial serial_string
      set the serial number of the adapter (configuration command)
cmsis_dap_usb <cmd>
      USB bulk backend-specific commands (command valid any time)
  cmsis_dap_usb interface <interface_number>
        set the USB interface number to use (for USB bulk backend only)
        (configuration command)
cmsis_dap_vid_pid (vid pid)* 
      the vendor ID and product ID of the CMSIS-DAP device (configuration
      command)
command
      core command group (introspection) (command valid any time)
  command mode [command_name ...]
        Returns the command modes allowed by a command: 'any', 'config', or
        'exec'. If no command is specified, returns the current command
        mode. Returns 'unknown' if an unknown command is given. Command can
        be multiple tokens. (command valid any time)
cortex_m
      Cortex-M command group
  cortex_m maskisr ['auto'|'on'|'off'|'steponly']
        mask cortex_m interrupts
  cortex_m reset_config ['sysresetreq'|'vectreset']
        configure software reset handling (command valid any time)
  cortex_m smp [on|off]
        smp handling
  cortex_m smp_gdb
        display/fix current core played to gdb
  cortex_m smp_off
        Stop smp handling
  cortex_m smp_on
        Restart smp handling
  cortex_m vector_catch ['all'|'none'|('bus_err'|'chk_err'|...)*]
        configure hardware vectors to trigger debug entry
cti
      CTI commands (configuration command)
  cti create name '-chain-position' name [options ...]
        Creates a new CTI object (command valid any time)
  cti names
        Lists all registered CTI objects by name (command valid any time)
dap
      DAP commands (configuration command)
  dap create name '-chain-position' name
        Creates a new DAP instance (command valid any time)
  dap info [ap_num]
        display ROM table for MEM-AP of current target (default currently
        selected AP)
  dap init
        Initialize all registered DAP instances (command valid any time)
  dap names
        Lists all registered DAP instances by name (command valid any time)
debug_level number
      Sets the verbosity level of debugging output. 0 shows errors only; 1
      adds warnings; 2 (default) adds other info; 3 adds debugging; 4 adds
      extra verbose debugging. (command valid any time)
dump_image filename address size
echo [-n] string
      Logs a message at "user" priority. Output message to stdout. Option
      "-n" suppresses trailing newline (command valid any time)
exit
      exit telnet session
fast_load
      loads active fast load image to current target - mainly for profiling
      purposes
fast_load_image filename address ['bin'|'ihex'|'elf'|'s19'] [min_address [max_length]]
      Load image into server memory for later use by fast_load; primarily
      for profiling (command valid any time)
find <file>
      print full path to file according to OpenOCD search rules (command
      valid any time)
flash
      NOR flash command group (command valid any time)
  flash bank bank_id driver_name base_address size_bytes chip_width_bytes
            bus_width_bytes target [driver_options ...]
        Define a new bank with the given name, using the specified NOR
        flash driver. (configuration command)
  flash banks
        Display table with information about flash banks. (command valid
        any time)
  flash erase_address ['pad'] ['unlock'] address length
        Erase flash sectors starting at address and continuing for length
        bytes.  If 'pad' is specified, data outside that range may also be
        erased: the start address may be decreased, and length increased,
        so that all of the first and last sectors are erased. If 'unlock'
        is specified, then the flash is unprotected before erasing.
  flash erase_check bank_id
        Check erase state of all blocks in a flash bank.
  flash erase_sector bank_id first_sector_num (last_sector_num|'last')
        Erase a range of sectors in a flash bank.
  flash fillb address value n
        Fill n bytes with 8-bit value, starting at word address.  (No
        autoerase.)
  flash filld address value n
        Fill n double-words with 64-bit value, starting at word address. 
        (No autoerase.)
  flash fillh address value n
        Fill n halfwords with 16-bit value, starting at word address.  (No
        autoerase.)
  flash fillw address value n
        Fill n words with 32-bit value, starting at word address.  (No
        autoerase.)
  flash info bank_id ['sectors']
        Print information about a flash bank.
  flash init
        Initialize flash devices. (configuration command)
  flash list
        Returns a list of details about the flash banks. (command valid any
        time)
  flash mdb address [count]
        Display bytes from flash.
  flash mdh address [count]
        Display half-words from flash.
  flash mdw address [count]
        Display words from flash.
  flash padded_value bank_id value
        Set default flash padded value
  flash probe bank_id
        Identify a flash bank.
  flash protect bank_id first_block [last_block|'last'] ('on'|'off')
        Turn protection on or off for a range of protection blocks or
        sectors in a given flash bank. See 'flash info' output for a list
        of blocks.
  flash read_bank bank_id filename [offset [length]]
        Read binary data from flash bank to file. Allow optional offset
        from beginning of the bank (defaults to zero).
  flash verify_bank bank_id filename [offset]
        Compare the contents of a file with the contents of the flash bank.
        Allow optional offset from beginning of the bank (defaults to
        zero).
  flash verify_image filename [offset [file_type]]
        Verify an image against flash. Allow optional offset from beginning
        of bank (defaults to zero)
  flash write_bank bank_id filename [offset]
        Write binary data from file to flash bank. Allow optional offset
        from beginning of the bank (defaults to zero).
  flash write_image [erase] [unlock] filename [offset [file_type]]
        Write an image to flash.  Optionally first unprotect and/or erase
        the region to be used. Allow optional offset from beginning of bank
        (defaults to zero)
gdb_breakpoint_override ('hard'|'soft'|'disable')
      Display or specify type of breakpoint to be used by gdb 'break'
      commands. (command valid any time)
gdb_flash_program ('enable'|'disable')
      enable or disable flash program (configuration command)
gdb_memory_map ('enable'|'disable')
      enable or disable memory map (configuration command)
gdb_port [port_num]
      Normally gdb listens to a TCP/IP port. Each subsequent GDB server
      listens for the next port number after the base port number
      specified. No arguments reports GDB port. "pipe" means listen to
      stdin output to stdout, an integer is base port number, "disabled"
      disables port. Any other string is are interpreted as named pipe to
      listen to. Output pipe is the same name as input pipe, but with 'o'
      appended. (configuration command)
gdb_report_data_abort ('enable'|'disable')
      enable or disable reporting data aborts (configuration command)
gdb_report_register_access_error ('enable'|'disable')
      enable or disable reporting register access errors (configuration
      command)
gdb_save_tdesc
      Save the target description file
gdb_sync
      next stepi will return immediately allowing GDB to fetch register
      state without affecting target state (command valid any time)
gdb_target_description ('enable'|'disable')
      enable or disable target description (configuration command)
halt [milliseconds]
      request target to halt, then wait up to the specified number of
      milliseconds (default 5000) for it to complete
help [command_name]
      Show full command help; command can be multiple tokens. (command
      valid any time)
init
      Initializes configured targets and servers.  Changes command mode
      from CONFIG to EXEC.  Unless 'noinit' is called, this command is
      called automatically at the end of startup. (command valid any time)
itm
      itm command group (command valid any time)
  itm port <port> (0|1|on|off)
        Enable or disable ITM stimulus port (command valid any time)
  itm ports (0|1|on|off)
        Enable or disable all ITM stimulus ports (command valid any time)
jsp_port [port_num]
      Specify port on which to listen for incoming JSP telnet connections.
      (command valid any time)
load_image filename address ['bin'|'ihex'|'elf'|'s19'] [min_address] [max_length]
log_output [file_name | "default"]
      redirect logging to a file (default: stderr) (command valid any time)
mdb ['phys'] address [count]
      display memory bytes
mdd ['phys'] address [count]
      display memory double-words
mdh ['phys'] address [count]
      display memory half-words
mdw ['phys'] address [count]
      display memory words
measure_clk
      Runs a test to measure the JTAG clk. Useful with RCLK / RTCK.
      (command valid any time)
mem2array arrayname bitwidth address count
      read 8/16/32 bit memory and return as a TCL array for script
      processing
mmw address setbits clearbits
      Modify word in memory. new_val = (old_val & ~clearbits) | setbits;
      (command valid any time)
mrb address
      Returns value of byte in memory. (command valid any time)
mrh address
      Returns value of halfword in memory. (command valid any time)
mrw address
      Returns value of word in memory. (command valid any time)
ms
      Returns ever increasing milliseconds. Used to calculate differences
      in time. (command valid any time)
mwb ['phys'] address value [count]
      write memory byte
mwd ['phys'] address value [count]
      write memory double-word
mwh ['phys'] address value [count]
      write memory half-word
mww ['phys'] address value [count]
      write memory word
nand
      NAND flash command group (command valid any time)
  nand device bank_id driver target [driver_options ...]
        defines a new NAND bank (configuration command)
  nand drivers
        lists available NAND drivers (command valid any time)
  nand init
        initialize NAND devices (configuration command)
noinit
      Prevent 'init' from being called at startup. (configuration command)
ocd_find file
      find full path to file (command valid any time)
pld
      programmable logic device commands (command valid any time)
  pld device driver_name [driver_args ... ]
        configure a PLD device (configuration command)
  pld init
        initialize PLD devices (configuration command)
poll ['on'|'off']
      poll target state; or reconfigure background polling
poll_period
      set the servers polling period (command valid any time)
power_restore
      Overridable procedure run when power restore is detected. Runs 'reset
      init' by default. (command valid any time)
profile seconds filename [start end]
      profiling samples the CPU PC
program <filename> [address] [pre-verify] [verify] [reset] [exit]
      write an image to flash, address is only required for binary images.
      verify, reset, exit are optional (command valid any time)
ps  
      list all tasks 
rbp 'all' | address
      remove breakpoint
reg [(register_number|register_name) [(value|'force')]]
      display (reread from target with "force") or set a register; with no
      arguments, displays all registers and their values
reset [run|halt|init]
      Reset all targets into the specified mode. Default reset mode is run,
      if not given.
reset_config [none|trst_only|srst_only|trst_and_srst]
          [srst_pulls_trst|trst_pulls_srst|combined|separate]
          [srst_gates_jtag|srst_nogate] [trst_push_pull|trst_open_drain]
          [srst_push_pull|srst_open_drain]
          [connect_deassert_srst|connect_assert_srst]
      configure adapter reset behavior (command valid any time)
reset_nag ['enable'|'disable']
      Nag after each reset about options that could have been enabled to
      improve performance.  (command valid any time)
resume [address]
      resume target execution from current PC or address
rp2040.core0
      target command group (command valid any time)
  rp2040.core0 arm
        ARM command group (command valid any time)
    rp2040.core0 arm core_state ['arm'|'thumb']
          display/change ARM core state
    rp2040.core0 arm disassemble address [count ['thumb']]
          disassemble instructions 
    rp2040.core0 arm mcr cpnum op1 CRn CRm op2 value
          write coprocessor register
    rp2040.core0 arm mrc cpnum op1 CRn CRm op2
          read coprocessor register
    rp2040.core0 arm reg
          display ARM core registers
    rp2040.core0 arm semihosting ['enable'|'disable']
          activate support for semihosting operations
    rp2040.core0 arm semihosting_cmdline arguments
          command line arguments to be passed to program
    rp2040.core0 arm semihosting_fileio ['enable'|'disable']
          activate support for semihosting fileio operations
    rp2040.core0 arm semihosting_resexit ['enable'|'disable']
          activate support for semihosting resumable exit
  rp2040.core0 arp_examine ['allow-defer']
        used internally for reset processing
  rp2040.core0 arp_halt
        used internally for reset processing
  rp2040.core0 arp_halt_gdb
        used internally for reset processing to halt GDB
  rp2040.core0 arp_poll
        used internally for reset processing
  rp2040.core0 arp_reset
        used internally for reset processing
  rp2040.core0 arp_waitstate
        used internally for reset processing
  rp2040.core0 array2mem arrayname bitwidth address count
        Writes Tcl array of 8/16/32 bit numbers to target memory
  rp2040.core0 cget target_attribute
        returns the specified target attribute (command valid any time)
  rp2040.core0 configure [target_attribute ...]
        configure a new target for use (command valid any time)
  rp2040.core0 cortex_m
        Cortex-M command group
    rp2040.core0 cortex_m maskisr ['auto'|'on'|'off'|'steponly']
          mask cortex_m interrupts
    rp2040.core0 cortex_m reset_config ['sysresetreq'|'vectreset']
          configure software reset handling (command valid any time)
    rp2040.core0 cortex_m smp [on|off]
          smp handling
    rp2040.core0 cortex_m smp_gdb
          display/fix current core played to gdb
    rp2040.core0 cortex_m smp_off
          Stop smp handling
    rp2040.core0 cortex_m smp_on
          Restart smp handling
    rp2040.core0 cortex_m vector_catch ['all'|'none'|('bus_err'|'chk_err'|...)*]
          configure hardware vectors to trigger debug entry
  rp2040.core0 curstate
        displays the current state of this target
  rp2040.core0 eventlist
        displays a table of events defined for this target
  rp2040.core0 examine_deferred
        used internally for reset processing
  rp2040.core0 invoke-event event_name
        invoke handler for specified event
  rp2040.core0 itm
        itm command group (command valid any time)
    rp2040.core0 itm port <port> (0|1|on|off)
          Enable or disable ITM stimulus port (command valid any time)
    rp2040.core0 itm ports (0|1|on|off)
          Enable or disable all ITM stimulus ports (command valid any time)
  rp2040.core0 mdb address [count]
        Display target memory as 8-bit bytes
  rp2040.core0 mdd address [count]
        Display target memory as 64-bit words
  rp2040.core0 mdh address [count]
        Display target memory as 16-bit half-words
  rp2040.core0 mdw address [count]
        Display target memory as 32-bit words
  rp2040.core0 mem2array arrayname bitwidth address count
        Loads Tcl array of 8/16/32 bit numbers from target memory
  rp2040.core0 mwb address data [count]
        Write byte(s) to target memory
  rp2040.core0 mwd address data [count]
        Write 64-bit word(s) to target memory
  rp2040.core0 mwh address data [count]
        Write 16-bit half-word(s) to target memory
  rp2040.core0 mww address data [count]
        Write 32-bit word(s) to target memory
  rp2040.core0 rtt
        RTT target commands
    rp2040.core0 rtt channellist
          list available channels
    rp2040.core0 rtt channels
          list available channels
    rp2040.core0 rtt polling_interval [interval]
          show or set polling interval in ms
    rp2040.core0 rtt setup <address> <size> <ID>
          setup RTT (command valid any time)
    rp2040.core0 rtt start
          start RTT
    rp2040.core0 rtt stop
          stop RTT
  rp2040.core0 tpiu
        tpiu command group (command valid any time)
    rp2040.core0 tpiu config (disable | ((external | internal (<filename> | <:port> | -)) (sync <port
              width> | ((manchester | uart) <formatter enable>))
              <TRACECLKIN freq> [<trace freq>]))
          Configure TPIU features (command valid any time)
  rp2040.core0 was_examined
        used internally for reset processing
rp2040.core0.dap
      dap instance command group (command valid any time)
  rp2040.core0.dap apcsw [value [mask]]
        Set CSW default bits (command valid any time)
  rp2040.core0.dap apid [ap_num]
        return ID register from AP (default currently selected AP)
  rp2040.core0.dap apreg ap_num reg [value]
        read/write a register from AP (reg is byte address of a word
        register, like 0 4 8...)
  rp2040.core0.dap apsel [ap_num]
        Set the currently selected AP (default 0) and display the result
        (command valid any time)
  rp2040.core0.dap baseaddr [ap_num]
        return debug base address from MEM-AP (default currently selected
        AP)
  rp2040.core0.dap dpreg reg [value]
        read/write a register from DP (reg is byte address (bank << 4 |
        reg) of a word register, like 0 4 8...)
  rp2040.core0.dap info [ap_num]
        display ROM table for MEM-AP (default currently selected AP)
  rp2040.core0.dap memaccess [cycles]
        set/get number of extra tck for MEM-AP memory bus access [0-255]
  rp2040.core0.dap ti_be_32_quirks [enable]
        set/get quirks mode for TI TMS450/TMS570 processors (configuration
        command)
rp2040.core1
      target command group (command valid any time)
  rp2040.core1 arm
        ARM command group (command valid any time)
    rp2040.core1 arm core_state ['arm'|'thumb']
          display/change ARM core state
    rp2040.core1 arm disassemble address [count ['thumb']]
          disassemble instructions 
    rp2040.core1 arm mcr cpnum op1 CRn CRm op2 value
          write coprocessor register
    rp2040.core1 arm mrc cpnum op1 CRn CRm op2
          read coprocessor register
    rp2040.core1 arm reg
          display ARM core registers
    rp2040.core1 arm semihosting ['enable'|'disable']
          activate support for semihosting operations
    rp2040.core1 arm semihosting_cmdline arguments
          command line arguments to be passed to program
    rp2040.core1 arm semihosting_fileio ['enable'|'disable']
          activate support for semihosting fileio operations
    rp2040.core1 arm semihosting_resexit ['enable'|'disable']
          activate support for semihosting resumable exit
  rp2040.core1 arp_examine ['allow-defer']
        used internally for reset processing
  rp2040.core1 arp_halt
        used internally for reset processing
  rp2040.core1 arp_halt_gdb
        used internally for reset processing to halt GDB
  rp2040.core1 arp_poll
        used internally for reset processing
  rp2040.core1 arp_reset
        used internally for reset processing
  rp2040.core1 arp_waitstate
        used internally for reset processing
  rp2040.core1 array2mem arrayname bitwidth address count
        Writes Tcl array of 8/16/32 bit numbers to target memory
  rp2040.core1 cget target_attribute
        returns the specified target attribute (command valid any time)
  rp2040.core1 configure [target_attribute ...]
        configure a new target for use (command valid any time)
  rp2040.core1 cortex_m
        Cortex-M command group
    rp2040.core1 cortex_m maskisr ['auto'|'on'|'off'|'steponly']
          mask cortex_m interrupts
    rp2040.core1 cortex_m reset_config ['sysresetreq'|'vectreset']
          configure software reset handling (command valid any time)
    rp2040.core1 cortex_m smp [on|off]
          smp handling
    rp2040.core1 cortex_m smp_gdb
          display/fix current core played to gdb
    rp2040.core1 cortex_m smp_off
          Stop smp handling
    rp2040.core1 cortex_m smp_on
          Restart smp handling
    rp2040.core1 cortex_m vector_catch ['all'|'none'|('bus_err'|'chk_err'|...)*]
          configure hardware vectors to trigger debug entry
  rp2040.core1 curstate
        displays the current state of this target
  rp2040.core1 eventlist
        displays a table of events defined for this target
  rp2040.core1 examine_deferred
        used internally for reset processing
  rp2040.core1 invoke-event event_name
        invoke handler for specified event
  rp2040.core1 itm
        itm command group (command valid any time)
    rp2040.core1 itm port <port> (0|1|on|off)
          Enable or disable ITM stimulus port (command valid any time)
    rp2040.core1 itm ports (0|1|on|off)
          Enable or disable all ITM stimulus ports (command valid any time)
  rp2040.core1 mdb address [count]
        Display target memory as 8-bit bytes
  rp2040.core1 mdd address [count]
        Display target memory as 64-bit words
  rp2040.core1 mdh address [count]
        Display target memory as 16-bit half-words
  rp2040.core1 mdw address [count]
        Display target memory as 32-bit words
  rp2040.core1 mem2array arrayname bitwidth address count
        Loads Tcl array of 8/16/32 bit numbers from target memory
  rp2040.core1 mwb address data [count]
        Write byte(s) to target memory
  rp2040.core1 mwd address data [count]
        Write 64-bit word(s) to target memory
  rp2040.core1 mwh address data [count]
        Write 16-bit half-word(s) to target memory
  rp2040.core1 mww address data [count]
        Write 32-bit word(s) to target memory
  rp2040.core1 rtt
        RTT target commands
    rp2040.core1 rtt channellist
          list available channels
    rp2040.core1 rtt channels
          list available channels
    rp2040.core1 rtt polling_interval [interval]
          show or set polling interval in ms
    rp2040.core1 rtt setup <address> <size> <ID>
          setup RTT (command valid any time)
    rp2040.core1 rtt start
          start RTT
    rp2040.core1 rtt stop
          stop RTT
  rp2040.core1 tpiu
        tpiu command group (command valid any time)
    rp2040.core1 tpiu config (disable | ((external | internal (<filename> | <:port> | -)) (sync <port
              width> | ((manchester | uart) <formatter enable>))
              <TRACECLKIN freq> [<trace freq>]))
          Configure TPIU features (command valid any time)
  rp2040.core1 was_examined
        used internally for reset processing
rp2040.core1.dap
      dap instance command group (command valid any time)
  rp2040.core1.dap apcsw [value [mask]]
        Set CSW default bits (command valid any time)
  rp2040.core1.dap apid [ap_num]
        return ID register from AP (default currently selected AP)
  rp2040.core1.dap apreg ap_num reg [value]
        read/write a register from AP (reg is byte address of a word
        register, like 0 4 8...)
  rp2040.core1.dap apsel [ap_num]
        Set the currently selected AP (default 0) and display the result
        (command valid any time)
  rp2040.core1.dap baseaddr [ap_num]
        return debug base address from MEM-AP (default currently selected
        AP)
  rp2040.core1.dap dpreg reg [value]
        read/write a register from DP (reg is byte address (bank << 4 |
        reg) of a word register, like 0 4 8...)
  rp2040.core1.dap info [ap_num]
        display ROM table for MEM-AP (default currently selected AP)
  rp2040.core1.dap memaccess [cycles]
        set/get number of extra tck for MEM-AP memory bus access [0-255]
  rp2040.core1.dap ti_be_32_quirks [enable]
        set/get quirks mode for TI TMS450/TMS570 processors (configuration
        command)
rtt
      RTT (command valid any time)
  rtt channellist
        list available channels
  rtt channels
        list available channels
  rtt polling_interval [interval]
        show or set polling interval in ms
  rtt server
        RTT server (command valid any time)
    rtt server start <port> <channel>
          Start a RTT server (command valid any time)
    rtt server stop <port>
          Stop a RTT server (command valid any time)
  rtt setup <address> <size> <ID>
        setup RTT (command valid any time)
  rtt start
        start RTT
  rtt stop
        stop RTT
rwp address
      remove watchpoint
script <file>
      filename of OpenOCD script (tcl) to run (command valid any time)
shutdown
      shut the server down (command valid any time)
sleep milliseconds ['busy']
      Sleep for specified number of milliseconds.  "busy" will busy wait
      instead (avoid this). (command valid any time)
soft_reset_halt
      halt the target and do a soft reset
srst_deasserted
      Overridable procedure run when srst deassert is detected. Runs 'reset
      init' by default. (command valid any time)
step [address]
      step one instruction from current PC or address
swd
      SWD command group (command valid any time)
  swd newdap
        declare a new SWD DAP (configuration command)
target
      configure target (configuration command)
  target create name type '-chain-position' name [options ...]
        Creates and selects a new target (configuration command)
  target current
        Returns the currently selected target (command valid any time)
  target init
        initialize targets (configuration command)
  target names
        Returns the names of all targets as a list of strings (command
        valid any time)
  target smp targetname1 targetname2 ...
        gather several target in a smp list (command valid any time)
  target types
        Returns the available target types as a list of strings (command
        valid any time)
target_request
      target request command group (command valid any time)
  target_request debugmsgs ['enable'|'charmsg'|'disable']
        display and/or modify reception of debug messages from target
targets [target]
      change current default target (one parameter) or prints table of all
      targets (no parameters) (command valid any time)
tcl_notifications [on|off]
      Target Notification output
tcl_port [port_num]
      Specify port on which to listen for incoming Tcl syntax.  Read help
      on 'gdb_port'. (configuration command)
tcl_trace [on|off]
      Target trace output
telnet_port [port_num]
      Specify port on which to listen for incoming telnet connections. 
      Read help on 'gdb_port'. (configuration command)
test_image filename [offset [type]]
test_mem_access size
      Test the target's memory access functions
tpiu
      tpiu command group (command valid any time)
  tpiu config (disable | ((external | internal (<filename> | <:port> | -)) (sync <port
            width> | ((manchester | uart) <formatter enable>)) <TRACECLKIN
            freq> [<trace freq>]))
        Configure TPIU features (command valid any time)
trace
      trace command group
  trace history ['clear'|size]
        display trace history, clear history or set size
  trace point ['clear'|address]
        display trace points, clear list of trace points, or add new
        tracepoint at address
transport
      Transport command group (command valid any time)
  transport init
        Initialize this session's transport (command valid any time)
  transport list
        list all built-in transports (command valid any time)
  transport select [transport_name]
        Select this session's transport (command valid any time)
usage [command_name]
      Show basic command usage; command can be multiple tokens. (command
      valid any time)
verify_image filename [offset [type]]
verify_image_checksum filename [offset [type]]
version
      show program version (command valid any time)
virt2phys virtual_address
      translate a virtual address into a physical address (command valid
      any time)
wait_halt [milliseconds]
      wait up to the specified number of milliseconds (default 5000) for a
      previously requested halt
wp [address length [('r'|'w'|'a') value [mask]]]
      list (no params) or create watchpoints
```