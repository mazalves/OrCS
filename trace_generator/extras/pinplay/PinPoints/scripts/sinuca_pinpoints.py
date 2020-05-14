#!/usr/bin/env python

# BEGIN_LEGAL
# BSD License
#
# Copyright (c)2012 Intel Corporation. All rights reserved.
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
# $Id: sinuca_pinpoints.py,v 1.19 2014/05/27 22:28:26 tmstall Exp tmstall $

# This is a script to replay one pinplay process
#

import sys
import os

# Local modules
#
import cmd_options
import config
import msg
import pinpoints
import kit
import sinuca_kit
import phases
import util


def Sinuca_tracer_output(parser):
    parser.add_option('--sinuca_tracer_output', dest="sinuca_tracer_output", default='',
    help="Sinuca tracer output trace dir/name.")

def Sinuca_tracer_threads(parser):
    parser.add_option('--sinuca_tracer_threads', dest="sinuca_tracer_threads", default="0", type="int",
    help="Sinuca tracer number of threads.")

def Sinuca_tracer_parallel_start(parser):
    parser.add_option('--sinuca_tracer_parallel_start', dest="sinuca_tracer_parallel_start", default="-2", type="int",
    help="Sinuca tracer begin of parallel trace.")

def Sinuca_tracer_parallel_end(parser):
    parser.add_option('--sinuca_tracer_parallel_end', dest="sinuca_tracer_parallel_end", default="-2", type="int",
    help="Sinuca tracer end of parallel trace.")



class Sinuca_TracerPinPoints(pinpoints.PinPoints):

    # Use the PinPlay scripts for logging/replay.
    #
    logger_cmd      = 'logger.py'
    replay_cmd      = 'replay_dir.py'
    replayer_cmd    = 'replayer.py'

    # Simulator replay script
    #

    sim_replay_cmd  = config.all_scripts_path + 'sinuca_replay_dir.py'

    # Local simulator object.
    #
    sim_kit = None

    #########################################################################
    #
    # Options for script: sinuca_pinpoints.py
    #
    #########################################################################

    def GetKit(self):
        """ Get the PinPlay kit. """

        kit_obj = kit.Kit()
        self.pin  = kit_obj.pin
        self.path = kit_obj.path
        self.kit_type = kit_obj.kit_type
        return kit_obj

    def GetSimKit(self):
        """ Get the simulator kit. """

        kit_obj = sinuca_kit.Sinuca_TracerKit()
        self.pin  = kit_obj.pin
        self.path = kit_obj.path
        self.kit_type = kit_obj.kit_type
        return kit_obj

    ###################################################################################
    #
    # Add additional options not in PinPlay
    #
    ###################################################################################
    def AddAdditionalOptions(self, parser):
        """Add additional phase options which Sniper needs, but PinPlay doesn't use."""

        Sinuca_tracer_output(parser)
        Sinuca_tracer_threads(parser)

        Sinuca_tracer_parallel_start(parser)
        Sinuca_tracer_parallel_end(parser)

        return


    def AddAdditionalPhaseOptions(self, parser, phase_group):
        """Add additional phase options which the simulator needs, but PinPlay doesn't use."""

        cmd_options.region_sim(parser, phase_group)
        cmd_options.whole_sim(parser, phase_group)
        return

    ###################################################################################
    #
    # Extra phases run for simulator
    #
    ###################################################################################

    def RunAdditionalPhases(self, wp_pb_dir, sim_replay_cmd, options):
        """
        Run the additional phases for the simulator which are not run as
        part of the usual PinPlay phases.
        """

        phases_obj = phases.Phases()
        self.sim_kit = self.GetSimKit()

        msg.PrintMsg('sinuca_tracer_output:         ' + config.sinuca_tracer_output)
        msg.PrintMsg('sinuca_tracer_threads:        ' + str(config.sinuca_tracer_threads))
        msg.PrintMsg('sinuca_tracer_parallel_start: ' + str(config.sinuca_tracer_parallel_start))
        msg.PrintMsg('sinuca_tracer_parallel_end:   ' + str(config.sinuca_tracer_parallel_end))


        if not options.sinuca_tracer_output:
            msg.PrintMsgPlus('ERROR: Please set sinuca_tracer_output.')
            util.CheckResult(-1, options, 'Could not find the sinuca_tracer_output parameter.') # Force error with -1
# ~ # ~
        if not options.sinuca_tracer_threads:
            msg.PrintMsgPlus('ERROR: Please set sinuca_tracer_threads.')
            util.CheckResult(-1, options, 'Could not find the sinuca_tracer_threads parameter.') # Force error with -1
# ~ # ~
        if not options.sinuca_tracer_parallel_start:
            msg.PrintMsgPlus('ERROR: Please set sinuca_tracer_parallel_start.')
            util.CheckResult(-1, options, 'Could not find the sinuca_tracer_parallel_start parameter.') # Force error with -1
# ~ # ~
        if not options.sinuca_tracer_parallel_end:
            msg.PrintMsgPlus('ERROR: Please set sinuca_tracer_parallel_end.')
            util.CheckResult(-1, options, 'Could not find the sinuca_tracer_parallel_end parameter.') # Force error with -1


        # Run branch predictor simulator on region pinballs.
        #
        if options.region_sim or options.default_phases:
            # Print out simulator results every warmup_length instructions.
            #
            for pp_dir in util.GetRegionPinballDir():
                phase_length = options.warmup_length
                if phase_length == 0:
                    phase_length = options.slice_size
                if not options.list:
                    msg.PrintMsgDate('Running simulator on region pinballs: %s' % \
                        config.PhaseStr(config.sim_regions))
                util.PhaseBegin(options)
                result = self.sim_kit.RunSimulator(pp_dir, sim_replay_cmd, options)
                if not options.list:
                    msg.PrintMsgDate('Finished running simulator on region pinballs: %s' % \
                        config.PhaseStr(config.sim_regions))
                util.CheckResult(result, options, 'simulator on region pinballs: %s' % \
                    config.PhaseStr(config.sim_regions))

        # Run branch predictor simulator on whole program pinballs.
        #
        if options.whole_sim or options.default_phases:
            # Set phase_length to print out simulator results every slice_size instructions.
            #
            phase_length = options.slice_size

            if not options.list:
                msg.PrintMsgDate('Running simulator on whole program pinballs: %s' % \
                    config.PhaseStr(config.sim_whole))
            util.PhaseBegin(options)
            result = self.sim_kit.RunSimulator(wp_pb_dir, sim_replay_cmd, options)
            if not options.list:
                msg.PrintMsgDate('Finished running simulator on whole program pinballs: %s' % \
                    config.PhaseStr(config.sim_whole))
            util.CheckResult(result, options, 'simulator on whole program pinballs: %s' % \
                config.PhaseStr(config.sim_whole))

        return 0

def main():
    """ Process command line arguments and run the script """

    pp = Sinuca_TracerPinPoints()
    result = pp.Run()
    return result

# If module is called in stand along mode, then run it.
#
if __name__ == "__main__":
    result = main()
    sys.exit(result)
