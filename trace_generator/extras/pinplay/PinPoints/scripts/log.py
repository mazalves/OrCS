#!/usr/bin/env python
# BEGIN_LEGAL
# BSD License
#
# Copyright (c)2015 Intel Corporation. All rights reserved.
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
#
#
# @ORIGINAL_AUTHORS: T. Mack Stallcup, Cristiano Pereira, Harish Patil, Chuck Yount
#
#
# $Id: log.py,v 1.14 2015/08/15 20:02:00 tmstall Exp tmstall $

import sys
import os
import random
import subprocess
import optparse

# Local modules
#
import cmd_options
import config
import kit
import msg
import util


class Log(object):
    """
    Generate log files for an application

    This class is the low level primitive which runs an application with the
    logger to generate pinballs (log files).  It works with 6 classes of
    applications:
        st - single-threaded
        mt - multi-threaded
        mpi - MPI single-threaded
        mpi_mt - MPI multi-threaded
        mp - multi-process, single-threaded (using fork/exec)
        mp_mt - multi-process, multi-threaded (using fork/exec)

    This module is used by both the PinPlay and DrDebug tool chains to generate
    pinballs.
    """

    def GetKit(self):
        """
        Get the PinPlay kit.

        @return PinPlay kit
        """

        return kit.Kit()

    def AddOptionIfNotPresent(self, options, opt, arg):
        """
        This function adds an option to the tool if the option was not already
        passed in the --log_options switch

        @param options Options given on cmd line
        @param opt     Option to add
        @param arg     Argument for option

        @return String containing option and argument
        """

        # import pdb;  pdb.set_trace()
        raw_options = options.log_options

        # option was not passed in the command line
        #
        if raw_options.find(opt) == -1:
            return ' ' + opt + ' ' + arg + ' '
        return ''

    def MpModeOptions(self, options):
        """
        Add multi-process logging mode options.

        @param options Options given on cmd line

        @return String of knobs required for MT apps
        """

        pintool_options = self.AddOptionIfNotPresent(options, '-log:mp_mode', '')

        # print out information about shared-segments and allow MP apps to attach
        # to memory pool
        #
        pintool_options += self.AddOptionIfNotPresent(options,
                                                      '-log:mp_verbose', '1')
        pintool_options += ' -log:mp_attach '

        return pintool_options

    def ParseCommandLine(self):
        """
        Parse command line arguments and check to make sure all required options were given.

        @return List of options
        """

        # command line options for the driver
        #
        version = '$Revision: 1.14 $'
        version = version.replace(' ', '')
        ver = version.replace(' $', '')
        us = '%prog [options] --mode MODE binary args'
        desc = 'Runs binary with the logger Pintool and generates whole program '\
               'pinballs. Required arguments include:\n'\
               '   1) Binary to run and it\'s arguments.\n'\
               '   2) Mode of the binary using \'--mode MODE\'.\n'\
               'If the option \'--log_file\' is not given, the default pinball name is \'log\'.'

        util.CheckNonPrintChar(sys.argv)
        parser = optparse.OptionParser(
            usage=us,
            version=ver,
            description=desc,
            formatter=cmd_options.BlankLinesIndentedHelpFormatter())

        # Define the command line options which control the behavior of the
        # script.  Some of these methods take a 2nd argument which is the empty
        # string ''.   If the script uses option groups, then this parameter is
        # the group.  However, this script does not use option groups, so the
        # argument is empty.
        #
        cmd_options.arch(parser, '')
        cmd_options.compressed(parser, '')
        cmd_options.config_file(parser)
        cmd_options.debug(parser)
        cmd_options.global_file(parser)
        cmd_options.log_file(parser)
        cmd_options.log_options(parser)
        cmd_options.mode(parser, '')
        cmd_options.mpi_options(parser, '')
        cmd_options.msgfile_ext(parser)
        cmd_options.no_log(parser)
        cmd_options.no_print_cmd(parser)
        cmd_options.pid(parser)
        cmd_options.pinplayhome(parser, '')
        cmd_options.pintool(parser)
        cmd_options.pintool_help(parser)
        cmd_options.pin_options(parser)
        cmd_options.save_global(parser)
        cmd_options.sdehome(parser, '')
        cmd_options.verbose(parser)

        # import pdb;  pdb.set_trace()
        (options, args) = parser.parse_args()

        # Added method cbsp() to 'options' to check if running CBSP.
        #
        util.AddMethodcbsp(options)

        # Translate the 'arch' string given by the user into
        # the internal arch type used by the scripts.
        #
        util.SetArch(options)

        # If user just wants pintool_help, return
        #
        if (hasattr(options, 'pintool_help') and options.pintool_help):

            return options

        # Read in configuration files and set global variables.
        # No requirment to read in a config file, but it's OK
        # to give one.
        #
        config_obj = config.ConfigClass()
        config_obj.GetCfgGlobals(options,
                                 False)  # No, don't need required parameters

        # Make sure user gave an application mode
        #
        if options.mode:
            options.mode = util.ParseMode(options.mode)
        else:
            parser.error("Application mode was not given.\n" \
                "Need to use option --mode MODE. Choose MODE from: 'st', 'mt', 'mpi', 'mpi_mt', 'mp', 'mp_mt'")

        # Get the application command line
        #
        # import pdb;  pdb.set_trace()
        cmd_line = " ".join(args)
        if not options.pid:
            if cmd_line:
                setattr(options, 'command', cmd_line)
            else:
                parser.error('no program command line specified.\n'
                             'Need to add binary and it\'s arguments.')

        return options

    def RunMPCreate(self, kit, options):
        """
        Create mp_pool when logging MPI apps.

        @param kit     Kit instance for these scripts
        @param options Options given on cmd line

        @return String with MP key and a knob required for mp_pools, or null string
        """

        # Start command with pin and pintool.
        #
        # import pdb;  pdb.set_trace()
        cmd = os.path.join(kit.path, kit.pin)
        cmd += kit.GetPinToolKnob()

        # Logging mp_pool creation knobs.
        #
        log_key = ' -log:mp_key ' + str(random.randint(0, 32728))
        cmd += ' -log '
        cmd += log_key
        cmd += ' -log:mp_create_pool '
        cmd += ' -- echo'

        # Execute the cmd
        #
        if (not hasattr(options, 'no_print_cmd') or not options.no_print_cmd) or\
            hasattr(options, 'verbose') and options.verbose:
            msg.PrintMsgPlus('Creating mp_pools for MPI tracing')
            msg.PrintMsg(cmd)
        if not config.debug:
            p = subprocess.Popen(cmd, shell=True)
            p.communicate()
            del p

        # Return the key to the memory pool just created.
        #
        return log_key

    def RunMPDelete(self, kit, log_key, options):
        """
        Delete mp_pool when logging MPI apps.

        @param kit     Kit instance for these scripts
        @param log_key  Key to mp_pool to delete
        @param options Options given on cmd line

        @return No return value
        """

        # Start command with pin and pintool.
        #
        # import pdb;  pdb.set_trace()
        cmd = os.path.join(kit.path, kit.pin)
        cmd += kit.GetPinToolKnob()

        # Remove the string '-log:mode_attach' because it's only
        # used for logging.
        #
        pos = log_key.find('-log:mode_attach')
        if pos != -1:
            log_key = log_key[0:pos]

        # Logging mp_pool deletion knobs.
        #
        cmd += ' -log '
        cmd += log_key
        cmd += ' -log:mp_delete_pool '
        cmd += ' -- echo'

        # Execute the cmd
        #
        if (not hasattr(options, 'no_print_cmd') or not options.no_print_cmd) or\
            hasattr(options, 'verbose') and options.verbose:
            msg.PrintMsgPlus('Deleting mp_pools for MPI tracing')
            msg.PrintMsg(cmd)
        if not config.debug:
            p = subprocess.Popen(cmd, shell=True)
            p.communicate()
            del p

        return

    def Run(self):
        """
        Get all the user options and run the logger.

        @return Exit code from the logger pintool
        """

        # import pdb;  pdb.set_trace()
        options = self.ParseCommandLine()

        # Get the kit to be used for logging.
        #
        kit = self.GetKit()

        # Set the binary type in the kit.  Assume the first string in the
        # command line is the binary.  Only do this if user hasn't given 'pid'
        # or 'pintool_help'.
        #
        if not (hasattr(options, 'pid') and options.pid) and \
           not (hasattr(options, 'pintool_help') and options.pintool_help):
            binary = options.command.split()[0]
            kit.SetBinaryType(binary)
            if kit.binary_type == config.ARCH_INVALID:
                msg.PrintMsg(
                    '\nWARNING: Unable to determine binary file type.\n'
                    'Perhaps the string assumed to be the binary is incorrect.\n'
                    '   Command line:     ' + options.command + '\n'
                    '   Binary: (assumed) ' + binary + '\n'
                    'Setting binary type to \'Intel64\' as the default value.\n')
        else:
            # If user has given 'pid' or wants 'pintool_help', then need to
            # explictly set the architecture of the binary in the kit.
            #
            kit.binary_type = options.arch

        # Now that we know the type of the binary, set the user defined pintool,
        # if one exists.  Need to wait until now to set the tool because the
        # user may only have the tool in the architecture dependent directory
        # for this type of application.  Thus we need the binary type in order
        # to find it.
        #
        if hasattr(options, 'pintool') and options.pintool:
            kit.SetPinTool(options.pintool)

        # If user just wants 'pintool_help' go ahead and print it, then exit
        # the script.  Need to do this after we get the kit in order to print
        # the help for the correct kit.  Also needs to be after any user
        # defined pintools have been added to the kit. This ensures the
        # correct pintool help msg will be displayed.
        #
        if hasattr(options, 'pintool_help') and options.pintool_help:
            result = util.PintoolHelpKit(kit, options)

            return result

        # Get path to the kit pin binary, any user defined Pin knobs and
        # the pintool.
        #
        # import pdb;  pdb.set_trace()
        cmd = os.path.join(kit.path, kit.pin)
        if hasattr(options, 'pin_options') and options.pin_options:
            cmd += ' ' + options.pin_options
        if hasattr(options, 'pid') and options.pid:
            if kit.kit_type == config.PINPLAY:
                cmd += ' -pid ' + str(options.pid)
            elif kit.kit_type == config.SDE:
                cmd += ' -attach-pid ' + str(options.pid)

        cmd += kit.GetPinToolKnob()

        # Pintool knobs required for logging.
        #
        if not (hasattr(options, 'no_log') and options.no_log):
            cmd += ' -log'
            cmd += ' -xyzzy '

            # Add any knobs required by user options or the type of binary
            #
            if hasattr(options, 'log_file') and options.log_file:
                cmd += self.AddOptionIfNotPresent(options, '-log:basename',
                                                  options.log_file)
            cmd += util.AddMt(options)
            cmd += util.AddCompressed(options)
            cmd += util.GetMsgFileOption(options.log_file)

            # Add any user logging options given by the user
            #
            if hasattr(options, 'log_options') and options.log_options:
                cmd += options.log_options

        # Need to add shared memory knobs and create memory pools for MPI/MP apps.
        #
        create_mem_pool = False
        if options.mode == config.MPI_MODE or options.mode == config.MPI_MT_MODE or \
           options.mode == config.MP_MODE or options.mode == config.MP_MT_MODE:

            # Need to add the MPI options to the existing MT options already in
            # 'pintool_options'.
            #
            if not (hasattr(options, 'no_log') and options.no_log):
                cmd += self.MpModeOptions(options)
                create_mem_pool = True
            else:
                create_mem_pool = False

        # Format the MPI command line, if required to run the command line.
        #
        if options.mode == config.MPI_MODE or options.mode == config.MPI_MT_MODE:
            cmd = util.MPICmdLine(options) + ' ' + cmd

        # If logging enabled for multi-threaded app, generate the mp_pool.
        #
        if create_mem_pool:
            log_key = self.RunMPCreate(kit, options)
            cmd += log_key

        # Add program and arguments 
        #
        if not options.pid:
            cmd += ' -- ' + options.command

        # Print out command line used for pin and pintool
        #
        if not (hasattr(options, 'no_print_cmd') and
                options.no_print_cmd) or options.verbose:
            string = '\n' + cmd
            msg.PrintMsg(string)

        # Finally execute the command line and gather stdin and stdout.
        # Exit with the return code from executing the logger.
        #
        result = 0
        # import pdb;  pdb.set_trace()
        if not config.debug:
            platform = util.Platform()
            if not (hasattr(options, 'no_print_cmd') and options.no_print_cmd):
                if platform != config.WIN_NATIVE:
                    cmd = 'time ' + cmd
            p = subprocess.Popen(cmd, shell=True)
            p.communicate()
            result = p.returncode

        # If logging enabled for multi-threaded app, delete the mp_pool.
        #
        if create_mem_pool:
            self.RunMPDelete(kit, log_key, options)

        return result


def main():
    """
       This method allows the script to be run in stand alone mode.

       @return Exit code from running the script
       """

    log = Log()
    result = log.Run()
    return result

# If module is called in stand along mode, then run it.
#
if __name__ == "__main__":
    result = main()
    sys.exit(result)
