## Developers Documentation

### General information

Developing software on microcontrollers requires a lot of software tools and is generally
more complicated than Desktop application development. Tools like QEMU or docker can/will be used
to simplify the proccess as they provide virtualization and encapsulation of the embedded
environment without the need of countless software tools and real hardware.
However, testing on the real hardware will still be very important to ensure
the software runs successfully on the target environment (iOBC by ISIS) without any issues.
The following instructions contain all required tools to set up a decent 
development environment on Windows to work with the hardware.
The steps will be easier on Linux and the Unit Test and Linux binaries can only
be used in a Linux Virtual Machine or full Linux OS installation, so it is
worth considering setting up dual boot with linux or setting up a virtual machine.
Not all steps might be necessary depending on the experiences and already available tools of a
new developer.


### Installation Linux Subsystem (WSL) or any other command line program for windows

A command line program like WSL (Ubuntu Subsystem) or MSYS2 can be useful
because of used tools make and useful for tools like git.

WSL Can be installed by following the [installation instructions](https://docs.microsoft.com/de-de/windows/wsl/install-win10)
for Ubuntu Subsystem. An IDE like Eclipse for C/C++ is very useful and has been chosen for this project.

Alternatively, make and git can also be used in windows. For make, the
windows build tools must be installed (can be done with the xpm packet manager).

1. For installaton on windows, install Linux Subsystem (WSL in Windows Store)
or similar command line programm (z.B. MSYS2/MinGW)
2. Install git
	```sh
 	sudo apt-get install git
	```
3. Install Make
	```sh
  	sudo apt-get install make
  	```
3. Install editor programm like vim or atom. Notepad++ can be used to
but needs to be included in the Windows Environment Variables. After that,
notepad++.exe can be called. (or an alias like np='notepad++.exe' can be used)
	```sh
 	sudo apt-get install vim
 	sudo apt-get install atom
	```
4. An alias (shortcut) in ubuntu is very useful to navigate to the
   windows and/or development directories quickly. Any editor can be used to create an alias
```sh
 > cd ~
 > nano .bashrc
````
Add new line
```sh
 > alias shortcut="cd /mnt/c/Users/..."
````
Restart command line programm and test the alias by typing
```sh
> shortcut
````
5. Update everything
```sh
  > sudo apt-get update
  > sudo apt-get
```
### git and doxygen

#### git basics

[Complicated git reference manual](https://git-scm.com/book/en/v2)<br>
[Better git reference manual](https://rogerdudler.github.io/git-guide/)<br>
General sequence to update:
1. Please note that framework changes need to be commited and pushed automatically while being in the ksat_branch.
git pull is not strictly necessary but ensures that any changes are included before pushing own content
```sh
git add .
git status
git commit -m "<commit message>"
git pull
git push
````
2. Useful commands
```sh
git checkout <branch>
git diff (--staged)
git log
git merge <branch to be merged into current branch>
git remote update origin --prune
```
3. Submodule Operations
```sh
git submodule init
git submodule update
git submodule sync
```
4. Create Tag for important branches/merges and push them to gitlab
```sh
git tag -a <VersionTag> -m <VersionMessage>
git push origin tag <VersionTag>
````
5. Create new branch (personal branch like <name>\_branch or feature
branch <feature>\_featureDetails). git checkout -b copies the state of the
current branch
```sh
git checkout -b <new branch name>
git merge <any other branches to include>
```
6. If you worked in wrong branch accidentally and want to apply changes to
another branch
```sh
git stash
git checkout <target branch>
git stash apply
```
7. Rename branch and remote branch
```sh
> git checkout <target branch>
git branch -m new-name
git push origin -u new-name
git push -d origin old-name
````
8. Delete branch and remote branch
```sh
git branch -d branch
git push origin -d remote_branch
```
9. Add new submodule (= other repository) to repository.
Run normal submodule sequence (3.) after this.
```sh
git submodule add <repository address> <folder name>
```
10. Revert commit but keep changes (e.g. to stash them and apply them somewhere
else)
```sh
git reset --soft HEAD~<numberOfCommitsToGoBack>
```

- git checkout is used to switch the currently used branch.
- git diff lists the differences of current branch to last local commit.
  Use --staged if new content was already added.
- git log lists the last few commits.

The submodule commands are useful because the FSFW is integrated as a submodule.
Generally, a new branch is created for each new user and for each new feature.
Name convention:


#### git branching models

Generally, there are guidelines on how to use git and how to name branches.
For the sourceobsw, the following guideline can be used:
- lastname/master as personal branch
- lastname/feature/featurename as feature branch
- lastname/test/testname as a test branch

The feature branch is merged into the master once it has been tested thoroughly.
If work was done in wrong branch accidentaly, use git stash, git apply or git
pop to move changes to different branch (see 6.). If there are wrong commits,
consider 10. Merge requests can be performed with a GUI in GitLab.

The FSFW is included as a submodule. The devel branch generally points to the
main repository on [egit](https://egit.irs.uni-stuttgart.de/fsfw/fsfw), provided by the IRS.
The main repository was forked for [KSat](https://egit.irs.uni-stuttgart.de/KSat).
The master of this fork will always point to the main repository, while other
branches can be used as feature branches.
If any changes in the framework a required for mission features, create a new
branch in the fork for this new feature. An issue and merge request can then be issued.

The ideal git development cycle:
1. Create an issue for the feature that is being implemented
2. Develop the feature
3. Create a pull request
4. Other developers can comment the code in the pull request
5. The pull request is approved and merged into the master.

### Code Documentation with Doxygen

Doxygen was used as a tool to generate the documentation. PDFs can not be produced yet because of a doxygen bug.
The documentation can be accessed by finding the index.html file in doc/doxy/html/
To generate new documentation on Windows, following steps have to be taken:

1. Install [doxygen](http://www.doxygen.nl/download.html)
2. Install [graphviz](https://graphviz.gitlab.io/_pages/Download/Download_windows.html)
3. Add the graphviz binary folder to PATH/system variables
4. Start doxyfile OPUS.doxyfile located in doc/doxy with the doxywizard gui to configure the documentation
5. Generate documentation gui or run doxyfile with doxygen

### <a name="cpp"></a>C++
#### Coding Standards and C++ version
The framework uses the C++11 version which offers a lot of useful features of modern C++ and
the mission code uses C++17 for now. The indexer can be set up to index with C++17:

- Right-click the sourceobsw project and go to Properties->C/C++ General->Preprocessor Includes->CDT ARM Cross and enable the global provider.
- After that change the global provider settings by clicking on workspace settings and going to Discovery->CDT Arm Cross
- Put into the command line:
```sh
${COMMAND} ${FLAGS} ${cross_toolchain_flags} -std=c++17 -E -P -v -dD "${INPUTS}"
```

There are a lot of coding standards resources for the C++ language.
The first resource usually is the [C++ coding standard](https://isocpp.org/wiki/faq/coding-standards)
information page. There are many coding standards, for example by

- [ESA](http://www.esa.int/TEC/Software_engineering_and_standardisation/TECT5CUXBQE_0.html)
- [C++ core guidelines](http://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines) by the C++ creator himself
- [FSFW coding guidelines, not public yet](https://egit.irs.uni-stuttgart.de/fsfw/Coding_Guidelines)
- [JSF guidelines](http://stroustrup.com/JSF-AV-rules.pdf) by Lockheed Martin
- [Google C++ guidelines](https://google.github.io/styleguide/cppguide.html)

- Try to group includes logically, listing mission includes first, then configuration  
  includes, framework includes, driver includes and lastly standard library includes.
  Try to include in the source file(.cpp) if possible.
- Keep columns width to 80. The column width can be set higher in Eclipse by going
  to Window->C/C++->CodeStyle->Formatter  and defining a custom profile built on top of
  the K&R profile with LineWrapping->MaximumLineWidth set to the desired value
  (default value is 80). This doesn't have to be followed stricly
  but adhering to this column width consistently helps with code readability.
- Prefer explicit types like uint8_t, uint16_t, uint32_t
- Prefer nullptr over NULL
- Member variables which are zero or nullptr initialized, can be initialized in header file
  directly instead of using an initializer list in the source file
- Try to keep the scope of variables as small as possible

#### Common errors and crash causes in C++/C and basic concepts

In C/C++, the programmer is given a lot of power over how to use the given hardware
without abstraction layers like in other high level languages like Python
or Java. Not knowing how to use this power properly leads to undefined behaviour
in many cases. TLDR: In C/C++, one often gets crashes where the root of the problematic
is difficult to find. Memory allocation is a powerful tool which can also lead to many difficult-to-track
problems at run-time because any allocated memory needs to be freed.
As such, it should be avoided in the Flight Software, unless the size is fixed at compile time / code start-up time.
Here is a list of common errors (please correct if anything is wrong...)

1. Avoid dynamic memory allocation during run-time (e.g. in performOperation() method). the keyword new
allocated dynamically and must be followed by a delete eventually. Try to use static/local variables
where possible and/or initialize arrays or buffers with a maximum size at class instantiation. std libraries
and functions like map and vector use dynamic memory allocation and should be used with care.
However, also keep in mind that most objects have the whole run-time lifetime.
2. Uninitialized variables can lead to undefined behaviour, especially in optimized builds ! It is preferrable to always initialize variables. It is perfectly possible that code works with uninitialized variables but some compiler optimizations can lead to undefined behaviour where debug code previously worked.
3. When initializing pointers, be careful with nullptr pointer initializations !
E.g. dereferencing a nullptr pointer leads to a nullptr-Pointer exception (crash 0x4/0x10).
In general, accessing or dereferencing any forbidden memory areas leads to undefined behaviour / crashes.
4. One should get familiar with the concept of pointers and OOP when working with the flight software.
Pointers are uses extensively for buffered data (a buffer is basically an array of bytes).
A pointer is always just an address to a memory location, not an array/list like in other languages like Python !
Therefore, when passing buffered data in C/C++, the size of the data is always needed in addition to the pointer to the start of the buffered data.

Recommended book to learn C++: A Tour of C++ (2nd Edition) by the creator of C++.
