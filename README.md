# Simulation environment for the IFIP Networking 2021 paper "How to network delay-sensitive applications"

Assuming, the repo has been cloned.

## Build instructions

  * compile _and install_ library from `delay_transport` dir (use CMake)

  * compile _and install_ NS2 simulator from `ns2` dir (use CMake)

  * go to `scripts` dir, build _and install_ the python package inside

## Run instructions

The `run` command (from the python scripts) should be in the environment, using
the `--help` argument you can find the extensive list of options it can run
with.

__NOTE__ the `run` command must be from a directory containing `*.tcl` and
the `config.toml` file (all found in `scripts` dir).
