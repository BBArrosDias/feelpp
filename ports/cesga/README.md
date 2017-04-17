# Building

## Full install.

To compile all librairies just type

```
./install.sh
```

You can modify the profile name.

It will create 3 directories
- `src/`          compile libraries here (clean after install)
- `install/`      install libraries here
- `archive/`      keep an archive of install here

The `environment` files contains all install/compilation path.
The `modules` files contains all modules to be loaded.
You can source this file in your bashrc.


## Install per library

To install by hand a library do for example for petsc

```
./petsc.sh gcc630
```
where gcc630 is the profile name.
To remove install files for a library+profile, it will remove all files in
directories `src/` and `install/profile/`

```
./petsc.sh -r gcc630
```



