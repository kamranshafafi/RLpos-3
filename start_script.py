# -*- coding: utf-8 -*-
'''
The starting script.
'''

__header__ = r'''
    ______                     ____  __    ____ 
   / ____/      __            / __ \/ /_  / __ \
  / /_  | | /| / /  ______   / /_/ / __ \/ / / /
 / __/  | |/ |/ /  /_____/  / ____/ / / / /_/ / 
/_/     |__/|__/           /_/   /_/ /_/_____/  
  
    Author: kamran shafafi
''' # patorjk.com/software/taag  (font=Slant)


import logging
import os
import subprocess
import argparse
import sys
from rukutils import ns3_path
import coloredlogs
import json
from rl.agent import init
import datetime
from logPath import generate_log_directory


print(f"\n {__header__}\n Starting...\n")

src_dir = os.path.abspath(os.path.dirname(__file__))


def argument_parser(json_configs):
    """
    Parses the parameters from `json_configs`.

    :param json_configs: A dictionary of configuration parameters read from a JSON file.
    :type json_configs: dict

    :return: An `argparse` namespace object with the parsed parameters.
    :rtype: argparse.Namespace

    :raises: argparse.ArgumentError, if any of the parameters are invalid.

    - simType: valid options: train;eval
    - raAlg: valid options: dara;min;id;fixed (DARA Minstrel-HT Ideal or Fixed)
    - trafficProtool: valid options: udp;tcp - Generated traffic in ns3 is UDP or TCP
    - port: Port to initialize ns3 sim
    - seed: Seed for ns3 sim
    - mcs: MCS chosen for the fixed Rate Algorithm
    - dir: Replace naming convention mechanism
    """
    logging.info("Parsing the arguments...")
    sim_defaults = json_configs['ns3Sim']['defaultValues']
    sim_configs = json_configs['ns3Sim']['configSim']
    parser = argparse.ArgumentParser(
        description='Agent Arguments')  # Command Line Interface
    parser.add_argument('--simType', dest='simType',
                        type=str, default=json_configs['Agent'])  
    parser.add_argument('--trafficProtocol', dest='trafficProtocol', type=str,
                        default=sim_configs['trafficProtocol'])
    parser.add_argument('--port', dest='port', type=int,
                        default=sim_defaults['openGymPort'])
    parser.add_argument('--seed', dest='seed', type=int,
                        default=sim_defaults['simSeed'])  
    parser.add_argument('--dir', dest='dir', type=str, default="")
    argparse.ArgumentParser()

    logging.info("Parsing the arguments... Ok!")
    return parser.parse_args()




def sim_start(fwdir, sim_args=None):
   
    logging.info("Starting Simulation without agent.")
    sim_script_name = os.path.basename(fwdir)
    the_ns3_path = ns3_path.find(fwdir)
    base_ns3_dir = os.path.dirname(the_ns3_path)

    os.chdir(base_ns3_dir)

    ns3_string = the_ns3_path + ' run "' + sim_script_name + '/sim' + '"' 
    logging.info("Start command: %s", ns3_string)
    subprocess.run(ns3_string, shell=True, stdout=None, stderr=None, check=True)

def main():
    json_file_path = '/home/kshafafi/workspace/fw-phd/config/input.json'
    # Open the JSON file for reading
    with open(json_file_path, 'r') as json_file:
        json_configs = json.load(json_file)

    # Set up logging with the debug level and format specified in the config.json file
    coloredlogs.install(level=json_configs['StartScript']['debugLevel'],
                        fmt='%(asctime)s - %(filename)s:'
                        '%(funcName)s:%(lineno)s | '
                        '%(levelname)s | %(message)s')
    # Read the simulation parameters from the command line
    args = argument_parser(json_configs)
   

    # print("==========================================================",args.simType)

    
    
    # Set up the log file for the simulation
    log_file_path = subprocess.run(["bash", "lib/setuplogfile.sh", str(args.simType), str(args.dir)],
                                    check=True, stdout=subprocess.PIPE).stdout.decode()
    
    

    # Create the simulation arguments
    sim_args = {
        "simType": args.simType,
        "trafficProtocol": args.trafficProtocol,        
        "path": log_file_path,
        "enableGym": True
    }

    # Create a list with all the configuration information
    config_list = [json_configs, args, sim_args, log_file_path, src_dir]
 

    # Set up the log file for the simulation     
    start = json_configs['StartScript']['start']
    sim_file=json_configs["StartScript"]["simFile"]
    

    if start == "sim":
        sim_start(src_dir)
    elif start == "agent":
        init(config_list, sim_file)
    
    logging.info("Finished! Exiting...")
    sys.exit()

if __name__ == "__main__":
    main()
