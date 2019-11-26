# ckb-duktape

[mruby](http://mruby.org/) build for CKB environment

# Build note

Since CKB VM doesn't have MMU available, a different solution to sbrk() should be leveraged to make sure a C script running in CKB VM can use malloc(). We have a patched [libc](https://github.com/nervosnetwork/riscv-newlib) serving as a PoC showing how this can be done but you are more than welcome to practice a different solution.

This means the easiest way to build this repository, is to use the GNU toolchain provided in this [docker image](https://hub.docker.com/r/nervos/ckb-riscv-gnu-toolchain). We have packed the GNU toolchain together with our modified libc, so everything should work out of the box. Notice that CKB VM doesn't require any modifications to gcc itself, so you should also be able to just use upstream gcc, and customize the libc used in linking phase.

Or if you don't want to read, a simple way to build this project is:

```bash
$ git clone --recursive https://github.com/nervosnetwork/ckb-mruby
$ cd ckb-mruby
$ sudo docker run --rm -it -v `pwd`:/code nervos/ckb-riscv-gnu-toolchain:bionic-20191012 bash
root@982d1e906b76:/# apt-get install -y ruby
root@982d1e906b76:/# cd /code
root@982d1e906b76:/code# make
root@982d1e906b76:/code# exit
```

Now you will have 2 binaries in `build` folder:

* `entry` can be used to load a Ruby script and run it on CKB.
* `repl` can be used to start a Ruby REPL to play with CKB in the exact [same way](https://xuejie.space/2019_10_18_introduction_to_ckb_script_programming_debugging/) as [ckb-duktape](https://github.com/nervosnetwork/ckb-duktape).
