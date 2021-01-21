from typing import List, Any
from itertools import chain, product
import asyncio
import os
from os import path
import json
import multiprocessing
from shutil import which

import click

from congestion_runner.config import Config, config_enumerator, Aspect, Run

SCRIPT_LOCATION = path.dirname(path.abspath(__file__))
DELAY_ASSIGNMENT_JSON_FNAME = 'delay_assignment.json'
PACKET_PROPERTIES_ASSIGNMENT_JSON_FNAME = 'packet_properties_assignment.json'


class Options:
    def __init__(
            self,
            dry_run: bool,
            valgrind: bool,
            perf: bool,
            debug: bool,
            ):
        self.dry_run = dry_run
        self.valgrind = valgrind
        self.perf = perf
        self.debug = debug


async def run_it(
        exe: str,
        args: List[str],
        directory_name: str,
        delay_assignment_json: dict,
        packet_properties_assignment_json: dict,
        opt: Options,
        sem: asyncio.Semaphore):

    # Make directory to save results
    if not os.path.exists(directory_name):
        os.makedirs(directory_name)

    with open(path.join(directory_name, DELAY_ASSIGNMENT_JSON_FNAME),
              'w') as delay_file:
        json.dump(delay_assignment_json, delay_file)

    with open(path.join(
            directory_name,
            PACKET_PROPERTIES_ASSIGNMENT_JSON_FNAME
            ), 'w') as packet_properties_file:
        json.dump(packet_properties_assignment_json, packet_properties_file)

    exe_args = [exe]

    print(' '.join(f'"{arg}"' for arg in args))

    if opt.dry_run:
        return 0

    if opt.valgrind:
        exe_args = ['valgrind'] + exe_args

    if opt.perf:
        debug_args = ['--call-graph', 'dwarf'] if opt.debug else []
        exe_args = ['perf', 'record'] + debug_args + exe_args

    with open(path.join(directory_name, 'stdout.log'), 'w') as stdout:
        with open(path.join(directory_name, 'stderr.log'), 'w') as stderr:
            async with sem:
                process = await asyncio.create_subprocess_exec(
                        *exe_args, *args, stdout=stdout, stderr=stderr)

                return await process.wait()


async def run_single_config(
        run: Run,
        ns_path: str,
        results_dir: str,
        opt: Options,
        sem: asyncio.Semaphore):
    config = Config.from_file('config.toml', run)

    futures = []

    for load, values in config_enumerator(results_dir, config):
        for directory_name, delay_assignment_json in values:
            # Directory name: workload_scheme_load_[load]

            # TODO: magic constant, fix!
            sim_script = os.path.abspath(
                    config.custom_script or 'spine_empirical.tcl')

            delay_assignment_json_filename = None

            if delay_assignment_json is not None:
                delay_assignment_json_filename = path.abspath(path.join(
                    directory_name, DELAY_ASSIGNMENT_JSON_FNAME))

            packet_properties_assignment_json_filename = None

            if config.packet_properties_assignment_json is not None:
                packet_properties_assignment_json_filename = path.abspath(
                        path.join(directory_name,
                                  PACKET_PROPERTIES_ASSIGNMENT_JSON_FNAME))

            args = [
                   sim_script,
                   config['sim_end'],
                   config['link_rate'],
                   config['mean_link_delay'],
                   config['host_delay'],
                   config['queue_size'],
                   config['connections_per_pair'],
                   config.get_flow_arrival_command(load),
                   config['enable_multi_path'],
                   config['per_flow_mp'],
                   config['source_alg'],
                   config['init_window'],
                   config['ack_ratio'],
                   config['slow_start_restart'],
                   config['dctcp_g'],
                   config['min_rto'],
                   config['prob_cap'],
                   config['switch_alg'],
                   config['dctcp_k'],
                   config['drop_prio'],
                   config['prio_scheme'],
                   config['deque_prio'],
                   config['keep_order'],
                   config['prio_num'],
                   config['ecn_scheme'],
                   config.get_pias_threshold(load, 0),
                   config.get_pias_threshold(load, 1),
                   config.get_pias_threshold(load, 2),
                   config.get_pias_threshold(load, 3),
                   config.get_pias_threshold(load, 4),
                   config.get_pias_threshold(load, 5),
                   config.get_pias_threshold(load, 6),
                   config['topology_spt'],
                   config['topology_tors'],
                   config['topology_spines'],
                   config['topology_x'],
                   path.abspath(path.join(directory_name, 'flow.tr')),
                   delay_assignment_json_filename,
                   config['use_fifo_processing_order'],
                   config['use_deadline'],
                   packet_properties_assignment_json_filename,
                   config['expiration_time_controller'],
                   config['enable_delay'],
                   config['enable_dupack'],
                   config['enable_early_expiration']
                   ]

            args = [_to_tcl_arg(arg) for arg in args]

            futures.append(run_it(
                ns_path,
                args,
                path.abspath(directory_name),
                delay_assignment_json,
                config.packet_properties_assignment_json,
                opt,
                sem))

    return await asyncio.gather(*futures)


def _to_tcl_arg(arg: Any) -> str:
    if isinstance(arg, str):
        return arg
    elif isinstance(arg, bool):
        return str(arg).lower()
    if arg is None:
        return 'NA'
    else:
        return str(arg)


async def run_all(runs: List[Run], ns_path: str, results_dir, opt: Options):
    number_worker_threads = multiprocessing.cpu_count()
    semaphore = asyncio.Semaphore(number_worker_threads)

    tasks = (run_single_config(run, ns_path, results_dir, opt, semaphore)
             for run in runs)

    return list(chain.from_iterable(await asyncio.gather(*tasks)))


def config_params(func):
    def wrapper(**kwargs):
        runs = [{aspect: choice for aspect, choice in zip(Aspect, run)}
                for run in product(*(kwargs[aspect.name.lower()]
                                     for aspect in Aspect))]

        for aspect in Aspect:
            del kwargs[aspect.name.lower()]

        func(runs=runs, **kwargs)

    for aspect in Aspect:
        wrapper = click.option(
                f'--{aspect.name.lower()}',
                type=str,
                multiple=True)(wrapper)

    return wrapper


def _get_ns_path(debug: bool, ns_executable: str):
    if ns_executable is not None:
        return ns_executable
    else:
        if which('ns') is not None:
            return which('ns')
        elif 'DEV_NS_DIR' in os.environ:
            ns_path_template = path.join(
                    os.environ['DEV_NS_DIR'], 'cmake-build-{}', 'ns')

            if debug:
                return ns_path_template.format('debug')
            else:
                return ns_path_template.format('release')
        else:
            raise RuntimeError('ns executable not found')


@click.command()
@click.option('--dry-run', is_flag=True)
@click.option('--valgrind', is_flag=True)
@click.option('--perf', is_flag=True)
@click.option('--debug', is_flag=True)
@click.option('--ns-executable', type=click.Path())
@click.option('--results-dir', type=click.Path(), default='results')
@config_params
def main(debug: bool,
         runs: List[Run],
         dry_run: bool,
         valgrind: bool,
         perf: bool,
         ns_executable: str,
         results_dir: str):
    ns_path = _get_ns_path(debug, ns_executable)
    opt = Options(dry_run=dry_run, valgrind=valgrind, perf=perf, debug=debug)

    results = asyncio.run(run_all(runs, ns_path, results_dir, opt))

    if [x for x in results if x != 0]:
        exit(1)


if __name__ == '__main__':
    main()
