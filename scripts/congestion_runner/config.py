import toml
from typing import Optional
from itertools import count, product, chain
import json
from os import path
from typing import List, Set, Tuple, Dict, MutableMapping, Mapping, Any
import os
import functools
from enum import Enum, auto


class PriorityScheme(Enum):
    UNKNOWN = 5
    REMAINING_SIZE = 2
    BYTES_SENT = 3


PACKET_SIZE = 1460


_FNAME_SEPARATOR = '.'


class DelayAssignmentConfig:
    def __init__(self, config: Dict[str, str], link_rate: float, load: int):
        if self.is_multi_config(config):
            self._configs: List[DelayAssignmentConfig] = []
            for i in count(0):
                sub_config = {
                    key[len(f"{i}_"):]: value
                    for key, value in config.items()
                    if key.startswith(f"{i}_")
                }

                if len(sub_config) == 0:
                    break
                else:
                    self._configs.append(
                            DelayAssignmentConfig(sub_config, link_rate, load))
        else:
            self._config = config
            self._link_rate = link_rate
            self._load = load

    def get_jsons(self):
        if self.is_multi():
            return [self.get_json_min(jsons)
                    for jsons in product(*(
                        config.get_jsons() for config in self._configs))]
        else:
            if 'delay' in self._config:
                return [self.get_json_fixed_delay(
                    float(self._config['delay']))]
            elif 'delay_min' in self._config:
                return self.get_json_range()
            elif 'exp' in self._config:
                return [self.get_exp_json(
                    float(self._config['exp']),
                    self._config['use_capping'] == 'true')]
            elif 'gp' in self._config:
                return [self.get_json_gp(int(self._config['gp']))]
            elif 'nfct' in self._config:
                return [self.get_json_gp(
                    int(8.0 / float(self._config['nfct'])))]
            elif 'file' in self._config:
                return [self.get_json_file(self._config['file'])]
            elif 'uniform_min_delay' in self._config:
                return [self._get_json_uniform(
                    float(self._config['uniform_min_delay']),
                    float(self._config['uniform_max_delay']))]
            else:
                return [None]

    @staticmethod
    def is_multi_config(config: Dict[str, str]):
        for key in config.keys():
            if key.startswith('0_'):
                return True
        return False

    def is_multi(self):
        return hasattr(self, '_configs')

    def get_json_fixed_delay(self, delay: float):
        result = {
            'type': 'fixed',
            'parameters': {
                'delay': delay,
            }
        }

        self.attach_size_bounds(result)

        return result

    @staticmethod
    def get_json_min(subjsons):
        return {
            'type': 'min',
            'parameters': subjsons
        }

    def get_json_range(self):
        min_delay, max_delay = float(self._config['delay_min']), float(self._config['delay_max'])
        num_steps = int(self._config['count'])
        step = (max_delay - min_delay) / (num_steps - 1)
        return [self.get_json_fixed_delay(min_delay + step * i) for i in range(num_steps)]

    def get_exp_json(self, delay, use_capping):
        result = {
            'type': 'exponential',
            'parameters': {
                'average': delay,
                'link_speed': self._link_rate * 10 ** 9,
                'use_capping': use_capping,
            }
        }

        self.attach_size_bounds(result)

        return result

    def get_json_file(self, file_pattern):
        return {
            'type': 'file',
            'parameters': os.path.abspath(file_pattern.format(self._load))
        }

    def attach_size_bounds(self, result: dict):
        if 'size_upper_bound' in self._config:
            result['parameters']['size_upper_bound'] = int(self._config['size_upper_bound'])
        if 'size_lower_bound' in self._config:
            result['parameters']['size_lower_bound'] = int(self._config['size_lower_bound'])

    def get_json_gp(self, goodput):
        result = {
            'type': 'goodput',
            'parameters': {'goodput': goodput}
        }

        self.attach_size_bounds(result)

        return result

    @staticmethod
    def _get_json_uniform(min_delay, max_delay):
        return {
            'type': 'uniform',
            'parameters': {
                'min_delay': min_delay,
                'max_delay': max_delay,
            }
        }


class PacketPropertiesAssignerConfig:
    def __init__(self, config: Dict[str, str]):
        self._config = config

    @property
    def json(self) -> Optional[dict]:
        if len(self._config) == 0:
            return None
        if 'uniform' in self._config:
            return {'type': 'uniform'}
        elif 'inv_size' in self._config:
            return {'type': 'inv_size'}
        elif 'inv_rem_size' in self._config:
            return {'type': 'inv_rem_size'}
        elif 'las' in self._config:
            return {'type': 'las'}
        else:
            raise AssertionError


class ArrivalsConfig:
    def __init__(self, config: Dict[str, str]):
        self._config = config

    @property
    def process_code_name(self) -> str:
        if 'flow_cdf' in self._config:
            return 'PC'
        elif 'fixed' in self._config:
            return 'PF'
        elif 'uniform_min' in self._config and 'uniform_max' in self._config:
            return 'PU'
        else:
            raise AssertionError

    @property
    def args(self) -> str:
        if 'flow_cdf' in self._config:
            return self._config['flow_cdf']
        elif 'fixed' in self._config:
            return self._config['fixed']
        elif 'uniform_min' in self._config and 'uniform_max' in self._config:
            return f'{self._config["uniform_min"]} {self._config["uniform_max"]}'
        else:
            raise AssertionError

    @property
    def mean_flow_size(self) -> int:
        return int(self._config['mean_flow_size'])

    @property
    def mean_flow_size_in_bytes(self) -> int:
        return self.mean_flow_size * PACKET_SIZE


