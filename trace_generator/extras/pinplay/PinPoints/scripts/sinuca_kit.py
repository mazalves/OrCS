#!/usr/bin/env python

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
#
#
# @ORIGINAL_AUTHORS: T. Mack Stallcup, Cristiano Pereira, Harish Patil, Chuck Yount
#
#
# $Id: sinuca_kit.py,v 1.24 2014/05/27 22:28:26 tmstall Exp tmstall $

#
# Branch predictor simulator: Contains methods use to generate prediction error.
#

import sys
import os

# Local modules
#
import config
import msg
import kit
import util

class Sinuca_TracerKit(kit.Kit):
    """
    This class contains methods to run the branch predictor simulator and extract
    the metric of interest from the output files it generates.

    Users can modifying the code in the module to use a different simulator.
    The modules 'config.py' and 'replay_dir.py' must also be modified to
    reflect the new simulator.  All other code should be independent of the
    simulator.

    Alternatively, the code in this module can be used as a template to add a new simulator,
    in addition to the branch predictor simulator. To do this, all files which start with
    the string 'sinuca_' must be cloned/modified for the new simulator.

    The following instructions are based on modifying the existing simulator
    (first case listed above).  If adding a new simulator, the additional modifications
    to the other 'sinuca_' files, beyond the instructions given below, should be self-evident.

    I. It is assumed the simulator has at least these two options:

        1) Specify name of the file for output data
                Knob for branch predictor is '-statfile file'

        2) Print simulator data for the metric of interest every 'icount'
           instructions.  The code that runs the simulator uses this option
           with the tracing parameter 'warmup_length'.  See method GetRegionMetric()
           for more information.
                Knob for branch predictor is '-phaselen icount'

    II. Steps required in this module to add a new simulator:

        1) Modify the attributes in this class to reflect the requirements of
           the new simulator (in section "simulator control knobs").  If the only
           changes required to create a simulator kit are in this module, then the
           remaining changes are described in section III.

        2) The base class for this class is in the module 'kit.py'.  If you
           need to modify values in the base class, then there may be other
           locations in the scripts which may also need to be modified.

        3) Modify the following method to reflect the knobs in the new simulator:
                GetSimOutputFile()

        4) Modify method RunSimulator() to run the new simulator.

    III. Steps required in other modules to add a new simulator:

        1) Add a type for the new simulator in the module 'config.py' under "Types of kits".

        2) Modify the method Replay() in 'replay_dir.py'.  Search for the
           string 'options.sim_add_filename' and add code to instantiate a kit for
           the new simulator type.
    """

    # What type of a kit is this.
    #
    kit_type = config.SINUCA

    pintool = 'sinuca_tracer.so'

    ###################################################################
    #
    # Define simulator control knobs & file extension
    #
    ###################################################################


    ###################################################################
    #
    # Methods used to run the Branch Predictor simulator and get information
    # from output files required to calculate prediction error.
    #
    ###################################################################

    def RunSimulator(self, dirname, sim_replay_cmd, options):
        """
        Run Branch Predictor simulator on all the pinballs in a set of
        directories given by 'dirname'.


        Return: error code from running the simulator
        """

        # Don't do anything if running in debug mode.
        #
        if options.debug:
            return 0

        gv = config.GlobalVar()

        # Need to let script 'sim_replay_cmd' know type of kit is calling it.
        #
        config.sim_kit_type = self.kit_type

        # Run on all the pinballs in 'dirname'.
        #
        cmd = 'pwd;' + sim_replay_cmd + ' --replay_dir ' + dirname
        cmd += ' --log_options'

        # SiNUCA options
        cmd += ' "'
        cmd += ' -output ' + str(options.sinuca_tracer_output)
        cmd += ' -threads ' + str(options.sinuca_tracer_threads)
        cmd += ' -parallel_start ' + str(options.sinuca_tracer_parallel_start)
        cmd += ' -parallel_end ' + str(options.sinuca_tracer_parallel_end)
        cmd += ' "'

        cmd += ' --sim_add_filename'      # This causes simulator output file name to be added to cmd.
        cmd += util.AddCfgFile(options)
        cmd += util.AddGlobalFile(gv.DumpGlobalVars(), options)

        msg.PrintMsg('DEBUG>> ' + cmd + '\n')

        end_str = ''                      # Don't want to print anything when this cmd finishes.
        # import pdb;  pdb.set_trace()
        result = util.RunCmd(cmd, options, end_str, False)

        return result


