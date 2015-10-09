# kaleidoscope
A kaleidoscope programming language compiler base on LLVM



## Build

### 1. install llvm (3.6.2 and above)

+ for OS X (with [Homebrew](http://brew.sh) and [Xcode](http://www.baidu.com/link?url=jy41fRcYpq3-1tgPOTReo6HIM68bOBb7SqQvgmJ9b0CpRJO61P1xJCBEwxPmZ8GK76fykKOzQ6T0P7viJxELFa&wd=&eqid=ff1b1fe30000a6ba000000045617a315) installed)

> Old version homebrew can't download llvm36,run `brew update` first.
> 
> If compile error, try to install llvm 3.7.0 manural

```bash
brew install llvm36
```

+ for UNIX/Linux manural ([3.7.0 Release Build](http://llvm.org/releases/download.html#3.7.0))

```bash
cd /usr/local/Cellar/
curl "http://llvm.org/releases/3.7.0/clang+llvm-3.7.0-x86_64-apple-darwin.tar.xz"
tar -xvf ./clang+llvm-3.7.0-x86_64-apple-darwin.tar.xz
mv clang+llvm-3.7.0-x86_64-apple-darwin.tar.xz llvm/3.7.0
cd /usr/local/opt
ln ../Cellar/llvm/3.7.0 llvm
```

and then add to your `PATH`.

> for OS X, because Xcode built-in clang-llvm, so you must add it to path or brew link to do that
> 
> for Linux, just add it to PATH

```bash
export PATH=$PATH:/usr/local/opt/llvm
```
or

```bash
brew link llvm --force
```

### 2. Build

```bash
clang++ -g -O3 main.cpp `llvm-config --cxxflags --ldflags --system-libs --libs core` -o main
./main
```

### 3. Test

```bash
cd ./test/
clang++ -g -O3 toy.cpp `llvm-config --cxxflags --ldflags --system-libs --libs core` -o toy
./toy
```
## Grammar

```ks
# Compute the x'th fibonacci number.
def fib(x)
  if x < 3 then
    1
  else
    fib(x-1)+fib(x-2)

# This expression will compute the 40th number.
fib(40)


# Call standard library use 'extern' keyword
extern sin(arg);
extern cos(arg);
extern atan2(arg1 arg2);

atan2(sin(.4), cos(42))
```

## Path
1. `doc`: language grammer, doc, example code,etc
2. `test`: standard compiler ,test
3. `main.cpp`: now just put all code into single main.cpp file