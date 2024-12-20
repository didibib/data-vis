# SimpleMatrix [![Build Status](https://travis-ci.org/wq2012/SimpleMatrix.svg?branch=master)](https://travis-ci.org/wq2012/SimpleMatrix)


## About
SimpleMatrix is an extremely lightweight matrix library, containing a single
header file.

* It uses the namespace `smat`.
* The `Matrix` class is a template class.
* It implements basic matrix representations and operations, such as
multiplication, transpose, and submatrix.
* It does **NOT** implement complicated operations such as inverse, determinant,
eigenvector, or decompositions.
* It implements the Multidimensional Scaling (MDS) algorithm.

This library is developed as part of the MDS Feature Learning project.

* The wiki site of the SimpleMatrix C++ library:
  https://sites.google.com/site/simpmatrix/
* The wiki site of the MDS Feature Learning project:
  https://sites.google.com/site/mdsfeature/

To give credit to this library, you may cite:

```
@inproceedings{wang2013feature,
  title={Feature learning by multidimensional scaling and its applications in object recognition},
  author={Wang, Quan and Boyer, Kim L},
  booktitle={Graphics, Patterns and Images (SIBGRAPI), 2013 26th SIBGRAPI-Conference on},
  pages={8--15},
  year={2013},
  organization={IEEE}
}
```

![swiss](resources/swiss.png)

## Files

### SimpleMatrix.h
* This is an extremely lightweight matrix library, containing a single header file.
* It uses the namespace `smat`.
* The `Matrix` class is a template class.
* It implements basic matrix representations and operations, such as multiplication, transpose, and submatrix.
* It does **NOT** implement complicated operations such as inverse, determinant, eigenvector, or decompositions.
* It implements the Multidimensional Scaling (MDS) algorithm with two versions: the UCF version and the SMACOF version.

### demo_Matrix.cpp
* This is a demo showing how to use this library, including reading matrix from txt file, matrix multiplication, and MDS.
* A Makefile is provided to compile this demo with g++.

### visualize_MDS_results.m
* This is a MATLAB script for showing the MDS results of running `demo_Matrix.exe`.
* The `demo_Matrix.exe` performs MDS on the matrix in s`wissD.txt`, which is the geodesic distance matrix of a Swiss roll surface.
* The `swissX0.txt` contains the initialization matrix for the Swiss roll flattening problem.
* The MDS results are saved in `swissX1.txt`, `swissX2.txt`, `swissX3.txt` and `swissX4.txt`.
* `swissroll.mat` stores data for the visualization.


## Copyright

```
Copyright (C) 2013 Quan Wang <wangq10@rpi.edu>
Signal Analysis and Machine Perception Laboratory
Department of Electrical, Computer, and Systems Engineering
Rensselaer Polytechnic Institute, Troy, NY 12180, USA
```