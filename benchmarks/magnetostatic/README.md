# Benchmark files for precAFP

Provided
- 2D/3D
- various (constant in space) permeability
- various test case (sinusaïdale and polynomiale)
- script to generate batch (for Atlas) [`batchGenerator`](batchGenerator.sh)
- Script to parse resultst [`logParser`](logParser.sh)
- latex file that read parsed results [displayRes](displayRes.tex)

# Regularized problem

The AMS preconditioner internally solve a regularized problem (block 11).
This it the one that *crash* in some configuration.
I reproduce here a simulation where the regularized simulation **can** *crash*

##### Equation
We solve
```
a (curl, curl) + b (id, id) = c (f, id)
```
To reproduce the configuration that is problematic, I have to set (see config file `regul.cfg`)
- a = 1
- b = 5e-3
- c = 5e-3 (or whatever actually)

##### Solver
We use `CG/LU` to solve the problem.
LU to exactly solve the system
CG to check if PETSc warn about the non SDP property.
In every article I read, CG is the ksp to solve the regularized problem.

In `parallel hierarchical matrix preconditioners for the curl-curl operator` (See DropBox - Bebendorf-magnetostatic-parallel-precond.pdf), we found a demonstration that the matrix is definite positive for `b > 0` from equation (2.3).
My understanding for that particular article is that Bebendorf set `b > a` to obtain fast results, but the matrix is SDP for `b>0`.

##### Geometry
I provide two meshes that are generated from the same `.geo` file.
It was actually generated with a Feel++ executable (`loadMesh...`).

We do not have an analytical solution for the following configuration even if CT can produce an evaluation of the solution)

##### Compilation/Execution
In the configuration - to check what pc/ksp is used - I have set `pc-view=1`.

**The meshes are available in DropBox: TheseDaversin/FerroMagnetism/Preconditioner/meshToRemove `

```sh
make feelpp_test_regul3D
./feelpp_test_regul3D --config-file regul.cfg --gmsh.filename torus_quart_Working.msh
./feelpp_test_regul3D --config-file regul.cfg --gmsh.filename torus_quart_NotWorking.msh
./feelpp_test_regul3D --config-file regul.cfg --gmsh.filename torus_quart_NotWorking.msh --ms.kps-type=gmres
```

##### Observed behavior
With the `torus_quart_Working.msh` mesh, the simulation goes well (PETSc does not warn the matrix or the PC is indefinite OR negative definite).
It is not the case with `torus_quart_Working.msh` which produce, for various norm evaluation :
- unpreconditioned norm -> diverging due to indefinite preconditioner
- preconditioned norm -> diverging due to indefinite or negative definite matrix 
- natural -> diverging due to indefinite preconditioner

