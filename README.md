# VECBEE: A Versatile Efficiency-Accuracy Configurable Batch Error Estimation Method for Greedy Approximate Logic Synthesis

This project implements an efficient batch error estimation method for approximate logic synthesis.
The error estimation method is applied into two local approximate changes,
that is, SASIMI and ANS.

For more details, please refer to:

- [1]: [_Efficient Batch Statistical Error Estimation for Iterative Multi-level Approximate Logic Synthesis_](http://umji.sjtu.edu.cn/~wkqian/papers/Su_Wu_Qian_Efficient_Batch_Statistical_Error_Estimation_for_Iterative_Multi_level_Approximate_Logic_Synthesis.pdf)

- [2]: [_An efficient method for multi-level approximate logic synthesis under error rate constraint_](http://umji.sjtu.edu.cn/~wkqian/papers/Wu_Qian_An_Efficient_Method_for_Multi-level_Approximate_Logic_Synthesis_under_Error_Rate_Constraint.pdf)

## Requirements
To compile both sub-projects SASIMI and ANS, you need:
- gcc
- g++
- make
- ctags

To compile the ANS sub-project, you also need:
- libreadline
- libboost

The ANS sub-project also requires ABC.

## SASIMI
The sub-project SASIMI is very easy to compile and use.
```
cd SASIMI
make
./main < circuit-name > (i.e., ./main c880)
```

## ANS
The sub-project ANS requires ABC.
Clone `ABC` into `ANS/abc/`.
```
cd ANS
git clone git@github.com:berkeley-abc/abc.git
```

Build a static library `libabc.a`.
```
cd abc/
make libabc.a
cd ..
```

Generate a executable program:
```
make
```
It will generate a program named `main`.

Use `./main -h` to get help.
