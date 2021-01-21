#!/usr/bin/env python3

import subprocess
import json
from csv import DictWriter
from collections import namedtuple
from typing import List

import click

from congestion_runner.config import Config, config_enumerator, Run
from congestion_runner.run import config_params

RunResult = namedtuple('RunResult', ['load', 'results'])


@click.command()
@click.option('--results-dir', type=str, default='results')
@click.option('--plot-dir', type=str, default='./')
@config_params
def generate(plot_dir: str, results_dir: str, runs: List[Run]):
    for run in runs:
        generate_for_name(plot_dir, results_dir, run)


def generate_for_name(plot_dir: str, results_dir: str, run: Run):
    config = Config.from_file('config.toml', run)

    load_result = []

    for load, values in config_enumerator(results_dir, config):
        results = [_get_all_results(dirname, config)
                   for dirname, _ in values]
        load_result.append(RunResult(load=load, results=results))

    if len(load_result[0].results) == 1:
        save_result(plot_dir, load_result, 0, config.run_name)
    else:
        for idx in range(len(load_result[0].results)):
            save_result(plot_dir, load_result, idx, config.run_name)

        for load, results in load_result:
            filename = f'{plot_dir}/{config.run_name}_{int(load * 100)}.csv'
            fields = ['id'] + [f'{flow_kind}_{metric}'
                               for flow_kind in results[0].keys()
                               for metric in results[0][flow_kind].keys()]
            with open(filename, 'w') as csvfile:
                writer = DictWriter(csvfile, fields)
                writer.writeheader()
                for i, result in enumerate(results):
                    row_dict = {
                        f'{flows}_{value}': result[flows][value]
                        for flows in result.keys()
                        for value in result[flows].keys()
                    }
                    row_dict['id'] = load
                    writer.writerow(row_dict)


def save_result(plot_dir: str, load_result, idx: int, name: str):
    filename = f'{plot_dir}/{name}{f"_{idx}" if idx > 0 else ""}.csv'

    first_result = load_result[0].results[idx]

    fields = ['load'] + [f'{flow_kind}_{metric}'
                         for flow_kind in first_result.keys()
                         for metric in first_result[flow_kind].keys()]

    with open(filename, 'w') as csvfile:
        writer = DictWriter(csvfile, fields)
        writer.writeheader()
        for load, results in load_result:
            result = results[idx]
            row_dict = {
                f'{flows}_{value}': result[flows][value]
                for flows in result.keys()
                for value in result[flows].keys()
            }
            row_dict['load'] = load
            writer.writerow(row_dict)


def _get_all_results(directory_name: str, config: Config) -> dict:
    args = ['python', 'result.py', '-i', f'{directory_name}/flow.tr',
            '-o', '-s', '-m', '-l', '--avg', '--avgn', '--tail',
            '--thd', '--json', '--gp']
    for gb in config.plot_generation_bounds:
        args.extend(['--bound', gb])

    result = subprocess.run(args, stdout=subprocess.PIPE)
    return json.loads(result.stdout)


if __name__ == '__main__':
    generate()
