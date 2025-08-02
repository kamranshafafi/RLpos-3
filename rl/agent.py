# -*- coding: utf-8 -*-
'''
The Agent main script.
'''
from __future__ import absolute_import, division, print_function

import os
os.environ['TF_CPP_MIN_LOG_LEVEL']='3' #Mute the warnings of tensorflow (cuda, etc...)

from ns3gym import ns3env
import coloredlogs
from tf_agents.train.utils import train_utils
from tf_agents.replay_buffers import tf_uniform_replay_buffer
from tf_agents.trajectories import trajectory
from tf_agents.utils import common
from tf_agents.networks import q_network
from tf_agents.environments import tf_py_environment
from tf_agents.environments import suite_gym
from tf_agents.agents.dqn import dqn_agent
import tensorflow as tf
import logging
import subprocess
import sys
import numpy as np
from tf_agents.environments import utils
from tf_agents.policies import random_tf_policy
from tf_agents.policies import policy_saver
from tf_agents.agents.random import random_agent

# Inside the train_policy function
def train_policy(agent_configs, env, policy, buffer, dataset, agent):
    hyperparams = agent_configs['hyperParameters']
    iterator = iter(dataset)
    agent.train = common.function(agent.train)
    agent.train_step_counter.assign(0)
    cont = 0
    step = 0
    for i in range(hyperparams['numTrainEpisodes']):
        logging.info("Starting Episode %i" % i)
        time_step = env.reset()        

        while not time_step.is_last():
            time_step = env.current_time_step()
            action_step = policy.action(time_step)

            # Adjusting the dimensions for logging
            adjusted_observation = tf.reshape(time_step.observation, [-1])
            adjusted_reward = tf.reshape(time_step.reward, [-1])

            print("Action Step:", action_step)
            logging.info("Action: %s | Observation: %s | Reward: %s" % (
                action_step.action.numpy(),
                adjusted_observation.numpy(),
                adjusted_reward.numpy()
            ))
            
            next_time_step = env.step(action_step.action)            
            traj = trajectory.from_transition(
                time_step, action_step, next_time_step)
             
            # Add trajectory to the replay buffer
            buffer.add_batch(traj)
            # experience, unused_info = next(iterator)
            # train_loss = agent.train(experience).loss
            step = agent.train_step_counter.numpy()
            if cont == 100:
                experience, unused_info = next(iterator)
                step = agent.train_step_counter.numpy()
                train_loss = agent.train(experience).loss
                # logging.debug('step = %i: loss = %i' % (step, train_loss))
                # cont=0
            elif cont < 100:
                cont = cont+1
               
    return step


def eval_policy(agent_configs, environment, policy, path):
    
    original_stdout = sys.stdout
    hyperparams=agent_configs['hyperParameters']
    n_steps = 0
    total_return = 0.0
    max_rate = 65
    for _ in range(hyperparams['numEvalEpisodes']):
        logging.info("Before reset.")
        time_step = environment.reset()
        logging.info(f"time_step is: {time_step}")
        logging.info("After reset.")
        episode_return = 0.0
        rep = 0

        while not time_step.is_last():
            rep = rep+1
            action_step = policy.action(time_step)
            logging.info(f"Action_step is: {action_step}")
            time_step = environment.step(action_step.action)
            logging.info("Action: %i | Observation: %.2f | Reward: %.2f" % (action_step.action.numpy()[0], time_step.observation.numpy()[0][0], time_step.reward.numpy()[0]))
            episode_return += time_step.reward
            n_steps = n_steps+1
            if (rep == 5 and agent_configs['actionDebug']):
                print("The action was: " + str(action_step.action.numpy()[0]))
                print("The Observation was: " +
                      str(time_step.observation.numpy()[0][0]))
                print("The reward was: " + str(time_step.reward.numpy()[0]))
                rep = 0
            if agent_configs['logSteps']:
                with open(path+'/logSteps.csv', 'a', encoding="utf-8") as file:
                    sys.stdout = file
                # print(str(time_step))
                    print(str(n_steps) + ";" + str(time_step.reward.numpy()[0])
                          + ";" + str(time_step.observation.numpy()[0][0]) + ";"
                          + str(action_step.action.numpy()[0]))
                    sys.stdout = original_stdout

            total_return += episode_return*max_rate/n_steps
        #avg_return = total_return / hyperparams['numEvalEpisodes']
        #print('Average Return: {0} '.format(avg_return()[0]))
        # return avg_return.numpy()[0]

