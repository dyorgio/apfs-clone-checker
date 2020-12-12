# apfs-clone-checker
An utility to check if two files are clones in macOs APFS.


## How it works (and motivation)

Some years ago (2017-09-26) I asked in [stackoverflow](https://stackoverflow.com/questions/46417747/apple-file-system-apfs-check-if-file-is-a-clone-on-terminal-shell) and in [Apple Developer Forum](https://developer.apple.com/forums/thread/81100) if there is a way to identify if a file is a clone.

3 years later, no response or update of macOs tools to get this.

My motivation, like others who also want an answer ( I guess :smile: ), is create a tool that analyze entire disk and create clones of files with same content. (use clone APFS feature at maximum possible).

Many tools are created in this space of time, but all of then works creating hash of file content (expensive operation).

This tool use another aproach: file blocks physical location.

All cloned files point to the same blocks, this utility get this info from both files and compare.

It also make some validations and fast checking:
* Both files are in an APFS device.
* Files are in same device.(CLONE only supported in same device)
* Files are normal files.
* Files are not the same.
* Files have same size and blocks count.

In initial tests it can speedup clone checking in at least 200% (compared with shasum or md5) full file verification.

But the optimization is much better for no full cloned files, it can stops on first different block, and it can be the first one :nerd_face:.

## Usage
```.sh
./clone_checker [-fqv] pathOfFileA pathOfFileB
```
If exit code 0 (OK) than print in stdout:
* 1 = Clones
* 0 = Not clones (maybe partial if -q option is used)

If exit code not 0 (NOK) than:
* Print in stderr what was the problem.

## Options

* -f (forced mode): Ignore read/validation errors and return 0 (not clones).
* -q (quick mode): Just verify first and last blocks (fast, but not 100%).
* -v: Print version.
* -?,h: Print usage.

## Compilation
Have gcc installed (XCode).<br>
Copy clone_checker.c (or content) to your computer.<br>
Run gcc:
```.sh
gcc clone_checker.c -o clone_checker
```
Optional (mark binary executable):
```.sh
chmod +x clone_checker
```

## Work TODO
* Investigate about directory cloning.
* ~~Investigate how to get j_inode_flags and check INODE_WAS_CLONED flag (optimization) [Apple APFS Reference](https://developer.apple.com/support/downloads/Apple-File-System-Reference.pdf)~~ 2020-12-12 - To get it is necessary to use reverse engineer and parse all device fs structure.
* Support to percentual of clone mode (A clone can have only some blocks altered).
