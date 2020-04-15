# BEGIN_LEGAL 
# BSD License 
# 
# Copyright (c)2014 Intel Corporation. All rights reserved.
#  
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
# 
# Redistributions of source code must retain the above copyright notice,
# this list of conditions and the following disclaimer.  Redistributions
# in binary form must reproduce the above copyright notice, this list of
# conditions and the following disclaimer in the documentation and/or
# other materials provided with the distribution.  Neither the name of
# the Intel Corporation nor the names of its contributors may be used to
# endorse or promote products derived from this software without
# specific prior written permission.
#  
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE INTEL OR
# ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# END_LEGAL 

import gdb
import re
import sys
import traceback
import pin

class SliceCommand(gdb.Command):
    """Create a program slice

    pin slice <instance> <thread> 
               { <variableName> | %<register> | <address> <length> } at
               { <sourceLocation> | *<startAddress> *<endAddress> }

          instance: DART log instance
          thread: Target thread id
          variableName: Name of the variable to generate slice for
          register: Name of register to generate slice for
          address: Address of the variable to generate slice for
          length: Length of variable
          sourceLocation: file:lineNumber or line number to generate slice
          startAddress: Starting code address to generate slice
          endAddress: Ending code address to generate slice"""

    def __init__(self):
        """Initialize the SliceCommand object"""
        super(SliceCommand,
              self).__init__(name="pin slice",
                             command_class = gdb.COMMAND_DATA,
                             prefix = False)

    def invoke(self, argList, from_tty):
        """Top level handler for the slice command"""

            # Don't repeat the command if the user presses enter. That just 
            # generates duplicate slice files
        super(SliceCommand, self).dont_repeat()
        args = gdb.string_to_argv(argList)
        if len(args) < 5:
            raise gdb.GdbError("Invalid slice command syntax")
        try:
            index = args.index("at")
        except IndexError:
            raise gdb.GdbError("Invalid slice command syntax")
        if len(args) == index + 2:
                # Make sure user didn't specify an instruction address
            if args[index + 1][0] == '*':
                raise gdb.GdbError("ERROR: Cannot specify only a start address")

            # User specified a source file/line
            (startAddress, endAddress) = pin.getLineAddressRange(args[index + 1])
        else:
            startAddress = args[index + 1][1:]
            endAddress = args[index + 2][1:]
        if index == 3:
            # User specified a variable name or register name
            if args[2].startswith("%"):
                variableAddress = args[2]
                variableLength = ""
            else:
                (variableAddress, variableLength) = pin.resolveScopedVariable(
                                  args[2], "*" + startAddress, "*" + endAddress)
                if variableAddress[0] == '[' or variableAddress[0] == '(':
                        # If the variable address starts with '[' or '(' then
                        # it is a base+displacement address which is not
                        # supported. Try to get its address using
                        # 'print &variable'. If the variable is not in scope
                        # that will fail with an error message.
                    (variableAddress, variableLength) = pin.resolveVariable(args[2])
        else:
            # User specified an address and length
            variableAddress = args[2]
            variableLength = args[3]
        try:
            label = "#" + args[2] + ":<" + args[index + 1] + ">"
            pin.execute("monitor slice %s %s %s %s %s %s %s" % (args[1],
                    startAddress, endAddress, args[0],
                    variableAddress, variableLength, label))
        except gdb.error:
            reason = sys.exc_info()
            raise gdb.GdbError("ERROR: Error generating slice: %s" % reason[1])

class PruneCommand(gdb.Command):
    """Prune a program slice, removing variable references

    pin prune <slice_id> { <variable> | %<register> | <address> <length> }

          slice_id: slice id number
          variable: Name of variable to prune from slice
          register: Name of register to prune from slice, %rax, %rbx, etc
          address: Address of the variable to prune from the slice
          length: Length of the variable to prune from the slice"""

    def __init__(self):
        """Initialize the PruneCommand object"""
        super(PruneCommand,
              self).__init__(name="pin prune",
                             command_class = gdb.COMMAND_DATA,
                             prefix = False)

    def invoke(self, argList, from_tty):
        """Top level handler for the prune command"""

            # Don't repeat the command if the user presses enter. There's
            # probably no harm done other than wasting a bit of time if the
            # command is repeated
        super(PruneCommand, self).dont_repeat()
        args = gdb.string_to_argv(argList)
        if len(args) == 2:
                # If command has 2 arguments then 2nd argument is variable name
                # or register.
            if str(args[1]).startswith("%"):
                try:
                    pin.execute("monitor prune %s %s %s" % (args[0], args[1]),
                            "#" + args[1])
                except gdb.error:
                    reason = sys.exc_info()
                    raise gdb.GdbError("ERROR: Error pruning slice: %s" %
                                       reason[1])
            else:
                (variableAddress, variableLength) = pin.resolveVariable(
                                  args[1])
                try:
                    pin.execute("monitor prune %s %s %s" % (args[0],
                            variableAddress, variableLength)) 
                except gdb.error:
                    reason = sys.exc_info()
                    raise gdb.GdbError("ERROR: Error pruning slice: %s" %
                                       reason[1])
        elif len(args) == 3:
                # 2nd and 3rd arguments are address and length
            try:
                pin.execute("monitor prune %s %s %s" % (args[0], args[1],
                        args[2]))
            except gdb.error:
                reason = sys.exc_info()
                raise gdb.GdbError("ERROR: Error pruning slice: %s" %
                                   reason[1])
        else:
            raise gdb.GdbError("ERROR: Invalid prune command syntax")
PruneCommand()
SliceCommand()
pin.TraceCommand()
pin.BreakCommand()
