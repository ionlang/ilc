#### ilc

ilc (ionlang command-line utility) is a CLI tool for compiling and manipulating Ion & IonIR code.

#### Building

Requirements:

* [ionir's dependencies](https://github.com/ionlang/ionir#requirements)
* CMake
* C++ compiler

#### Usage

#### Options

1. Interactive mode **[-i, --interactive]**

Use interactive mode to process Ion or IonIR interactively within the same terminal instance.
Interactive mode will allow for a continuous input prompt within your current terminal instance.

Example:

```shell
$ ilc --interactive
```

2. Output file **[-o, --out]**

Specify an output file path onto which write result(s). If omitted, the filename of the input file
provided will be used along with the IonIR file extension: `.ix`.

Example:

```shell
$ ilc input.ion --out output.ix
```

#### Flags

1. Target IonIR **[-r, --ir]**

Informs the compiler that the input source code provided is IonIR.

*More information pending.*

#### Common problems

* *Imported target "ionir::ionir" includes non-existent path "/ionir"*

1. Delete all installation directories (from both `Program Files` and `Program Files (x86)` of `ionir`).
2. Re-install the `ionir` project.
3. Reload the project.