class Aspect(Enum):
    INPUT = auto()
    BUFFER = auto()
    CONTROL = auto()
    SLACKS = auto()
    SCALE = auto()


Run = Mapping[Aspect, str]


class Config:
    DELAYS = [0.5, 0.6, 0.7, 0.8, 0.9]

    def __init__(
            self,
            run: Run,
            default: Mapping[Any, Any],
            inherited: List[Dict[Any, Any]]):
        self._run = run
        self._default = default
        self._inherited = inherited

    @classmethod
    def from_file(cls, filename: str, run: Run):
        config = toml.load(filename)

        inherited = []
        for k, v in run.items():
            inherited.append(dict(config[k.name.lower()][v]))
            del config[k.name.lower()]

        return cls(run, config, inherited)

    def get_pias_threshold(self, load: float, idx: int) -> int:
        return self.get_load_specific_var(f'pias_thresh_{idx}', load) * PACKET_SIZE

    @property
    def load_arr(self):
        return self['load_arr']

    @property
    def num_servers(self) -> int:
        return int(self['topology_spt']) * int(self['topology_tors'])

    def get_arrival_rate(self, load: float, mean_flow_size_in_bytes: int):
        return float(self['link_rate']) * load * 10**9 / (mean_flow_size_in_bytes * 8 / 1460 * 1500)

    def get_flow_arrival_command(self, load: float) -> str:
        arr_conf = ArrivalsConfig(self._get_sub_config('arrivals', load))
        server_arrival_rate = self.get_arrival_rate(load, arr_conf.mean_flow_size_in_bytes) / (self.num_servers - 1)
        return f"set_{arr_conf.process_code_name}arrival_process {server_arrival_rate} {arr_conf.args}"

    @property
    def plot_generation_bounds(self):
        return self['plot_generation_bounds'].split() if 'plot_generation_bounds' in self else []

    @property
    def priority_scheme(self):
        return [PriorityScheme[x.upper()] for x in self['prio_scheme_arr'].split()]

    @property
    def packet_properties_assignment_json(self) -> Optional[dict]:
        packet_properties_assignment_config = PacketPropertiesAssignerConfig(
            self._get_sub_config('packet_properties_assignment'))
        return packet_properties_assignment_config.json

    def get_delay_assignment_jsons(self, load: float) -> List[dict]:
        delay_assignment_config = DelayAssignmentConfig(
            self._get_sub_config('delay_assignment', load), float(self['link_rate']), int(load * 100))
        return delay_assignment_config.get_jsons()

    def _get_sub_config(self, prefix, load: Optional[float] = None) -> dict:
        return {k.replace(f'{prefix}_', ''):
                self.get_load_specific_var(k, load) if load is not None else self[k]
                for k in self.keys() if k.startswith(f'{prefix}_')}

    def get_load_specific_var(self, var: str, load: float) -> Any:
        if isinstance(self[var], list):
            return {d: v for d, v in zip(self.DELAYS, self[var])}[load]
        else:
            return self[var]

    def keys(self) -> Set[str]:
        return functools.reduce(
            lambda x, y: x | y,
            [self._default.keys()] + [inh.keys() for inh in self._inherited],
            set())

    def __getitem__(self, item: str) -> Any:
        inherited = [inh[item] for inh in self._inherited if item in inh]
        if len(inherited) == 1:
            return inherited[0]
        if len(inherited) > 1:
            raise KeyError(f'key {item} is inherited from multiple bases')
        return self._default[item]

    def get(self, item: str, default=None):
        try:
            return self[item]
        except KeyError:
            return default

    def __contains__(self, item):
        return any(item in inh for inh in self._inherited + [self._default])

    @property
    def run_name(self):
        return _FNAME_SEPARATOR.join(self._run[aspect] for aspect in Aspect)

    @property
    def custom_script(self):
        return self.get('custom_script', None)


def config_enumerator(results_dir: str,
                      config: Config
                      ) -> List[Tuple[float, List[Tuple[str, dict]]]]:
    result = []
    for load in config.load_arr:
        delay_assignment_jsons = config.get_delay_assignment_jsons(load)
        values = []
        for i, delay_assignment_json in enumerate(delay_assignment_jsons):
            aux_id = os.path.basename(config.custom_script) + _FNAME_SEPARATOR if config.custom_script else ''
            directory_name = path.join(
                results_dir,
                f'{aux_id}{config.run_name}'.lower()
                + (f'{_FNAME_SEPARATOR}{i}' if len(delay_assignment_jsons) > 1 else '')
                + _FNAME_SEPARATOR
                + f'{int(load * 100)}'
            )
            values.append((directory_name, delay_assignment_json))
        result.append((load, values))

    return result


def _strip_prefix(text: str, prefix: str) -> str:
    if text.startswith(prefix):
        return text[len(prefix):]
    else:
        raise ValueError(f'"{text}" does not have prefix "{prefix}"')
