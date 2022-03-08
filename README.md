# VECBEE: A Versatile Efficiency-Accuracy Configurable Batch Error Estimation Method for Greedy Approximate Logic Synthesis

This project implements an efficient batch error estimation method for approximate logic synthesis.
The error estimation method is applied into two local approximate changes,
that is, SASIMI and ANS.

For more details, please refer to:

- [1]: [VECBEE: A Versatile Efficiency-Accuracy Configurable Batch Error Estimation Method for Greedy Approximate Logic Synthesis](https://ieeexplore.ieee.org/document/9706238)

- [2]: [_Efficient Batch Statistical Error Estimation for Iterative Multi-level Approximate Logic Synthesis_](http://umji.sjtu.edu.cn/~wkqian/papers/Su_Wu_Qian_Efficient_Batch_Statistical_Error_Estimation_for_Iterative_Multi_level_Approximate_Logic_Synthesis.pdf)

- [3]: [_An efficient method for multi-level approximate logic synthesis under error rate constraint_](http://umji.sjtu.edu.cn/~wkqian/papers/Wu_Qian_An_Efficient_Method_for_Multi-level_Approximate_Logic_Synthesis_under_Error_Rate_Constraint.pdf)

## Requirements
To compile the project, you need:
- gcc 10.3.0
- g++ 10.3.0
- cmake 3.16.3
- make 4.2.1
- libboost 1.75.0
- ctags
- libreadline
- abc

A reference docker environment is also provided here:
https://hub.docker.com/repository/docker/changmeng/als_min

## Download the project
```
git clone https://github.com/SJTU-ECTL/VECBEE.git --recursive
```
The `--recursive` option is necessary to download the submodule `abc`.

## Sub-project: SASIMI-VECBEE
Generate a executable program:
```
cd VECBEE/SASIMI-VECBEE
mkdir build
cd build
cmake ..
make -j <number_of_cpu_cores>
cd ..
```

A program named `sasimi-vecbee.out` will be generated in the folder `SASIMI-VECBEE`.

Use `./sasimi-vecbee.out -h` to get help.

## Sub-project: ANS-VECBEE
Generate a executable program:
```
cd VECBEE/ANS-VECBEE
mkdir build
cd build
cmake ..
make -j <number_of_cpu_cores>
cd ..
```

A program named `ans-vecbee.out` will be generated in the folder `ANS-VECBEE`.

Use `./ans-vecbee.out -h` to get help.