def env_setup(port, seed, sim_args, src_dir, sim_file):
    '''
    Configures the NS3 Gym env. (initiates the simulation during this configuration) 
    '''
    logging.info("Setting up the environment...")    
    
    ns3_gym_env = ns3env.Ns3Env(port=port,
                                simSeed=seed,
                                startSim=True,
                                simArgs=sim_args,
                                debug=True,
                                src_dir=src_dir,
                                sim_file = sim_file) 

    print("Observation Space:", ns3_gym_env.observation_space)
    print("Action Space:", ns3_gym_env.action_space)

   # Print an example observation
    time_step = ns3_gym_env.reset()
    print("Time Step Structure:", time_step)
    
    # Check if the observation is present in the time_step
    if isinstance(time_step, np.ndarray):
        example_observation = time_step
        print("Example Observation:", example_observation)
    else:
        print("Observation not found in the time_step object.")

    #the simulation is launched during a while... (without agent) and simulation is interrupted.
    env = tf_py_environment.TFPyEnvironment(suite_gym.wrap_env(ns3_gym_env))
    logging.info("Setting up the environment... Ok!")
    return env

def init(config_list, sim_file):
    
    logging.info("Initializing Agent...")    

    
    tf.compat.v1.enable_v2_behavior()
    tf.compat.v1.logging.set_verbosity(tf.compat.v1.logging.ERROR) # Mute the deprecation warnings. 

       #parse list
    json_configs=config_list[0]
    args=config_list[1]
    sim_args=config_list[2]
    log_file_path=config_list[3]
    src_dir=config_list[4]
    sim_args["enableGym"] = True  # assuming that gym will only work together with agent
    agent_configs = json_configs['Agent'] 
    hyperparams = agent_configs['hyperParameters']
    


    coloredlogs.install(level=json_configs['Agent']['debugLevel'],
                        fmt='%(asctime)s - %(filename)s:'
                        '%(funcName)s:%(lineno)s | '
                        '%(levelname)s | %(message)s')
  
    env = env_setup(args.port, args.seed, sim_args, src_dir, sim_file) 


    time_step = env.reset()
    print("Observation:", time_step.observation)
    print("Reward:", time_step.reward)
    print("Step Type:", time_step.step_type)
    print("Discount Factor:", time_step.discount)

    # Assuming train_env is an instance of your Ns3Env class
    time_step_spec = env.time_step_spec()
    action_spec = env.action_spec()

    print("Time Step Specification:")
    print(time_step_spec)

    print("\nAction Specification:")
    print(action_spec)



    q_net = q_network.QNetwork(
        env.observation_spec(),
        env.action_spec(),
        fc_layer_params=hyperparams['fullyConnectedLayer'].split(","))

    optimizer = tf.compat.v1.train.AdamOptimizer(
        learning_rate=hyperparams['learningRate'])

    train_step_counter = tf.Variable(0)  # Counter of Steps.
    train_step = train_utils.create_train_step()

    
    epsilon_config = hyperparams['epsilon']
    if args.simType == "train":
        epsilon = tf.compat.v1.train.polynomial_decay(
            learning_rate=1.0,
            global_step=train_step,
            decay_steps=epsilon_config['decaySteps'],
            end_learning_rate=epsilon_config['finalValue'])
    elif args.simType == "eval":
        epsilon = epsilon_config['evalValue']

    agent = dqn_agent.DqnAgent(
        env.time_step_spec(),
        env.action_spec(),
        q_network=q_net,
        optimizer=optimizer,
        td_errors_loss_fn=common.element_wise_squared_loss,
        gamma=hyperparams['discount'],
        epsilon_greedy=epsilon,
        train_step_counter=train_step_counter)    
    

    agent.initialize()

    replay_buffer = tf_uniform_replay_buffer.TFUniformReplayBuffer(
        data_spec=agent.collect_data_spec,
        batch_size=env.batch_size,
        max_length=hyperparams['replayBufferMaxLength'])

    dataset = replay_buffer.as_dataset(
        num_parallel_calls=3,
        sample_batch_size=hyperparams['batchSize'],
        num_steps=2).prefetch(3)

    ckpt_dir = os.path.join(src_dir, 'checkpoint')
    if (agent_configs['newPolicy'] and args.simType == "train"):
        subprocess.run(["rm", "-rf", ckpt_dir], check=True)
        logging.info("Deleted existing policy and starting a new train.")

    train_checkpointer = common.Checkpointer(
        ckpt_dir=ckpt_dir,
        max_to_keep=1,
        agent=agent,
        policy=agent.policy,
        replay_buffer=replay_buffer,
        global_step=train_step_counter
    )
    logging.info("Initializing Agent ... Ok!")
    if args.simType == "eval":  # Evaluate the agent's policy.
        eval_policy(agent_configs, env, agent.policy, log_file_path)
        if args.raAlg == "drl":
            subprocess.run(["cp", "-r", ckpt_dir, log_file_path], check=True)

    elif args.simType == "train":
        ckpt_step = train_policy(agent_configs, env, agent.collect_policy, replay_buffer,
                                 dataset, agent)
        train_checkpointer.save(global_step=ckpt_step)
        subprocess.run(["cp", "-r", ckpt_dir, log_file_path], check=True)
        print("Saved Checkpoint!")

    logging.info("Finished Agent.")

