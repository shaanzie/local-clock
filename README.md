# LoRaWAN ns-3 module

This is an [ns-3](https://www.nsnam.org "ns-3 Website") module that can be used
to instantiate node-local clocks on ns-3 that exhibit skews. We also provide examples 
of different clocks that can be used to provide different clock properties to a simulation.
Scheduling is still done through the Simulator, and is purely for observing the clocks. 
A model for scheduling is being worked on in [MR2619](https://gitlab.com/nsnam/ns-3-dev/-/merge_requests/2619).

Quick links:

* [Paper Reference](https://dl.acm.org/doi/pdf/10.1145/3747204.3747208)

## Getting started

### Prerequisites

To run simulations using this module, you first need to install ns-3. If you are on Ubuntu/Debian/Mint, you can install the minimal required packages as follows:

```bash
sudo apt install g++ python3 cmake ninja-build git ccache
```

Otherwise please directly refer to the [prerequisites section of the ns-3 installation page](https://www.nsnam.org/wiki/Installation#Prerequisites).

> Note: While the `ccache` package is not strictly required, it is highly recommended. It can significantly enhance future compilation times by saving tens of minutes, albeit with a higher disk space cost of approximately 5GB. This disk space usage can be eventually reduced through a setting.

Then, you need to:

1. Clone the main ns-3 codebase,
1. Clone this repository inside the `src` directory therein, and
1. Checkout the current ns-3 version supported by this module.

To install this module at the latest commit, you can use the following all-in-one command:

```bash
git clone https://gitlab.com/nsnam/ns-3-dev.git && cd ns-3-dev &&
git clone https://github.com/shaanzie/local-clock src/local-clock &&
tag=$(< src/local-clock/NS3-VERSION) && tag=${tag#release } && git checkout $tag -b $tag
```

**Note**: When switching to any previous commit, *including the latest release*, always make sure to also checkout ns-3 to the correct version (`NS3-VERSION` file at the root of this repository) supported at that point in time.

### Compilation

Ns-3 adopts a development-oriented philosophy. Before you can run anything, you'll need to compile the ns-3 code. You have two options:

1. **Compile ns-3 as a whole:** Make all simulation modules available by configuring and building as follows (ensure you are in the `ns-3-dev` folder!):

   ```bash
   ./ns3 configure --enable-tests --enable-examples &&
   ./ns3 build
   ```

1. **Focus exclusively on the lorawan module:** To expedite the compilation process, as it can take more than 30/40 minutes on slow hardware, change the configuration as follows:

   ```bash
   ./ns3 clean &&
   ./ns3 configure --enable-tests --enable-examples --enable-modules lorawan &&
   ./ns3 build
   ```

   The first line ensures you start from a clean build state.

Finally, ensure tests run smoothly with:

```bash
./test.py
```

If the script reports that all tests passed you are good to go.

If some tests fail or crash, consider filing an issue.

## Usage examples

The module includes the following examples:

* `unbounded-skew-clock-example`
* `lamport-logical-clock-example`
* `vector-clock-example`
* `hybrid-logical-clock-example`
* `replay-clock-example`

Examples can be run via the `./ns3 run example-name` command (refer to `./ns3 run --help` for more options).

## Documentation

* Local Clock Model Overview - A description of the foundational models of this module is located at `doc/local-clock.rst`.

Other useful documentation sources:

* [Ns-3 tutorial](https://www.nsnam.org/docs/tutorial/html/ "ns-3 Tutorial"): **Start here if you are new to ns-3!**
* [Ns-3 manual](https://www.nsnam.org/docs/manual/html/ "ns-3 Manual"): Overview of the fundamental tools and abstractions in ns-3.

## Getting help

To discuss and get help on how to use this module, you can open an issue here.

## Contributing

Refer to the [contribution guidelines](.github/CONTRIBUTING.md) for information
about how to contribute to this module.

## Authors

* Ishaan Lagwankar

## License

This software is licensed under the terms of the GNU GPLv2 (the same license
that is used by ns-3). See the LICENSE.md file for more details.

## Acknowledgments and relevant publications

The initial version of this code was developed as part of a conference submission to ICNS3 2025.

Publications:

* Ishaan Kiran Lagwankar and Sandeep S. Kulkarni. 2025. Clock Skew Models for ns-3. In Proceedings of the 2025 
International Conference on ns-3 (ICNS3 '25). Association for Computing Machinery, New York, NY, USA, 
70–81. https://doi.org/10.1145/3747204.3747208