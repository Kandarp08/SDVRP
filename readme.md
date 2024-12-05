# Implementation of "An Efficient Iterated Local Search Heuristic for the Split Delivery Vehicle Routing Problem"

## Introduction

This project was completed as part of the course *GNL 801 - Optimization* by a team of three members. It implements the paper **"An Efficient Iterated Local Search Heuristic for the Split Delivery Vehicle Routing Problem"** by Weibo Lin, Zhu He, Shibiao Jiang, Fuda Ma, and Zhipeng Lu.  

The full paper is available in the file `./ref/Split Delivery VRP (Algorithm).pdf`. We have followed the pseudocode and methods described in the paper, along with the references cited within it.



## Prerequisites
To run this project, ensure the following tools are installed on your system:

- **Make**: You can install it by running:
  ```bash
  sudo apt update
  sudo apt install make

## Project Structure

```bash
.
├── src/                       # Source code directory
│   ├── intra_operator/        # Intra-operator implementations
│   ├── inter_operator/        # Inter-operator implementations
│   └── *.cpp                  # Other source files
├── data/                      # Input data files for test cases
├── output/                    # Directory for output files
├── main.cpp                   # Entry point for the program
├── Makefile                   # Build automation file
└── README.md                  # Project documentation
```


## How to Build and Run

#### Build the Project
Run the following command to compile the project:
```bash
make build
```

#### Run All Test Cases
To execute all test cases (from 1 to 21), run:
```bash
make run-all
```

#### Run a Range of Test Cases
To execute a range of test cases, run:
```bash
make run-range
```

Enter the range when prompted (e.g., ```5 10```).
#### Run a Single Test Case
To execute a specific test case, run:
```bash
make run-one
```

Enter the test case number when prompted (e.g., ```7```).
#### Clean the Build
To remove compiled files and output files, run:
```bash
make clean
```


## Adding Test Cases
- Place the input files for the test cases in the data/ directory. Files should follow the naming convention: SD1.txt, SD2.txt, ..., SD21.txt.
- The program reads these files based on the specified test case number.


## Output
- Results are saved in the output/ directory.
- Each test case produces an output file named solution<number>.txt (e.g.,solution1.txt).


## Authors

- **IMT2022086** - Ananthakrishna K  
- **IMT2022103** - Anurag Ramaswamy  
- **IMT2022115** - Kandarp Dave  

> Note: The authors are listed in the order of their roll numbers, not in the order of their contributions. Each author has contributed equally to the project.



