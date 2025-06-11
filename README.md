# Key Value DB

*A persistent networked key/value store written on top of the [GSDK](https://github.com/Jacob-C-Smith/gsdk)*

<p align="center">
    <img src="https://img.shields.io/badge/language-C-blue.svg" alt="Language: C">
    <img src="https://img.shields.io/badge/build-make-green.svg" alt="Build: make">
    <img src="https://img.shields.io/badge/license-MIT-lightgrey.svg" alt="License: MIT">
</p>

> **Jump to:**  
> [Features](#features) • [Getting Started](#getting-started) • [Usage](#usage) • [Options](#options) • [Comamnds](#commands) • [Files](#files) • [Build](#build) • [License](#license)

---

## Features
TODO

## Getting started
To download the project, run
```bash
$ git clone https://github.com/Jacob-C-Smith/key_value_db
```

## Usage
```bash
$ ./key_value_db [-h | --help] [-f | --file <path=tmp.db>] [-p | --port <port_number=6710>] [-t | --threads <quantity=4>]
```

## Options
| Option        | Description                                         |
| ------------- | --------------------------------------------------- | 
| -h, --help    | Prints a help message                               |
| -f, --file    | Load a database from a file                         |
| -p, --port    | Use a specific port number                          |
| -t, --threads | Specifies the upper limit of concurrent connections |

## Commands

| Command | Syntax                              | Description                                      |
| ------- | ----------------------------------- | ------------------------------------------------ |
| Get     | ``` get <key> ```                   | Access a singe value from it's corresponding key |
| Get All | ``` getall <key> [limit] [start]``` | Access a range of values from a prefix key       |
| Set     | ``` set <key> <json-value> ```      | Create / Update a property                       | 
| List    | ``` list [depth]```                 | List all properties in the database              |
| Write   | ``` write [path/to/file.db] ```     | Flush the database                               |
| Exit    | ``` exit ```                        | Close the application                            |

## Files

| Folder                       | Description             |
| ---------------------------- | ----------------------- |
| [main.c](./main.c)           | Application entry point |
| [key_value.c](./key_value.c) | Database implementation |
| [key_value.h](./key_value.h) | Database interface      |
| [Makefile](./Makefile)       | Makefile                |
| [gsdk](./gsdk/README.md)     | GSDK submodule          |
| [README.md](./README.md)     | This file               |

## Build
To build the project, run
```bash
$ make
```

## License
See [LICENSE](./LICENSE) for details.
