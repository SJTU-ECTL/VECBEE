# Batch Error Estimation for Approximate Logic Synthesis
It is an approximate logic synthesis system,
which applies the batch error estimation technique[1] on the single selection method[2].

The project is an extension of [_ABC_](https://people.eecs.berkeley.edu/~alanmi/abc/)
(A System for Sequential Synthesis and Verification).

For more details, please refer to:

- [1]: [_Efficient Batch Statistical Error Estimation for Iterative Multi-level Approximate Logic Synthesis_](http://umji.sjtu.edu.cn/~wkqian/papers/Su_Wu_Qian_Efficient_Batch_Statistical_Error_Estimation_for_Iterative_Multi_level_Approximate_Logic_Synthesis.pdf)

- [2]: [_An efficient method for multi-level approximate logic synthesis under error rate constraint_](http://umji.sjtu.edu.cn/~wkqian/papers/Wu_Qian_An_Efficient_Method_for_Multi-level_Approximate_Logic_Synthesis_under_Error_Rate_Constraint.pdf)

## Requirements
To compile [_ABC_](https://github.com/berkeley-abc/abc), you need:
- gcc
- make
- libreadline

To compile the project, further need:
- libboost

## Getting Started
### Build _ABC_
In Ubuntu, you can simply run:
```
sudo chmod +x configure.sh
```
```
sh configure.sh
```
It will automatically clone _ABC_ to `./abc/`,
compile it and generate a static library.

### Build Project
To generate a executable program, just use:
```
make
```
And it will generate a program named `main`.

Use `./main -?` to get help.
- usage: ./main [options] ...
- options:
-   -f, --file      Circuit file (string [=data/sop/c432.blif])
-   -g, --genlib    Map library file (string [=data/genlib/mcnc.genlib])
-   -e, --error     Error rate (float [=0.05])
-   -n, --number    Frame number (int [=1024])
-   -?, --help      print this message

The benchmarks are saved in `BLIF` format in the folder `./data/gate/` and `.data/sop/`,
which are circuit in gate netlist style and SOP style, respectively.

The standard cell libraries are saved in `GENLIB` format in the folder `./data/genlib/`.

You can change the error rate for approximate computing with the argument `-e`,
and set the simulation frame number with the argument `-n`.
