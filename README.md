# HamArc
`hamarc` is a flexible archiver that leverages Hamming error-correcting codes to preserve integrity of archived files. Each file is splitted into blocks of configurable size and each block is supplemented with error-correcting (or _control_) bits according to the Hamming algorithm. It allows to find and correct single-bit errors and report 2-bit errors in each block when the archive is unpacked.

The project also includes my self-written `argparser` library to take command-line parameters from the user.

> This project was completed as part of my undergraduate C++ course. Also, it is my first-year project, so the code might need some refactoring.

## Archive format
An archiver uses `.haf` (Hamming Archive Format) binary file format to store multiple items added to the archive. It stores file metadata (such as its name, size and encoding block size) protected by control bits and file content. Files are stored consecutively: first metadata, then file content and then all control bits associated with content blocks. `.haf` file is required to be non-empty, i. e. contain at least one file (which may be empty). The precise scheme is as follows:

```
Meta: 
    <4 bytes: file name size in bytes><8 bytes: file content size in bytes>
    <8 bytes: block size in bytes><ctl><file name><ctl for file name>

.haf:
    [<Meta><file content><ctl for file content>]+

```
(here `ctl` refers to control bits).

## Usage
### Build and run
To build with `cmake`, use:
```shell
$ cmake -S . -B build
$ cmake --build build
```

Display command line options:
```shell
$ build/hamarc --help
hamarc
Hamming-based archiver

        <string>,       Files to process [repeated, min args = 0]
-f,     --file=<string>,        An archive file
-D,     --directory=<string>,   Override working directory
-A,     --concatenate,  Merge archives [default = false]
-a,     --append,       Append files to an archive [default = false]
-x,     --extract,      Extract specified files (all, if no files specified) [default = false]
-l,     --list, List files in archive [default = false]
-d,     --delete,       Delete files from an archive [default = false]
-c,     --create,       Create an archive [default = false]

-h,     --help, Display this help and exit
```

### Tests
To launch tests, use:
```shell
$ build/hamarc_tests
<...>
[==========] 21 tests from 4 test suites ran. (153 ms total)
[  PASSED  ] 21 tests.
```
This test suite tests archiver components against manually processed examples, which include error correction checking.
