# Configuration File

Below is a list of the available configuration options for Revizor, which are passed down to Revizor via a config file.
For an example of how to write the config file, see [src/tests/big-fuzz.yaml](src/tests/big-fuzz.yaml).
 
Some of the options are omitted from the list as they should not be used in a normal fuzzing campaign.
For a complete list, see `src/config.py`.

## Contract

* `contract_execution_clause` : List[str]: Execution clause.
  Available options: "seq", "cond", "bpas", "null-injection".
* `contract_observation_clause` [str]: Observation clause.
  Available options: "l1d", "memory", "ct", "pc", "ct-nonspecstore", "ctr", "arch".

## General Configuration

* `no_priming` [bool]: Disable priming.
* `logging_modes` List[str]: Verbosity of the output.
  Available options:
  `info` - general information about the progress of fuzzing;
  `stat` - statistics the end of the fuzzing campaign;
  `fuzzer_debug` - detailed information about the fuzzing progress and about the detected vulnerabilities;
  `fuzzer_trace` - very detailed information about the fuzzing progress (not recommended);
  `model_debug` - detailed information about the execution of each test case on the model;
  `coverage_debug` - detailed information about the changes in coverage.
* `multiline_output` [bool]: Print each output message on a separate line.
  Preferred when piping the log into a file.

# Model Configuration

* `model` [str]: Model type.
  Only one option is currently supported - "x86-unicorn" (default).
* `model_max_nesting` [int]: Maximum number of simulated mispredictions.
* `model_max_spec_window` [int]: Size of the speculation window.

# Generator Configuration

* `instruction_set`  [str]: Tested ISA.
  Only one option is currently supported - "x86-64" (default).
* `generator` [str]: Test case generator type.
  Only one option is currently supported - "random" (default).
* `test_case_generator_seed` [int]: Test case generation seed.
  Will use a random seed if set to zero.
* `min_bb_per_function` [int]: Minimum number of basic blocks per test case.
* `max_bb_per_function` [int]: Maximum number of basic blocks per test case.
* `test_case_size` [int]: Number of instructions per test case.
  The actual size might be larger because of the instrumentation.
* `avg_mem_accesses` [int]: Average number of memory accesses per test case.
* `supported_categories` [list(str)]: List of instruction categories to be used when generating a test case.
  Used to filter out instructions from the instruction set file passed via command line (`--instruction-set`).
* `extended_instruction_blocklist` [list(str)]: List of instructions to be excluded by the generator.
  Used to filter out instructions from the instruction set file passed via command line (`--instruction-set`).

# Input Generator Configuration

* `input_generator` [str]: Input generator type.
  Only one option is currently supported - "random" (default)
* `input_gen_seed` [int]: Input generation seed.
  Will use a random seed if set to zero.
* `input_gen_entropy_bits` [int]: Entropy of the random values created by the input generator.
* `inputs_per_class` [int]: Number of inputs per input class.

# Executor Configuration

* `executor` [str]: Executor type.
  Only one option is currently supported - "x86-intel" (default).
* `executor_mode` [str]: Hardware trace collection mode.
  Available options: 'P+P' - prime and probe; 'F+R' - flush and reload; 'E+R' - evict and reload.
* `executor_warmups` [int]: Number of warmup rounds executed before starting to collect hardware traces.
* `executor_repetitions` [int]: Number of repetitions while collecting hardware traces.
* `executor_taskset` [int]: CPU number on which the executor is running test cases.
* `enable_ssbp_patch` [bool]: Enable a patch against Speculative Store Bypass (Intel-only).
  Enabled by default.
* `enable_pre_run_flush` [bool]: If enabled, the executor will do its best to flush the microarchitectural state before running test cases.
  Enabled by default.
* `enable_faulty_page` [bool]: If enabled, only of the sandbox memory pages will have the accessed bit set to zero, which will cause a microcode assist on the fist load/store to this page.
  Disabled by default.

# Analyser Configuration

* `analyser` [str]: Analyser type.
  Only one option is currently supported - "equivalence-classes" (default).
* `analyser_permit_subsets` [bool]: If enabled, the analyser will not label hardware traces as mismatching if they form a subset relation.
  Enabled by default.

# Coverage Configuration

* `coverage_type` [str]: Coverage type.
  Available options:
  'none' - disable coverage tracking;
  'dependent-pairs' - coverage of pairs of dependent instructions.
